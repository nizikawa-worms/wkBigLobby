#include <stdexcept>
#include <sstream>
#include <PatternScanner.h>
#include "Hooks.h"
#include "Debugf.h"
#include <fstream>
#include <format>
#include <format>
#include <polyhook2/CapstoneDisassembler.hpp>
#include <mutex>

void Hooks::hook(std::string name, DWORD pTarget, DWORD *pDetour, DWORD *ppOriginal, const char * line) {
//	static std::once_flag flag;
//	std::call_once(flag, [](){
//		static auto logger = std::make_shared<PLH::ErrorLog>();
//		logger->setLogLevel(PLH::ErrorLevel::INFO);
//		PLH::Log::registerLogger(logger);
//	});

	static PLH::CapstoneDisassembler dis(PLH::Mode::x86);
	if(!pTarget)
		throw std::runtime_error("Hook adress is null: " + name);
	if(hookNameToAddr.find(name) != hookNameToAddr.end())
		throw std::runtime_error("Hook name reused: " + name);
	if(hookAddrToName.find(pTarget) != hookAddrToName.end()) {
		std::stringstream ss;
		ss << "The specified address is already hooked: " << name << "(0x" << std::hex << pTarget << "), " << hookAddrToName[pTarget];
		throw std::runtime_error(ss.str());
	}

	uint64_t trampoline = 0;
	auto detour = std::make_unique<PLH::x86Detour>(pTarget, (const uint64_t)pDetour, &trampoline, dis, 20);
	if(!detour->hook()) {
		throw std::runtime_error("Failed to create hook: " + name);
	}
	detours.push_back(std::move(detour));
	*ppOriginal = (DWORD)trampoline;

	hookAddrToName[pTarget] = name;
	hookNameToAddr[name] = pTarget;
	if(!line) {
		debugf("%s 0x%X -> 0x%X\n", name.c_str(), pTarget, pDetour);
	} else {
		printf("%s: hook: %s 0x%X -> 0x%X\n", line, name.c_str(), pTarget, pDetour);
	}
}

void Hooks::hookIat(std::string dllName, std::string apiName, DWORD *pDetour, DWORD *ppOriginal) {
	static PLH::CapstoneDisassembler dis(PLH::Mode::x86);
	uint64_t trampoline = 0;
	const std::wstring module;
	auto hook = std::make_unique<PLH::IatHook>(dllName, apiName, (const uint64_t)pDetour, &trampoline, module);
	if(!hook->hook()) {
		throw std::runtime_error("Failed to create IAT hook: " + dllName + " " + apiName);
	}
	*ppOriginal = (DWORD)trampoline;
	iathooks.push_back(std::move(hook));
	debugf("%s::%s -> 0x%X\n", dllName.c_str(), apiName.c_str(), pDetour);
}


//Worms development tools by StepS
BOOL Hooks::PatchMemData(PVOID pAddr, size_t buf_len, PVOID pNewData, size_t data_len) {
	if (!buf_len || !data_len || !pNewData || !pAddr || buf_len < data_len) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return 0;
	}
	DWORD dwLastProtection;
	if (!VirtualProtect((void *) pAddr, data_len, PAGE_EXECUTE_READWRITE, &dwLastProtection))
		return 0;
	memcpy_s(pAddr, buf_len, pNewData, data_len);
	return VirtualProtect((void *) pAddr, data_len, dwLastProtection, &dwLastProtection);
}

#define IJ_JUMP 0 //Insert a jump (0xE9) with InsertJump
#define IJ_CALL 1 //Insert a call (0xE8) with InsertJump
#define IJ_FARJUMP 2 //Insert a farjump (0xEA) with InsertJump
#define IJ_FARCALL 3 //Insert a farcall (0x9A) with InsertJump
#define IJ_PUSHRET 4 //Insert a pushret with InsertJump
BOOL  Hooks::InsertJump(PVOID pDest, size_t dwPatchSize, PVOID pCallee, DWORD dwJumpType) {
	if (dwPatchSize >= 5 && pDest) {
		DWORD OpSize = 5, OpCode = 0xE9;
		PBYTE dest = (PBYTE) pDest;
		switch (dwJumpType) {
			case IJ_PUSHRET:
				OpSize = 6;
				OpCode = 0x68;
				break;
			case IJ_FARJUMP:
				OpSize = 7;
				OpCode = 0xEA;
				break;
			case IJ_FARCALL:
				OpSize = 7;
				OpCode = 0x9A;
				break;
			case IJ_CALL:
				OpSize = 5;
				OpCode = 0xE8;
				break;
			default:
				OpSize = 5;
				OpCode = 0xE9;
				break;
		}
		if (dwPatchSize < OpSize)
			return 0;
		PatchMemVal(dest, (BYTE) OpCode);
		switch (OpSize) {
			case 7:
				PatchMemVal(dest + 1, pCallee);
				WORD w_cseg;
				__asm mov[w_cseg], cs;
				PatchMemVal(dest + 5, w_cseg);
				break;
			case 6:
				PatchMemVal(dest + 1, pCallee);
				PatchMemVal<BYTE>(dest + 5, 0xC3);
				break;
			default:
				PatchMemVal(dest + 1, (ULONG_PTR) pCallee - (ULONG_PTR) pDest - 5);
				break;
		}
		for (size_t i = OpSize; i < dwPatchSize; i++)
			PatchMemVal<BYTE>(dest + i, 0x90);
	}
	return 0;
}

void Hooks::hookAsm(DWORD startAddr, DWORD hookAddr, const char * line) {
	if(!line) {
		debugf("hookAsm: 0x%X -> 0x%X\n", startAddr, hookAddr);
	} else {
		printf("%s hookAsm: 0x%X -> 0x%X\n", line, startAddr, hookAddr);
	}
	InsertJump((PVOID)startAddr, 6, (PVOID)hookAddr, IJ_PUSHRET);
}

void Hooks::patchAsm(DWORD addr, unsigned char * op, size_t opsize, const char * line) {
	if(Config::isDevConsoleEnabled()) {
		if(!line) {
			debugf("0x%X : ", addr);
		} else {
			printf("%s patchAsm: 0x%X : ",line, addr);
		}
		for (size_t i = 0; i < opsize; i++) {
			printf("%02X ", *(unsigned char *) (addr + i));
		}
		printf(" -> ");
		for (size_t i = 0; i < opsize; i++) {
			printf("%02X ", op[i]);
		}
		printf("\n");
	}
	PatchMemData((PVOID)addr, opsize, (PVOID)op, opsize);
}

void Hooks::hookVtable(const char * classname, int offset, DWORD addr, DWORD hookAddr, DWORD *original, const char * line) {
	if(!line) {
		debugf("%s::0x%X 0x%X -> 0x%X\n", classname, offset, *(DWORD *) addr, addr);
	} else {
		printf("%s hookVtable: %s::0x%X 0x%X -> 0x%X\n", line, classname, offset, *(DWORD *) addr, addr);
	}
	*original = *(DWORD*)addr;
	int dest = hookAddr;
	PatchMemData((PVOID)addr, sizeof(dest), &dest, sizeof(dest));
}


DWORD Hooks::scanPattern(const char *name, const char *pattern, const char* mask, DWORD expected, const char * line) {
	auto it = scanNameToAddr.find(name);
	uintptr_t ret = 0;
	if(it != scanNameToAddr.end()) {
		ret = it->second;
	} else {
		ret = hl::FindPatternMask(pattern, mask);
		if(ret) {
			scanNameToAddr[name] = ret;
		}
	}
	if(!line) {
		debugf("%s = 0x%X\n", name, ret);
	} else {
		printf("%s scanPattern: %s = 0x%X\n", line, name, ret);
	}
	if(!ret){
		throw std::runtime_error(std::format("scanPattern: failed to find memory pattern: {}", name));
	}
	return ret;
}
