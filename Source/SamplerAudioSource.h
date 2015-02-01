#ifndef SAMPLER_AUDIO_SOURCE_H_INCLUDED
#define SAMPLER_AUDIO_SOURCE_H_INCLUDED

#include "Synth.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/AppConfig.h"

using namespace std;


class SamplerAudioSource : public AudioSource {

public:
    SamplerAudioSource()
	{
        AddVoices(&synth1, AppConfig_NumberOfVoices);
        AddVoices(&synth2, AppConfig_NumberOfVoices);
        AddVoices(&synth3, AppConfig_NumberOfVoices);

        LoadSounds(&synth1, AppConfig_FileFormat, AppConfig_FilePath1);
        LoadSounds(&synth2, AppConfig_FileFormat, AppConfig_FilePath2);
        LoadSounds(&synth3, AppConfig_FileFormat, AppConfig_FilePath3);
    }
    void AddVoices(Synth* ptrSynth, int numVoices)
    {
    	for (int i = 0; i <= numVoices; i++)
    	{ptrSynth->addVoice (new SamplerVoice());}
    }
    void LoadSounds(Synth* ptrSynth, String FileFormat, String FilePath)
    {
    	String strNote;
    	for(int intNote=0; intNote++ <= 127;)
    	{
    		strNote.clear(); //clear string
    		strNote.append(FilePath,FilePath.length()); //create string of directory containing samples
    	    strNote.operator+=(intNote);  //select the note within sample directory
    	    strNote.append(FileFormat,FileFormat.length());  //append the file format of the samples
    	    if(AppConfig_PrintDiagnosticInfo) cout << "Loading sound: " << strNote << endl;
    	    setUsingSampledSound(ptrSynth, strNote, intNote);  //add sound for specified note
    	}
    }
    void setUsingSampledSound(Synth* ptrSynth, String FilePath, int intNote)

        {
            WavAudioFormat wavFormat;
            if(File(FilePath).exists())
            {
                ScopedPointer<AudioFormatReader> audioReader (wavFormat.createReaderFor
                                                           (new FileInputStream(FilePath),true));
                BigInteger NoteRange;
                NoteRange.setRange (intNote, 1, true);
                ptrSynth->addSound (new SamplerSound ("demo sound",
                                              *audioReader,
                                              NoteRange,
                                              intNote,   // root midi note
                                              0.01,  // attack time
                                              0.01,  // release time
                                              10.0)); // maximum sample length
            }else if(AppConfig_PrintDiagnosticInfo) cout << "not found: " << FilePath << endl;
        }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        midiCollector.reset(sampleRate);
        synth1.setCurrentPlaybackSampleRate(sampleRate);
        synth2.setCurrentPlaybackSampleRate(sampleRate);
        synth3.setCurrentPlaybackSampleRate(sampleRate);
        int sys = system("sudo sh -c 'echo 1 > /sys/class/gpio/gpio60/value'");
    }
    void releaseResources() override {}

    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
    {
        // the synth always adds its output to the audio buffer, so we have to clear it first
        bufferToFill.clearActiveBufferRegion();

        // fill a midi buffer with incoming messages from the midi input
        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);

        // and now get the synth to process the midi events and generate its output
        synth1.renderNextBlock(*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);
        synth2.renderNextBlock(*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);
        synth3.renderNextBlock(*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);
    }

    // this collects real-time midi messages from the midi input device, and
    // turns them into blocks that we can process in our audio callback
    MidiMessageCollector midiCollector;

    //Using one synth per rocker switch
    Synth synth1;
    Synth synth2;
    Synth synth3;
};

#endif // SAMPLER_AUDIO_SOURCE_H_INCLUDED
