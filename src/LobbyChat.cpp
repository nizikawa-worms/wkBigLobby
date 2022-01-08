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


void LobbyChat::install() {
	DWORD addrLobbyClientCommands = SlotList::rebase(0x4AABB0);
	DWORD addrLobbyHostCommands = SlotList::rebase(0x4B9B00);
	addrLobbySendGreentext = (void (__fastcall *)(const char*,void *,void *,int, int)) SlotList::rebase(0x4AA990);
	addrMyNickname = *(char**)((DWORD)addrLobbySendGreentext + 0x4D);

	origLobbyDisplayMessage = (int (__stdcall *)(DWORD,const char *))SlotList::rebase(0x493CB0);

	_HookDefault(LobbyClientCommands);
	_HookDefault(LobbyHostCommands);
}

void LobbyChat::printLobby(HostScreen *hostScreen, std::string msg) {
	if(!Config::isGreentextEnabled()) return;
	msg = "SYS:wkBigLobby:ALL:" + msg;
	origLobbyDisplayMessage((DWORD)hostScreen + 0x10318, msg.c_str());
}
