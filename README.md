# DelayPrototype
Prototype of LoFi Delay plugin written in C++ using JUCE framework, a delay buffer methodology from an AudioProgrammer juice tutorial and various other DSP elements from my other projects.

Working Features:
- Delay section works with time and feedback controls
- Distortion section works with single knob control. Maths included for proportionate control of output gain stage and mix control

Bugs:
- SVT filter implementation from JUCE DSP module not working lots clicks pops buzzes, thinking this is to do with the way circular buffers are handled in my algorithms compared to JUCE's
- AutoPan function for ping pong effect not working, audio panned to RHS and cracks pops buzzes in left channel, possibly to do with the way the auto pan algorithm splits channels for its panning.
- Various clicks & pops when using with sinewave synth in plugin tester

going to do a full reworking using a different more modular circular buffer method with seperate cpp and h files for each element of the plugin.
