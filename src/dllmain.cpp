#include <windows.h>
#include <stdexcept>
#include <ctime>

#include "DevConsole.h"
#include "Config.h"
#include "SlotList.h"
#include "LobbyChat.h"
#include <chrono>

void install() {
	SlotList::install();
	LobbyChat::install();
}

// Thanks StepS
bool LockGlobalInstance(LPCTSTR MutexName)
{
	if (!CreateMutex(NULL, 0, MutexName)) return 0;
	if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
	return 1;
}

char LocalMutexName[MAX_PATH];
bool LockCurrentInstance(LPCTSTR MutexName)
{
	if(!Config::isMutexEnabled()) return true;
	_snprintf_s(LocalMutexName, _TRUNCATE,"P%u/%s", GetCurrentProcessId(), MutexName);
	return LockGlobalInstance(LocalMutexName);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch(ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			auto start = std::chrono::high_resolution_clock::now();
			decltype(start) finish;
			try {
				Config::readConfig();
				if (Config::isModuleEnabled() && Config::waVersionCheck() && LockCurrentInstance("wkBigLobby")) {
					if (Config::isDevConsoleEnabled()) DevConsole::install();
					install();
				}
				finish = std::chrono::high_resolution_clock::now();
			} catch (std::exception &e) {
				finish = std::chrono::high_resolution_clock::now();
				MessageBoxA(0, e.what(), Config::getFullStr().c_str(), MB_ICONERROR);
			}
			std::chrono::duration<double> elapsed = finish - start;
			printf("wkBigLobby startup took %lf seconds\n", elapsed.count());
		}
		break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
		default:
			break;
	}
	return TRUE;
}
