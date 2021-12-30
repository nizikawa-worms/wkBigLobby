

#ifndef WKBIGLOBBY_SLOTLIST_H
#define WKBIGLOBBY_SLOTLIST_H

#include <vector>
#include <array>
#include "Hooks.h"

#pragma pack(push, 1)
	static const int oldNumSlots = 6;
	static const int newNumSlots = 12;

	struct PlayerSlot {
		unsigned char unknown[0x19118];
	};

	struct PlayerInfo {
		char name[0x11];
		unsigned char unknown[0x63];
		unsigned char present;
		unsigned char unk2;
		unsigned char unk3;
		unsigned char unk4;
	};

	struct HostStruct {
		unsigned char unknown1[0x1A4]; // 0x0
		std::array<PlayerSlot, oldNumSlots> playerSlots; // 0x1A4
		unsigned char unknown2[0x268]; // 0x96834
		unsigned char unknown3[0xBC]; // 0x96A9C,
		PlayerSlot * ptrToPlayerSlots; // 0x96B58
		unsigned char unknown4[0xC]; // 0x96B5C
		std::array<PlayerSlot, 1> clientSlot; // 0x96B68,
	};

	struct NewHostSlots {
		unsigned char start[0x1A4]; // 0x0
		std::array<PlayerSlot, newNumSlots> slots; // 0x1A4
	};

	struct ExtraInfo {
		bool connected = false;
		bool hasModule = false;
		std::string nickname;
		void reset() {
			connected = false;
			hasModule = false;
			nickname = "";
		}
	};

#pragma pack(pop)

class SlotList {
private:
	static inline NewHostSlots newHostSlots;
	static inline HostStruct * hostStruct;
	static inline PlayerInfo * playerInfoStructs;
	static inline int playerColors[] = {0, 1, 2, 3, 7, 9, 10, 11, 12, 13, 0, 15, 1};
	static inline std::array<ExtraInfo, newNumSlots> extraInfo;
	static inline unsigned char bigLobbyFlag = 128;

	static int __fastcall hookCloseSlot(DWORD This);
	static inline int (__fastcall *origCloseSlot)(DWORD This);
public:
	static void install();
	static DWORD rebase(DWORD addr);

	static bool areExtraPlayersInRoom();
	static bool arePlayersWithoutModuleInRoom();
	static bool __stdcall shouldAllowPlayerInRoom(DWORD slot_offset, unsigned char * payload);
	static void setModuleFlag(int id, bool hasModule);
	static void setConnected(int id, bool connected, std::string nickname);
	static std::string getPlayerInfoString();
};


#endif //WKBIGLOBBY_SLOTLIST_H
