#include <AuxSense/AuxRenderClient.hpp>
#include <AuxSense/AuxClient.hpp>

#include <iostream>
#include <detours.h>

#include <AuxSense/Log.hpp>

namespace AuxSense
{
    IAudioRenderClient *AuxRenderClient::pRenderClient;
    IAudioClient       *AuxRenderClient::pAudioClient;
    UINT32              AuxRenderClient::requestSize;

    INT64(__fastcall* TrueStream)(
        INT64 a1,
        int   a2
    );

    ULONG (STDMETHODCALLTYPE *TrueRelease)(
        IAudioRenderClient *instance
    );

    HRESULT (STDMETHODCALLTYPE *TrueGetBuffer)(
        IAudioRenderClient *instance,
        UINT32 NumFramesRequested,
        BYTE **ppData
    );

    HRESULT (STDMETHODCALLTYPE *TrueReleaseBuffer)(
        IAudioRenderClient *instance,
        UINT32 NumFramesWritten,
        DWORD dwFlags
    );

    void AuxRenderClient::Hook(IAudioRenderClient* renderClient, IAudioClient *audioClient)
    {
        static bool hooked = false;
        if (hooked)
            return;

        // Fetch original function
        pAudioClient      = audioClient;
        pRenderClient     = renderClient;
        TrueRelease       = pRenderClient->lpVtbl->Release;
        TrueGetBuffer     = pRenderClient->lpVtbl->GetBuffer;
        TrueReleaseBuffer = pRenderClient->lpVtbl->ReleaseBuffer;

        // Hook render client functions
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueRelease, Release);
        DetourAttach(&(PVOID&)TrueGetBuffer, GetBuffer);
        DetourAttach(&(PVOID&)TrueReleaseBuffer, ReleaseBuffer);
        DetourTransactionCommit();

        hooked = true;
    }

    ULONG AuxRenderClient::Release(IAudioRenderClient *instance)
    {
        Log::Write("::Release\n");

        HRESULT hr = TrueRelease(instance);
        if (FAILED(hr))
            Log::Write("FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxRenderClient::GetBuffer(IAudioRenderClient *instance, UINT32 NumFramesRequested, BYTE **ppData)
    {
        // Force the audio keep running by any means, even if the audio became weird
        // This may degrade audio performance, please proven otherwise lol

        UINT32 bufferSize = 0, padding = 0;
        if (pAudioClient->lpVtbl != NULL)
        {
            HRESULT hr = AuxClient::GetBufferSize(pAudioClient, &bufferSize);
            if (FAILED(hr))
                Log::Write("FAIL 0x%08lx (%s)\n", hr, std::system_category().message(hr).c_str());

            hr = AuxClient::GetCurrentPadding(pAudioClient, &padding);
            if (FAILED(hr))
                Log::Write("FAIL 0x%08lx (%s)\n", hr, std::system_category().message(hr).c_str());
        }

        // Readjust the number of frames, however there's no way to tell the game the actual frame
        // So most likely still bugged anyway
        requestSize = bufferSize - padding;
        if (NumFramesRequested > requestSize)
            NumFramesRequested = requestSize;

        HRESULT hr = TrueGetBuffer(instance, NumFramesRequested, ppData);
        if (FAILED(hr))
            Log::Write("FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxRenderClient::ReleaseBuffer(IAudioRenderClient *instance, UINT32 NumFramesWritten, DWORD dwFlags)
    {
        // Align with previous request size
        if (NumFramesWritten > requestSize)
            NumFramesWritten = requestSize;

        HRESULT hr = TrueReleaseBuffer(instance, NumFramesWritten, dwFlags);
        if (FAILED(hr))
            Log::Write("FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }
}