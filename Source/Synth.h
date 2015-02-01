

#ifndef SYNTH_H_
#define SYNTH_H_
#include "../JuceLibraryCode/JuceHeader.h"


class Synth: public juce::Synthesiser {
public:
	 void handleMidiEvent (const MidiMessage& m) override;
	 //void renderNextBlock (AudioSampleBuffer & outputAudio,const MidiBuffer & inputMidi, int startSample, int numSamples) override;
};


#endif /* SYNTH_H_ */
