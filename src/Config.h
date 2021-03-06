#ifndef WKBIGLOBBY_CONFIG_H
#define WKBIGLOBBY_CONFIG_H

#include <string>
#include <set>
#include <filesystem>
#include "version.h"

class Config {
public:
	static inline const std::string iniFile = "wkBigLobby.ini";

private:
	static inline bool moduleEnabled = true;
	static inline bool devConsoleEnabled = true;
	static inline bool useMutex = false;
	static inline bool greentextEnabled = true;
public:
	static void readConfig();
	static bool isModuleEnabled();
	static bool isDevConsoleEnabled();
	static int waVersionCheck();
	static bool isMutexEnabled();

	static bool isGreentextEnabled();

	static std::string getVersionStr();
	static int getVersionInt();
	static std::string getBuildStr();
	static std::string getModuleStr();
	static std::string getFullStr();
	static std::string getGitStr();
};


#endif //WKBIGLOBBY_CONFIG_H
