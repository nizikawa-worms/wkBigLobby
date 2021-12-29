
#ifndef WKBIGLOBBY_LOBBYCHAT_H
#define WKBIGLOBBY_LOBBYCHAT_H

#include <string>
#include <vector>

typedef unsigned long       DWORD;
class LobbyChat {
private:
	static inline DWORD lobbyHostScreen = 0;
	static inline DWORD lobbyClientScreen = 0;
	static inline DWORD lobbyOfflineScreen = 0;
	static int __fastcall hookLobbyClientCommands(void *This, void *EDX, char **commandstrptr, char **argstrptr);
public:
	static void install();
	static void sendGreentextMessage(std::string msg);
};


#endif //WKBIGLOBBY_LOBBYCHAT_H
