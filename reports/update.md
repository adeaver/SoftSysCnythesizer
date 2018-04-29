## Cynthesizer: Software Synthesizers in C

Software Systems Spring 2018
Andrew Deaver, Kai Levy, and Patrick Huston


### Project Goals
In this project, we hope to explore audio synthesis in C. As a lower bound on our goal, we plan on creating a software-based implementation of a synthesizer on an Arduino that can (a) produce an audio signal and (b) include at least some basic controls to modify the signal to create new sounds. 

Once we get a basic implementation working, our next goals might be:

1. Working on lowering latency and general optimizations
2. Implementing finer/more interesting controls to modify the audio signal

### Learning Goals

Some of our shared learning goals include:

- Learn a bit about DSP and audio generation
- Use external packages
- Combine our interests in programming and electronic music

### Completed Steps

So far:
- We have done some reading in The Audio Programming Book. We have found that it contains a huge amount of reference information about audio synthesis, but less parts that are directly relateable to our basic arduino implementation. We will keep the book as a reference as we move forward.
- We have completed a basic working implementation of the Arduino-based synthesizer described in the ThinkDSP project page. It makes sound. People have said that it is very annoying. 
- We have run the first couple of example scripts (`wave1`) provided in the ThinkDSP course GitHub repository. Additionally, we started to make some modifications to this script to produce different notes and sequences. 

### What we're working on now

We would like to improve on our Arduino synthesizer to make it more robust and fun to play with.
- We are doing research into more fully understanding the audio generation methods used for in the Arduino scripts. (**All**)
- We are implementing more hardware interfaces (e.g. buttons, knobs) that can make the synthesizer more interactive. (Kai)
- We are researching ways to implement features (either in software or in hardware) that can modify our sythesized sound, so that we can apply filters and effects to the output. (Andrew + Patrick)

