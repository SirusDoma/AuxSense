#ifndef AUXSENSE_AUXDEVICE_HPP
#define AUXSENSE_AUXDEVICE_HPP

#include <mmdeviceapi.h>

namespace AuxSense
{
    class AuxDevice
    {
    private:
        static IMMDevice *pDevice;

    public:
        static void Hook(IMMDevice* device);
        static HRESULT STDMETHODCALLTYPE Activate(
            IMMDevice* instance,
            REFIID iid,
            DWORD dwClsCtx,
            PROPVARIANT *pActivationParams,
            void **ppInterface
        );
    };
}

#endif
