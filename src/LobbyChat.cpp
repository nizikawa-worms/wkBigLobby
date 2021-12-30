#include "LobbyChat.h"
#include "Hooks.h"
#include "Config.h"
#include "SlotList.h"
#include "Debugf.h"
#include <sstream>


void (__fastcall *addrLobbySendGreentext)(const char * msg, void * EDX, void* This, int a3, int a4);
char * addrMyNickname;
int (__fastcall *origLobbyClientCommands)(void* This, void* EDX, char **commandstrptr, char **argstrptr);
int __fastcall LobbyChat::hookLobbyClientCommands(void *This, void *EDX, char **commandstrptr, char **argstrptr) {
	std::string command = std::string(commandstrptr[0]);
	std::string args = std::string(argstrptr[0]);

	if(command == "lobby") {
		std::string out = std::format("{} is using {}", addrMyNickname, Config::getFullStr());
		addrLobbySendGreentext(out.c_str(), 0, This, 0, 0);
		return 1;
	}

	return origLobbyClientCommands(This, EDX, commandstrptr, argstrptr);
}



int (__fastcall *origLobbyHostCommands)(void *This, void *EDX, char **commandstrptr, char **argstrptr);
int __fastcall LobbyChat::hookLobbyHostCommands(void *This, void *EDX, char **commandstrptr, char **argstrptr) {
	std::string command = std::string(commandstrptr[0]);
	std::string args = std::string(argstrptr[0]);

	if(command == "lobby") {
		std::string out = SlotList::getPlayerInfoString();
		addrLobbySendGreentext(out.c_str(), 0, This, 0, 0);
	}

	return origLobbyHostCommands(This, EDX, commandstrptr, argstrptr);
}





int (__stdcall *origConstructLobbyHostScreen)(int a1, int a2);
int __stdcall LobbyChat::hookConstructLobbyHostScreen(int a1, int a2) {
	auto ret = origConstructLobbyHostScreen(a1, a2);
	lobbyHostScreen = a1;
	return ret;
}

int (__stdcall *origConstructLobbyHostEndScreen)(DWORD a1, unsigned int a2, char a3, int a4);
int __stdcall LobbyChat::hookConstructLobbyHostEndScreen(DWORD a1, unsigned int a2, char a3, int a4) {
	auto ret = origConstructLobbyHostEndScreen(a1, a2, a3, a4);
	lobbyHostScreen = a1;
	return ret;
}

int (__fastcall *origDestructLobbyHostScreen)(void *This, int EDX, char a2);
int __fastcall LobbyChat::hookDestructLobbyHostScreen(void *This, int EDX, char a2) {
	lobbyHostScreen = 0;
	return origDestructLobbyHostScreen(This, EDX, a2);
}

int (__stdcall *origDestructLobbyHostEndScreen)(int a1);
int __stdcall LobbyChat::hookDestructLobbyHostEndScreen(int a1) {
	lobbyHostScreen = 0;
	return origDestructLobbyHostEndScreen(a1);
}

void LobbyChat::sendHostGreentextMessage(std::string msg) {
	debugf("%s\n", msg.c_str());
	if(lobbyHostScreen) {
		addrLobbySendGreentext(msg.c_str(), 0, (void*)lobbyHostScreen, 0, 0);
	}
}

void LobbyChat::install() {
	DWORD addrLobbyClientCommands = SlotList::rebase(0x4AABB0);
	DWORD addrLobbyHostCommands = SlotList::rebase(0x4B9B00);
	addrLobbySendGreentext = (void (__fastcall *)(const char*,void *,void *,int, int)) SlotList::rebase(0x4AA990);
	addrMyNickname = *(char**)((DWORD)addrLobbySendGreentext + 0x4D);

	addrConstructLobbyHostScreen = SlotList::rebase(0x4B0160);
	addrConstructLobbyHostEndScreen = SlotList::rebase(0x4AB890);
	addrConstructLobbyHostEndScreenWrapper = SlotList::rebase(0x4ADCA0);
	addrDestructLobbyHostEndScreen = SlotList::rebase(0x4AB970);

	DWORD * addrLobbyHostScreenVtable = *(DWORD**)(addrConstructLobbyHostScreen + 0x41);
	DWORD addrDestructLobbyHostScreen = addrLobbyHostScreenVtable[1];

	_HookDefault(LobbyClientCommands);
	_HookDefault(LobbyHostCommands);

	_HookDefault(ConstructLobbyHostScreen);
	_HookDefault(DestructLobbyHostScreen);
	_HookDefault(ConstructLobbyHostEndScreen);
	_HookDefault(DestructLobbyHostEndScreen);
}
