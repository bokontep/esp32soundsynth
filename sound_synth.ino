//#include <MIDI.h>
//#include "WiFi.h"
//#include <HardwareSerial.h>
#include "SynthVoice.h"
#define ANALOG_IN 36
#define RED_BUTTON 4
#define LED 21
#define YELLOW_BUTTON 2
#define AUDIOBUFSIZE 64000
#define SAMPLE_RATE 8000
#define NUM_VOICES 16
#define WTLEN 256
#define MIDI_COMMAND 128
hw_timer_t * timer = NULL;
volatile long t = 0;
//HardwareSerial hSerial(0);

int8_t fp_sinWaveTable[WTLEN];
int8_t fp_sawWaveTable[WTLEN];
int8_t fp_triWaveTable[WTLEN];
int8_t fp_squWaveTable[WTLEN];

uint8_t audio_buffer[AUDIOBUFSIZE];
double f = 22.5;
int notes[] = {0,3,5,12,15,17,24,27,29};
int notelen = 9;
int noteidx = 0;
int voices_notes[NUM_VOICES];
void initFpSin()
{
  for(int i=0;i<WTLEN;i++)
  {
    fp_sinWaveTable[i] = (int8_t)(127*(sin(2*(PI/(float)WTLEN)*i)));
  }
  
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


double beatlen(double bpm)
{
  return (double)60000.0/(bpm*4);
}
SynthVoice voices[NUM_VOICES];

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
  
  for(int i=0;i<NUM_VOICES;i++)
  {
    s = s + (int32_t)(voices[i].Process() ); 
  }
  data = (s+128*NUM_VOICES)>>5;
    //s = ((int32_t)nsinosc.Process())+128;
     //s = s+(fpsinosc[i].Process())>>16;
     //s = nsinosc.Process()>>10;
  
  
  dacWrite(25, data);
  dacWrite(26, data);
  t_end = micros();
  t_diff = t_end-t_start;
  t_counter++;
  avg_time_micros = t_diff;
}
void setup()
{
  Serial.begin(115200);
  //hSerial.begin(115200);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,HIGH);
  delay(2000);
  digitalWrite(LED,LOW);
  
  for(int i=0;i<NUM_VOICES;i++)
  {
    voices_notes[i] = -1;
  }
  //WiFi.mode(WIFI_OFF);
  btStop();
  initFpSaw();
  initFpSin();
  initFpSqu();
  initFpTri();
  pinMode(RED_BUTTON,INPUT);
  pinMode(YELLOW_BUTTON,INPUT);
  pinMode(ANALOG_IN,INPUT);  
  
  for(int i =0;i<NUM_VOICES;i++)
  {
    voices[i] = SynthVoice(SAMPLE_RATE);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_triWaveTable[0],0.5);
    voices[i].SetOsc1ADSR(8000,1,1.0,8000);
    voices[i].AddOsc2WaveTable(WTLEN,&fp_sinWaveTable[0],0.5);
    voices[i].SetOsc2ADSR(8000,1,1.0,8000);
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
  
}


void testChords()
{
  if(t_counter%32000==0)
  {
    for(int i = 0;i<NUM_VOICES;i++)
    {
      Serial.print("Voice ");
      Serial.print(i);
      if(voices[i].IsPlaying())
      {
        voices[i].MidiNoteOff();
        Serial.println("noteoff");  
        
      }
      else
      {
        int root = random(40,60);
        Serial.println("noteon");
        if(noteidx>=notelen)
        {
          noteidx=0;
        }
        voices[i].MidiNoteOn(notes[noteidx]+root);
        noteidx++;
      }
      
    }
  }
  
  if(t_counter%8000==0)
  {
    Serial.println(avg_time_micros);
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
  digitalWrite(LED,HIGH);
  for(int i=0;i<NUM_VOICES;i++)
  {
    if(voices_notes[i]==-1)
    {
      voices_notes[i]=note;
      voices[i].MidiNoteOn(note);
      break;
    }
  }
}
void handleNoteOff(byte channel, byte note, byte velocity)
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
void loop()
{
  pinMode(LED,OUTPUT);
  if(firsttime)
  {
    Serial.write(144);
    Serial.write(59);
    Serial.write(120);
    Serial.flush();
    firsttime = false;
  }
  switch(mstate)
  {
    case WAIT_COMMAND:
      serialData = Serial.read();  
      if(serialData>-1)
      {
        commandByte = serialData;
        command = (commandByte>>4)&7;
        channel = commandByte & 15;
        
        Serial.write(144);
        Serial.write(command);
        Serial.write(channel);
        Serial.flush();
        
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
          break;
          case 7: // NON MUSICAL COMMANDS
          break;        
        }
        
      }
    return;
    case WAIT_DATA1:
      serialData = Serial.read();
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
        }
        
      }
    return;
    case WAIT_DATA2:
      serialData = Serial.read();
      if(serialData>-1)
      {
        data2 = serialData;
        switch(command)
        {
          case 0:
          handleNoteOff(channel,data1,data2);
          mstate = WAIT_COMMAND;
          break;
          case 1:
          handleNoteOn(channel,data1,data2);
          mstate = WAIT_COMMAND;
          break;
        }
        
      }
    return;
  }
  
}
