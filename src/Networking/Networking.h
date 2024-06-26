#pragma once

#include "NetworkingPublic.h"
#include "Loader/AddonAPI.h"
#include <WinSock2.h>
#include <map>

namespace Networking
{
	/// Networking is now ready. As networking requires an addon, this event in not guaranteed to ever be raised.
	/// no payload
	constexpr char const EV_NETWORKING_READY[] = "EV_NW_RDY";
	/// Networking is about to unload, this is your last chance to get packets out.
	/// no payload
	constexpr char const EV_NETWORKING_UNLOADING[] = "EV_NW_UNL";

	typedef UUID UserId;
	typedef UUID SessionId;

	extern std::map<AddonSignature, PacketHandler*> Handlers;

	enum ModuleState {
		_UNKNOWN                 = 0,
		Initializing             = 1,
		WaitingForJoin           = 2,
		WaitingForServerResponse = 3,
		SessionEstablished       = 4
	};
	extern ModuleState State;
	extern bool AddonLoaded;
	void Init();

	bool PrepareAndBroadcastPacket(Packet* packet);
	void LeaveSession();

	PacketHandler* GetPacketHandlerFromApi(int version, AddonAPI* api);
	void ResetHandlerPtr(int version, AddonAPI* api);
}
