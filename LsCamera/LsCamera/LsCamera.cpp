// LsCamera.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <string>
#include <dshow.h>
#pragma comment(lib, "strmiids.lib")


void main()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM library";
        return;
    }

    ICreateDevEnum* pDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDevEnum);
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator";
        CoUninitialize();
        return;
    }

    IEnumMoniker* pEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr == S_FALSE) {
        std::cerr << "No video capture devices found.";
        pDevEnum->Release();
        CoUninitialize();
        return;
    }

    IMoniker* pMoniker = NULL;
    while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
        IPropertyBag* pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);
        if (SUCCEEDED(hr)) {
            VARIANT varName;
            VariantInit(&varName);
            hr = pPropBag->Read(L"FriendlyName", &varName, 0);
            if (SUCCEEDED(hr)) {
                std::wcout << varName.bstrVal << L"\n";
            }
            VariantClear(&varName);
            pPropBag->Release();
        }
        pMoniker->Release();
    }

    pEnum->Release();
    pDevEnum->Release();
    CoUninitialize();
    return;
}