#define MAX_WAVEFORM 256
const double PI2 = 6.283185;
byte counter = 0;

float sinWave[256];

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
   sinWave[i] = reverse(byte(v+128));
   // store reversed value in lookup table
  }
}

int pot = A0;
long waveFreq = 2500;

void setup(){
  //set input output modes of pins that write to the DAC
  DDRB = 0B11111111; //set pins 8 to 13 as outputs
  DDRD = 0B11000000; //set pins 6 to 7 as outputs
  //turn off interrupts
  cli();
  initializeTimerOneInterrupt();
  setTimerOneInterrupt(waveFreqToCompareReg(waveFreq));
  //enable interrupts
  sei();
  createSinLookup();
  pinMode(pot, INPUT);
}


 //timer one outputs waveforms
void initializeTimerOneInterrupt(){
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


void setTimerOneInterrupt(short compareReg){
  //Set compare register
  OCR1A = compareReg; //(must be <65536)
}


short waveFreqToCompareReg(long waveFreq){
  long WAVELENGTH = 256;
  long interruptFreq = waveFreq * WAVELENGTH;
  short compareReg = (16000000L) / (interruptFreq) - 1;
  return compareReg;
}


void writeByte(byte val){
  byte portDbyte = val << 6;
  byte portBbyte = val >> 2;
  PORTD = portDbyte;
  PORTB = portBbyte;
}

//Timer one interrupt handler that writes out waves to DAC
ISR(TIMER1_COMPA_vect){
  byte sinVal = sinWave[counter];
  writeByte(sinVal);
  counter++; 
  if(counter >= MAX_WAVEFORM) {
    counter = 0;
  }
  //this will overflow and turn back to 0  after 255 
  //because counter is a byte value
}

void loop() {
    //int pitchShift = analogRead(pot);
    //waveFreq = 750;
    //setTimerOneInterrupt(waveFreqToCompareReg(waveFreq));
}
