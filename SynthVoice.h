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
        this->modulation = 0;
        this->pwm = Num(0.5);
        this->fmod1 = Num(1.0);
        this->fmod2 = Num(1.0);
        this->fmod3 = Num(0.0);
    }
    SynthVoice(double sampleRate) {
        this->sampleRate = sampleRate;
        this->modulation = 0;
        this->pwm = Num(0.5);
        this->fmod1 = Num(1.0);
        this->fmod2 = Num(1.0);
        this->fmod3 = Num(0.0);
    }
    ~SynthVoice(void) {
        
    }
    void MidiNoteOn(uint8_t note, uint8_t vel)
    {
        
        double f = pow(2.0,(note*1.0-69.0)/12.0)*440.0;
        velocity = Num(vel/128.0); 
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
    void AddOsc1WaveTable(int len, int8_t *waveTableIn)
    {
        osc[0].AddWaveTable(len,waveTableIn);
    }
    void AddOsc2WaveTable(int len, int8_t *waveTableIn)
    {
        osc[1].AddWaveTable(len,waveTableIn);
    }
    void SetOsc1ADSR(Num a, Num d, Num s, Num r)
    {
        adsr[0].SetADSR(a,d,s,r);
    }
    void SetOsc2ADSR(Num a, Num d, Num s, Num r)
    {
        adsr[1].SetADSR(a,d,s,r);
    }
    void SetFmod1(double fmod)
    {
      this->fmod1 = Num(fmod);
    }
    void SetFmod2(double fmod)
    {
      this->fmod2 = Num(fmod);
    }
    void SetFmod3(double fmod)
    {
      this->fmod3 = Num(fmod);
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
    void MidiMod(uint8_t newmod)
    {
      modulation = Num(newmod)/Num(127.0);
      fmod1 = Num(1.0)-Num(modulation)/Num(127.0);
      fmod2 = Num(1.0)-Num(modulation)/Num(127.0);
      fmod3 = modulation; 
    }
    void MidiPwm(uint8_t newmod)
    {
      pwm = Num(newmod)/Num(128);
      if(newmod == 0)
      {
        osc[0].SetPhaseOffset(0);
        osc[1].SetPhaseOffset(0);
      }
      else
      {
        osc[0].SetPhaseOffset(pwm);
        osc[1].SetPhaseOffset(pwm);
      }
    }
    int GetOsc1WaveTableCount()
    {
      return osc[0].GetWaveTableCount();
    }
    int GetOsc2WaveTableCount()
    {
      return osc[1].GetWaveTableCount();
    }
    void SetOsc1PhaseOffset(uint8_t newphase)
    {
      osc[0].SetPhaseOffset(newphase/127.0);
    }
    void SetOsc2PhaseOffset(uint8_t newphase)
    {
      osc[1].SetPhaseOffset(newphase/127.0);
    }
    void MidiOsc1Wave(uint8_t newwave)
    {
      osc[0].SetWaveTable(newwave);
    }
    void MidiOsc2Wave(uint8_t newwave)
    {
      osc[1].SetWaveTable(newwave);
    }

    Num Process()
    {
      if(modulation==Num(0))
      {
        return (velocity*adsr[0].Process()*osc[0].Process()*fmod1+velocity*adsr[1].Process()*osc[1].Process()*fmod2)>>1;
      }
      else
      {
        return  ((velocity*adsr[0].Process()*osc[0].Process()*fmod1) + (velocity*adsr[1].Process()*osc[1].Process()*fmod2) + (velocity*(adsr[0].Process()*osc[0].Process()*osc[1].Process()*fmod3)))>>3;
      }
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
    Num velocity;
    Num modulation;
    Num pwm;
    Num fmod1;
    Num fmod2;
    Num fmod3;
};

#endif
