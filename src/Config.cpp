
#include <windows.h>
#include "Config.h"
#include "Debugf.h"
#include <filesystem>

namespace fs = std::filesystem;

void Config::readConfig() {
	char wabuff[MAX_PATH];
	GetModuleFileNameA(0, (LPSTR)&wabuff, sizeof(wabuff));
	std::filesystem::path waDir = fs::path(wabuff).parent_path();
	auto inipath = (waDir / iniFile).string();
	moduleEnabled = GetPrivateProfileIntA("general", "EnableModule", 1, inipath.c_str());
	useMutex = GetPrivateProfileIntA("general", "UseMutex", 0, inipath.c_str());
	devConsoleEnabled = GetPrivateProfileIntA("debug", "EnableDevConsole", 0, inipath.c_str());
}

bool Config::isDevConsoleEnabled() {
	return devConsoleEnabled;
}

bool Config::isModuleEnabled() {
	return moduleEnabled;
}

//StepS tools
typedef unsigned long long QWORD;
#define MAKELONGLONG(lo,hi) ((LONGLONG(DWORD(lo) & 0xffffffff)) | LONGLONG(DWORD(hi) & 0xffffffff) << 32 )
#define QV(V1, V2, V3, V4) MAKEQWORD(V4, V3, V2, V1)
#define MAKEQWORD(LO2, HI2, LO1, HI1) MAKELONGLONG(MAKELONG(LO2,HI2),MAKELONG(LO1,HI1))
QWORD GetModuleVersion(HMODULE hModule)
{
	char WApath[MAX_PATH]; DWORD h;
	GetModuleFileNameA(hModule,WApath,MAX_PATH);
	DWORD Size = GetFileVersionInfoSizeA(WApath,&h);
	if(Size)
	{
		void* Buf = malloc(Size);
		GetFileVersionInfoA(WApath,h,Size,Buf);
		VS_FIXEDFILEINFO *Info; DWORD Is;
		if(VerQueryValueA(Buf, "\\", (LPVOID*)&Info, (PUINT)&Is))
		{
			if(Info->dwSignature==0xFEEF04BD)
			{
				return MAKELONGLONG(Info->dwFileVersionLS, Info->dwFileVersionMS);
			}
		}
		free(Buf);
	}
	return 0;
}

int Config::waVersionCheck() {
	auto version = GetModuleVersion((HMODULE)0);
	char versionstr[64];
	_snprintf_s(versionstr, _TRUNCATE, "Detected game version: %u.%u.%u.%u", PWORD(&version)[3], PWORD(&version)[2], PWORD(&version)[1], PWORD(&version)[0]);
	debugf("%s\n", versionstr);

	std::string tversion = getFullStr();
	char buff[512];
	if (version != QV(3,8,1,0)) {
		_snprintf_s(buff, _TRUNCATE, "wkBigLobby is only compatible with WA version 3.8.1.0.\n\n%s", versionstr);
		MessageBoxA(0, buff, tversion.c_str(), MB_OK | MB_ICONERROR);
		return 0;
	}
	return 1;
}


std::string Config::getModuleStr() {
	return "wkBigLobby";
}
std::string Config::getVersionStr() {
	return "v0.2.2";
}

std::string Config::getBuildStr() {
	return __DATE__ " " __TIME__;
}

std::string Config::getFullStr() {
	return getModuleStr() + " " + getVersionStr() + " (build: " + getBuildStr() + ")";
}

bool Config::isMutexEnabled() {
	return useMutex;
}
