#ifndef AUXSENSE_AUDIOSYSTEM_HPP
#define AUXSENSE_AUDIOSYSTEM_HPP

#include <windows.h>
#include <initguid.h>
#include <audioclient.h>
#include <ksmedia.h>
#include <mmdeviceapi.h>

struct Module
{
    DWORD dwBase, dwSize;
};


namespace AuxSense
{
    class AudioSystem
    {
    private:
        static HRESULT WINAPI CoCreateInstance(
            REFCLSID rclsid,
            LPUNKNOWN pUnkOuter,
            DWORD dwClsContext,
            REFIID riid,
            LPVOID *ppv
        );

    public:
        static void Initialize(HMODULE hMod);
        static bool Ready();
    };
}

#endif
