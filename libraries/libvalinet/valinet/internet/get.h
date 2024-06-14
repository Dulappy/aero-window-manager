#ifndef LIBVALINET_INTERNET_GET_H_
#define LIBVALINET_INTERNET_GET_H_
#include <stdio.h>
#include <Windows.h>
#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")

DWORD VnDownloadFile(
    char* filename,
    char* hostname,
    char* path,
    char* userAgent,
    INTERNET_PORT nServerPort,
    DWORD dwService,
    char* referrer,
    char* headers,
    DWORD bufsiz
)
{
    DWORD dwRet = 0;
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
            hostname,
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
                "GET",
                path,
                NULL,
                referrer,
                NULL,
                NULL,
                NULL
            ))
            {
                char data[1] = "";
                if (HttpSendRequestA(
                    hRequest,
                    headers,
                    strlen(headers),
                    (LPVOID)(data),
                    strlen(data) * sizeof(char)
                ))
                {
                    FILE* f = NULL; 
                    
                    fopen_s(
                        &f,
                        filename,
                        "wb"
                    );
                    if (!f)
                    {
                        dwRet = 7;
                    }
                    else
                    {
                        BYTE* buffer = (BYTE*)malloc(bufsiz);
                        if (buffer == NULL)
                        {
                            dwRet = 6;
                        }
                        else
                        {
                            DWORD dwRead;
                            BOOL bRet = TRUE;
                            while (bRet = InternetReadFile(
                                hRequest,
                                buffer,
                                bufsiz,
                                &dwRead
                            ))
                            {
                                if (dwRead == 0)
                                {
                                    break;
                                }
                                fwrite(
                                    buffer, 
                                    sizeof(BYTE),
                                    dwRead, 
                                    f
                                );
                                dwRead = 0;
                            }
                            if (bRet == FALSE)
                            {
                                dwRet = 5;
                            }
                            free(buffer);
                        }
                        fclose(f);
                    }
                }
                else
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