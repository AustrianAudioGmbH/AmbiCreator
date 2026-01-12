#include "PluginProcessor.h"
#include "../resources/irs.h"
#include "PluginEditor.h"

/* We use versionHint of ParameterID from now on - rigorously! */
#define PD_PARAMETER_V1 1

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;

    using API = AudioParameterInt;
    using APF = AudioParameterFloat;
    using APB = AudioParameterBool;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto intAttributes = AudioParameterIntAttributes().withStringFromValueFunction (
        [] (int value, int)
        { return (value == eChannelOrder::ACN) ? "AmbiX (WYZX)" : "FuMa (WXYZ)"; });

    layout.add (std::make_unique<API> (ParameterID { "channelOrder", PD_PARAMETER_V1 },
                                       "channel order",
                                       eChannelOrder::ACN,
                                       eChannelOrder::FUMA,
                                       eChannelOrder::ACN,
                                       intAttributes));

    auto floatAttributes =
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) { return String (value, 1); })
            .withLabel ("dB")
            .withCategory (AudioParameterFloatAttributes::Category::genericParameter);

    layout.add (std::make_unique<APF> (ParameterID { "outGainDb", PD_PARAMETER_V1 },
                                       "output gain",
                                       NormalisableRange<float> (-40.0f, 10.0f, 0.1f),
                                       0.0f,
                                       floatAttributes));

    floatAttributes = floatAttributes.withStringFromValueFunction (
        [] (float value, int)
        {
            return (value > AmbiCreatorAudioProcessor::MIN_Z_GAIN_DB
                                + AmbiCreatorAudioProcessor::GAIN_TO_ZERO_THRESH_DB)
                       ? String (value, 1)
                       : "-inf";
        });

    layout.add (std::make_unique<APF> (
        ParameterID { "zGainDb", PD_PARAMETER_V1 },
        "z gain",
        NormalisableRange<float> (AmbiCreatorAudioProcessor::MIN_Z_GAIN_DB, 10.0f, 0.1f),
        0.0f,
        floatAttributes));

    floatAttributes =
        floatAttributes.withLabel (std::string ("°"))
            .withStringFromValueFunction ([] (float value, int) { return String (value, 1); });

    layout.add (std::make_unique<APF> (ParameterID { "horRotation", PD_PARAMETER_V1 },
                                       "horizontal rotation",
                                       NormalisableRange<float> (-180.0f, 180.0f, 1.0f),
                                       0.0f,
                                       floatAttributes));

    const auto boolAttributes = AudioParameterBoolAttributes().withStringFromValueFunction (
        [] (bool value, [[maybe_unused]] int maximumStringLength)
        { return (value) ? "on" : "off"; });

    layout.add (std::make_unique<APB> (ParameterID { "legacyMode", PD_PARAMETER_V1 },
                                       "Legacy Mode",
                                       true,
                                       boolAttributes));

    return layout;
}

//==============================================================================
AmbiCreatorAudioProcessor::AmbiCreatorAudioProcessor() :
    AudioProcessor (BusesProperties()
                        .withInput ("Input", juce::AudioChannelSet::ambisonic (1), true)
                        .withOutput ("Output", juce::AudioChannelSet::ambisonic (1), true)),
    params (*this, nullptr, "AmbiCreator", createParameterLayout()),
    firLatencySec ((static_cast<float> (FIR_LEN) / 2 - 1) / FIR_SAMPLE_RATE),
    currentSampleRate (48000),
    isBypassed (false),
    editorWidth (EDITOR_DEFAULT_WIDTH),
    editorHeight (EDITOR_DEFAULT_HEIGHT),
    layerA (nodeA),
    layerB (nodeB),
    allValueTreeStates (allStates)
{
    using namespace juce;

    channelOrder = static_cast<int> (params.getParameterAsValue ("channelOrder").getValue());
    outGainLin = Decibels::decibelsToGain (
        static_cast<float> (params.getParameterAsValue ("outGainDb").getValue()));
    zGainLin = Decibels::decibelsToGain (
        static_cast<float> (params.getParameterAsValue ("zGainDb").getValue()));
    horRotationDeg = static_cast<float> (params.getParameterAsValue ("horRotation").getValue());

    params.addParameterListener ("channelOrder", this);
    params.addParameterListener ("outGainDb", this);
    params.addParameterListener ("zGainDb", this);
    params.addParameterListener ("horRotation", this);
    params.addParameterListener ("legacyMode", this);

    legacyModePtr = params.getRawParameterValue ("legacyMode");
}

AmbiCreatorAudioProcessor::~AmbiCreatorAudioProcessor()
{
}

//==============================================================================
const juce::String AmbiCreatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int AmbiCreatorAudioProcessor::getChannelOrder()
{
    return channelOrder;
}

bool AmbiCreatorAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AmbiCreatorAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AmbiCreatorAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AmbiCreatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmbiCreatorAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AmbiCreatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmbiCreatorAudioProcessor::setCurrentProgram (int)
{
}

const juce::String AmbiCreatorAudioProcessor::getProgramName (int)
{
    return {};
}

void AmbiCreatorAudioProcessor::changeProgramName (int, const juce::String&)
{
}

//==============================================================================
void AmbiCreatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    using namespace juce;

    currentSampleRate = sampleRate;
    firLatencySec = (static_cast<float> (FIR_LEN) / 2 - 1)
                    / static_cast<float> (sampleRate); // Update latency based on actual sample rate

    if (getTotalNumInputChannels() != 4 || getTotalNumOutputChannels() != 4)
    {
        wrongBusConfiguration.set (true);
    }
    else
    {
        wrongBusConfiguration.set (false);
    }

    foaChannelBuffer.setSize (4, samplesPerBlock);
    foaChannelBuffer.clear();

    rotatorBuffer.setSize (2, samplesPerBlock);
    rotatorBuffer.clear();

    legacyModeReorderBuffer.setSize (4, samplesPerBlock);
    legacyModeReorderBuffer.clear();

    previousOutGainLin = outGainLin;
    previousZGainLin = zGainLin;
    previousCosPhi = std::cos (degreesToRadians (horRotationDeg));
    previousSinPhi = std::sin (degreesToRadians (horRotationDeg));

    // low frequency compensation IIR for differential z signal
    //dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 1 };
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = 1;
    spec.maximumBlockSize = static_cast<uint32> (samplesPerBlock);

    iirLowShelf.prepare (spec);
    iirLowShelf.reset();
    setLowShelfCoefficients (sampleRate);

    AudioBuffer<float> zFilterIR (1, FIR_LEN);
    AudioBuffer<float> xFilterIR (1, FIR_LEN);
    AudioBuffer<float> yFilterIR (1, FIR_LEN);
    AudioBuffer<float> omniFilterIR (1, FIR_LEN);

    zFilterIR.copyFrom (0, 0, DIFF_Z_EIGHT_EQ_COEFFS, FIR_LEN);
    xFilterIR.copyFrom (0, 0, COINC_EIGHT_EQ_COEFFS, FIR_LEN);
    yFilterIR.copyFrom (0, 0, COINC_EIGHT_EQ_COEFFS, FIR_LEN);
    omniFilterIR.copyFrom (0, 0, COINC_OMNI_EQ_COEFFS, FIR_LEN);

    // prepare fir filters
    zFilterConv.prepare (spec); // must be called before loading an ir
    zFilterConv.loadImpulseResponse (std::move (zFilterIR),
                                     FIR_SAMPLE_RATE,
                                     dsp::Convolution::Stereo::no,
                                     dsp::Convolution::Trim::no,
                                     dsp::Convolution::Normalise::no);
    zFilterConv.reset();
    coincXEightFilterConv.prepare (spec); // must be called before loading an ir
    coincXEightFilterConv.loadImpulseResponse (std::move (xFilterIR),
                                               FIR_SAMPLE_RATE,
                                               dsp::Convolution::Stereo::no,
                                               dsp::Convolution::Trim::no,
                                               dsp::Convolution::Normalise::no);
    coincXEightFilterConv.reset();

    coincYEightFilterConv.prepare (spec); // must be called before loading an ir
    coincYEightFilterConv.loadImpulseResponse (std::move (yFilterIR),
                                               FIR_SAMPLE_RATE,
                                               dsp::Convolution::Stereo::no,
                                               dsp::Convolution::Trim::no,
                                               dsp::Convolution::Normalise::no);
    coincYEightFilterConv.reset();

    coincOmniFilterConv.prepare (spec); // must be called before loading an ir
    coincOmniFilterConv.loadImpulseResponse (std::move (omniFilterIR),
                                             FIR_SAMPLE_RATE,
                                             dsp::Convolution::Stereo::no,
                                             dsp::Convolution::Trim::no,
                                             dsp::Convolution::Normalise::no);
    coincOmniFilterConv.reset();

    // set delay compensation
    updateLatency();
}

void AmbiCreatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AmbiCreatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannels() != 4 || layouts.getMainOutputChannels() != 4)
    {
        return false;
    }

    return true;
}

void AmbiCreatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiBuffer)
{
    using namespace juce;

    ignoreUnused (midiBuffer);

    const auto numSamples = buffer.getNumSamples();

    if (numSamples == 0 || wrongBusConfiguration.get() || buffer.getNumChannels() != 4
        || getTotalNumInputChannels() != 4 || getTotalNumOutputChannels() != 4)
    {
        buffer.clear();
        for (int i = 0; i < buffer.getNumChannels(); ++i)
        {
            outRms[i].set (0.0f);
        }
        return;
    }

    // In legacy mode, input channels are expected in order: Left (0), Right (1), Front (2), Back (3)
    // Reordered to: Front (0), Back (1), Left (2), Right (3)
    if (isNormalLRFBMode())
    {
        legacyModeReorderBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples); // L
        legacyModeReorderBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples); // R
        legacyModeReorderBuffer.copyFrom (2, 0, buffer, 2, 0, numSamples); // F
        legacyModeReorderBuffer.copyFrom (3, 0, buffer, 3, 0, numSamples); // B

        buffer.clear();
        buffer.copyFrom (0, 0, legacyModeReorderBuffer, 2, 0, numSamples); // F
        buffer.copyFrom (1, 0, legacyModeReorderBuffer, 3, 0, numSamples); // B
        buffer.copyFrom (2, 0, legacyModeReorderBuffer, 0, 0, numSamples); // L
        buffer.copyFrom (3, 0, legacyModeReorderBuffer, 1, 0, numSamples); // R

        for (int i = 0; i < buffer.getNumChannels(); ++i)
        {
            inRms[i].set (legacyModeReorderBuffer.getRMSLevel (i, 0, numSamples));
            channelActive[i].set (inRms[i].get() >= 0.000000001f);
        }
    }
    else
    {
        for (int i = 0; i < buffer.getNumChannels(); ++i)
        {
            inRms[i].set (buffer.getRMSLevel (i, 0, numSamples));
            channelActive[i].set (inRms[i].get() >= 0.000000001f);
        }
    }

    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    AudioPlayHead* playHead = getPlayHead();
    AudioPlayHead::PositionInfo playHeadPosition;

    if (playHead != nullptr)
    {
        if (auto position = playHead->getPosition())
        {
            playHeadPosition = *position;
        }
    }
    isPlaying.set (playHeadPosition.getIsPlaying());

    if (isBypassed)
    {
        isBypassed = false;
        updateLatency();
    }

    if (wrongBusConfiguration.get())
        return;

    jassert (buffer.getNumChannels() == 4 && totalNumOutputChannels == 4
             && totalNumInputChannels == 4);

    jassert (numSamples != 0);
    if (numSamples == 0)
        return;

    if (isBypassed)
    {
        isBypassed = false;
        updateLatency();
    }

    const float* readPointerFront = buffer.getReadPointer (0);
    const float* readPointerBack = buffer.getReadPointer (1);
    const float* readPointerLeft = buffer.getReadPointer (2);
    const float* readPointerRight = buffer.getReadPointer (3);

    // internally everything is in ACN order
    float* writePointerW = foaChannelBuffer.getWritePointer (eChannelOrderACN::W);
    float* writePointerX = foaChannelBuffer.getWritePointer (eChannelOrderACN::X);
    float* writePointerY = foaChannelBuffer.getWritePointer (eChannelOrderACN::Y);
    float* writePointerZ = foaChannelBuffer.getWritePointer (eChannelOrderACN::Z);

    // W: take omni from mic 1
    FloatVectorOperations::copy (writePointerW, readPointerLeft, numSamples);
    FloatVectorOperations::add (writePointerW, readPointerRight, numSamples);

    // X
    FloatVectorOperations::copy (writePointerX, readPointerFront, numSamples);
    FloatVectorOperations::subtract (writePointerX, readPointerBack, numSamples);

    // Y
    FloatVectorOperations::copy (writePointerY, readPointerLeft, numSamples);
    FloatVectorOperations::subtract (writePointerY, readPointerRight, numSamples);

    // Z: differential from both omnis, second mic is upper mic (positive z)
    FloatVectorOperations::copy (writePointerZ, readPointerLeft, numSamples);
    FloatVectorOperations::add (writePointerZ, readPointerRight, numSamples);
    FloatVectorOperations::subtract (writePointerZ, readPointerFront, numSamples);
    FloatVectorOperations::subtract (writePointerZ, readPointerBack, numSamples);

    // equalization of differential z channel
    dsp::AudioBlock<float> zEqualizationBlock (&writePointerZ, 1, static_cast<size_t> (numSamples));
    dsp::ProcessContextReplacing<float> zEqualizationContext (zEqualizationBlock);
    iirLowShelf.process (zEqualizationContext);
    zFilterConv.process (zEqualizationContext);

    // equalization of coincident channels
    dsp::AudioBlock<float> wEqualizationBlock (&writePointerW, 1, static_cast<size_t> (numSamples));
    dsp::ProcessContextReplacing<float> wEqualizationContext (wEqualizationBlock);
    coincOmniFilterConv.process (wEqualizationContext);

    dsp::AudioBlock<float> xEqualizationBlock (&writePointerX, 1, static_cast<size_t> (numSamples));
    dsp::ProcessContextReplacing<float> xEqualizationContext (xEqualizationBlock);
    coincXEightFilterConv.process (xEqualizationContext);

    dsp::AudioBlock<float> yEqualizationBlock (&writePointerY, 1, static_cast<size_t> (numSamples));
    dsp::ProcessContextReplacing<float> yEqualizationContext (yEqualizationBlock);
    coincYEightFilterConv.process (yEqualizationContext);

    // apply z gain
    foaChannelBuffer.applyGainRamp (eChannelOrderACN::Z, 0, numSamples, previousZGainLin, zGainLin);
    previousZGainLin = zGainLin;

    // apply output gain to all channels
    foaChannelBuffer.applyGainRamp (0, numSamples, previousOutGainLin, outGainLin);
    previousOutGainLin = outGainLin;

    // rotate
    ambiRotateAroundZ (&foaChannelBuffer);

    // write to output
    buffer.clear();

    if (buffer.getNumChannels() == 4 && totalNumOutputChannels == 4 && totalNumInputChannels == 4)
    {
        if (channelOrder == eChannelOrder::FUMA)
        {
            // reorder and normalize for FUMA (N3D)
            FloatVectorOperations::multiply (writePointerX,
                                             static_cast<float> (SQRT_THREE),
                                             numSamples);
            FloatVectorOperations::multiply (writePointerY,
                                             static_cast<float> (SQRT_THREE),
                                             numSamples);
            FloatVectorOperations::multiply (writePointerZ,
                                             static_cast<float> (SQRT_THREE),
                                             numSamples);

            buffer.copyFrom (0, 0, foaChannelBuffer, eChannelOrderACN::W, 0, numSamples);
            buffer.copyFrom (1, 0, foaChannelBuffer, eChannelOrderACN::X, 0, numSamples);
            buffer.copyFrom (2, 0, foaChannelBuffer, eChannelOrderACN::Y, 0, numSamples);
            buffer.copyFrom (3, 0, foaChannelBuffer, eChannelOrderACN::Z, 0, numSamples);
        }
        else
        {
            // only reorder for AmbiX (ACN + SN3D) -> SN3D in FOA applies same weights to all components
            buffer.copyFrom (0, 0, foaChannelBuffer, eChannelOrderACN::W, 0, numSamples);
            buffer.copyFrom (1, 0, foaChannelBuffer, eChannelOrderACN::Y, 0, numSamples);
            buffer.copyFrom (2, 0, foaChannelBuffer, eChannelOrderACN::Z, 0, numSamples);
            buffer.copyFrom (3, 0, foaChannelBuffer, eChannelOrderACN::X, 0, numSamples);
        }
    }

    for (int i = 0; i < buffer.getNumChannels(); ++i)
        outRms[i] = buffer.getRMSLevel (i, 0, numSamples);
}

void AmbiCreatorAudioProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer,
                                                      juce::MidiBuffer& midiBuffer)
{
    using namespace juce;

    ignoreUnused (midiBuffer);

    if (! isBypassed)
    {
        isBypassed = true;
        updateLatency();
    }

    jassert (getLatencySamples() == 0);

    for (int ch = getMainBusNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

//==============================================================================
bool AmbiCreatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AmbiCreatorAudioProcessor::createEditor()
{
    return new AmbiCreatorAudioProcessorEditor (*this, params);
}

void AmbiCreatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    using namespace juce;

    ScopedLock lock (stateLock);
    if (abLayerState == eCurrentActiveLayer::layerA)
    {
        layerA = params.copyState();
    }
    else
    {
        layerB = params.copyState();
    }

    ValueTree vtsState = params.copyState();
    ValueTree AState = layerA.createCopy();
    ValueTree BState = layerB.createCopy();

    allValueTreeStates.removeAllChildren (nullptr);
    allValueTreeStates.addChild (vtsState, 0, nullptr);
    allValueTreeStates.addChild (AState, 1, nullptr);
    allValueTreeStates.addChild (BState, 2, nullptr);
    allValueTreeStates.setProperty ("activeLayer", abLayerState, nullptr); // Save active layer

    std::unique_ptr<XmlElement> xml (allValueTreeStates.createXml());
    copyXmlToBinary (*xml, destData);
}

void AmbiCreatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    using namespace juce;

    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        ScopedLock lock (stateLock);
        if (xmlState->hasTagName (allValueTreeStates.getType()))
        {
            allValueTreeStates = ValueTree::fromXml (*xmlState);
            params.replaceState (allValueTreeStates.getChild (0));
            layerA = allValueTreeStates.getChild (1).createCopy();
            layerB = allValueTreeStates.getChild (2).createCopy();
            abLayerState =
                allValueTreeStates.getProperty ("activeLayer", eCurrentActiveLayer::layerA);
        }
        else if (xmlState->hasTagName (params.state.getType()))
        {
            params.replaceState (ValueTree::fromXml (*xmlState));
            layerA = params.copyState();
            layerB = params.copyState();
            abLayerState = eCurrentActiveLayer::layerA;
        }

        MessageManager::callAsync (
            [this]
            {
                static const StringArray paramIDs = { "outGainDb",
                                                      "horRotation",
                                                      "zGainDb",
                                                      "channelOrder",
                                                      "legacyMode" };
                for (const auto& id : paramIDs)
                {
                    if (auto* param = params.getParameter (id))
                    {
                        param->setValueNotifyingHost (param->getValue());
                    }
                }
            });
    }
}

void AmbiCreatorAudioProcessor::changeAbLayerState (int desiredLayer)
{
    using namespace juce;

    ScopedLock lock (stateLock); // Protect ValueTree operations
    DBG ("Switching to layer: " << (desiredLayer == eCurrentActiveLayer::layerA ? "A" : "B"));

    if (desiredLayer == eCurrentActiveLayer::layerA)
    {
        layerB = params.copyState();
        params.replaceState (layerA.createCopy());
        abLayerState = eCurrentActiveLayer::layerA;
        DBG ("Layer A loaded: legacyMode=" << (isNormalLRFBMode() ? "ON" : "OFF"));
    }
    else
    {
        layerA = params.copyState();
        params.replaceState (layerB.createCopy());
        abLayerState = eCurrentActiveLayer::layerB;
        DBG ("Layer B loaded: legacyMode=" << (isNormalLRFBMode() ? "ON" : "OFF"));
    }

    MessageManager::callAsync (
        [this]
        {
            static const StringArray paramIDs = { "outGainDb",
                                                  "horRotation",
                                                  "zGainDb",
                                                  "channelOrder",
                                                  "legacyMode" };
            for (const auto& id : paramIDs)
            {
                if (auto* param = params.getParameter (id))
                {
                    param->setValueNotifyingHost (param->getValue());
                }
            }
        });
}

//==============================================================================
void AmbiCreatorAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    using namespace juce;

    if (parameterID == "channelOrder")
    {
        channelOrder = (static_cast<int> (newValue) == eChannelOrder::FUMA) ? eChannelOrder::FUMA
                                                                            : eChannelOrder::ACN;
        DBG (
            "ChannelOrder changed to: " << (channelOrder == eChannelOrder::ACN ? "AmbiX" : "FUMA"));
    }
    else if (parameterID == "outGainDb")
    {
        outGainLin = Decibels::decibelsToGain (newValue);
        DBG ("OutGainDb changed to: " << newValue);
    }
    else if (parameterID == "zGainDb")
    {
        if (newValue < MIN_Z_GAIN_DB + GAIN_TO_ZERO_THRESH_DB)
            zGainLin = 0.0f;
        else
            zGainLin = Decibels::decibelsToGain (newValue);
        DBG ("ZGainDb changed to: " << newValue);
    }
    else if (parameterID == "horRotation")
    {
        horRotationDeg = newValue;
        DBG ("HorRotation changed to: " << newValue);
    }
    else if (parameterID == "legacyMode")
    {
        legacyModePtr->store (newValue);
        DBG ("LegacyMode changed to: "
             << (newValue > 0.5f ? "ON" : "OFF")
             << ", Layer=" << (abLayerState == eCurrentActiveLayer::layerA ? "A" : "B"));
    }
}

//========================= CUSTOM METHODS =====================================
void AmbiCreatorAudioProcessor::setLowShelfCoefficients (double sampleRate)
{
    using namespace juce;
    const double wc2 = 8418.4865639164;
    const double wc3 = 62.831853071795862;
    const double T = 1 / sampleRate;

    float b0 = static_cast<float> (T / 4 * (wc2 - wc3) + 0.5f);
    float b1 = static_cast<float> (-0.5f * std::exp (-wc3 * T) * (1 - T / 2 * (wc2 - wc3)));
    float a0 = 1.0f;
    float a1 = static_cast<float> (-std::exp (-wc3 * T));

    *iirLowShelf.coefficients = dsp::IIR::Coefficients<float> (b0, b1, a0, a1);
}

void AmbiCreatorAudioProcessor::ambiRotateAroundZ (juce::AudioBuffer<float>* ambiBuffer)
{
    using namespace juce;

    auto numSamples = ambiBuffer->getNumSamples();
    jassert (numSamples == rotatorBuffer.getNumSamples());

    float cosPhi = std::cos (degreesToRadians (horRotationDeg));
    float sinPhi = std::sin (degreesToRadians (horRotationDeg));

    // write result to rotator buffer, it holds Y in channel 0 and X in channel 1
    rotatorBuffer.clear();
    rotatorBuffer.addFromWithRamp (0,
                                   0,
                                   ambiBuffer->getReadPointer (eChannelOrderACN::Y),
                                   numSamples,
                                   previousCosPhi,
                                   cosPhi);
    rotatorBuffer.addFromWithRamp (0,
                                   0,
                                   ambiBuffer->getReadPointer (eChannelOrderACN::X),
                                   numSamples,
                                   previousSinPhi,
                                   sinPhi);
    rotatorBuffer.addFromWithRamp (1,
                                   0,
                                   ambiBuffer->getReadPointer (eChannelOrderACN::Y),
                                   numSamples,
                                   -previousSinPhi,
                                   -sinPhi);
    rotatorBuffer.addFromWithRamp (1,
                                   0,
                                   ambiBuffer->getReadPointer (eChannelOrderACN::X),
                                   numSamples,
                                   previousCosPhi,
                                   cosPhi);

    ambiBuffer->copyFrom (eChannelOrderACN::Y, 0, rotatorBuffer, 0, 0, numSamples);
    ambiBuffer->copyFrom (eChannelOrderACN::X, 0, rotatorBuffer, 1, 0, numSamples);

    previousCosPhi = cosPhi;
    previousSinPhi = sinPhi;
}

void AmbiCreatorAudioProcessor::updateLatency()
{
    if (isBypassed)
        setLatencySamples (0);
    else
        setLatencySamples (static_cast<int> (firLatencySec * currentSampleRate));
}

void AmbiCreatorAudioProcessor::setAbLayer (int desiredLayer)
{
    if (desiredLayer != abLayerState)
    {
        changeAbLayerState (desiredLayer);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmbiCreatorAudioProcessor();
}
