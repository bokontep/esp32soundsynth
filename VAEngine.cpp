#include "SynthVoice.h"
#define NUM_VOICES 2
class VAEngine
{
  public:
    VAEngine()
    {
      
    }
    ~VAEngine()
    {
      
    }
    Num Process()
    {
      s = 0;
      for (int i = 0; i < NUM_VOICES; i++)
      {
        s = s + (int32_t)(voices[i].Process() + Num(127));
      }
    }
    void handleNoteOn(byte channel, byte note, byte velocity)
    {
      bool found = false;
      int maxnote = -1;
      int maxnoteidx = -1;
      for (int i = 0; i < NUM_VOICES; i++)
      {
        if (voices_notes[i] == -1)
        {
          voices_notes[i] = note;
          voices[i].MidiNoteOn(note, velocity);
          found = true;
          return;
        }
        if (voices_notes[i] > maxnote)
        {
          maxnote = voices_notes[i];
          maxnoteidx = i;
        }
      }
      voices_notes[maxnoteidx] = note;
      voices[maxnoteidx].MidiNoteOn(note, velocity);
    }
    
    void handleNoteOff(byte channel, byte note, byte velocity)
    {
      digitalWrite(LED, LOW);
      for (int i = 0; i < NUM_VOICES; i++)
      {
        if (voices_notes[i] == note)
        {
          voices_notes[i] = -1;
          voices[i].MidiNoteOff();
          //break;
        }
      }
    }
    private:
      SynthVoice mSynthVoice[NUM_VOICES];
      int voices_notes[NUM_VOICES];
      volatile int64_t s = 0; 
      
  
}
