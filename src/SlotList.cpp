
#include "SlotList.h"
#include "Debugf.h"
#include "Hooks.h"
#include "Utils.h"
#include "LobbyChat.h"

#include <asmjit/asmjit.h>
using namespace asmjit;

class AsmjitPatcher {
public:
	JitRuntime rt;
	CodeHolder code;
	std::unique_ptr<x86::Assembler> a;
	DWORD offset;

	AsmjitPatcher(DWORD addr) : offset(addr) {
		code.init(rt.environment());
		a = std::make_unique<x86::Assembler>(&code);
		a->setOffset(offset);
	}
	~AsmjitPatcher() {
		auto & buffer = code.sectionById(0)->buffer();
		Hooks::patchAsm(offset, buffer.data(), buffer.size());
	}
};

DWORD SlotList::rebase(DWORD addr) {
	static DWORD base = (DWORD) GetModuleHandle(0);
	return addr - 0x400000 + base;
}


DWORD hookUnicast_patch1_ret;
DWORD slotlist_newhosts_start;
DWORD slotlist_newhosts_slots;

void __declspec(naked) hookUnicast_patch1() {
	_asm mov eax, slotlist_newhosts_start
	_asm imul ecx, 0x19118
	_asm add eax, ecx
	_asm cmp [eax+0x25C], 3
	_asm jmp hookUnicast_patch1_ret
}


DWORD hookCheckShouldAllowInRoom_allow_ret;
DWORD hookCheckShouldAllowInRoom_deny_ret;
void __declspec(naked) hookCheckShouldAllowInRoom() {
	_asm pushad
	_asm push ebx
	_asm push ebp
	_asm push ecx
	_asm call SlotList::shouldAllowPlayerInRoom
	_asm test eax,eax
	_asm popad
	_asm je deny
	_asm jmp hookCheckShouldAllowInRoom_allow_ret
	_asm deny: jmp hookCheckShouldAllowInRoom_deny_ret
}

void SlotList::install() {
	hostStruct = (HostStruct*) rebase(0x7C0ED4);
	playerInfoStructs = (PlayerInfo*)rebase(0x8779E4);

	void (__thiscall *slot_constructor)(PlayerSlot*) = (void (__thiscall *)(PlayerSlot*)) rebase(0x58FBB0);
	for(auto & slot : newHostSlots.slots) {
		slot_constructor(&slot);
	}
	void (__thiscall *slot_destructor)(PlayerSlot*) = (void (__thiscall *)(PlayerSlot*)) rebase(0x58FC80);
	for(auto & slot : hostStruct->playerSlots) {
		slot_destructor(&slot);
	}
	memset((void*)&hostStruct->playerSlots, 0xAA, sizeof(hostStruct->playerSlots));

	// lol
	unsigned char newNumSlotsChar = newNumSlots;
	unsigned char newNumSlotsMinus1Char = newNumSlots - 1;
	unsigned char newNumSlotsPlus1Char = newNumSlots + 1;
	slotlist_newhosts_start = (DWORD)&newHostSlots.start;
	slotlist_newhosts_slots = (DWORD)&newHostSlots.slots;

	// void __thiscall host_incoming_connection_sub_58E060(int this, int a2)
	// mov     esi, [edi+0BCh]
	{AsmjitPatcher p(rebase(0x58E06A)); p.a->mov(x86::esi, (DWORD)&newHostSlots.start); p.a->nop();}
	Hooks::patchAsm(rebase(0x58E086 + 2), &newNumSlotsChar, 1);

	// int __usercall sub_58F0E0@<eax>(int a1@<eax>)
	// add     esi, 1A4h
	{AsmjitPatcher p(rebase(0x058F10F)); p.a->mov(x86::esi, (DWORD)&newHostSlots.slots); p.a->nop();}
	Hooks::patchAsm(rebase(0x58F115 + 1), &newNumSlotsChar, 1);

	//int __userpurge host_broadcast_data_sub_58F8E0
	// lea     esi, [eax+1A4h]
	{AsmjitPatcher p(rebase(0x58F8F3)); p.a->mov(x86::esi, (DWORD)&newHostSlots.slots); p.a->nop();}
	Hooks::patchAsm(rebase(0x58F8F9 + 2), &newNumSlotsMinus1Char, 1);

	//int __userpurge host_broadcast_data2_sub_58F960
	{AsmjitPatcher p(rebase(0x58F975)); p.a->mov(x86::esi, (DWORD)&newHostSlots.slots); p.a->nop();}
	Hooks::patchAsm(rebase(0x058F9BF + 2), &newNumSlotsChar, 1);

	//int __userpurge host_unicast_data_sub_58F9F0
	hookUnicast_patch1_ret = rebase(0x58FA09);
	Hooks::hookAsm(rebase(0x58F9FA), (DWORD)&hookUnicast_patch1);

	// void __thiscall sub_58E4F0(int this)
	// add     edi, 1A4h
	{AsmjitPatcher p(rebase(0x58E5C2)); p.a->mov(x86::edi, (DWORD)&newHostSlots.slots); p.a->nop();}
	Hooks::patchAsm(rebase(0x58E5BB + 1), &newNumSlotsChar, 1);

	std::map<DWORD, std::vector<DWORD>> xrefs_start = {
			{ 0x0, {0x00466fa9, 0x00466fca, 0x00466fee, 0x00467012, 0x00467036, 0x0046705a, 0x004abc29, 0x004abd60, 0x004abd66, 0x004abe32, 0x004abfc2, 0x004abfcb, 0x004ac05c, 0x004ac068, 0x004ac584, 0x004ac7ff, 0x004ac808, 0x004ad528, 0x004ae7bb, 0x004ae7c6, 0x004af5a1, 0x004af85c, 0x004b1907, 0x004b1910, 0x004b1933, 0x004b193c, 0x004b1b77, 0x004b1c05, 0x004b4719, 0x004b5589, 0x004b55b9, 0x004b55f5, 0x004b55fe, 0x004b566e, 0x004b567a, 0x004b5f53, 0x004b6220, 0x004b6849, 0x004b6852, 0x004b6aa1, 0x004b6aaa, 0x004b6c73, 0x004b80a6, 0x004b9a71, 0x004b9a77, 0x0058d294}},
			{ 0x8c, {0x004b71a3}},
			{ 0xb4, {0x004ac5cd, 0x004ac5d1}},
			{ 0xb8, {0x0046afde, 0x0046afe2, 0x004b4712, 0x004b4716, 0x004b99aa}},
			{ 0xbc, {0x004aefd5, 0x004b7b3d}},
			{ 0xbd, {0x004abb57, 0x004ac09a, 0x004b2233, 0x004b30c3, 0x004b30f9, 0x004b314a, 0x004b4624, 0x004b5736, 0x004b5797, 0x004b6783, 0x004b67d5, 0x004b696b, 0x004b757c, 0x004b7580, 0x004b95fa, 0x004ba652}},
			{ 0xbe, {0x004b6d71, 0x004b6e79, 0x004b6e7d, 0x004b6f88, 0x004b6f8c, 0x004b6fb3, 0x004b6fb7, 0x004b6ff7, 0x004b6ffb, 0x004b7023, 0x004b7027, 0x004b7041, 0x004b7045, 0x004b708e, 0x004b7092, 0x004b712d, 0x004b7131, 0x004b7189, 0x004b718d, 0x004b7265, 0x004b7269, 0x004b7583, 0x004b7587, 0x004b773e, 0x004b7742}},
			{ 0xc0, {0x004abd49, 0x004abd4d, 0x004abe79, 0x004abefb, 0x004abeff, 0x004abf70, 0x004afbd1, 0x004b1b3d, 0x004b1b41, 0x004b62f5, 0x004b62f9, 0x004b66f3, 0x004b66f7, 0x004b69c4, 0x004b69ce, 0x004b6d37, 0x004b6d3b, 0x004b6d87, 0x004b6d8b, 0x004b6d98, 0x004b6e26, 0x004b6e2a, 0x004b6ee6, 0x004b6eea, 0x004b6f25, 0x004b6f29, 0x004b702c, 0x004b7030, 0x004b7136, 0x004b713a, 0x004b71bd, 0x004b742e, 0x004b7432}},
			{ 0xc4, {0x004b6db6, 0x004b6dc7}},
			{ 0xc8, {0x004b6dcd}},
			{ 0xcc, {0x004ac2e2, 0x004ad377, 0x004ae333, 0x004b6323, 0x004b68da, 0x004b69eb, 0x004b6b2b, 0x004b6b4b, 0x004b6b4f, 0x004b7343}},
			{ 0xd0, {0x004b6ed0, 0x004b71d1}},
			{ 0xe4, {0x004abcce, 0x004abd8f, 0x004abd97, 0x004abd9b, 0x004abf02, 0x004abf06, 0x004abf22, 0x004abf26, 0x004ac125}},
			{ 0xe8, {0x004b6ced}},
			{ 0xec, {0x004abeb1, 0x004abeb5, 0x004abfb6, 0x004b6a54, 0x004b6a58, 0x004b6cf3}},
			{ 0xed, {0x004b6cf9, 0x004b7036, 0x004b7140, 0x004b73b5, 0x004b73b9}},
			{ 0xee, {0x004b55d4, 0x004b55d8, 0x004b5652, 0x004b5656, 0x004b6cff, 0x004b75b6, 0x004b75ba}},
			{ 0xef, {0x004b5579, 0x004b557d, 0x004b5582, 0x004b5586, 0x004b55b0, 0x004b55b4, 0x004b55c8, 0x004b55cc, 0x004b6d05}},
			{ 0xf0, {0x004b6d0b, 0x004b762c, 0x004b7630}},
			{ 0xf1, {0x004b6d11, 0x004b7659, 0x004b765d}},
			{ 0xf2, {0x004b6d17, 0x004b7683, 0x004b7687}},
			{ 0xf3, {0x004b6d1d, 0x004b76ad, 0x004b76b1}},
			{ 0xf4, {0x004b6d23, 0x004b76ce, 0x004b76d2}},
			{ 0xf5, {0x004b6d29, 0x004b96a1}},
			{ 0xf6, {0x004b6d2f, 0x004b7247}},
	};

	std::map<DWORD, std::vector<DWORD>> xrefs_end = {
			{ 0x0, {0x004af5cd, 0x004b1c91, 0x004b6143, 0x004b624c, 0x004b9aba,}},
			{ 0xb8, {0x004b99e2}},
			{ 0xbc, {0x004aefe9, 0x004b7b4a}},
			{ 0xbd, {0x004abbcb, 0x004ac0be, 0x004b2261, 0x004b30f2, 0x004b3143, 0x004b31b4, 0x004b463e, 0x004b578a, 0x004b57c1, 0x004b67b4, 0x004b6833, 0x004b6995, 0x004b9617, 0x004ba685}},
			{ 0xc0, {0x00481e14}},
			{ 0xcc, {0x004ad4db, 0x004ae551}},
			{ 0xf5, {0x004b96e2}},
	};
	//removed: 0x004b6142,0x004ba396, 0x004ba3a1, 0x004ba397,0x004ba3a2

	for(auto & entry : xrefs_start) {
		DWORD value = entry.first + (DWORD) &newHostSlots.slots;
		for(auto & xref : entry.second) {
			DWORD opval = *(DWORD*) rebase(xref);
			if(opval >= (DWORD) hostStruct && opval <= (DWORD) hostStruct + sizeof(HostStruct)) {
				Hooks::patchAsm(rebase(xref), (unsigned char*)&value, sizeof(value));
			}
		}
	}
	for(auto & entry : xrefs_end) {
		DWORD value = entry.first + (DWORD) &newHostSlots.slots + sizeof(newHostSlots.slots);
		for(auto & xref : entry.second) {
			DWORD opval = *(DWORD*) rebase(xref);
			if(opval >= (DWORD) hostStruct && opval <= (DWORD) hostStruct + sizeof(HostStruct)) {
				Hooks::patchAsm(rebase(xref), (unsigned char*)&value, sizeof(value));
			}
		}
	}

	/// artificial player limit
	Hooks::patchAsm(rebase(0x43A94E + 6), &newNumSlotsChar, 1);
	Hooks::patchAsm(rebase(0x4C14C8 + 6), &newNumSlotsChar, 1);

//	unsigned char jmp1[] = {0xEB, 0x18};
//	Hooks::patchAsm(rebase(0x004B6DE7), (unsigned char*)&jmp1, sizeof(jmp1));
	//replaced with hook

	unsigned char nopnop[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
	Hooks::patchAsm(rebase(0x4B716A), (unsigned char*)&nopnop, sizeof(nopnop));
	Hooks::patchAsm(rebase(0x004B706F), (unsigned char*)&nopnop, sizeof(nopnop));

	// set special flag at byte 59 of client join message to signal wkBigLobby is in use
	Hooks::patchAsm(rebase(0x043A1C6 + 4), &bigLobbyFlag, 1);

	// and check it
	hookCheckShouldAllowInRoom_allow_ret = rebase(0x4B6E01);
	hookCheckShouldAllowInRoom_deny_ret = rebase(0x04B6DE7);
	Hooks::hookAsm(rebase(0x4B6DE1), (DWORD)&hookCheckShouldAllowInRoom);

	// name check
	Hooks::patchAsm(rebase(0x46AA7B + 2), &newNumSlotsPlus1Char, 1);

	// player list box max height
//	Hooks::patchAsm(Utils::rebase(0x4861E5 + 2), &newNumSlotsPlus1Char, 1);
//	Hooks::patchAsm(Utils::rebase(0x4861EA + 1), &newNumSlotsPlus1Char, 1);

	// player list box colors
	DWORD addrPlayerColors = (DWORD)&playerColors;
	Hooks::patchAsm(rebase(0x48243F + 3), (unsigned char*)&addrPlayerColors, 4); // player list colors
	Hooks::patchAsm(rebase(0x482A7D + 3), (unsigned char*)&addrPlayerColors, 4);
	Hooks::patchAsm(rebase(0x4C1E07 + 3), (unsigned char*)&addrPlayerColors, 4);
	Hooks::patchAsm(rebase(0x4C22E3 + 3), (unsigned char*)&addrPlayerColors, 4);

	// populating player box
	DWORD addrEndPlayerInfo = *(DWORD*) rebase(0x4823AE + 1) - oldNumSlots * 120 + newNumSlots * 120;
	DWORD addrEndPlayerInfo2 = *(DWORD*) rebase(0x4823A6 + 4) - oldNumSlots * 48 + newNumSlots * 48;

	Hooks::patchAsm(rebase(0x4823AE + 1), (unsigned char*)&addrEndPlayerInfo, 4);
	Hooks::patchAsm(rebase(0x4823A6 + 4), (unsigned char*)&addrEndPlayerInfo2, 4);
	Hooks::patchAsm(rebase(0x48239E + 4), &newNumSlotsChar, 1);

	DWORD addrCloseSlot = rebase(0x590020);
	_HookDefault(CloseSlot);

//	DWORD dwLastProtection;
//	VirtualProtect((void *) &hostStruct->playerSlots[3], sizeof(PlayerSlot), PAGE_NOACCESS, &dwLastProtection);
}


int __fastcall SlotList::hookCloseSlot(DWORD This) {
	if(This >= (DWORD)&newHostSlots.slots && This <= (DWORD)&newHostSlots.slots + sizeof(newHostSlots.slots)) {
		int id = (This - (DWORD)&newHostSlots.slots) / sizeof(PlayerSlot);
		debugf("Close extra slot: 0x%X = id: %d\n", This, id);
		extraInfo[id].reset();
	} else {
		debugf("Close regular slot: 0x%X\n", This);
	}
	return origCloseSlot(This);
}

bool SlotList::areExtraPlayersInRoom() {
	for(size_t i=6; i < newNumSlots; i++) {
		auto & info = extraInfo[i];
		debugf("i: %d connected: %d\n", i, info.connected);
		if(info.connected) return true;
	}
	return false;
}

std::string SlotList::getPlayerInfoString() {
	std::stringstream ss;
	ss << "Status of network slots:";
	for (size_t i=0; i < newNumSlots; i++) {
		auto & info = extraInfo[i];
		if(info.connected) {
			ss << "\n\tSlot " << i << ": " << info.nickname << " - " << (info.hasModule ? "(has module)" : "(no module)");
		}
	}
	return ss.str();
}



bool SlotList::arePlayersWithoutModuleInRoom() {
//	for(size_t i=0; i <= 13; i++) {
//		debugf("i: %d addr: 0x%X present: %d hasmodule: %d\n", i, &playerInfoStructs[i], playerInfoStructs[i].present, extraInfo[i].hasModule);
//		if(playerInfoStructs[i].present && !extraInfo[i].hasModule) return true;
//	}
	for(auto & info : extraInfo) {
		if(info.connected && !info.hasModule) return true;
	}
	return false;
}



bool __stdcall SlotList::shouldAllowPlayerInRoom(HostScreen * hostScreen, DWORD slot_offset, unsigned char * payload) {
	int id = slot_offset / sizeof(PlayerSlot);
	PlayerSlot * slot = &newHostSlots.slots[id];

	unsigned short int * type = (unsigned short int*)payload;
	unsigned char flag = payload[59];
	bool hasmodule = flag & bigLobbyFlag;
	char * nickname = (char*)&payload[2];
//	debugf("Slot: 0x%X id: %d nickname: %s payload: 0x%X type: 0x%X flag: 0x%X hasmodule: %d\n", slot, id, nickname, payload, *type, flag, hasmodule);
	LobbyChat::printLobby(hostScreen, std::format("{} is connecting to the lobby at slot: {} - {}", nickname, id, hasmodule ? "has wkBigLobby installed" : "does not have wkBigLobby installed"));

//	Utils::hexDump("Payload", payload, 128);

	setConnected(id, true, nickname);
	setModuleFlag(id, hasmodule);

	bool extraPlayers = areExtraPlayersInRoom();
	bool playersWithoutModule = arePlayersWithoutModuleInRoom();
	debugf("extraPlayers: %d playersWithoutModule: %d\n", extraPlayers, playersWithoutModule);

	char playersInRoom = *(char*)rebase(0x87D0DE);
	debugf("playersInRoom: %d playerBoxSize: %d\n", playersInRoom, hostScreen->playerBoxSize);

	if(hostScreen->playerBoxSize < 7) {
		if(playersInRoom >= hostScreen->playerBoxSize) {
			LobbyChat::printLobby(hostScreen, std::format("{} is not allowed in lobby - host has limited number of players to {:d}", nickname, hostScreen->playerBoxSize));
			return false;
		}
	}

	if(id >= newNumSlots - 1) {
		LobbyChat::printLobby(hostScreen, std::format("{} is not allowed in lobby - lobby is full", nickname));
		return false;
	}

	if(hasmodule) {
		if(id > 5 && playersWithoutModule) {
			LobbyChat::printLobby(hostScreen, std::format("{} is not allowed in lobby - has module installed and there is extra space for him, but there are already players in room without module installed", nickname));
			return false;
		}
		return true;
	} else {
		if(id <= 5) {
			if(extraPlayers) {
				LobbyChat::printLobby(hostScreen, std::format("{} is not allowed in lobby - does not have module installed and there is normal space for him, but there are already extra players in room\n", nickname));
				return false;
			}
			return true;
		} else {
			LobbyChat::printLobby(hostScreen, std::format("{} is not allowed in lobby - does not have module and there is no normal space for him\n", nickname));
			return false;
		}
	}

	return true;
}

void SlotList::setModuleFlag(int id, bool hasModule) {
	auto & info = extraInfo[id];
	info.hasModule = hasModule;
}

void SlotList::setConnected(int id, bool connected, std::string nickname) {
	auto & info = extraInfo[id];
	info.connected = connected;
	info.nickname = std::string(nickname);
}

