/*
  

 Author: Allen Downey 
 
 Based on http://arduino.cc/en/Tutorial/AnalogInput
 Created by David Cuartielles
 modified 30 Aug 2011
 By Tom Igoe

 License: Public Domain
 
 */

#define MAX_WAVETYPE 1
#define MAX_WAVEFORM 256
const double PI2 = 6.283185;

float sinWave[1][256];

byte reverse(byte inb) {
  //reverse function taken from http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
  byte b = inb;
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void createSinLookup(){
  for (int i=0; i<MAX_WAVEFORM; i++) { // Step across wave tables
   float v = (127*sin((PI2/256)*i)); // Compute value
   sinWave[0][i] = reverse(byte(v+128));
   // store reversed value in lookup table
  }
}
 
int ledPin = 5;       // select the pin for the LED
int buttonPin1 = 2;
int buttonPin2 = 3;
int buttonPin3 = 4;
int pot = A0;

void setup() {
  Serial.begin(9600);

  pinMode(pot, INPUT);
  
  pinMode(buttonPin1, INPUT_PULLUP);  
  pinMode(buttonPin2, INPUT_PULLUP); 

  pinMode(buttonPin3, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);
  
  pinMode(13, OUTPUT);  
  pinMode(12, OUTPUT);  
  pinMode(11, OUTPUT);  
  pinMode(10, OUTPUT);  
  pinMode(9, OUTPUT);  
  pinMode(8, OUTPUT);  
  pinMode(7, OUTPUT);  
  pinMode(6, OUTPUT);  

  createSinLookup();
}

void writeByte(byte val){
  byte portDbyte = val << 6;
  byte portBbyte = val >> 2;
  PORTD = portDbyte;
  PORTB = portBbyte;
}

int counter = 0, delayCounter = 0, delayValue = 0;
unsigned long BASE_WAVELENGTH = 1000000;
int pitchShift = 0, BASE_DELAY_SHIFT = 50;
float delayGain = 3.0;
unsigned long SHIFT_FACTOR = 15;
int waveType = 1;
unsigned long lastButton = 0, lastDelay = 0, lastOutput = 0;
unsigned long pitchDelay = 31, BUTTON_DEBOUNCE = 500, DELAY_DEBOUNCE = 500, outputDebounce = 0;
int button2State = LOW, button3State = LOW;
int button1, button2, button3;


void loop() {
  button1 = digitalRead(buttonPin1);
  button2 = digitalRead(buttonPin3);
  button3 = digitalRead(buttonPin2);
  
  if (micros() - lastButton >= BUTTON_DEBOUNCE) {
    lastButton = micros();
    if (button2 != button2State) {
      button2State = button2;

      if(button2State == HIGH) {
        waveType++;
        if (waveType >= MAX_WAVETYPE) waveType = 0;
      }
    }
  }

  if(micros() - lastDelay >= DELAY_DEBOUNCE) {
    lastDelay = micros();
    if(!(button3) != button3State) {
      button3State = !(button3);
      if(button3State) {
        delayCounter = counter + BASE_DELAY_SHIFT;
        delayGain = 3;
      } else {
        delayValue = 0;
      }
    }

    if(button3State) {
      delayCounter++;
      delayGain += .1;
      delayValue = sinWave[waveType][delayCounter];
    }
  }

  pitchShift = analogRead(pot);
  pitchDelay = (SHIFT_FACTOR * pitchShift + BASE_WAVELENGTH)/MAX_WAVEFORM;
  counter++;
  if (counter > MAX_WAVEFORM) counter = 0;
  writeByte(sinWave[waveType][counter]);
  delayMicroseconds(pitchDelay);
}
