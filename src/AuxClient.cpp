#include <AuxSense/AuxClient.hpp>
#include <AuxSense/AudioSystem.hpp>
#include <AuxSense/AuxRenderClient.hpp>
#include <AuxSense/Utilities.hpp>

#include <initguid.h>
#include <system_error>
#include <iostream>
#include <detours.h>

#include <AuxSense/Log.hpp>

#pragma warning(disable: 26812)
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

DEFINE_GUID(IID_IAudioRenderClient,
    0xf294acfc, 0x3146, 0x4483, 0xa7, 0xbf,
    0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2
);

static const IID IID_IAudioClockAdjustment = {
    0xf6e4c0a0, 0x46d9, 0x4fb8, {0xbe, 0x21, 0x57, 0xa3, 0xef, 0x2b, 0x62, 0x6c}
};

DEFINE_GUID(GUID_KSDATAFORMAT_SUBTYPE_PCM,
    0x00000001, 0x0000, 0x0010,
    0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
);

DEFINE_GUID(GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT,
    0x00000003, 0x0000, 0x0010,
    0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
);

namespace AuxSense
{
    IAudioClient *AuxClient::pAudioClient;

    HRESULT (STDMETHODCALLTYPE *TrueInitialize)(
        IAudioClient * instance,
        AUDCLNT_SHAREMODE ShareMode,
        DWORD StreamFlags,
        REFERENCE_TIME hnsBufferDuration,
        REFERENCE_TIME hnsPeriodicity,
        const WAVEFORMATEX *pFormat,
        LPCGUID AudioSessionGuid
    );

    HRESULT (STDMETHODCALLTYPE *TrueGetMixFormat)(
        IAudioClient *instance,
        WAVEFORMATEX **ppDeviceFormat
    );

    HRESULT(STDMETHODCALLTYPE* TrueIsFormatSupported)(
        IAudioClient* instance,
        AUDCLNT_SHAREMODE ShareMode,
        const WAVEFORMATEX* pFormat,
        WAVEFORMATEX** ppClosestMatch
    );

    HRESULT(STDMETHODCALLTYPE* TrueGetDevicePeriod)(
        IAudioClient* instance,
        REFERENCE_TIME* phnsDefaultDevicePeriod,
        REFERENCE_TIME* phnsMinimumDevicePeriod
    );

    HRESULT(STDMETHODCALLTYPE* TrueGetBufferSize)(
        IAudioClient* instance,
        UINT32* pNumBufferFrames
    );


    HRESULT(STDMETHODCALLTYPE* TrueGetStreamLatency)(
        IAudioClient* instance,
        REFERENCE_TIME* phnsLatency
    );

    HRESULT(STDMETHODCALLTYPE* TrueGetCurrentPadding)(
        IAudioClient* instance,
        UINT32* pNumPaddingFrames
    );

    HRESULT(STDMETHODCALLTYPE* TrueStart)(
        IAudioClient* instance
    );

    HRESULT(STDMETHODCALLTYPE* TrueStop)(
        IAudioClient* instance
    );

    HRESULT(STDMETHODCALLTYPE* TrueReset)(
        IAudioClient* instance
    );

    HRESULT(STDMETHODCALLTYPE* TrueSetEventHandle)(
        IAudioClient* instance,
        HANDLE eventHandle
    );

    HRESULT(STDMETHODCALLTYPE* TrueGetService)(
        IAudioClient* instance,
        REFIID riid,
        void** ppv
    );

    void AuxClient::Hook(IAudioClient *client)
    {
        static bool hooked = false;
        if (hooked)
            return;

        // Fetch original function
        pAudioClient          = client;
        TrueInitialize        = pAudioClient->lpVtbl->Initialize;
        TrueIsFormatSupported = pAudioClient->lpVtbl->IsFormatSupported;
        TrueGetMixFormat      = pAudioClient->lpVtbl->GetMixFormat;
        TrueGetDevicePeriod   = pAudioClient->lpVtbl->GetDevicePeriod;
        TrueGetBufferSize     = pAudioClient->lpVtbl->GetBufferSize;
        TrueGetStreamLatency  = pAudioClient->lpVtbl->GetStreamLatency;
        TrueGetCurrentPadding = pAudioClient->lpVtbl->GetCurrentPadding;
        TrueGetService        = pAudioClient->lpVtbl->GetService;
        TrueStart             = pAudioClient->lpVtbl->Start;
        TrueStop              = pAudioClient->lpVtbl->Stop;
        TrueReset             = pAudioClient->lpVtbl->Reset;
        TrueSetEventHandle    = pAudioClient->lpVtbl->SetEventHandle;

        // Hook client functions
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueInitialize       , Initialize);
        DetourAttach(&(PVOID&)TrueIsFormatSupported, IsFormatSupported);
        DetourAttach(&(PVOID&)TrueGetMixFormat     , GetMixFormat);
        DetourAttach(&(PVOID&)TrueGetDevicePeriod  , GetDevicePeriod);
        DetourAttach(&(PVOID&)TrueGetBufferSize    , GetBufferSize);
        DetourAttach(&(PVOID&)TrueGetStreamLatency , GetStreamLatency);
        DetourAttach(&(PVOID&)TrueGetCurrentPadding, GetCurrentPadding);
        DetourAttach(&(PVOID&)TrueGetService       , GetService);
        DetourAttach(&(PVOID&)TrueStart            , Start);
        DetourAttach(&(PVOID&)TrueStop             , Stop);
        DetourAttach(&(PVOID&)TrueReset            , Reset);
        DetourAttach(&(PVOID&)TrueGetStreamLatency , GetStreamLatency);
        DetourTransactionCommit();

        hooked = true;
    }

    HRESULT AuxClient::Initialize(
        IAudioClient *instance,
        AUDCLNT_SHAREMODE ShareMode,
        DWORD StreamFlags,
        REFERENCE_TIME hnsBufferDuration,
        REFERENCE_TIME hnsPeriodicity,
        const WAVEFORMATEX *pFormat,
        LPCGUID AudioSessionGuid
    )
    {
        Log::Write("::Initialize\n");
        if (ShareMode == AUDCLNT_SHAREMODE_EXCLUSIVE)
            Log::Write("Exclusive mode detected on initial init. Shared mode may not producing proper audio output.\n");

        ShareMode = AUDCLNT_SHAREMODE_SHARED;
        PrintFormat(ShareMode, pFormat, hnsBufferDuration, hnsPeriodicity);

        // Query format from the system
        WAVEFORMATEX* pClosest, * pMixFormat;
        HRESULT hr = GetMixFormat(instance, &pMixFormat);
        if (FAILED(hr))
        {
            Log::Write("=> FAILed to retrive mix pMixFormat\n");
            return hr;
        }

        /** -- Mix Format ----
          * I've probed various parameters with 44100hz sample rate many times and it always fail
          * User probably need to change their setting by themselves
          * 
          * https://docs.microsoft.com/en-us/windows/win32/coreaudio/device-formats#specifying-the-device-format
          */
        hr = IsFormatSupported(instance, ShareMode, pMixFormat, &pClosest);
        if (hr == S_FALSE)
        {
            Log::Write("=> FAIL: Mix Format is not supported\n\n");
            return hr;
        }

        auto target    = const_cast<WAVEFORMATEX*>(pFormat);
        auto pFormatEx = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pMixFormat);
        
        if (pMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            IsEqualGUID(GUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pFormatEx->SubFormat))
        {
            pFormatEx->Format.wBitsPerSample       = 16;
            pFormatEx->Format.nBlockAlign          = pFormatEx->Format.nChannels * (pFormatEx->Format.wBitsPerSample / 8);
            pFormatEx->Format.nAvgBytesPerSec      = pFormatEx->Format.nSamplesPerSec * pFormatEx->Format.nBlockAlign;
            pFormatEx->Samples.wValidBitsPerSample = 16;
            pFormatEx->SubFormat                   = GUID_KSDATAFORMAT_SUBTYPE_PCM;

            *target = *reinterpret_cast<WAVEFORMATEX*>(pFormatEx);
        }
        else  if (pMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            pMixFormat->wFormatTag      = WAVE_FORMAT_PCM;
            pMixFormat->nBlockAlign     = pMixFormat->nChannels * (pMixFormat->wBitsPerSample / 8);
            pMixFormat->nAvgBytesPerSec = pMixFormat->nSamplesPerSec * pMixFormat->nBlockAlign;
            pMixFormat->wBitsPerSample  = 16;

            *target = *pMixFormat;
        }
        else if (pClosest != NULL)
            *target = *pClosest;
        else
            *target = *pMixFormat;

        /** Device Period
         * Need to be readjusted for shared mode
         * The game may crash or ended up with weird latency (it will still probably run with weird latency for non 44100hz tho)
         *
         * https://docs.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize
         */
        if (ShareMode == AUDCLNT_SHAREMODE_SHARED)
        {
            REFERENCE_TIME devicePeriod = 0, minimumPeriod = 0;
            hr = GetDevicePeriod(instance, &devicePeriod, &minimumPeriod);
            if (SUCCEEDED(hr))
            {
                hnsPeriodicity = 0;
                if (hnsBufferDuration != devicePeriod)
                    hnsBufferDuration = devicePeriod;
                else
                    Log::Write("-> Device period match\n");
            }
            else
                Log::Write("-> Failed to readjust device period (%s)\n", std::system_category().message(hr).c_str());

            // Also set stream flags
            StreamFlags = StreamFlags | AUDCLNT_STREAMFLAGS_RATEADJUST;
            //StreamFlags = StreamFlags | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM;
        }

        Log::Write("::OK!\n\n");
        Log::Write("::TrueInitialize\n");
        PrintFormat(ShareMode, pFormat, hnsBufferDuration, hnsPeriodicity);
        hr = TrueInitialize(
            instance,
            ShareMode,
            StreamFlags,
            hnsBufferDuration,
            hnsPeriodicity,
            pFormat,
            AudioSessionGuid
        );

        // Let's hope the audio isn't going wild
        if (SUCCEEDED(hr))
        {
            // Readjust sample rate
            IAudioClockAdjustment* clock;
            hr = GetService(instance, IID_IAudioClockAdjustment, (void**)&clock);
            if (SUCCEEDED(hr))
            {
                // Resample it to 44100hz
                hr = clock->lpVtbl->SetSampleRate(clock, 44100.0f);
                if (FAILED(hr))
                    Log::Write("-> Failed to set clock sample rate (%s)\n", std::system_category().message(hr).c_str());
            }
            else
                Log::Write("-> Failed to readjust sample rate (%s)\n", std::system_category().message(hr).c_str());

            Log::Write("=> SUCCESS!\n\n");
        }
        else
            Log::Write("=> FAIL: %x (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::GetMixFormat(IAudioClient *instance, WAVEFORMATEX **ppDeviceFormat)
    {
        Log::Write("::GetMixFormat\n");
        return TrueGetMixFormat(
            instance,
            ppDeviceFormat
        );
    }
    
    HRESULT AuxClient::IsFormatSupported(IAudioClient* instance, AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX* pFormat, WAVEFORMATEX** ppClosestMatch)
    {
        Log::Write("::IsFormatSupported\n");
        PrintFormat(ShareMode, pFormat);

        auto pClosest = new WAVEFORMATEX{};
        HRESULT hr = TrueIsFormatSupported(
            instance,
            ShareMode,
            pFormat,
            ppClosestMatch == NULL ? &pClosest : ppClosestMatch
        );

        if (SUCCEEDED(hr))
            Log::Write("=> OK!\n\n");
        else
            Log::Write("=> INVALID: 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::GetDevicePeriod(
        IAudioClient* instance,
        REFERENCE_TIME* phnsDefaultDevicePeriod,
        REFERENCE_TIME* phnsMinimumDevicePeriod
    )
    {
        HRESULT hr = TrueGetDevicePeriod(instance, phnsDefaultDevicePeriod, phnsMinimumDevicePeriod);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::GetBufferSize(IAudioClient *instance, UINT32 *pNumBufferFrames)
    {
        HRESULT hr = TrueGetBufferSize(instance, pNumBufferFrames);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::GetStreamLatency(IAudioClient *instance, REFERENCE_TIME *phnsLatency)
    {
        Log::Write("::GetStreamLatency\n");
        HRESULT hr = TrueGetStreamLatency(instance, phnsLatency);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::GetCurrentPadding(IAudioClient *instance, UINT32 *pNumPaddingFrames)
    {
        HRESULT hr = TrueGetCurrentPadding(instance, pNumPaddingFrames);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::Start(IAudioClient *instance)
    {
        Log::Write("::Start\n");
        HRESULT hr = TrueStart(instance);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::Stop(IAudioClient *instance)
    {
        Log::Write("::Stop\n");
        HRESULT hr = TrueStop(instance);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::Reset(IAudioClient *instance)
    {
        Log::Write("::Reset\n");
        HRESULT hr = TrueReset(instance);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::SetEventHandle(IAudioClient *instance, HANDLE eventHandle)
    {
        Log::Write("::SetEventHandle\n");
        HRESULT hr = TrueSetEventHandle(instance, eventHandle);
        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }

    HRESULT AuxClient::GetService(IAudioClient *instance, REFIID riid, void **ppv)
    {
        Log::Write("::GetService\n");
        HRESULT hr = TrueGetService(instance, riid, ppv);

        if (SUCCEEDED(hr) && riid == IID_IAudioRenderClient)
            AuxRenderClient::Hook((IAudioRenderClient*)*ppv, instance);

        if (FAILED(hr))
            Log::Write("=> FAIL 0x%08lx (%s)\n\n", hr, std::system_category().message(hr).c_str());

        return hr;
    }
}

