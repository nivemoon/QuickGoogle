#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <strsafe.h>
#include <new>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shell32.lib")

static const UINT MENU_ID = 1;

class GoogleSearchContextMenu final
    : public IShellExtInit,
      public IContextMenu
{
private:
    LONG refCount;
    WCHAR filePath[MAX_PATH];

public:
    GoogleSearchContextMenu()
        : refCount(1)
    {
        filePath[0] = L'\0';
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void** object
    ) override
    {
        if (!object) {
            return E_POINTER;
        }

        *object = nullptr;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IShellExtInit)) {
            *object = static_cast<IShellExtInit*>(this);
        } else if (IsEqualIID(riid, IID_IContextMenu)) {
            *object = static_cast<IContextMenu*>(this);
        } else {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return static_cast<ULONG>(
            InterlockedIncrement(&refCount)
        );
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG value = static_cast<ULONG>(
            InterlockedDecrement(&refCount)
        );

        if (value == 0) {
            delete this;
        }

        return value;
    }

    HRESULT STDMETHODCALLTYPE Initialize(
        PCIDLIST_ABSOLUTE,
        IDataObject* dataObject,
        HKEY
    ) override
    {
        if (!dataObject) {
            return E_INVALIDARG;
        }

        FORMATETC formatEtc = {};
        formatEtc.cfFormat = CF_HDROP;
        formatEtc.dwAspect = DVASPECT_CONTENT;
        formatEtc.lindex = -1;
        formatEtc.tymed = TYMED_HGLOBAL;

        STGMEDIUM storageMedium = {};

        HRESULT result = dataObject->GetData(
            &formatEtc,
            &storageMedium
        );

        if (FAILED(result)) {
            return result;
        }

        HDROP drop = static_cast<HDROP>(
            GlobalLock(storageMedium.hGlobal)
        );

        if (!drop) {
            ReleaseStgMedium(&storageMedium);
            return E_FAIL;
        }

        UINT count = DragQueryFileW(
            drop,
            0xFFFFFFFF,
            nullptr,
            0
        );

        if (count != 1) {
            GlobalUnlock(storageMedium.hGlobal);
            ReleaseStgMedium(&storageMedium);
            return E_FAIL;
        }

        DragQueryFileW(
            drop,
            0,
            filePath,
            ARRAYSIZE(filePath)
        );

        GlobalUnlock(storageMedium.hGlobal);
        ReleaseStgMedium(&storageMedium);

        return filePath[0] ? S_OK : E_FAIL;
    }

    HRESULT STDMETHODCALLTYPE QueryContextMenu(
        HMENU menu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT,
        UINT flags
    ) override
    {
        if (flags & CMF_DEFAULTONLY) {
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 0);
        }

        InsertMenuW(
            menu,
            indexMenu,
            MF_BYPOSITION | MF_STRING,
            idCmdFirst + MENU_ID,
            L"Искать в Google"
        );

        return MAKE_HRESULT(
            SEVERITY_SUCCESS,
            0,
            MENU_ID + 1
        );
    }

    HRESULT STDMETHODCALLTYPE InvokeCommand(
        LPCMINVOKECOMMANDINFO commandInfo
    ) override
    {
        if (!commandInfo) {
            return E_INVALIDARG;
        }

        if (HIWORD(commandInfo->lpVerb) != 0) {
            return E_FAIL;
        }

        if (LOWORD(commandInfo->lpVerb) != MENU_ID) {
            return E_FAIL;
        }

        WCHAR fileName[MAX_PATH];
        WCHAR* slash;
        WCHAR* dot;
        WCHAR url[4096];

        StringCchCopyW(
            fileName,
            ARRAYSIZE(fileName),
            filePath
        );

        slash = wcsrchr(fileName, L'\\');

        if (slash) {
            StringCchCopyW(
                fileName,
                ARRAYSIZE(fileName),
                slash + 1
            );
        }

        dot = wcsrchr(fileName, L'.');

        if (dot && dot != fileName) {
            *dot = L'\0';
        }

        StringCchPrintfW(
            url,
            ARRAYSIZE(url),
            L"https://www.google.com/search?q=%s",
            fileName
        );

        ShellExecuteW(
            nullptr,
            L"open",
            url,
            nullptr,
            nullptr,
            SW_SHOWNORMAL
        );

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetCommandString(
        UINT_PTR command,
        UINT flags,
        UINT*,
        LPSTR text,
        UINT textLength
    ) override
    {
        if (command != MENU_ID || !text || textLength == 0) {
            return E_INVALIDARG;
        }

        if (flags == GCS_HELPTEXTA) {
            StringCchCopyA(
                text,
                textLength,
                "Search selected file in Google"
            );
            return S_OK;
        }

        return E_NOTIMPL;
    }
};

class GoogleSearchClassFactory final : public IClassFactory
{
private:
    LONG refCount;

public:
    GoogleSearchClassFactory()
        : refCount(1)
    {
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void** object
    ) override
    {
        if (!object) {
            return E_POINTER;
        }

        *object = nullptr;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IClassFactory)) {
            *object = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return static_cast<ULONG>(
            InterlockedIncrement(&refCount)
        );
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG value = static_cast<ULONG>(
            InterlockedDecrement(&refCount)
        );

        if (value == 0) {
            delete this;
        }

        return value;
    }

    HRESULT STDMETHODCALLTYPE CreateInstance(
        IUnknown* outer,
        REFIID riid,
        void** object
    ) override
    {
        if (outer) {
            return CLASS_E_NOAGGREGATION;
        }

        GoogleSearchContextMenu* handler =
            new (std::nothrow) GoogleSearchContextMenu();

        if (!handler) {
            return E_OUTOFMEMORY;
        }

        HRESULT result = handler->QueryInterface(
            riid,
            object
        );

        handler->Release();
        return result;
    }

    HRESULT STDMETHODCALLTYPE LockServer(
        BOOL
    ) override
    {
        return S_OK;
    }
};

static const CLSID CLSID_GoogleSearchContext =
{
    0x9d7e5f81,
    0x26a4,
    0x4f32,
    {0x91, 0x6d, 0x43, 0x8d, 0xa1, 0x72, 0x5b, 0x09}
};

static HMODULE moduleHandle;
static LONG serverLocks;

extern "C" HRESULT __declspec(dllexport)
DllGetClassObject(
    REFCLSID clsid,
    REFIID riid,
    void** object
)
{
    if (!IsEqualCLSID(
            clsid,
            CLSID_GoogleSearchContext
        )) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    GoogleSearchClassFactory* factory =
        new (std::nothrow) GoogleSearchClassFactory();

    if (!factory) {
        return E_OUTOFMEMORY;
    }

    HRESULT result = factory->QueryInterface(
        riid,
        object
    );

    factory->Release();
    return result;
}

extern "C" HRESULT __declspec(dllexport)
DllCanUnloadNow()
{
    return serverLocks == 0
        ? S_OK
        : S_FALSE;
}

BOOL WINAPI DllMain(
    HINSTANCE instance,
    DWORD reason,
    LPVOID
)
{
    if (reason == DLL_PROCESS_ATTACH) {
        moduleHandle = instance;
        DisableThreadLibraryCalls(instance);
    }

    return TRUE;
}
