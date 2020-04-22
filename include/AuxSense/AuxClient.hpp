#ifndef AUXSENSE_AUXCLIENT_HPP
#define AUXSENSE_AUXCLIENT_HPP

#include <audioclient.h>

namespace AuxSense
{
    class AuxClient
    {
    private:
        static IAudioClient *pAudioClient;
        static WAVEFORMATEX *pMixFormat;

    public:
        static void Hook(IAudioClient* client);

        static HRESULT STDMETHODCALLTYPE Initialize(
            IAudioClient * instance,
            AUDCLNT_SHAREMODE ShareMode,
            DWORD StreamFlags,
            REFERENCE_TIME hnsBufferDuration,
            REFERENCE_TIME hnsPeriodicity,
            const WAVEFORMATEX *pFormat,
            LPCGUID AudioSessionGuid
        );

        static HRESULT STDMETHODCALLTYPE GetMixFormat(
            IAudioClient *instance,
            WAVEFORMATEX **ppDeviceFormat
        );

        static HRESULT STDMETHODCALLTYPE IsFormatSupported(
            IAudioClient* instance,
            AUDCLNT_SHAREMODE ShareMode,
            const WAVEFORMATEX* pFormat,
            WAVEFORMATEX** ppClosestMatch
        );

        static HRESULT STDMETHODCALLTYPE GetBufferSize(
            IAudioClient* instance,
            UINT32* pNumBufferFrames
        );

        static HRESULT STDMETHODCALLTYPE GetStreamLatency(
            IAudioClient* instance,
            REFERENCE_TIME* phnsLatency
        );

        static HRESULT STDMETHODCALLTYPE GetCurrentPadding(
            IAudioClient* instance,
            UINT32* pNumPaddingFrames
        );

        static HRESULT STDMETHODCALLTYPE GetDevicePeriod(
            IAudioClient* instance,
            REFERENCE_TIME* phnsDefaultDevicePeriod,
            REFERENCE_TIME* phnsMinimumDevicePeriod
        );

        static HRESULT STDMETHODCALLTYPE Start(
            IAudioClient* instance
        );

        static HRESULT STDMETHODCALLTYPE Stop(
            IAudioClient* instance
        );

        static HRESULT STDMETHODCALLTYPE Reset(
            IAudioClient* instance
        );

        static HRESULT STDMETHODCALLTYPE SetEventHandle(
            IAudioClient* instance,
            HANDLE eventHandle
        );

        static HRESULT STDMETHODCALLTYPE GetService(
            IAudioClient* instance,
            REFIID riid,
            void** ppv
        );
    };
}

#endif
