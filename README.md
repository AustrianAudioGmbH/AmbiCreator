# AmbiCreator

Our open-source Ambisonics plug-in developed by [Thomas](https://github.com/thomasdeppisch) and [AA](https://austrian.audio/).

AmbiCreator allows you to record first-order Ambisonics material by using two OC818 microphones stacked on top of each other.
Installers for as VST3, AAX and AU available soon at [austrian.audio](https://austrian.audio/).

Give it a listen [here](https://austrian.audio/austrian-audio-ambicreator/)!  
You can find technical details in an [AES e-Brief](http://www.aes.org/e-lib/browse.cfm?elib=20827).

<img width="762" alt="AmbiCreator Screenshot" src="docs/AmbiCreator3.png">

## Building AmbiCreator3 from source

Requirements:

- cmake ( >= v3.24.1)
- a C++20 compatible compiler (GCC, clang, MSVC)

To build from source, you need to clone the repository and its submodules

```bash
git clone https://github.com/AustrianAudioGmbH/AmbiCreator.git
cd AmbiCreator
git submodule update --init --recursive
```

After that, create a build directory, configure & compile with

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Acknowledgements:

AmbiCreator 3 makes use of the following projects:

- [JUCE (audio application framework)](https://juce.com)
- [FFTW3 (Windows/Linux FFT backend)](https://fftw.org/)
- [Apple vDSP (MacOS FFT backend)](https://developer.apple.com/accelerate/)
- [Pamplejuce (Audio plugin template)](https://github.com/sudara/Pamplejuce)
- [pluginval (VST Plugin validation tests)](https://github.com/Tracktion/pluginval)
- [Catch2 (Unit testing framework)](https://github.com/catchorg/Catch2)
- [IEM Plugin Suite](https://git.iem.at/audioplugins/IEMPluginSuite)
