# AmbiCreator
Our open-source Ambisonics plug-in developed by [Thomas](https://github.com/thomasdeppisch) and [AA](https://austrian.audio/).

AmbiCreator allows you to record first-order Ambisonics material by using two OC818 microphones stacked on top of each other.
Installers for as VST3, AAX and AU available soon at [austrian.audio](https://austrian.audio/).

Give it a listen [here](https://austrian.audio/austrian-audio-ambicreator/)!  
You can find technical details in an AES e-Brief.

## Building AmbiCreator
AmbiCreator is based on [JUCE](https://juce.com/). To build AmbiCreator, get a recent version of JUCE and open AmbiCreator.jucer in Projucer. Select an exporter of your choice (e.g. Visual Studio or XCode) to create and open a project file in your IDE.

## Requirements
* For building AAX plugins you need to add the [AAX SDK](http://developer.avid.com/) location to your Projucer paths.

## Related repositories
Parts of the code are based on the [IEM Plugin Suite](https://git.iem.at/audioplugins/IEMPluginSuite) - check it out, it's awesome!
