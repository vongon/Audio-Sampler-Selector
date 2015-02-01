/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

SamplerSound::SamplerSound (const String& soundName,
                            AudioFormatReader& source,
                            const BigInteger& notes,
                            const int midiNoteForNormalPitch,
                            const double attackTimeSecs,
                            const double releaseTimeSecs,
                            const double maxSampleLengthSeconds)
    : name (soundName),
      midiNotes (notes),
      midiRootNote (midiNoteForNormalPitch)
{
    sourceSampleRate = source.sampleRate;

    if (sourceSampleRate <= 0 || source.lengthInSamples <= 0)
    {
        length = 0;
        attackSamples = 0;
        releaseSamples = 0;
    }
    else
    {
        length = jmin ((int) source.lengthInSamples,
                       (int) (maxSampleLengthSeconds * sourceSampleRate));

        data = new AudioSampleBuffer (jmin (2, (int) source.numChannels), length + 4);

        source.read (data, 0, length + 4, 0, true, true);

        attackSamples = roundToInt (attackTimeSecs * sourceSampleRate);
        releaseSamples = roundToInt (releaseTimeSecs * sourceSampleRate);
    }

    xfadeSamples = 300; //number of samples long the xfade will be
    xloopBuffer = 1500; //xfade buffer length from start and end of sound file, for smooth looping
}

SamplerSound::~SamplerSound()
{
}

bool SamplerSound::appliesToNote (const int midiNoteNumber)
{
    return midiNotes [midiNoteNumber];
}

bool SamplerSound::appliesToChannel (const int requested_midiChannel)
{
    	return true;
}

//==============================================================================
SamplerVoice::SamplerVoice()
    : pitchRatio (0.0),
      sourceSamplePosition (0.0),
      lgain (0.0f), rgain (0.0f),
      attackReleaseLevel (0), attackDelta (0), releaseDelta (0),
      isInAttack (false), isInRelease (false)
{
}

SamplerVoice::~SamplerVoice()
{
}

bool SamplerVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<const SamplerSound*> (sound) != nullptr;
}

void SamplerVoice::startNote (const int midiNoteNumber,
                              const float velocity,
                              SynthesiserSound* s,
                              const int /*currentPitchWheelPosition*/)
{
    if (const SamplerSound* const sound = dynamic_cast <const SamplerSound*> (s))
    {
        pitchRatio = pow (2.0, (midiNoteNumber - sound->midiRootNote) / 12.0)
                        * sound->sourceSampleRate / getSampleRate();

        sourceSamplePosition = 0.0;
        lgain = velocity;
        rgain = velocity;

        isInAttack = (sound->attackSamples > 0);
        isInRelease = false;

        if (isInAttack)
        {
            attackReleaseLevel = 0.0f;
            attackDelta = (float) (pitchRatio / sound->attackSamples);
        }
        else
        {
            attackReleaseLevel = 1.0f;
            attackDelta = 0.0f;
        }

        if (sound->releaseSamples > 0)
            releaseDelta = (float) (-pitchRatio / sound->releaseSamples);
        else
            releaseDelta = 0.0f;

        xAttackDelta = (float)(pitchRatio / sound -> xfadeSamples); //compute xfade attack delta
        xReleaseDelta = (float)(-pitchRatio / sound -> xfadeSamples); //compute xfade release delta
    }


    else
    {
        jassertfalse; // this object can only play SamplerSounds!
    }
}


void SamplerVoice::stopNote (const bool allowTailOff)
{
    if (allowTailOff)
    {
        isInAttack = false;
        isInRelease = true;
    }
    else
    {
        clearCurrentNote();
    }
}

void SamplerVoice::pitchWheelMoved (const int pitchWheel)
{
	//should make pitch wheel sweep an octave
	pitchRatio = pow(2.0, ((pitchWheel / 8192.0) - 1));


}

void SamplerVoice::controllerMoved (const int /*controllerNumber*/,
                                    const int /*newValue*/)
{
}

//==============================================================================
void SamplerVoice::renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
{
    if (const SamplerSound* const playingSound = static_cast <SamplerSound*> (getCurrentlyPlayingSound().get()))
    {
        const float* const inL = playingSound->data->getReadPointer (0);
        const float* const inR = playingSound->data->getNumChannels() > 1
                                    ? playingSound->data->getReadPointer (1) : nullptr;

        float* outL = outputBuffer.getWritePointer (0, startSample);
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer (1, startSample) : nullptr;

        while (--numSamples >= 0)
        {
            const int pos = (int) sourceSamplePosition;
            const float alpha = (float) (sourceSamplePosition - pos);
            const float invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = (inL [pos] * invAlpha + inL [pos + 1] * alpha);
            float r = (inR != nullptr) ? (inR [pos] * invAlpha + inR [pos + 1] * alpha)
                                       : l;



            //xfade code below.

            if (sourceSamplePosition > ((playingSound->length - playingSound->xloopBuffer) - playingSound->xfadeSamples)) //in xfade region
               {
                   if (xfade == false) //checking for first time in xfade
                   {
                    xAttackLevel = 0; //initialize attack envelope
                    xReleaseLevel = 1; //initialize release envelope
                    xAttackSamplePosition = playingSound->xloopBuffer; //initialize sample position of "fade in" portion
                    xfade = true; //set to true because we are now in a xfade
                   }

                   //set release sample values
                 float xRelease_l = l;
                 float xRelease_r = r;

                  //compute attack sample values
                 const int xpos = (int) xAttackSamplePosition;
                 const float xalpha = (float) (xAttackSamplePosition - xpos);
                 const float xinvAlpha = 1.0f - xalpha;
                 float xAttack_l = (inL [xpos] * xinvAlpha + inL [xpos + 1] * xalpha);
                 float xAttack_r = (inR != nullptr) ? (inR [xpos] * xinvAlpha + inR [xpos + 1] * xalpha)
                                                        : xAttack_l;

                  //multiply sample values by envelope level
                 xRelease_l *= xReleaseLevel;
                 xRelease_r *= xReleaseLevel;
                 xAttack_l *= xAttackLevel;
                 xAttack_r *= xAttackLevel;

                  //increment delta's
                 xReleaseLevel += xReleaseDelta;
                 xAttackLevel += xAttackDelta;

                  //write new sample values
                 l = xAttack_l + xRelease_l;
                 r = xAttack_r + xRelease_r;

                  //increment xAttackSamplePosition
                 xAttackSamplePosition += pitchRatio;

                  //Check if done with xfade
                 //if(xAttackSamplePosition > playingSound -> xfadeSamples + playingSound -> xloopBuffer)
                 if(xReleaseLevel <= 0.0f | xAttackLevel >= 1.0f){
                	 sourceSamplePosition = xAttackSamplePosition;
                	 xfade = false;
                   }

                }



            l *= lgain;
            r *= rgain;

            if (isInAttack)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += attackDelta;

                if (attackReleaseLevel >= 1.0f)
                {
                    attackReleaseLevel = 1.0f;
                    isInAttack = false;
                }
            }
            else if (isInRelease)
            {
                l *= attackReleaseLevel;
                r *= attackReleaseLevel;

                attackReleaseLevel += releaseDelta;

                if (attackReleaseLevel <= 0.0f)
                {
                    stopNote (false);
                    break;
                }
            }

            if (outR != nullptr)
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            sourceSamplePosition += pitchRatio;


        }
    }
}
