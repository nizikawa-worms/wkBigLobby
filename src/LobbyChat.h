
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
	static int __fastcall hookLobbyHostCommands(void *This, void *EDX, char **commandstrptr, char **argstrptr);

	static inline DWORD addrConstructLobbyHostScreen = 0;
	static inline DWORD addrConstructLobbyHostEndScreen = 0;
	static inline DWORD addrConstructLobbyHostEndScreenWrapper = 0;
	static inline DWORD addrDestructLobbyHostEndScreen = 0;

	static int __stdcall hookConstructLobbyHostScreen(int a1, int a2);
	static int __fastcall hookDestructLobbyHostScreen(void *This, int EDX, char a2);
	static int __stdcall hookConstructLobbyHostEndScreen(DWORD a1, unsigned int a2, char a3, int a4);
	static int __stdcall hookDestructLobbyHostEndScreen(int a1);
public:
	static void install();
	static void sendHostGreentextMessage(std::string msg);
};


#endif //WKBIGLOBBY_LOBBYCHAT_H
