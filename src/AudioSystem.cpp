#include <AuxSense/AudioSystem.hpp>
#include <AuxSense/AuxEnumerator.hpp>

#include <iostream>
#include <Psapi.h>
#include <detours.h>
#include <AuxSense/Log.hpp>

#pragma warning(disable: 26812)
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

namespace AuxSense
{
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    static HRESULT(WINAPI* TrueCoCreateInstance)(
        REFCLSID,
        LPUNKNOWN,
        DWORD,
        REFIID,
        LPVOID*
    ) = CoCreateInstance;

    void AudioSystem::Initialize(HMODULE hMod)
    {
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueCoCreateInstance, CoCreateInstance);
        DetourTransactionCommit();
    }

    bool AudioSystem::Ready()
    {
        return !DetourIsHelperProcess();
    }

    HRESULT AudioSystem::CoCreateInstance(
        REFCLSID rclsid,
        LPUNKNOWN pUnkOuter,
        DWORD dwClsContext,
        REFIID riid,
        LPVOID *ppv
    )
    {
        HRESULT hr = TrueCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
        if (FAILED(hr))
            return hr;

        if (IsEqualCLSID(rclsid, CLSID_MMDeviceEnumerator))
            AuxEnumerator::Hook((IMMDeviceEnumerator *) *ppv);

        return hr;
    }
}
