#include "stdafx.h"

#include "util.h"
#include "hooks.h"
#include "inifile.h"

HANDLE hXam;
BOOL bRunContinuous = FALSE;
BOOL bLoopHasComplete = FALSE;
DWORD LastTitleId;

VOID MainThread()
{
    if (!NT_SUCCESS(XexGetModuleHandle("xam.xex", &hXam)))
    {
        DebugPrint("Failed to get xam.xex module handle!\n");
        return;
    }
    if (MountPath(MOUNT_POINT, GetMountPath()) != 0)
    {
        DebugPrint("Failed to set mount point!\n");
        return;
    }
    while (bRunContinuous)
	{
		DWORD TitleID = XamGetCurrentTitleId();

		if (TitleID != LastTitleId)
		{
			LastTitleId = TitleID;

            Readini(); // reload ini file when new title loads

            // try hooking again (or just remove if unsupported)
            //RemoveHooks();
            if(MatchTitleID(TitleID))
            {
                DebugPrint("Title ID detected: %d (dec)\n", TitleID);
                SetupHooks(); // only init net hooks if this titleid is supported
            }
        }
        Sleep(500);
    }
    bLoopHasComplete = TRUE;
}

BOOL DllMain(HINSTANCE hModule, DWORD reason, void *pReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        bRunContinuous = TRUE;
        ThreadMe((LPTHREAD_START_ROUTINE)MainThread);
        DebugPrint("XenonHTTPnoS loaded!\n");
        break;
    case DLL_PROCESS_DETACH:
        if(bRunContinuous)
        {
            bRunContinuous = FALSE;
            while (!bLoopHasComplete)
                Sleep(100);
        }
        //RemoveHooks();
        Sleep(500);

        DebugPrint("XenonHTTPnoS unloaded!\n");
        break;
    }

    return TRUE;
}
