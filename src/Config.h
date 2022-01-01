#ifndef WKBIGLOBBY_CONFIG_H
#define WKBIGLOBBY_CONFIG_H

#include <string>
#include <set>
#include <filesystem>

class Config {
public:
	static inline const std::string iniFile = "wkBigLobby.ini";

private:
	static inline bool moduleEnabled = true;
	static inline bool devConsoleEnabled = true;
	static inline bool useMutex = false;
public:
	static void readConfig();
	static bool isModuleEnabled();
	static bool isDevConsoleEnabled();
	static int waVersionCheck();
	static bool isMutexEnabled();

	static std::string getVersionStr();
	static std::string getBuildStr();
	static std::string getModuleStr();
	static std::string getFullStr();
};


#endif //WKBIGLOBBY_CONFIG_H
