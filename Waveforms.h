#ifndef Waveforms_h
#define Waveforms_h
#define WAVEFORM_COUNT 256
int8_t Waveforms[WAVEFORM_COUNT][WTLEN];
void initWaveForms()
{
  for(int i=0;i<WTLEN;i++)
  {
    Waveforms[0][i] = (int8_t)(127 * (sin(2 * (PI / (float)WTLEN) * i)));
  }
  for (int i = 0; i < 128; i++)
  {
    Waveforms[1][i] = (int8_t)(127.0 * (-1.0 + i * (1.0 / ((double)WTLEN / 2.0))));
  }
  for (int i = 128; i < 256; i++)
  {
    Waveforms[1][i] = (int8_t)(127.0 * (1.0 - i * (1.0 / ((double)WTLEN / 2.0))));
  }
  for (int i = 0; i < 256; i++)
  {
    Waveforms[2][i] = (i < (WTLEN / 2) ? 127 : -127);
  }
  for (int i = 0; i < 256; i++)
  {
    Waveforms[3][i] = (int8_t)(127 * (-1.0 + (2.0 / WTLEN) * i));
  }
  for (int i = 0; i < WTLEN; i++)
  {
    Waveforms[4][i] = (int8_t)(random(0, 255) - 127);
  }
  
  for (int w=5;w<WAVEFORM_COUNT;w++)
  {
    for(int i=0;i<256;i++)
    {
      float f1 = (255.0-w)/250.0;
      float f2 = ((255-w)%125)/125.0;
      float f3 = ((255-w)%62)/62.0;
      float f4 = ((255-w)%31)/31.0; 
      Waveforms[w][i] = (int8_t)((f1*Waveforms[0][i]+f2*Waveforms[1][i]+f3*Waveforms[2][i]+f4*Waveforms[3][i])/4);   
    }
  }
}







#endif
