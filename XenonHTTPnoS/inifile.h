#pragma once

#ifndef INIFILE_H
#define INIFILE_H

#include "stdafx.h"
#include "simpleini.h"

#define PATH_INI MOUNT_POINT "\\XenonHTTPnoS.ini"
#define XHTTP_FLAG_SECURE 0x800000

extern BOOL bDebuggerLogging;

VOID Writeini(BOOL GenerateNew);
VOID Readini();

BOOL MatchTitleID(DWORD titleId);

VOID RewriteHttpRequest(
    CONST CHAR* pcszServerName,
    WORD nServerPort,
    DWORD dwFlags,
    const CHAR** pNewServerName,
    WORD* pNewServerPort,
    DWORD* pNewFlags
);

#endif
