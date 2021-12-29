#include "LobbyChat.h"
#include "Hooks.h"
#include "Config.h"
#include "SlotList.h"
#include <sstream>


void (__fastcall *addrLobbySendGreentext)(const char * msg, void * EDX, void* This, int a3, int a4);
char * addrMyNickname;
int (__fastcall *origLobbyClientCommands)(void* This, void* EDX, char **commandstrptr, char **argstrptr);
int __fastcall LobbyChat::hookLobbyClientCommands(void *This, void *EDX, char **commandstrptr, char **argstrptr) {
	std::string command = std::string(commandstrptr[0]);
	std::string args = std::string(argstrptr[0]);

	if(command == "lobby") {
		std::stringstream ss;
		ss << addrMyNickname << " is using " << Config::getFullStr();
		addrLobbySendGreentext(ss.str().c_str(), 0, This, 0, 0);
		return 1;
	}
	return origLobbyClientCommands(This, EDX, commandstrptr, argstrptr);
}


void LobbyChat::install() {
	DWORD addrLobbyClientCommands = SlotList::rebase(0x4AABB0);
	addrLobbySendGreentext = (void (__fastcall *)(const char*,void *,void *,int, int)) SlotList::rebase(0x4AA990);
	addrMyNickname = *(char**)((DWORD)addrLobbySendGreentext + 0x4D);
	_HookDefault(LobbyClientCommands);
}
