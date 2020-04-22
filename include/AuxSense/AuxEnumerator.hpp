#ifndef AUXSENSE_AUXENUMERATOR_HPP
#define AUXSENSE_AUXENUMERATOR_HPP

#include <mmdeviceapi.h>

namespace AuxSense
{
    class AuxEnumerator
    {
    private:
        static IMMDeviceEnumerator* pEnumerator;

    public:
        static void Hook(IMMDeviceEnumerator* enumerator);
        static HRESULT STDMETHODCALLTYPE GetDefaultAudioEndpoint(
            IMMDeviceEnumerator *instance,
            EDataFlow dataFlow,
            ERole role,
            IMMDevice **ppEndpoint
        );
    };
}

#endif
