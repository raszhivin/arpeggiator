
#include <TimerOne.h>
#include "engine.h"
arp a(C, 5, 2, 6, 200, c_harmonic, 0);
bool button_pressed;
int ButtonVal;

#define baseNotepin 1
#define baseOctavepin 5
#define octaveShiftpin 0
#define stepspin 4
#define indelaypin 3
#define orderpin 7
#define modepin 6
#define syncinpin 3

#define LEDPin 13

// Synchronization: choose one of two possible options:
#define EXT_SYNC
//#define INT_SYNC

void readPoties()
{
  unsigned i;
  a.setupArp(analogRead(baseNotepin), analogRead(baseOctavepin), analogRead(octaveShiftpin), analogRead(stepspin), analogRead(indelaypin), analogRead(orderpin), analogRead(modepin));
  
  // In my setup the buttons are connected to pins 6..12
  for (i=12;i>5;i--)
    if (!(digitalRead(i))) { button_pressed = true; ButtonVal = 13-i; return; }
}

void setup()
{
  a.midibegin();
  Timer1.initialize(200000);          // initialize timer1, and set a 1/10 second period
  Timer1.attachInterrupt(readPoties); // We will read potis and buttons values within timer interrupt
  
  // LED pin
  pinMode(LEDPin, OUTPUT);
  
  // Initialize pins for 2-pins pushbuttons with pullup enabled
  for (unsigned i=6;i<13;i++)
  {
    pinMode(i,INPUT_PULLUP);
    //pinMode(i, INPUT);
    //digitalWrite(i, 1);
  }
  button_pressed = false;
  ButtonVal = 1;
}

void loop()
{
    if (button_pressed)
    {
      a.setProgression(ButtonVal-1);
      button_pressed = false;
      
      // Switch on LED
      digitalWrite(LEDPin, HIGH);
      a.play();
      
      // Switch off LED
      digitalWrite(LEDPin, LOW);
    }
}
