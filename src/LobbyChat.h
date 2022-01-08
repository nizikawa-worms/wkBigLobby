
#ifndef WKBIGLOBBY_LOBBYCHAT_H
#define WKBIGLOBBY_LOBBYCHAT_H

#include <string>
#include <vector>

typedef unsigned long       DWORD;
struct HostScreen;
class LobbyChat {
private:
	static int __fastcall hookLobbyClientCommands(void *This, void *EDX, char **commandstrptr, char **argstrptr);
	static int __fastcall hookLobbyHostCommands(void *This, void *EDX, char **commandstrptr, char **argstrptr);
	static inline int (__stdcall *origLobbyDisplayMessage)(DWORD a1, const char *msg);
public:
	static void install();
	static void printLobby(HostScreen * hostScreen, std::string msg);
};


#endif //WKBIGLOBBY_LOBBYCHAT_H
