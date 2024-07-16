#ifndef ADDONDEF_H
#define ADDONDEF_H

#include <string>

#include "AddonAPI.h"
#include "EAddonFlags.h"
#include "EUpdateProvider.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

typedef void (*ADDON_LOAD)(AddonAPI* aAPI);
typedef void (*ADDON_UNLOAD)();

struct AddonVersion
{
	signed short	Major;
	signed short	Minor;
	signed short	Build;
	signed short	Revision;

	AddonVersion() = default;
	constexpr AddonVersion(signed short aMajor, signed short aMinor, signed short aBuild, signed short aRevision)
		: Major{ aMajor }
		, Minor{ aMinor }
		, Build{ aBuild }
		, Revision{ aRevision }
	{}
	AddonVersion(json& aJson);
	AddonVersion(std::string aVersionString);

	std::string string();
};

bool operator>(AddonVersion lhs, AddonVersion rhs);
bool operator<(AddonVersion lhs, AddonVersion rhs);
bool operator==(AddonVersion lhs, AddonVersion rhs);
bool operator!=(AddonVersion lhs, AddonVersion rhs);
bool operator<=(AddonVersion lhs, AddonVersion rhs);
bool operator>=(AddonVersion lhs, AddonVersion rhs);

struct Capability {
	char const* Name;
	AddonVersion Version;

	[[nodiscard]] inline constexpr bool IsTerminator() const noexcept { return !this->Name; }
};
struct CapabilityRequirement
{
	char const* Capability;
	AddonVersion MinVersion, MaxVersion;
	signed int SatisfiedBy; // set by nexus after loading the addon
	enum {
		Optional = 1 << 0,
	} Flags;

	static inline constexpr AddonVersion UNBOUNDED_VERSION {};
	static inline constexpr signed int UNSATISFIED {0};
	[[nodiscard]] inline constexpr bool IsTerminator() const noexcept { return !this->Capability; }
};


struct AddonDefinition
{
	/* required */
	signed int      Signature;      /* Raidcore Addon ID, set to random unqiue negative integer if not on Raidcore */
	signed int		APIVersion;		/* Determines which AddonAPI struct revision the Loader will pass, use the NEXUS_API_VERSION define from Nexus.h */
	const char*     Name;           /* Name of the addon as shown in the library */
	AddonVersion	Version;
	const char*     Author;         /* Author of the addon */
	const char*     Description;    /* Short description */
	ADDON_LOAD      Load;           /* Pointer to Load Function of the addon */
	ADDON_UNLOAD    Unload;         /* Pointer to Unload Function of the addon. Not required if EAddonFlags::DisableHotloading is set. */
	EAddonFlags     Flags;          /* Information about the addon */

	/* update fallback */
	EUpdateProvider Provider;       /* What platform is the the addon hosted on */
	const char*     UpdateLink;     /* Link to the update resource */

	Capability* ProvidesCapabilities; // terminated list. the terminator has a null name ptr 
	CapabilityRequirement* RequiredCapabilities; // terminated list. the terminator has a null capability ptr

	/* internal */
	bool HasMinimumRequirements();
	bool HasFlag(EAddonFlags aAddonFlag);

	static void Copy(AddonDefinition* aSrc, AddonDefinition** aDst);
	static void Free(AddonDefinition** aDefinitions);
};

#endif