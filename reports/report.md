## Cynthesizer: Software Synthesizers in C

Software Systems Spring 2018
Andrew Deaver, Kai Levy, and Patrick Huston


### Project Goals
In this project, we hope to implement a software synthesizer in C to run on an Arduino. Our basic feature set is made up of the following functions:

- The synthesis of sine, sawtooth, and square waves
- The ability to toggle through these types of waves
- The ability to trigger multiple notes through a hardware interface
- Implementation of a few basic scales that can be played through

### Useful Resources

We found the following resources to be useful in helping us gain some background information on audio generation and implement these ideas in practice using an arduino. 

- [The introduction to the SoftSysSynth on the ThinkDSP blog](http://thinkdsp.blogspot.com/2014/02/build-softsyssynth.html)
- [An instructables article](http://www.instructables.com/id/Arduino-Timer-Interrupts/) about Arduino timing interrupts
- [The Fur Elise on an Arduino Synthesizer blog post on the Think DSP blog](http://thinkdsp.blogspot.com/2015/10/fur-elise-on-arduino-synthesizer.html) by Thomas Chen, Pratool Gadtaula, and Dennis Chen

### Learning Goals

Some of our shared learning goals include:

- Learn a bit about DSP and audio generation
- Learn how about timing and interrupts in C
- Combine our interests in programming and electronic music production

### Results

#### Basic Implementation

To begin the implementation of our arduino-based synthesizer, we started with a functioning synthesizer per the specs in the ThinkDSP blog post. The schematic can be found [here](http://2.bp.blogspot.com/-X8u0IGkONZQ/UwN2YYzoMAI/AAAAAAAABVY/VNkUFH2RPj0/s1600/SoftSysSynth_bb.png). 

The basic circuitry, which we built following the directions above, involved a resistor ladder that allowed us to convert 8 binary outputs into an 8-bit analog signal. This is necessary because the arduino has no true analog output, but can only make PWM waves. This way, we can write an 8-bit value into the digital output pins one by one and approximate the desired signal. After that, we simply need some amplification to hear it out of the speaker itself.

Through this implementation, the synthesizer is able to produce basic tones, but the quality of the waves outputted is unclean. The waves are constructed by using `delay(_millis_)`, and looking up amplitudes from a table which contained patterns corresponding to sine waves, sawtooth waves, square waves, and triangular waves. This works fine as a first pass to output tones, but were heavily distorted and did not produce the desired tones associated with common waveforms. It also did not produce the desired note. Likewise, we were only able to produce about one note per octave since a 1 millisecond delay corresponds to roughly an octave. 

When trying to figure out the delay, we used the formula `delay = (1/frequency * 1000) / # of samples` where the number of samples was the number of samples in the waveform table that we were using. It became apparent that a sample rate of 1 sample per millisecond (i.e. a delay of 1 millisecond) did not produce exactly the frequencies that we wanted. Furthermore, a delay of 1 millisecond between each sample meant that we were missing a lot of possible notes. We figured with a smaller delay between samples, we could not only get more exact notes, but a wider range of notes. In order to achieve this, we decided to use the arduino's `delayMicroseconds()` function instead of the `delay()` function. This definitely gave us a wider range of notes, but still suffered from the distortion problem that we were experiencing before. 

#### Hardware Interface

We additionally spent some time building out the hardware of the synthesizer, so that it could be played more like an instrument, as opposed to a wave generator. This included attaching a slider potentiometer, which we decided to use to control the pitch of the tone, and additional buttons, which could be used to control more "states", such as the waveform of the sound or the scale. 

#### Generating Clean Waves

At this point, we were able to generate a wider range of tones, but inspecting the waveforms using an Analog Discovery oscilloscope made it clear that our waves were nowhere near the ideal waves we were trying to create (sine, sawtooth, square). The reason for this was because the `delayMicroseconds()` function represented the minimum amount of time between each of the samples. So any computation that needed to be done added to the delay, and therefore created a different frequency than we thought we were creating.  Additionally, the highest frequency we could not play higher frequencies because of the computational time that the arduino needed in order to process all of the instructions.

[Bad Waves](./images/micros.png)

This observation, combined with the advice of our professor to "look into timer interrupts" prompted us to look into [timer interrupts](https://learn.adafruit.com/multi-tasking-the-arduino-part-2/timers). In the script `/modified_dsp_interrupt_code/modified_dsp_interrupt_code.ino` is our final implementation of the synthesizer using timer interrupts. The basic idea of timer interrupts is that instead of outputting the signal in the `loop()` function, an additional structure is created that "wakes up" to a specified clock rate, and performs a unit of work. 

The initialization for one of our timers that outputs the waveforms looks like this:

```
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
```

To configure this properly, we used [this instructables post](http://www.instructables.com/id/Arduino-Timer-Interrupts/) as reference. We use the rate of this timer to control the frequency of the sound. 

```
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
```

The primary 'work' that we're doing during each timed interrupt is writing the correct byte to the Arduino, which is chosen using an index into a cached array that contains values that represent one period of each type of wave in sequence. Additionally, we check the value of the 'mute' button (pin A4) to determine whether the values should be written in the first place. 

Essentially, we're abstracting out the wave generation from the `loop()`, and inserting it into its own timed loop that should always take about the same amount of time to complete. 

#### Optimizations

After some preliminary experiments with timer interrupts, we were achieving more accurate frequencies with much less distortion. Still, we could not play as many clean frequencies (especially in the higher range) as we wanted, and so we sought to optimize further. We came across a very well-explained article by former SoftSys students that helped us realize a faster and more refined version of the synthesizer. Some of the optimizations that we found included:
- Using `PORTB` and `PORTD` (built in arduino port registers) to write digital outputs, as opposed to iterating through a `for` loop on each pin.
- Pregenerating the waveforms in arrays to be accessed on each loop, and shortening them so that higher frequencies could be achieved.


At this point, we had a refined program that did very little computational work on each loop. However, we noticed some glitching in the sound, which meant this could be optimized. This glitching was a very gradual timing issue, which didn't influence a single waveform, but only manifested over time. To fix it, we optimized:
- Converting the analog signal from the potentiometer to an index into our frequency array using bitwise operators to save time on division. This is seen here: `pitchShift = (analogRead(pot) & 0b1111100000) >> 5;`
- Inserting the line `ADCSRA = (ADCSRA & 0xf8) | 0x04;` which sped up the analog to digital conversion on the arduino board by a factor of 16. This helped decrease the amount of time spent reading the potentiometer value.

Finally, we had a clean waveform that could play high frequencies effectively!

[Good Waves](./images/sin.png)
[Nice Sawtooth](./images/saw.png)

Additionally, we implemented different scales using a lookup table for known note frequencies (as opposed to calculating them as we did in our first implementation).

#### Demo Video

The final version of the working code is [here](../modified_dsp_interrupt_code/modified_dsp_interrupt_code.ino).

A live demo of the synthesizer in action is available [here](https://www.youtube.com/watch?v=6wRB8D1njIc).

### Reflection

We managed to achieve our MVP of the project. Looking back, we had not anticipated the challenge of producing accurate waveforms. If we had more time on this project, we would like to implement more audio effects. During the course of this project, we attempted to implement a delay effect during one of our first iterations. However, once we began attempting to get a more accurate sound, we had to abandon this idea. We all learned a lot about the different challenges associated with getting a clean, accurate sounding synthesizer. 
