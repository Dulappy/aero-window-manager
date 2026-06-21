#ifndef LIBVALINET_INI_INI_H_
#define LIBVALINET_INI_INI_H_

// adapted from original source code: 
// https://www.codeproject.com/Articles/5401/CIni

#include <Windows.h>
#include <tchar.h>

#define DEF_PROFILE_NUM_LEN		            64
#define DEF_PROFILE_THRESHOLD           	512

LPTSTR libvalinet_ini_ini_GetStringDynamic(
    LPCTSTR lpSection,
    LPCTSTR lpKey,
    LPCTSTR lpDefault,
    LPCWSTR m_pszPathName
)
{
    TCHAR* psz = NULL;
    if (lpSection == NULL || lpKey == NULL)
    {
        // Invalid section or key name, just return the default string
        if (lpDefault == NULL)
        {
            // Empty string
            psz = (TCHAR*)malloc(sizeof(TCHAR));
            if (psz)
            {
                *psz = _T('\0');
            }
        }
        else
        {
            psz = (TCHAR*)malloc(sizeof(TCHAR) * (_tcslen(lpDefault) + 1));
            if (psz)
            {
                _tcscpy_s(
                    psz,
                    sizeof(TCHAR) * (_tcslen(lpDefault) + 1),
                    lpDefault
                );
            }
        }

        return psz;
    }

    // Keep enlarging the buffer size until being certain on that the string we
    // retrieved was original(not truncated).
    DWORD dwLen = DEF_PROFILE_THRESHOLD;
    psz = (TCHAR*)malloc(sizeof(TCHAR) * ((SIZE_T)dwLen + 1));
    DWORD dwCopied = GetPrivateProfileString(
        lpSection,
        lpKey,
        lpDefault == NULL ? _T("") : lpDefault,
        psz,
        dwLen,
        m_pszPathName
    );
    while (dwCopied + 1 >= dwLen)
    {
        dwLen += DEF_PROFILE_THRESHOLD;
        free(psz);
        psz = (TCHAR*)malloc(sizeof(TCHAR) * ((SIZE_T)dwLen + 1));
        dwCopied = GetPrivateProfileString(
            lpSection,
            lpKey,
            lpDefault == NULL ? _T("") : lpDefault,
            psz,
            dwLen,
            m_pszPathName
        );
    }

    return psz; // !!! Requires the caller to free this memory !!!
}

DWORD VnGetString(
    LPCTSTR lpSection,
    LPCTSTR lpKey,
    LPTSTR lpBuffer,
    DWORD dwBufferSize,
    DWORD dwBufSize,
    LPCTSTR lpDefault,
    LPCWSTR m_pszPathName
)
{
    if (lpBuffer != NULL)
        *lpBuffer = _T('\0');

    LPTSTR psz = libvalinet_ini_ini_GetStringDynamic(
        lpSection,
        lpKey,
        lpDefault,
        m_pszPathName
    );
    DWORD dwLen = _tcslen(psz);

    if (lpBuffer != NULL)
    {
        _tcsncpy_s(
            lpBuffer,
            dwBufferSize,
            psz,
            dwBufSize
        );
        dwLen = min(dwLen, dwBufSize);
    }

    free(psz);
    return dwLen;
}

UINT VnGetUInt(
    LPCTSTR lpSection,
    LPCTSTR lpKey,
    UINT nDefault,
    LPCWSTR m_pszPathName
)
{
    TCHAR sz[DEF_PROFILE_NUM_LEN + 1] = _T("");
    VnGetString(
        lpSection,
        lpKey,
        sz,
        sizeof(sz) / sizeof(TCHAR),
        DEF_PROFILE_NUM_LEN,
        NULL,
        m_pszPathName
    );
    return *sz == _T('\0') ?
        nDefault :
        (UINT)(_tcstoul(sz, NULL, 10));
}

BOOL VnWriteString(
    LPCTSTR lpSection,
    LPCTSTR lpKey,
    LPCTSTR lpValue,
    LPCWSTR m_pszPathName
)
{
    if (lpSection == NULL || lpKey == NULL)
        return FALSE;

    return WritePrivateProfileString(
        lpSection,
        lpKey,
        lpValue == NULL ? _T("") : lpValue,
        m_pszPathName
    );
}

BOOL VnWriteUInt(
    LPCTSTR lpSection,
    LPCTSTR lpKey,
    UINT nValue,
    LPCWSTR m_pszPathName
)
{
    TCHAR szValue[DEF_PROFILE_NUM_LEN + 1] = _T("");
    _stprintf_s(
        szValue,
        DEF_PROFILE_NUM_LEN + 1,
        _T("%u"),
        nValue
    );
    return VnWriteString(
        lpSection,
        lpKey,
        szValue,
        m_pszPathName
    );
}

#endif // #ifndef __INI_H_