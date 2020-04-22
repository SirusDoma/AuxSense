#ifndef AUXSENSE_LOG_HPP
#define AUXSENSE_LOG_HPP

#include <fstream>

namespace AuxSense
{
    class Log
    {
    public:

        static void Write(const char* format, ...) 
        {
            static bool init = true;
            auto file = fopen("audio.log", init ? "w" : "a");
            {
                va_list argptr;
                va_start(argptr, format);
                vfprintf(file, format, argptr);
                va_end(argptr);
            }
            fclose(file);
            init = false;
        }
    };
}

#endif