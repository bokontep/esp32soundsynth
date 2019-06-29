esp32soundsynth
===============

A simple synthesizer engine for the esp32

This is a midi controllable synthesizer engine for the esp32. The project does not aim to provide hifi audio. On the contrary a lofi 8bit wavetable engine is implemented and the sounds it produces have an 80s vibe. Currently the sound engine does not utilize an audio buffer and all synthesis takes place inside an Interrupt Service Routine that writes audio to the 2 8bit DACs of the esp32. For the audio, a wavetable engine is used based from http://www.earlevel.com/main/category/digital-audio/oscillators/wavetable-oscillators/. The envelope generator is from here: https://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code/. However the code from those two libraries is floating point and that is a no-no for code running on an ISR on the esp32. So instead of using floats, the library Fixie provides a fixed point implementation. Fixie can be found here: https://github.com/raroni/fixie

This is a work in progress so this file will be updated with additional info as the project progresses.

Hardware Testbed
================
For testing I assembled 2 boards using perfboards and thin wire. I used 2 different esp32 modules. One is an older ESP32DEVKIT DOIT V1 (plain module, 4MB flash) and the other one is a TTGO 1.7 esp32 module with 4MB of flash and 4MB of psram. Currently I do not use the psram. I added a midi in and midi out circuit, and a headphone jack for audio output. (pictures and videos coming soon...)
Video here:
https://www.youtube.com/watch?v=Zv0jsLpSGcA&feature=youtu.be
In the video I use a breadboarded esp32 with integrated oled IIC 128x64 pixel display. I use it to display the audio waveform and status messages/menus. I also plan to add support for text IIC 2x16 character displays.


