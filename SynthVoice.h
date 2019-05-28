#ifndef SynthVoice_h
#define SynthVoice_h
#include "Num.h"
#include "Util.h"
#include "NumWaveTableOsc.hpp"
#include "ADSR.h"
using namespace Fixie;

class SynthVoice {
public:
    SynthVoice()
    {
        this->sampleRate = 8000;
    }
    SynthVoice(double sampleRate) {
        this->sampleRate = sampleRate;
    }
    ~SynthVoice(void) {
        
    }
    void MidiNoteOn(uint8_t note)
    {
        double f = pow(2.0,(note*1.0-69.0)/12.0)*440.0; 
        freq1 = f;
        freq2 = f;
        osc[0].SetFrequency(freq1,sampleRate);
        osc[1].SetFrequency(freq2,sampleRate);
        adsr[0].Gate(1);
        adsr[1].Gate(1);
    }
    void MidiNoteOff()
    {
        adsr[0].Gate(0);
        adsr[1].Gate(0);
    }
    void AddOsc1WaveTable(int len, int8_t *waveTableIn, double topFreq)
    {
        osc[0].AddWaveTable(len,waveTableIn,topFreq);
    }
    void AddOsc2WaveTable(int len, int8_t *waveTableIn, double topFreq)
    {
        osc[1].AddWaveTable(len,waveTableIn,topFreq);
    }
    void SetOsc1ADSR(Num a, Num d, Num s, Num r)
    {
        adsr[0].SetADSR(a,d,s,r);
    }
    void SetOsc2ADSR(Num a, Num d, Num s, Num r)
    {
        adsr[1].SetADSR(a,d,s,r);
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
        return (adsr[0].Process()*osc[0].Process()+adsr[1].Process()*osc[1].Process())>>1;
    }
    bool IsPlaying()
    {
        if(adsr[0].GetState()==ADSR::envState::env_idle && adsr[0].GetState()==ADSR::envState::env_idle)
        {
            return false;
        }
        return true;
    }
protected:
    NumWaveTableOsc osc[2];
    ADSR adsr[2];
    double sampleRate;
    double freq1;
    double freq2;
    
};

#endif
