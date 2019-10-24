#include "FS.h"
#include "FFat.h"

// This file should be compiled with 'Partition Scheme' (in Tools menu)
// set to 'Default with ffat' if you have a 4MB ESP32 dev module or
// set to '16M Fat' if you have a 16MB ESP32 dev module.

// You only need to format FFat the first time you run a test
#define FORMAT_FFAT true
const char* SETTINGS_FILE = "settings.cfg";
const char* PATTERNS_FILE = "patterns.dat";
const char* TUNINGS_FILE = "tunings.dat";
const char* PATCHES_FILE = "patches.dat";
File file;
void initFileIO(char* msg)
{
  if(!FFat.begin())
  {
    sprintf(msg,"FILEIO INIT ERR");
  }
  else
  {
    sprintf(msg, "FILEIO OK");
  }
}
void formatFat()
{
  FFat.format();
}
bool openFile(const char* filename)
{
  file = FFat.open(filename);
  return file?true:false;
}
bool openFileForWriting(const char* filename)
{
  file = FFat.open(filename,FILE_WRITE);
  return (file>0)?true:false;
}

bool readLine(char* buf, int maxchars)
{
  int i = 0;
  while(file.available() && i<maxchars)
  {
    char c = file.read();
    if(c=='\n')
    {
      buf[i]=0;
      return true;
    }
    buf[i] = c;
    i++;
  }
  if(i>0)
  {
    return true;
  }
  return false;
}
bool writeLine(char* msg)
{
  
  if(file.print(msg) &&  file.print("\n"))
  {
    return true;
  }
  else
  {
    return false;
  }
}
void closeFile()
{
  file.close();
}
