//#include <MIDI.h>
#include "WiFi.h"
#include <HardwareSerial.h>
#include "SynthVoice.h"
#define ANALOG_IN 36

// specify the board to use for pinout
#define ESP32DEVKIT1_DOIT
#ifdef ESP32DEVKIT1_DOIT
#define LED 2
#define MIDIRX 16
#define MIDITX 17
#else
#define LED 21
#define MIDIRX 22
#define MIDITX 19
#endif
#define RED_BUTTON 4

#define YELLOW_BUTTON 2
#define AUDIOBUFSIZE 64000
#define SAMPLE_RATE 20000
#define NUM_VOICES 4
#define NUM_DRUMS 2
#define WTLEN 256
#define MIDI_COMMAND 128
hw_timer_t * timer = NULL;
volatile long t = 0;
HardwareSerial console(0);
HardwareSerial hSerial(2);

int8_t fp_sinWaveTable[WTLEN];
int8_t fp_sawWaveTable[WTLEN];
int8_t fp_triWaveTable[WTLEN];
int8_t fp_squWaveTable[WTLEN];
int8_t fp_plsWaveTable[WTLEN];
int8_t fp_rndWaveTable[WTLEN];

uint8_t audio_buffer[AUDIOBUFSIZE];
double f = 22.5;
int notes[] = {0,3,5,12,15,17,24,27,29};
int notelen = 9;
int noteidx = 0;
int voices_notes[NUM_VOICES];
int drums_notes[NUM_DRUMS];
void initFpSin()
{
  for(int i=0;i<WTLEN;i++)
  {
    fp_sinWaveTable[i] = (int8_t)(127*(sin(2*(PI/(float)WTLEN)*i)));
  }
  return;
}

void initFpTri()
{
  for(int i=0;i<128;i++)
  {
    fp_triWaveTable[i] = (int8_t)(127.0*(-1.0+i*(1.0/((double)WTLEN/2.0))));
  }
  for(int i=128;i<256;i++)
  {
    fp_triWaveTable[i] = (int8_t)(127.0*(1.0 - i*(1.0/((double)WTLEN/2.0))));
  }
  
}
void initFpSqu()
{
  for(int i=0;i<256;i++)
  {
    fp_squWaveTable[i] = (i<(WTLEN/2)?127:-127);
  }
  
}
void initFpSaw()
{
  for(int i = 0;i<256;i++)
  {
    fp_sawWaveTable[i] = (int8_t)(127*(-1.0 + (2.0/WTLEN)*i));
  }
  
}
void initFpRnd()
{
  for(int i=0;i<WTLEN;i++)
  {
    fp_rndWaveTable[i] = (int8_t)(random(0,255)-127);
  }
}
void initFpPls()
{
  for(int i=0;i<WTLEN;i++)
  {
    if(i ==WTLEN/4)
    {
      fp_plsWaveTable[i] = 127;
    }
    else if(i==WTLEN-(WTLEN/4))
    {
      fp_plsWaveTable[i] = -127;
    }
    else
    {
      fp_plsWaveTable[i] = 0;
    }
  }
}

double beatlen(double bpm)
{
  return (double)60000.0/(bpm*4);
}
SynthVoice voices[NUM_VOICES];
SynthVoice drums[NUM_DRUMS];

volatile bool play = false;
volatile unsigned long t_start;
volatile unsigned long t_end;
volatile unsigned long t_diff;
volatile unsigned long t_counter = 0;
volatile double avg_time_micros = 0;
volatile int shift = 0;
void IRAM_ATTR onTimer() {
  

  t_start = micros();
  int64_t s = 0;
  uint8_t data=0;
  uint8_t datar = 0;
  for(int i=0;i<NUM_VOICES;i++)
  {
    s = s + (int32_t)(voices[i].Process() + Num(127)); 
  }
  data = (s/(NUM_VOICES));
    //s = ((int32_t)nsinosc.Process())+128;
     //s = s+(fpsinosc[i].Process())>>16;
     //s = nsinosc.Process()>>10;
  s = 0;
  for(int i=0;i<NUM_DRUMS;i++)
  {
    s = s + (int32_t)(drums[i].Process() + Num(127));
  }
  datar = (s/NUM_DRUMS);
  dacWrite(25, data);
  dacWrite(26, datar);
  t_end = micros();
  t_diff = t_end-t_start;
  t_counter++;
  avg_time_micros = t_diff;
}
//TaskHandle_t Task1;
void setup()
{
  
  WiFi.mode(WIFI_OFF);
  btStop();
  console.begin(57600,SERIAL_8N1);
  //hSerial.setRxBufferSize(1);
  hSerial.begin(31250,SERIAL_8N1,MIDIRX,MIDITX);
  
  //hSerial.begin(115200);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  delay(2000);
  digitalWrite(LED,LOW);
  
  for(int i=0;i<NUM_VOICES;i++)
  {
    voices_notes[i] = -1;
  }
  for(int i=0;i<NUM_DRUMS;i++)
  {
    drums_notes[i] = -1;
  }
  initFpSaw();
  initFpSin();
  initFpSqu();
  initFpTri();  
  initFpRnd();
  initFpPls();
  
  for(int i =0;i<NUM_VOICES;i++)
  {
    voices[i] = SynthVoice(SAMPLE_RATE);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_triWaveTable[0],0.5);
    //voices[i].AddOsc1WaveTable(WTLEN,&fp_plsWaveTable[0],0.5);
    voices[i].SetOsc1ADSR(1000,1,1.0,1000);
    voices[i].AddOsc2WaveTable(WTLEN,&fp_sawWaveTable[0],0.5);
    //voices[i].AddOsc1WaveTable(WTLEN,&fp_plsWaveTable[0],0.5);
    voices[i].SetOsc2ADSR(1000,1,1.0,1000);
  }
  for(int i=0;i<NUM_DRUMS;i++)
  {
    drums[i] = SynthVoice(SAMPLE_RATE);
    drums[i].AddOsc1WaveTable(WTLEN,&fp_rndWaveTable[0],0.5);
    drums[i].SetOsc1ADSR(50,40,0.0,1);
    drums[i].AddOsc2WaveTable(WTLEN,&fp_rndWaveTable[0],0.5);
    drums[i].SetOsc2ADSR(45,40,0.0,1);
  }
  
  /* Use 1st timer of 4 */
  /* 1 tick take 1/(80MHZ/80) = 1u16s so we set divider 80 and count up */
  timer = timerBegin(0, 80, true);

  /* Attach onTimer function to our timer */
  timerAttachInterrupt(timer, &onTimer, true);

  /* Set alarm to call onTimer function every second 1 tick is 1us
    => 1 second is 1000000us */
  /* Repeat the alarm (third parameter) */
  //(int32_t)nsinosc.Process()+(128)
  timerAlarmWrite(timer, 1000000/SAMPLE_RATE, true);

  /* Start an alarm */
  timerAlarmEnable(timer);
  //Serial.println("stalnrt timer");
  //xTaskCreatePinnedToCore(scanMidi,"scanMidi",10000,NULL,1,&Task1,0);
}

void printMidiMessage(uint8_t command, uint8_t data1, uint8_t data2)
{
  console.print("MIDI DATA:");
  console.print(command);
  console.print(":");
  console.print(data1);
  console.print(":");
  console.println(data2);
  console.flush();
}
void testChords()
{
  if(t_counter%32000==0)
  {
    for(int i = 0;i<NUM_VOICES;i++)
    {
      console.print("Voice ");
      console.print(i);
      if(voices[i].IsPlaying())
      {
        voices[i].MidiNoteOff();
        console.println("noteoff");  
        
      }
      else
      {
        int root = random(40,60);
        console.println("noteon");
        if(noteidx>=notelen)
        {
          noteidx=0;
        }
        voices[i].MidiNoteOn(notes[noteidx]+root,127);
        noteidx++;
      }
      
    }
  }
  
  if(t_counter%8000==0)
  {
    console.println(avg_time_micros);
  }
  
}
uint8_t commandByte;
uint8_t  noteByte;
uint8_t  velocityByte;
uint8_t  noteOn = 144;
int serialData;
uint8_t  command;
uint8_t  channel;
int data1;
int data2;
enum midistate{WAIT_COMMAND,WAIT_DATA1,WAIT_DATA2};
bool firsttime = true;
midistate mstate=WAIT_COMMAND;

void handleNoteOn(byte channel, byte note, byte velocity)
{
  
  bool found=false;
  int maxnote = -1;
  int maxnoteidx = -1;
  digitalWrite(LED,HIGH);
  if(channel !=10)
  {
    for(int i=0;i<NUM_VOICES;i++)
    {
      if(voices_notes[i]==-1)
      {
        voices_notes[i]=note;
        voices[i].MidiNoteOn(note,velocity);
        found = true;
        return;
      }
      if(voices_notes[i]>maxnote)
      {
        maxnote = voices_notes[i];
        maxnoteidx = i;
      }
    }
    voices_notes[maxnoteidx]=note;
    voices[maxnoteidx].MidiNoteOn(note,velocity);
  }
  else
  {
    for(int i=0;i<NUM_DRUMS;i++)
    {
      if(drums_notes[i]==-1)
      {
        drums_notes[i]=note;
        drums[i].MidiNoteOn(note,velocity);
        found = true;
        return;
      }
      if(drums_notes[i]>maxnote)
      {
        maxnote = voices_notes[i];
        maxnoteidx = i;
      }
    }
    drums_notes[maxnoteidx]=note;
    drums[maxnoteidx].MidiNoteOn(note,velocity);
  }
  
}
void handleNoteOff(byte channel, byte note, byte velocity)
{
  if(channel!=10)
  {
    digitalWrite(LED,LOW);
    for(int i=0;i<NUM_VOICES;i++)
    {
      if(voices_notes[i]==note)
      {
        voices_notes[i]=-1;
        voices[i].MidiNoteOff();
        //break;
      }
    }
  }
  else
  {
    digitalWrite(LED,LOW);
    for(int i=0;i<NUM_DRUMS;i++)
    {
      if(drums_notes[i]==note)
      {
        drums_notes[i]=-1;
        drums[i].MidiNoteOff();
        //break;
      }
    }
  }
}
void handlePitchBend(byte channel, byte bendlsb, byte bendmsb)
{
  if(channel!=10)
  {
    uint16_t bend = bendmsb<<7 | bendlsb;
    for(int i=0;i<NUM_VOICES;i++)
    {
      if(voices[i].IsPlaying())
      {
        
        voices[i].MidiBend(bend);
      }
    }
  }
}
void loop()
{
  //testChords();
  scanMidi();
  /*
  if(t_counter%32000==0)
  {
    Serial.println(avg_time_micros);
  }
  */
  
}
void scanMidi()
{

  pinMode(LED,OUTPUT);
  if(firsttime)
  {
    //Serial.write(144);
    //Serial.write(59);
    //Serial.write(120);
    //Serial.flush();
    firsttime = false;
  }
  switch(mstate)
  {
    case WAIT_COMMAND:
      serialData = hSerial.read();  
      if(serialData>-1)
      {
        commandByte = serialData;
        command = (commandByte>>4)&7;
        channel = commandByte & 15;
        
        //Serial.write(144);
        //Serial.write(command);
        //Serial.write(channel);
        //Serial.flush();
        
        switch(command)
        {
          case 0: //NOTE OFF
            mstate = WAIT_DATA1;
          break;
          case 1: //NOTE ON
            mstate = WAIT_DATA1;
          break;
          case 2: //AFTERTOUCH
          break;
          case 3: //CONTINUOUS CONTROLLER (CC)
          break;
          case 4: // PATCH CHANGE
          break;
          case 5: // CHANNEL PRESSURE
          break;
          case 6: // PITCH BEND
            mstate = WAIT_DATA1;
          break;
          case 7: // NON MUSICAL COMMANDS
          break;        
        }
        
      }
    return;
    case WAIT_DATA1:
      serialData = hSerial.read();
      if(serialData>-1)
      {
        data1 = serialData;
        switch(command)
        {
          case 0:
          
          mstate = WAIT_DATA2;
          break;
          case 1:
          mstate = WAIT_DATA2;
          break;
          case 6:
          mstate = WAIT_DATA2;
          break;
        }
        
      }
    return;
    case WAIT_DATA2:
      serialData = hSerial.read();
      if(serialData>-1)
      {
        data2 = serialData;
        switch(command)
        {
          case 0:
          printMidiMessage(command,data1,data2);
          handleNoteOff(channel,data1,data2);
          mstate = WAIT_COMMAND;
          break;
          case 1:
          printMidiMessage(command,data1,data2);
          handleNoteOn(channel,data1,data2);
          mstate = WAIT_COMMAND;
          break;
          case 6:
          handlePitchBend(channel,data1,data2);
          mstate = WAIT_COMMAND;
          break;
        }
        
      }
    return;
  }
  
}
