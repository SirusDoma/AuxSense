#ifndef AUXSENSE_UTILITIES_HPP
#define AUXSENSE_UTILITIES_HPP

#include <string>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <AuxSense/Log.hpp>


#define GUID_FORMAT_ID "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX"
#define GUID_FORMAT(guid) guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]

#pragma warning(disable: 26812)
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

namespace AuxSense
{
    void PrintFormat(const WAVEFORMATEX* pFormat)
    {
        if (pFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
        {
            auto format = (const WAVEFORMATEXTENSIBLE*)pFormat;
            Log::Write("... SubFormat         : {" GUID_FORMAT_ID "}\n", GUID_FORMAT(format->SubFormat));
        }
        else
            Log::Write("... wFormatTag        : %d\n", pFormat->wFormatTag);

        Log::Write("... nChannels         : %d\n", pFormat->nChannels);
        Log::Write("... nSamplesPerSec    : %d\n", pFormat->nSamplesPerSec);
        Log::Write("... nAvgBytesPerSec   : %d\n", pFormat->nAvgBytesPerSec);
        Log::Write("... nBlockAlign       : %d\n", pFormat->nBlockAlign);
        Log::Write("... wBitsPerSample    : %d\n", pFormat->wBitsPerSample);
    }

    void PrintFormat(AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX* pFormat)
    {
        Log::Write("... ShareMode         : %s\n",
            ShareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ? "AUDCLNT_SHAREMODE_EXCLUSIVE" : "AUDCLNT_SHAREMODE_SHARED");

        PrintFormat(pFormat);
    }

    void PrintFormat(AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX* pFormat, REFERENCE_TIME hnsBufferDuration, REFERENCE_TIME hnsPeriodicity)
    {
        PrintFormat(ShareMode, pFormat);
        Log::Write("... hnsBufferDuration : %d\n", hnsBufferDuration);
        Log::Write("... hnsPeriodicity    : %d\n", hnsPeriodicity);
    }
}

#endif