#ifndef LIBVALINET_HOOKING_OSVERSION_H_
#define LIBVALINET_HOOKING_OSVERSION_H_
#include <Windows.h>

#define _LIBVALINET_HOOKING_OSVERSION_INVALID 0xffffffff

typedef LONG NTSTATUS, * PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS(WINAPI* VnRtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

// https://stackoverflow.com/questions/36543301/detecting-windows-10-version/36543774#36543774
inline BOOL VnGetOSVersion(PRTL_OSVERSIONINFOW lpRovi)
{
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod != NULL)
    {
        VnRtlGetVersionPtr fxPtr = (VnRtlGetVersionPtr)GetProcAddress(
            hMod,
            "RtlGetVersion"
        );
        if (fxPtr != NULL)
        {
            lpRovi->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
            if (STATUS_SUCCESS == fxPtr(lpRovi))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

// https://stackoverflow.com/questions/47926094/detecting-windows-10-os-build-minor-version
inline DWORD32 VnGetUBR()
{
    DWORD32 ubr = 0, ubr_size = sizeof(DWORD32);
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        wcschr(
            wcschr(
                wcschr(
                    UNIFIEDBUILDREVISION_KEY,
                    '\\'
                ) + 1,
                '\\'
            ) + 1,
            '\\'
        ) + 1,
        0,
        KEY_READ,
        &hKey
    );
    if (lRes == ERROR_SUCCESS)
    {
        RegQueryValueExW(
            hKey,
            UNIFIEDBUILDREVISION_VALUE,
            0,
            NULL,
            &ubr,
            &ubr_size
        );
    }
}

inline DWORD32 VnGetOSVersionAndUBR(PRTL_OSVERSIONINFOW lpRovi)
{
    if (!VnGetOSVersion(lpRovi))
    {
        return _LIBVALINET_HOOKING_OSVERSION_INVALID;
    }
    return VnGetUBR();
}
#endif