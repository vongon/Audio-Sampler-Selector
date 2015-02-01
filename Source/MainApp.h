#ifndef MAIN_APP_H_INCLUDED
#define MAIN_APP_H_INCLUDED

#include "SamplerAudioSource.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "Rockers.h"

class MainApp : public JUCEApplicationBase {
public:
    MainApp();
    ~MainApp();

    const String getApplicationName() override { return ProjectInfo::projectName; }
    const String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return false; }
    void anotherInstanceStarted(const String& commandLine) override {}
    void systemRequestedQuit() override { quit(); }
    void suspended() override {}
    void resumed() override {}
    void unhandledException (
            const std::exception *,
            const String &sourceFilename,
            int lineNumber) override { jassertfalse;}
    void initialise(const String& commandLine) override;
    void shutdown() override;

    void Print_ADM_State();

private:
    AudioDeviceManager* m_adm;
    AudioSourcePlayer m_asp;
    SamplerAudioSource m_sas;
    Rockers* Rkrs;
    int sys;
};

#endif // MAIN_APP_H_INCLUDED
