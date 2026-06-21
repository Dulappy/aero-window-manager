#ifndef LIBVALINET_UNIVERSAL_TOAST_TOAST_H_
#define LIBVALINET_UNIVERSAL_TOAST_TOAST_H_
#ifdef _LIBVALINET_INCLUDE_UNIVERSAL
#pragma comment(lib, "runtimeobject.lib")
#include <initguid.h>
#include <roapi.h>
#include <windows.data.xml.dom.h>
#include <windows.data.json.h>
// UUID obtained from <windows.ui.notifications.h>
//
// ABI.Windows.UI.Notifications.IToastNotificationManagerStatics
// 50ac103f-d235-4598-bbef-98fe4d1a3ad4
DEFINE_GUID(UIID_IToastNotificationManagerStatics,
    0x50ac103f,
    0xd235, 0x4598, 0xbb, 0xef,
    0x98, 0xfe, 0x4d, 0x1a, 0x3a, 0xd4
);

// UUID obtained from <windows.ui.notifications.h>
//
// ABI.Windows.Notifications.IToastNotificationFactory
// 04124b20-82c6-4229-b109-fd9ed4662b53
DEFINE_GUID(UIID_IToastNotificationFactory,
    0x04124b20,
    0x82c6, 0x4229, 0xb1, 0x09,
    0xfd, 0x9e, 0xd4, 0x66, 0x2b, 0x53
);

// UUID obtained from <windows.data.xml.dom.h>
//
// ABI.Windows.Data.Xml.Dom.IXmlDocument
// f7f3a506-1e87-42d6-bcfb-b8c809fa5494
DEFINE_GUID(UIID_IXmlDocument,
    0xf7f3a506,
    0x1e87, 0x42d6, 0xbc, 0xfb,
    0xb8, 0xc8, 0x09, 0xfa, 0x54, 0x94
);

// UUID obtained from <windows.data.xml.dom.h>
//
// ABI.Windows.Data.Xml.Dom.IXmlDocumentIO
// 6cd0e74e-ee65-4489-9ebf-ca43e87ba637
DEFINE_GUID(UIID_IXmlDocumentIO,
    0x6cd0e74e,
    0xee65, 0x4489, 0x9e, 0xbf,
    0xca, 0x43, 0xe8, 0x7b, 0xa6, 0x37
);
inline HRESULT ShowToastMessage(
    __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument* inputXml,
    wchar_t* pwszAppId,
    DWORD dwAppIdLength,
    void* stream
)
{
    HRESULT hr = S_OK;

#pragma warning (disable: 6031)
    CoInitialize(NULL);
    RoInitialize(RO_INIT_MULTITHREADED);
#pragma warning (default: 6031)

    HSTRING_HEADER header_AppIdHString;
    HSTRING AppIdHString;
    hr = WindowsCreateStringReference(
        pwszAppId,
        (UINT32)dwAppIdLength,
        &header_AppIdHString,
        &AppIdHString
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: WindowsCreateStringReference\n", __FUNCTION__, __LINE__);
        }
        goto exit1;
    }
    if (AppIdHString == NULL)
    {
        hr = E_POINTER;
        goto exit1;
    }

    HSTRING_HEADER header_ToastNotificationManagerHString;
    HSTRING ToastNotificationManagerHString;
    hr = WindowsCreateStringReference(
        RuntimeClass_Windows_UI_Notifications_ToastNotificationManager,
        (UINT32)(sizeof(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager) / sizeof(wchar_t) - 1),
        &header_ToastNotificationManagerHString,
        &ToastNotificationManagerHString
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: WindowsCreateStringReference\n", __FUNCTION__, __LINE__);
        }
        goto exit2;
    }
    if (ToastNotificationManagerHString == NULL)
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: ToastNotificationManagerHString == NULL\n", __FUNCTION__, __LINE__);
        }
        hr = E_POINTER;
        goto exit2;
    }

    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationManagerStatics* toastStatics = NULL;
    hr = RoGetActivationFactory(
        ToastNotificationManagerHString,
        &UIID_IToastNotificationManagerStatics,
        (LPVOID*)&toastStatics
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: RoGetActivationFactory\n", __FUNCTION__, __LINE__);
        }
        goto exit2;
    }

    __x_ABI_CWindows_CUI_CNotifications_CIToastNotifier* notifier;
    hr = toastStatics->lpVtbl->CreateToastNotifierWithId(
        toastStatics,
        AppIdHString,
        &notifier
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: CreateToastNotifierWithId\n", __FUNCTION__, __LINE__);
        }
        goto exit3;
    }

    HSTRING_HEADER header_ToastNotificationHString;
    HSTRING ToastNotificationHString;
    hr = WindowsCreateStringReference(
        RuntimeClass_Windows_UI_Notifications_ToastNotification,
        (UINT32)(sizeof(RuntimeClass_Windows_UI_Notifications_ToastNotification) / sizeof(wchar_t) - 1),
        &header_ToastNotificationHString,
        &ToastNotificationHString
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: WindowsCreateStringReference\n", __FUNCTION__, __LINE__);
        }
        goto exit4;
    }
    if (ToastNotificationHString == NULL)
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: ToastNotificationHString == NULL\n", __FUNCTION__, __LINE__);
        }
        hr = E_POINTER;
        goto exit4;
    }

    __x_ABI_CWindows_CUI_CNotifications_CIToastNotificationFactory* notifFactory = NULL;
    hr = RoGetActivationFactory(
        ToastNotificationHString,
        &UIID_IToastNotificationFactory,
        (LPVOID*)&notifFactory
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: RoGetActivationFactory\n", __FUNCTION__, __LINE__);
        }
        goto exit4;
    }

    __x_ABI_CWindows_CUI_CNotifications_CIToastNotification* toast = NULL;
    hr = notifFactory->lpVtbl->CreateToastNotification(notifFactory, inputXml, &toast);
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: CreateToastNotification\n", __FUNCTION__, __LINE__);
        }
        goto exit5;
    }

    hr = notifier->lpVtbl->Show(notifier, toast);
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream, "%s:%d:: Show\n", __FUNCTION__, __LINE__);
        }
        goto exit6;
    }

exit6:
    toast->lpVtbl->Release(toast);
exit5:
    notifFactory->lpVtbl->Release(notifFactory);
exit4:
    notifier->lpVtbl->Release(notifier);
exit3:
    toastStatics->lpVtbl->Release(toastStatics);
exit2:
    inputXml->lpVtbl->Release(inputXml);
exit1:
    return hr;
}

inline HRESULT String2IXMLDocument(
    wchar_t* pwszData,
    DWORD dwDataLength,
    __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocument** pxmlToastMessage,
    void* stream
)
{
    HRESULT hr = S_OK;

    CoInitialize(NULL);
    RoInitialize(RO_INIT_MULTITHREADED);

    HSTRING_HEADER hshXmlDocumentS_h;
    HSTRING hsXmlDocumentS;
    hr = WindowsCreateStringReference(
        RuntimeClass_Windows_Data_Xml_Dom_XmlDocument,
        (UINT32)(sizeof(RuntimeClass_Windows_Data_Xml_Dom_XmlDocument) / sizeof(wchar_t) - 1),
        &hshXmlDocumentS_h,
        &hsXmlDocumentS
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream,
                "%s:%d:: WindowsCreateStringReference = %d\n", __FUNCTION__, __LINE__,
                hr
            );
        }
        goto exit0;
    }

    HSTRING_HEADER hshxmlString_h;
    HSTRING hshxmlString;
    hr = WindowsCreateStringReference(
        pwszData,
        (UINT32)dwDataLength,
        &hshxmlString_h,
        &hshxmlString
    );
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream,
                "%s:%d:: WindowsCreateStringReference = %d\n", __FUNCTION__, __LINE__,
                hr
            );
        }
        goto exit1;
    }

    IInspectable* pInspectable = NULL;
    hr = RoActivateInstance(hsXmlDocumentS, &pInspectable);
    if (SUCCEEDED(hr))
    {
        hr = pInspectable->lpVtbl->QueryInterface(
            pInspectable,
            &UIID_IXmlDocument,
            pxmlToastMessage
        );
        pInspectable->lpVtbl->Release(pInspectable);
    }
    else
    {
        printf(
            "%s:%d:: RoActivateInstance = %d\n", __FUNCTION__, __LINE__,
            hr
        );
        goto exit2;
    }

    __x_ABI_CWindows_CData_CXml_CDom_CIXmlDocumentIO* pxmlDocumentIO = NULL;
    (*pxmlToastMessage)->lpVtbl->QueryInterface(
        (*pxmlToastMessage),
        &UIID_IXmlDocumentIO,
        &pxmlDocumentIO
    );
    if (FAILED(hr))
    {
        printf(
            "%s:%d:: QueryInterface = %d\n", __FUNCTION__, __LINE__,
            hr
        );
        goto exit3;
    }

    hr = pxmlDocumentIO->lpVtbl->LoadXml(pxmlDocumentIO, hshxmlString);
    if (FAILED(hr))
    {
        if (stream)
        {
            fprintf(stream,
                "%s:%d:: pxmlDocumentIO->LoadXml = %d\n", __FUNCTION__, __LINE__,
                hr
            );
        }
        goto exit3;
    }

exit3:
    pxmlDocumentIO->lpVtbl->Release(pxmlDocumentIO);
exit2:
    WindowsDeleteString(hshxmlString);
exit1:
    WindowsDeleteString(hsXmlDocumentS);
exit0:
    return hr;
}
#endif
#endif
