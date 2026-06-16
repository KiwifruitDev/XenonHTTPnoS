#pragma once

#ifndef UTIL_H
#define UTIL_H

#include "stdafx.h"

FARPROC ResolveFunction(HMODULE hHandle, DWORD Ordinal);
FARPROC ResolveFunction(CHAR* ModuleName, DWORD Ordinal);
VOID PatchInJump(DWORD* addr, DWORD dest, BOOL linked);
DWORD PatchModuleImport(PLDR_DATA_TABLE_ENTRY Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress);
DWORD PatchModuleImport(CHAR* Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress);
BOOL IsTrayOpen();
VOID ThreadMe(LPTHREAD_START_ROUTINE lpStartAddress);

DWORD MountPath(PCHAR Drive, PCHAR Device);
PCHAR GetMountPath();

BOOL CWriteFile(const CHAR* FilePath, const VOID* Data, DWORD Size);

VOID DebugPrint(const CHAR* fmt, ...);

#define BASE_ADDR 0x91A00000
#define MOUNT_POINT "HNS:"

#endif
