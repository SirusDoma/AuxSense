#include <AuxSense/AudioSystem.hpp>

using namespace AuxSense;

BOOL APIENTRY DllMain(HMODULE hMod, DWORD cause, LPVOID lpReserved)
{
    if (!AudioSystem::Ready())
        return TRUE;

    switch (cause)
    {
        case DLL_PROCESS_ATTACH:
            AudioSystem::Initialize(hMod);
            break;
        case DLL_PROCESS_DETACH:
            break;
        default:
            break;
    }

    return TRUE;
}


