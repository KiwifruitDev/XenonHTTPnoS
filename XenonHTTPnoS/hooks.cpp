#include "stdafx.h"
#include "util.h"
#include "inifile.h"
#include <vector>

PVOID NetDll_XHttpConnectHook(XNCALLER_TYPE CallerType, PVOID hSession, CONST CHAR * pcszServerName, WORD nServerPort, DWORD dwFlags)
{
    const CHAR* newServerName = pcszServerName;
    WORD newServerPort = nServerPort;
    DWORD newFlags = dwFlags;
    RewriteHttpRequest(pcszServerName, nServerPort, dwFlags, &newServerName, &newServerPort, &newFlags);

    return NetDll_XHttpConnect(CallerType, hSession, newServerName, newServerPort, newFlags);
}

VOID SetupHooks()
{
	PatchModuleImport((PLDR_DATA_TABLE_ENTRY)*XexExecutableModuleHandle, "xam.xex", 205, (DWORD)NetDll_XHttpConnectHook);
}
