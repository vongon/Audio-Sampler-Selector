#include "Synth.h"
#include "../JuceLibraryCode/JuceHeader.h"


void Synth::handleMidiEvent (const MidiMessage& m)
{
	        if (m.isNoteOn())
	        {
	            noteOn (m.getChannel(), m.getNoteNumber(), 0.25f);
	        }
	        else if (m.isNoteOff())
	        {
	            noteOff (m.getChannel(), m.getNoteNumber(), true);
	        }
	        else if (m.isAllNotesOff() || m.isAllSoundOff())
	        {
	            allNotesOff (m.getChannel(), true);
	        }
	        else if (m.isPitchWheel())
	        {
	            const int channel = m.getChannel();
	            const int wheelPos = m.getPitchWheelValue();
	            lastPitchWheelValues [channel - 1] = wheelPos;

	            handlePitchWheel (channel, wheelPos);
	        }
	        else if (m.isAftertouch())
	        {
	            handleAftertouch (m.getChannel(), m.getNoteNumber(), m.getAfterTouchValue());
	        }
	        else if (m.isController())
	        {
	            handleController (m.getChannel(), m.getControllerNumber(), m.getControllerValue());
	        }
}


