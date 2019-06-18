//#include <MIDI.h>
#include "WiFi.h"
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <HardwareSerial.h>
#include "SynthVoice.h"
#define ANALOG_IN 36

// specify the board to use for pinout
//#define TTGO16MPROESP32OLED

#define ESP32DEVKIT1_DOIT
//#define ESP32DEVKIT1_DOIT
#ifdef ESP32DEVKIT1_DOIT
#define LED 2
#define MIDIRX 16
#define MIDITX 17
#endif
#ifdef TTGO16MPROESP32OLED
//#define NOMIDI
#define LED 2
#define MIDIRX 36
#define MIDITX 37
#endif
#ifdef GENERIC
#define LED 21
#define MIDIRX 22
#define MIDITX 19
#endif
#define RED_BUTTON 4

#define YELLOW_BUTTON 2
#define AUDIOBUFSIZE 64000
#define SAMPLE_RATE 14000
#define NUM_VOICES 4
#define NUM_DRUMS 0
#define WTLEN 256
#define MIDI_COMMAND 128
hw_timer_t * timer = NULL;
volatile long t = 0;
HardwareSerial console(0);
HardwareSerial hSerial(2);
#ifdef TTGO16MPROESP32OLED

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 16,15,4);
#endif
int8_t fp_sinWaveTable[WTLEN];
int8_t fp_sawWaveTable[WTLEN];
int8_t fp_triWaveTable[WTLEN];
int8_t fp_squWaveTable[WTLEN];
int8_t fp_plsWaveTable[WTLEN];
int8_t fp_rndWaveTable[WTLEN];
uint8_t dwfbuf_l[128];
uint8_t dwfbuf_r[128];
char line1[17];
char line2[17];
uint8_t dwfidx=0;
uint8_t audio_buffer[AUDIOBUFSIZE];
double f = 22.5;
int notes[] = {0,3,5,12,15,17,24,27,29};
int notelen = 9;
int noteidx = 0;
int voices_notes[NUM_VOICES];
int drums_notes[NUM_DRUMS];
enum controller_states{CS_OSC=0, CS_ENV,CS_AMP,CS_FIL};
int controller_state = CS_OSC;
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
  dwfbuf_l[dwfidx]=data;
  
    //s = ((int32_t)nsinosc.Process())+128;
     //s = s+(fpsinosc[i].Process())>>16;
     //s = nsinosc.Process()>>10;
  s = 0;
  for(int i=0;i<NUM_DRUMS;i++)
  {
    s = s + (int32_t)(drums[i].Process() + Num(127));
  }
  datar = data;//(s/NUM_DRUMS);
  dwfbuf_r[dwfidx]=datar;
  dwfidx = (dwfidx+1)%128;
  dacWrite(25, data);
  dacWrite(26, datar);
  t_end = micros();
  t_diff = t_end-t_start;
  t_counter++;
  avg_time_micros = t_diff;
}
TaskHandle_t Task1;
void setup()
{
  
  WiFi.mode(WIFI_OFF);
  btStop();
  console.begin(57600,SERIAL_8N1);
  //hSerial.setRxBufferSize(1);
#ifndef NOMIDI
  hSerial.begin(31250,SERIAL_8N1,MIDIRX,MIDITX);
#endif
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
  strcpy(line1,"BOKONTEP    (C)");
  strcpy(line2,"ESP32SYNTH 2019");
  
  
  
  for(int i =0;i<NUM_VOICES;i++)
  {
    voices[i] = SynthVoice(SAMPLE_RATE);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_sinWaveTable[0]);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_sawWaveTable[0]);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_triWaveTable[0]);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_squWaveTable[0]);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_plsWaveTable[0]);
    voices[i].AddOsc1WaveTable(WTLEN,&fp_rndWaveTable[0]);

    
    //voices[i].AddOsc1WaveTable(WTLEN,&fp_plsWaveTable[0]);
    voices[i].SetOsc1ADSR(10,1,1.0,1000);
    voices[i].AddOsc2WaveTable(WTLEN,&fp_sinWaveTable[0]);
    //voices[i].AddOsc1WaveTable(WTLEN,&fp_plsWaveTable[0]);
    
    voices[i].AddOsc2WaveTable(WTLEN,&fp_sawWaveTable[0]);
    voices[i].AddOsc2WaveTable(WTLEN,&fp_triWaveTable[0]);
    voices[i].AddOsc2WaveTable(WTLEN,&fp_squWaveTable[0]);
    voices[i].AddOsc2WaveTable(WTLEN,&fp_plsWaveTable[0]);
    voices[i].AddOsc2WaveTable(WTLEN,&fp_rndWaveTable[0]);

    
    
    
    voices[i].SetOsc2ADSR(10,1,1.0,1000);
  }
  /*
  for(int i=0;i<NUM_DRUMS;i++)
  {
    drums[i] = SynthVoice(SAMPLE_RATE);
    drums[i].AddOsc1WaveTable(WTLEN,&fp_rndWaveTable[0]);
    drums[i].SetOsc1ADSR(50,40,0.0,1);
    drums[i].AddOsc2WaveTable(WTLEN,&fp_rndWaveTable[0]);
    drums[i].SetOsc2ADSR(45,40,0.0,1);
  }
  */
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
  #ifdef TTGO16MPROESP32OLED
    //Serial.println("stalnrt timer");
  xTaskCreatePinnedToCore(displayData,"displayData",20000,NULL,1,&Task1,0);
  #endif
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
byte rotaries[4][8];
void handleNoteOn(byte channel, byte note, byte velocity)
{
  char buf[17];
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
        sprintf(buf, "note:%d vel:%d",note, velocity);
        strcpy(line1,buf);
        strcpy(line2,"");
        
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
      voices[i].MidiBend(bend);
    }
  }
}
void handleCC(byte channel, byte cc, byte data)
{
  switch(cc)
  {
    case 1: //Modulation
      for(int i=0;i<NUM_VOICES;i++)
      {
        voices[i].MidiMod(data);
      }
    break;
    case 2: //PWM
      for(int i=0;i<NUM_VOICES;i++)
      {
        voices[i].MidiPwm(data);
      }
     
      
    break;
    case 64: //pedal osc1 waveform
      for(int i=0;i<NUM_VOICES;i++)
      {
        voices[i].MidiOsc1Wave(data);
      }
    break;
    case 65: //portamento osc2 waveform
      for(int i=0;i<NUM_VOICES;i++)
      {
        voices[i].MidiOsc1Wave(data);
      }
    break;
    case 91: //ROTARY 1 ON UMX490
      handleRotaryData(0, controller_state,data); 
    break;
    case 93: //ROTARY 2 ON UMX490
      handleRotaryData(1, controller_state,data);
    break;
    case 74: //ROTARY 3 ON UMX490
      handleRotaryData(2, controller_state,data);
    break;
    case 71: //ROTARY 4 ON UMX490
      handleRotaryData(3, controller_state,data);
    break;
    case 73: //ROTARY 5 ON UMX490
      handleRotaryData(4, controller_state,data);
    break;
    case 75: //ROTARY 6 ON UMX490
      handleRotaryData(5, controller_state,data);
    break;
    case 72: //ROTARY 7 ON UMX490
      handleRotaryData(6, controller_state,data);
    break;
    case 10: //ROTARY 8 ON UMX490
      handleRotaryData(7, controller_state,data);
    break;
    case 97: //button 1 on UMX490
      controller_state = CS_OSC;
    break;
    case 96: //button 2 on UMX490
      controller_state = CS_ENV;
    break;
    case 66: //button 3 on UMX490
      controller_state = CS_AMP;
    break;
    case 67: //button 4 on UMX490
      controller_state = CS_FIL;
    break;
  }
    
  
}
void handleRotaryData(int rotary, int state, byte data)
{
  switch(state)
  {
    case CS_OSC:
      switch(rotary)
      {
        case 0:
          for(int i=0;i<NUM_VOICES;i++)
          {
            voices[i].MidiOsc1Wave(data%voices[i].GetOsc1WaveTableCount());
          }
        break;
        case 1:
          for(int i=0;i<NUM_VOICES;i++)
          {
            voices[i].SetFmod1(data-63.0/64.0);
          }
        break;
        case 2:
          for(int i=0;i<NUM_VOICES;i++)
          {
            voices[i].SetOsc1PhaseOffset(data/127.0);
          }
          break;
        case 4:
          for(int i=0;i<NUM_VOICES;i++)
          {
            voices[i].MidiOsc2Wave(data%voices[i].GetOsc2WaveTableCount());
          }
        case 5:
        {
          for(int i=0;i<NUM_VOICES;i++)
          {
        
            voices[i].SetFmod2(data-63.0/64.0);
            
          }
        }
        case 6:
          for(int i=0;i<NUM_VOICES;i++)
          {
            voices[i].SetOsc2PhaseOffset(data/127.0);
          }
          break;
      }
    break;
    case CS_ENV:
    break;
    case CS_AMP:
    break;
    case CS_FIL:
    break;
  }
}
void displayData(void * parameter)
{
  #ifdef TTGO16MPROESP32OLED
  u8g2.begin();
  while(true)
  {
  u8g2.clearBuffer();          // clear the internal memory
  //u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  for (int i=0;i<127;i++)
  {
    u8g2.drawLine(i,(dwfbuf_l[i]>>2),i+1,(dwfbuf_l[i+1]>>2));
    u8g2.drawLine(i,(dwfbuf_r[i]>>2),i+1,(dwfbuf_r[i+1]>>2));
    
  }
  
  //u8g2.setFont( u8g2_font_unifont_t_cyrillic);
  //u8g2.drawExtUTF8(-8,10,0,NULL,line1);
  u8g2.setFont( u8g2_font_unifont_t_greek);
  u8g2.drawExtUTF8(-8,10,0,NULL,line1);
  u8g2.drawExtUTF8(-8,21,0,NULL,line2);
  
  u8g2.sendBuffer();          // transfer internal memory to the display
  delay(10);
  
  }
  #endif
}
void loop()
{
  //testChords();
  #ifndef NOMIDI
  scanMidi();
  #endif
  //displayData();
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
            mstate = WAIT_DATA1;
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
          case 3:
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
          case 3:
          handleCC(channel, data1, data2);
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
