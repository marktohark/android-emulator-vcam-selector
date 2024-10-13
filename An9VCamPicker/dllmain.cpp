#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <detours.h>
#include <combaseapi.h>
#include <dshow.h>
#include <tuple>
#include <iostream>
#include <fstream>
#include "ThreadResource.h"
#include "Communicate.h"
#include <codecvt>


#pragma comment(lib, "strmiids.lib")

ThreadResource thRes;
Communicate comm;
const char* printLogPath = "c:\\Users\\mark\\Desktop\\log.log";

typedef HRESULT(WINAPI* CreateClassEnumeratorFuncDef)(
    ICreateDevEnum*,
    REFCLSID,
    IEnumMoniker**,
    DWORD
);

typedef HRESULT(WINAPI* IEnumMoniker_NextDef)(
    IEnumMoniker*,
    ULONG,
    IMoniker**,
    ULONG*
);

static CreateClassEnumeratorFuncDef RealCreateClassEnumerator = nullptr;
static IEnumMoniker_NextDef RealIEnumMoniker_Next = nullptr;

bool CmpGuid(REFCLSID a, REFCLSID b) {
    return a.Data1 == b.Data1 && a.Data2 == b.Data2 && a.Data3 == b.Data3;
}

static HRESULT WINAPI MyCreateClassEnumerator(
    ICreateDevEnum* pDevEnum,
    REFCLSID     clsidDeviceClass,
    IEnumMoniker** ppEnumMoniker,
    DWORD        dwFlags) {
    HRESULT r = RealCreateClassEnumerator(pDevEnum, clsidDeviceClass, ppEnumMoniker, dwFlags);
    if (!CmpGuid(CLSID_VideoInputDeviceCategory, clsidDeviceClass) || FAILED(r)) {
        return r;
    }
    auto targetVCamName = comm.GetVCamWName();

    if (targetVCamName == L"") {
        return r;
    }

    IEnumMoniker* pEnum = *ppEnumMoniker;
    IMoniker* pMoniker = NULL;
    while (RealIEnumMoniker_Next(pEnum, 1, &pMoniker, NULL) == S_OK) {
        bool has = false;
        IPropertyBag* pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
        if (SUCCEEDED(hr)) {
            VARIANT varName;
            VariantInit(&varName);
            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            if (SUCCEEDED(hr)) {
                if (wcscmp(targetVCamName.c_str(), varName.bstrVal) == 0) {
                    auto ctx = thRes.GetContext();
                    ctx->m_IMoniker = pMoniker;
                    has = true;
                    comm.Send("selected;" + comm.GetVCamName() + ";");
                }
            }
            VariantClear(&varName);
            pPropBag->Release();
        }
        if(!has) pMoniker->Release();
    }

    return r;
}

static HRESULT WINAPI MyIEnumMoniker_Next(IEnumMoniker* pEnum, ULONG celt, IMoniker** rgelt, ULONG* pceltFetched) {
    auto r = RealIEnumMoniker_Next(pEnum, celt, rgelt, pceltFetched);
    if (FAILED(r)) {
        return r;
    }
    auto ctx = thRes.GetContext();
    
    if (ctx->m_IMoniker != nullptr) {
        (*rgelt)->Release();
        *rgelt = ctx->m_IMoniker;
        ctx->m_IMoniker = nullptr;
    }
    
    return r;
}

std::tuple<void*, void*> GetDshowVTableAddr() {
    auto hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        return std::make_tuple(nullptr, nullptr);
    }
    ICreateDevEnum* pDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDevEnum);
    if (FAILED(hr)) {
        CoUninitialize();
        return std::make_tuple(nullptr, nullptr);
    }
    IEnumMoniker* pEnum = nullptr;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr != S_OK) {
        pDevEnum->Release();
        CoUninitialize();
        return std::make_tuple(nullptr, nullptr);
    }

    IMoniker* pMoniker = nullptr;
    pEnum->Next(1, &pMoniker, nullptr);

    auto vtableAddr = *(void***)pDevEnum;
    auto CreateClassEnumeratorAddr = *(void**)(vtableAddr + 3);

    vtableAddr = *(void***)pEnum;
    auto NextAddr = *(void**)(vtableAddr + 3);

    return std::make_tuple(CreateClassEnumeratorAddr, NextAddr);
}

void StartHook() {
    // get vtable method addr
    void* CreateClassEnumeratorAddr = nullptr;
    void* IEnumMoniker_NextAddr = nullptr;

    std::tie(CreateClassEnumeratorAddr, IEnumMoniker_NextAddr) = GetDshowVTableAddr();
    RealCreateClassEnumerator = (CreateClassEnumeratorFuncDef)CreateClassEnumeratorAddr;
    RealIEnumMoniker_Next = (IEnumMoniker_NextDef)IEnumMoniker_NextAddr;

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    // do hook
    DetourAttach(&RealCreateClassEnumerator, MyCreateClassEnumerator);
    DetourAttach(&RealIEnumMoniker_Next, MyIEnumMoniker_Next);
    // do hook
    DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        comm.Start();
        StartHook();
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

std::string wstringToAscii(const std::wstring& wstr) {
    int bufferSize = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string asciiStr(bufferSize - 1, 0);

    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &asciiStr[0], bufferSize, nullptr, nullptr);

    return asciiStr;
}

extern "C" __declspec(dllexport) void LsCamera(char* out, char* err) //detours need it
{
    return;
}