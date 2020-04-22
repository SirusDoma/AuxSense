# AuxSense #

- **Author**: CXO2
- **Email**: com@cxo2.me
- **Version**: 0.9.8

WASAPI Audio Hook to force the client run under Shared Mode. ([Download](https://github.com/SirusDoma/AuxSense/releases))  

It make a use of `GetMixFormat` to retrieve proper `WAVEFORMATEX` for `IAudioClient` to run properly with Shared Mode.  
Note that this hook will not only adapt audio client to shared mode, it will also adjust audio stream sample rate to 44100hz.

## Usage ##

1. Build the project or download precompiled binary
2. Inject DLL into the target game or application
3. Profit

## License ##

This is an open-sourced library licensed under the [MIT License](http://github.com/SirusDoma/AuxSense/blob/master/LICENSE)
