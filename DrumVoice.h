#ifndef DrumVoice_h
#define DrumVoice_h
#include "Num.h"
#include "Util.h"
#include "MultiWaveNumOsc.h"
#include "AD.h"
using namespace Fixie;

class DrumVoice {
public:
    DrumVoice()
    {
        this->sampleRate = 8000;
    }
    DrumVoice(double sampleRate) {
        this->sampleRate = sampleRate;
    }
    ~DrumVoice(void) {
        
    }
    void MidiNoteOn(uint8_t note, uint8_t vel)
    {
        
        double f = pow(2.0,(note*1.0-69.0)/12.0)*440.0;
        velocity = Num(vel/128.0); 
        freq1 = f;
        freq2 = f;
        osc[0].SetFrequency(freq1,sampleRate);
        osc[1].SetFrequency(freq2,sampleRate);
        ad[0].Gate(1);
        ad[1].Gate(1);
    }
    void MidiNoteOff()
    {
        ad[0].Gate(0);
        ad[1].Gate(0);
    }
    void AddOsc1WaveTable(int len, int8_t *waveTableIn)
    {
        osc[0].AddWaveTable(len,waveTableIn);
    }
    void AddOsc2WaveTable(int len, int8_t *waveTableIn)
    {
        osc[1].AddWaveTable(len,waveTableIn);
    }
    void SetOsc1AD(Num a, Num d)
    {
        ad[0].SetAD(a,d);
    }
    void SetOsc2AD(Num a, Num d)
    {
        ad[1].SetAD(a,d);
    }
    void MidiBend(uint16_t bend)
    {
      double factor = ((bend - 8192.0)/8192.0);
      double mul = pow(2.0,(factor*12.0)/12.0);
      double bendfreq1 = freq1*mul;
      double bendfreq2 = freq2*mul;
      osc[0].SetFrequency(bendfreq1,sampleRate);
      osc[1].SetFrequency(bendfreq2,sampleRate);
      
    }
    Num Process()
    {
        return (velocity*ad[0].Process()*osc[0].Process()+velocity*ad[1].Process()*osc[1].Process())>>1;
    }
    bool IsPlaying()
    {
        if(ad[0].GetState()==AD::envState::env_idle && ad[0].GetState()==AD::envState::env_idle)
        {
            return false;
        }
        return true;
    }
protected:
    MultiWaveNumOsc osc[2];
    AD ad[2];
    double sampleRate;
    double freq1;
    double freq2;
    Num velocity;
};

#endif
