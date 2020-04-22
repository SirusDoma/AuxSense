#include <AuxSense/AuxDevice.hpp>
#include <AuxSense/AuxClient.hpp>

#include <initguid.h>
#include <audioclient.h>
#include <detours.h>

#pragma clang diagnostic ignored "-Wmicrosoft-cast"

DEFINE_GUID(IID_IAudioClient,
    0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78,
    0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2
);

namespace AuxSense
{
    IMMDevice *AuxDevice::pDevice;
    HRESULT (STDMETHODCALLTYPE *TrueActivate)(
        IMMDevice* instance,
        REFIID iid,
        DWORD dwClsCtx,
        PROPVARIANT *pActivationParams,
        void **ppInterface
    );

    void AuxDevice::Hook(IMMDevice *device)
    {
        static bool hooked = false;
        if (hooked)
            return;

        // Fetch original function
        pDevice = device;
        TrueActivate = pDevice->lpVtbl->Activate;

        // Hook device functions
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID &) TrueActivate, Activate);
        DetourTransactionCommit();

        hooked = true;
    }

    HRESULT AuxDevice::Activate(
        IMMDevice *instance,
        REFIID iid,
        DWORD dwClsCtx,
        PROPVARIANT *pActivationParams,
        void **ppInterface
    )
    {
        HRESULT hr = TrueActivate(
            instance,
            iid,
            dwClsCtx,
            pActivationParams,
            ppInterface
        );

        if (FAILED(hr))
            return hr;

        if (IsEqualIID(iid, IID_IAudioClient))
            AuxClient::Hook((IAudioClient *) *ppInterface);

        return hr;
    }
}
