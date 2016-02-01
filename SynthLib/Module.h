#pragma once

#include "Defines.h"
#include "Frame.h"
#include <yaml-cpp/yaml.h>

namespace mvSynth {

class Link
{
    friend class Module;
    friend class Synth;

    Module* source;
    int sourcePort;

    Module* dest;
    int destPort;

    SampleType mOffset;
    SampleType mScale;

public:
    Link();
    void SetScaling(const SampleType& offset, const SampleType& scale);
};

enum class PortType
{
    Input,
    Output
};

struct Port
{
    const char* name;
    std::set<Link*> links;
    Frame<SampleType> frame;
    int channels;

    Port();
};

enum class ModuleType
{
    Global,
    PerVoice,
};

class Module
{
    friend class Synth;

protected:
    ModuleType mModuleType;
    std::string mName;
    std::vector<Port> mInputs;
    std::vector<Port> mOutputs;

    // define I/O ports using this function in a module constructor
    void AddPort(PortType type, const char *name, int channels);

    void DisconnectInputs();

public:
    bool enabled;
    bool processed[MW_MAX_VOICES];

    Module();
    virtual ~Module();

    void ResizeFrames(size_t frameSize);
    void ClearAllVoices();
    void Process(int voiceID, Synth* synth);

    virtual bool ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr);

    virtual void OnProcess(int voiceID, Synth* synth) = 0;

    // Called when key is pressed. Module should initialize per voice data
    virtual void OnInitVoice(int voiceID, Synth* synth, SampleType freq);

    // Called when key is released
    virtual void OnReleaseVoice(int voiceID, Synth* synth);

    // get I/O port ID by name
    int GetPort(PortType type, const char* name) const;
};

} // namespace mvSynth