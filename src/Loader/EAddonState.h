#ifndef EADDONSTATE_H
#define EADDONSTATE_H

enum class EAddonState
{
	None = 0,

	NotLoaded,						/* Addon is not loaded. */
	NotLoadedDuplicate,				/* Addon is not loaded, because it has the same signature as another addon. */
	NotLoadedIncompatible,			/* The file is incompatible with Nexus. */
	NotLoadedIncompatibleAPI,		/* Addon requested an API that doesn't exist. */
	NotLoadedIncompatibleCaps,		/* Addon requested capabilities that cannot be satisfied. */

	Loaded,							/* Addon is loaded. */
	LoadedLOCKED					/* Addon is loaded, but locked and mustn't be unloaded. */
};

#endif
