#include "MainApp.h"
#include "stdlib.h"
#include "Rockers.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include <unistd.h>
#include "../JuceLibraryCode/AppConfig.h"

using namespace std;

MainApp::MainApp() {
    m_adm = new AudioDeviceManager();

}

MainApp::~MainApp() {
    m_asp.setSource(nullptr);
    m_adm->removeMidiInputCallback(String::empty, &(m_sas.midiCollector));
    m_adm->removeAudioCallback(&m_asp);
    delete m_adm;
    sys = system("sudo sh -c 'echo 0 > /sys/class/gpio/gpio60/value'");
}

void MainApp::initialise(const String& commandLine) {
	sys = 0;
	sys = system("killall jackd");
	sys = system("export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/dbus/system_bus_socket");
	sys = system("jackd -R -t2000 -dalsa -dhw:0 -r48000 -p64 -n2 -Xseq -Phw:0 &");
	sys = system("jack_wait -w");
	sys = system("sudo sh -c 'echo 60 > /sys/class/gpio/export' "); //green indicator light setup
	sys = system("sudo sh -c 'echo out > /sys/class/gpio/gpio60/direction'"); //green indicator light setup
	m_adm->setCurrentAudioDeviceType("JACK",true);
	String status = m_adm->initialise (0, 2, 0, true, String::empty, 0);
	cout << "initialise status: " << status << endl;
    m_adm->addAudioCallback(&m_asp);
    m_asp.setSource(&m_sas);
    //Rkrs = new Rockers(); //initializes rockers, must be called after SYNTHs have been initialized

    //All MIDI is handeled by JACK now, so this shouldn't be needed?
    StringArray midiDeviceList = MidiInput::getDevices();
    int midiDeviceIdx = MidiInput::getDefaultDeviceIndex();
    String midiDeviceName = midiDeviceList[midiDeviceIdx];
    m_adm->setMidiInputEnabled(midiDeviceName, true);
    m_adm->addMidiInputCallback(midiDeviceName, &m_sas.midiCollector);

    sys = system("jack_connect QX49:midi/playback_1 Juce-Midi-Input:midi/capture_1 &");
    //sys = system("jack_connect 12Step:midi/playback_1 Juce-Midi-Input:midi/capture_1 &");
    if(AppConfig_PrintDiagnosticInfo) Print_ADM_State();
}

void MainApp::shutdown() {

}

void MainApp::Print_ADM_State(){
	//-------Print Available Available Device Types---------------
	cout<<endl;
	for (int i=m_adm->getAvailableDeviceTypes().size(); i-- > 0;)
	cout << "Available Device Type: " << i << ":" << m_adm->getAvailableDeviceTypes().getUnchecked(i)->getTypeName()<< endl;
	cout<<endl;

    if (m_adm->getCurrentDeviceTypeObject() != nullptr)
    	cout << "current Device Type Object is: " << m_adm->getCurrentDeviceTypeObject()->getTypeName() << endl;
    else cout << "current Device Type Object is NULL" << endl;

    if (AudioIODevice* device = m_adm->getCurrentAudioDevice())
    {
    	cout << "Current audio device: " << device->getName().quoted() << endl;
        cout << "Sample rate: " << String (device->getCurrentSampleRate()) << endl;
        cout << "Block size: " << String (device->getCurrentBufferSizeSamples()) << std::endl;
        cout << "Bit depth: " << String (device->getCurrentBitDepth()) << endl;
        cout << "Input channel names: " << device->getInputChannelNames().joinIntoString (", ") << endl;
        cout << "Output channel names: " << device->getOutputChannelNames().joinIntoString (", ") << endl;
        cout << "Output latency in samples: " << int (device->getOutputLatencyInSamples())<< endl;
        //-------Print Available Available Sample---------------
        for (int i=device->getAvailableSampleRates().size(); i-- > 0;)
        cout << "available sample rate " << i << ":" << device->getAvailableSampleRates().getUnchecked(i)<< endl;
        //-------Print Available Buffer Sizes---------------
        for (int i=device->getAvailableBufferSizes().size(); i-- > 0;)
        cout << "available buffer " << i << ":" << device->getAvailableBufferSizes().getUnchecked(i)<< endl;
    }
    else cout << "No audio device open" << endl;
}

