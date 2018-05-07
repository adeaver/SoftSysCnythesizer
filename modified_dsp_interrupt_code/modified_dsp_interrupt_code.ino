#define NUMSCALES 2
#define SCALENOTES 32

const double PI2 = 6.283185;
const byte AMP = 127;
const byte OFFSET = 128;

//bitshift lookup tables
byte shiftLeftSix[256];
byte shiftRightTwo[256];

const int LENGTH = 32;
const byte SIN = 0;
const byte SQUARE = 1;
const byte SAW = 2;
const byte PAUSE = 3;

const byte ledPin = 5;
const byte inputButton1 = 2;

byte sinWave[LENGTH];
byte squareWave[LENGTH];
byte sawWave[LENGTH];
byte pauseWave[LENGTH];
byte *wave[4];
byte waveType;
byte scaleType;

//index of wave array we want to write next
byte waveIndex = 0;

short scales[NUMSCALES][SCALENOTES] = {
  // Chromatic Scale
  { 262, 277, 294, 311, 330, 
  349, 370, 392, 415, 440, 
  466, 494, 554, 587, 622, 
  659, 699, 740, 784, 831, 
  880, 932, 988, 1047, 1109, 
  1175, 1245,  1319,  1397,  1480,  
  1568, 1661 },
  // Blues Scale
  { 131, 131, 147, 147, 165, 
  185, 208, 233, 233, 262, 
  294, 330, 370, 415, 415, 
  466, 523, 587, 659, 740, 
  740, 831, 932, 1047, 1175,
  1175, 1319, 1319, 1480, 1661, 
  1865, 1865 }
};

const long DEBOUNCE_TIME = 50;
const long DEBOUNCE_TIME2 = 10;
long waveButtonLastPressed = 0;
long scaleButtonLastPressed = 0;
long randButtonLastPressed = 0;
long lastRand = 0;
int pot = A0;
int waveButton = A1;
int scaleButton = A2;
int randButton = A3;
int playButton = A4;

long currFreq = 3000;

int randState = 0;
int pitchShift = 0;
int pitchShift2 = 0;

void setup() {
  Serial.begin(9600);

  //set input output modes of pins that write to the DAC
  DDRB = 0B11111111; //set pins 8 to 13 as outputs
  DDRD |= 0B11110000; //set pins 4 to 7 as ouputs

  // LED Pin is already set to output as pin 5
  buildSinLookup();
  buildSquareLookup();
  buildSawLookup();
  wave[SIN] = sinWave;
  wave[SQUARE] = squareWave;
  wave[SAW] = sawWave;
  wave[PAUSE] = pauseWave;
  waveType = SIN;
  buildBitShiftTables();
  //set wave freq and interrupt handler stuff
  //turn off interrupts
  cli();
  initializeTimerTwoInterrupt();
  initializeTimerOneInterrupt();
  initializeTimerZeroInterrupt();
  setTimerOneInterrupt(waveFreqToCompareReg(currFreq));
  //enable interrupts
  sei();
  ADCSRA = (ADCSRA & 0xf8) | 0x04; // magic "Where did you find this" - Andrew "*laughs* In a forum" - Patrick
  pinMode(waveButton, INPUT_PULLUP);
  pinMode(scaleButton, INPUT_PULLUP);
  pinMode(randButton, INPUT_PULLUP);
  pinMode(playButton, INPUT_PULLUP);
  pinMode(pot, INPUT);
}

void buildSinLookup() {
  for (int i = 0; i < LENGTH; i++) { // Step across wave tables
    float v = (AMP * sin((PI2 / LENGTH) * i)); // Compute value
    sinWave[i] = reverse(byte(v + OFFSET)); // Store value as integer
  }
}

void buildSquareLookup() {
  for (int i = 0; i < LENGTH / 2; i++) {
    squareWave[i] = reverse(byte(255)); // First half of square wave is high
  }
  for (int i = LENGTH / 2; i < LENGTH; i++) {
    squareWave[i] = reverse(byte(0)); // Second half is low
  }
}

void buildSawLookup() {
  for (int i = 0; i < LENGTH; i++) {
    float v = 255 * i * 1.0 / LENGTH;
    sawWave[i] = reverse(byte(v));
  }
}

void buildBitShiftTables() {
  for (int i = 0; i < 256; i++) {
    shiftLeftSix[i] = i << 6;
    shiftRightTwo[i] = i >> 2;
  }
}

//timer two takes care of changing frequencies, checking at 100 Hz
void initializeTimerTwoInterrupt() {
  TCCR2A = 0;// set entire TCCR1A register to 0
  TCCR2B = 0;// same for TCCR1B
  TCNT2  = 0;//initialize counter value to 0
  //PRESCALER is x1024
  //Set compare register
  OCR2A = (16000000L) / (100 * 1024) - 1; //(must be <256)
  // turn on CTC mode
  TCCR2B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR2B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
}


//timer one outputs waveforms
void initializeTimerOneInterrupt() {
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  //PRESCALER = x1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 bit for 1 prescaler
  TCCR1B |= (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
}

void initializeTimerZeroInterrupt() {
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;
  //PRESCALER is x1024
  //Set compare register
  OCR0A = (16000000L) / (100 * 1024) - 1; //(must be <256)
  // turn on CTC mode
  TCCR0B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR0B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
}

void setTimerOneInterrupt(short compareReg) {
  //Set compare register
  OCR1A = compareReg;//(must be <65536)
}

short waveFreqToCompareReg(long waveFreq) {
  long interruptFreq = waveFreq * (long)LENGTH;
  short compareReg = (16000000L) / (interruptFreq) - 1;
  return compareReg;
}

void writeByte(byte val) {
  byte portDbyte = shiftLeftSix[val];
  byte portBbyte = shiftRightTwo[val];
  PORTD = portDbyte;
  PORTB = portBbyte;
}

byte reverse(byte inb) {
  //reverse function taken from http://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
  byte b = inb;
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void changeWaveType() {
  switch (waveType) {
    case SIN:
      waveType = SAW;
      break;
    case SAW:
      waveType = SQUARE;
      break;
    case SQUARE:
      waveType = SIN;
      break;
  }
}

void checkWaveChangeButton() {
   if(!digitalRead(waveButton)){
      long timePressed = millis();
      if(timePressed - waveButtonLastPressed > DEBOUNCE_TIME){
        changeWaveType();
        waveButtonLastPressed = timePressed;
      }
   }
   if(!digitalRead(scaleButton)) {
     long timePressed = millis();
     if(timePressed - scaleButtonLastPressed > DEBOUNCE_TIME) {
        scaleType++;
        if(scaleType >= NUMSCALES) {
          scaleType = 0;
        }
        scaleButtonLastPressed = timePressed;
     }
   }
   
   if(!digitalRead(randButton)) {
     long timePressed = millis();
     if(timePressed - randButtonLastPressed > DEBOUNCE_TIME) {
        randState = !randState;
        randButtonLastPressed = timePressed;
     }
   }
}

//Timer one checks button presses
ISR(TIMER0_COMPA_vect) {
  checkWaveChangeButton();
  pitchShift = (analogRead(pot) & 0b1111100000) >> 5;
}

//Timer one writes out waves to DAC
ISR(TIMER1_COMPA_vect) { //timer1 interrupt writes bytes onto D6 to D13
  //use the bitmask B00011111 to take the index mod32 and quickly index into the array
  if(!digitalRead(A4)) {
    writeByte(wave[waveType][B00011111 & waveIndex]);
    waveIndex++;
  } else {
    writeByte(LOW);
  }
}

//100 Hz timer two interrupt changes frequencies
ISR(TIMER2_COMPA_vect) {
  if (randState) {
//    pitchShift += 2 - random(0, 4);
//    if (pitchShift < 0) {
//      pitchShift = 0;
//    } else if (pitchShift > SCALENOTES) {
//      pitchShift = 32;
//    }
//    currFreq = scales[scaleType][pitchShift];
    long now = millis();
    if (now - lastRand > DEBOUNCE_TIME2) {
      pitchShift2 = random(0, SCALENOTES);
//      currFreq = random(131, 1865);
      lastRand = now;
    }
    currFreq = scales[scaleType][pitchShift2];
    setTimerOneInterrupt(waveFreqToCompareReg(currFreq));
  }
  else if (scales[scaleType][pitchShift] != currFreq) {
    currFreq = scales[scaleType][pitchShift];
    setTimerOneInterrupt(waveFreqToCompareReg(currFreq));
  }
}

void loop() {
}
