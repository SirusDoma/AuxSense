#ifndef AUXSENSE_AUXRENDERCLIENT_HPP
#define AUXSENSE_AUXRENDERCLIENT_HPP

#include <audioclient.h>

namespace AuxSense
{
    class AuxRenderClient
    {
    private:
        static IAudioRenderClient *pRenderClient;
        static IAudioClient       *pAudioClient;

        static UINT32 requestSize;

    public:
        static void Hook(IAudioRenderClient *renderClient, IAudioClient *audioClient);

        static ULONG STDMETHODCALLTYPE Release(
            IAudioRenderClient *instance
        );

        static HRESULT STDMETHODCALLTYPE GetBuffer(
            IAudioRenderClient *instance,
            UINT32 NumFramesRequested,
            BYTE **ppData
        );

        static HRESULT STDMETHODCALLTYPE ReleaseBuffer(
            IAudioRenderClient *instance,
            UINT32 NumFramesWritten,
            DWORD dwFlags
        );
    };
}

#endif
