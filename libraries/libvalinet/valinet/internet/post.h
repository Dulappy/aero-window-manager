#ifndef LIBVALINET_INTERNET_POST_H_
#define LIBVALINET_INTERNET_POST_H_
#include <stdio.h>
#include <Windows.h>
#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")

DWORD VnPostData(
    char* data,
    char* hostname,
    char* path,
    char* userAgent,
    INTERNET_PORT nServerPort,
    DWORD dwService,
    char* referrer,
    char* headers
)
{
	DWORD dwRet = ERROR_SUCCESS;
	HINTERNET hInternet;
	if (hInternet = InternetOpenA(
		userAgent,
		INTERNET_OPEN_TYPE_DIRECT,
		NULL,
		NULL,
		NULL
	))
	{
		HINTERNET hConnect;
		if (hConnect = InternetConnectA(
			hInternet,
			hostName,
			nServerPort,
			NULL,
			NULL,
			dwService,
			NULL,
			NULL
		))
		{
			HINTERNET hRequest;
			if (hRequest = HttpOpenRequestA(
				hConnect,
				"POST",
				path,
				NULL,
				referrer,
				NULL,
				NULL,
				NULL
			))
			{
				if (!HttpSendRequestA(
					hRequest,
					headers,
					wcslen(headers),
					reinterpret_cast<LPVOID>(const_cast<char*>(data)),
					strlen(data) * sizeof(char)
				))
				{
					dwRet = 4;
				}
				InternetCloseHandle(hRequest);
			}
			else
			{
				dwRet = 3;
			}
			InternetCloseHandle(hConnect);
		}
		else
		{
			dwRet = 2;
		}
		InternetCloseHandle(hInternet);
	}
	else
	{
		dwRet = 1;
	}
	return dwRet;
}
#endif