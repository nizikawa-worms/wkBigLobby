

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

	struct HostStruct {
		unsigned char unknown1[0x1A4]; // 0x0
		std::array<PlayerSlot, oldNumSlots> playerSlots; // 0x1A4
		unsigned char unknown2[0x268]; // 0x96834
		unsigned char unknown3[0xBC]; // 0x96A9C,
		PlayerSlot * ptrToPlayerSlots; // 0x96B58
		unsigned char unknown4[0xC]; // 0x96B5C
		std::array<PlayerSlot, 1> unkSlot; // 0x96B68,
	};

	struct NewHostSlots {
		unsigned char start[0x1A4]; // 0x0
		std::array<PlayerSlot, newNumSlots> slots; // 0x1A4
	};

#pragma pack(pop)

class SlotList {
private:
	static inline NewHostSlots newHostSlots;
	static inline HostStruct * hostStruct;
	static inline int playerColors[] = {0, 1, 2, 3, 7, 9, 10, 11, 12, 13, 0, 15, 1};
public:
	static void install();
	static DWORD rebase(DWORD addr);
};


#endif //WKBIGLOBBY_SLOTLIST_H
