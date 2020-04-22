#include <AuxSense/AuxEnumerator.hpp>
#include <AuxSense/AuxDevice.hpp>

#include <detours.h>

#pragma warning(disable: 26812)
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

namespace AuxSense
{
    IMMDeviceEnumerator *AuxEnumerator::pEnumerator;
    HRESULT (STDMETHODCALLTYPE *TrueGetDefaultAudioEndpoint)(
        IMMDeviceEnumerator *instance,
        EDataFlow dataFlow,
        ERole role,
        IMMDevice **ppEndpoint
    );

    void AuxEnumerator::Hook(IMMDeviceEnumerator *enumerator)
    {
        static bool hooked = false;
        if (hooked)
            return;

        // Fetch original function
        pEnumerator = enumerator;
        TrueGetDefaultAudioEndpoint = pEnumerator->lpVtbl->GetDefaultAudioEndpoint;

        // Hook enumerator functions
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID &) TrueGetDefaultAudioEndpoint, GetDefaultAudioEndpoint);
        DetourTransactionCommit();

        hooked = true;
    }

    HRESULT AuxEnumerator::GetDefaultAudioEndpoint(
        IMMDeviceEnumerator *instance,
        EDataFlow dataFlow,
        ERole role,
        IMMDevice **ppEndpoint
    )
    {
        HRESULT hr = TrueGetDefaultAudioEndpoint(instance, dataFlow, role, ppEndpoint);
        if (SUCCEEDED(hr))
            AuxDevice::Hook(*ppEndpoint);

        return hr;
    }
}
