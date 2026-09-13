# DynaFAT
DynaFAT is a real time audio processing VST made for electronic music production. DynaFAT preserves dynamics while adding richness to bass synths through aggressive distortion, multiband compression, and envelope following.

Requirements:
- C++
- CMake
- JUCE framework

Goals:
- Implement multiband compression class
- Reduce user controls to be concise while allowing for customizability
- Optimize CPU performance in buffer processing
- Uncomment filter implementation and turn it into a class like the other effects

Future Features:
- Adjustable curve of envelope follower, allowing for a sharp J-curve (sharp vs. shallow fall off)
- Allow user to save and load presets

Future Knobs:
- Envelope Follower:
  - Mix/"Dynamix"
- Compressor
  - Input gain
- Master
  - Master gain

Known bugs:
- Alg options set to SoftClip whenever menu is closed.
- Slight click artifact on upward compression (only?). Maybe a soft knee would help.
- Compressing even in dead zone.

Current To-Do List:
- Fix followEnv for the compressor class. It must be different than the standalone envelope follower for attack/release to function properly.
- Incorporate knee into downward compression

Effect Class Layout:
- Member variables store state of parameters through setter functions
- Member functions process single samples of audio, only passing in the sample as a parameter
   → Access member variables for parameter values

Reminder for Developer:
- PC VST Path: "F:/ProgramData/Ableton/Live 9 Standard"
- Laptop VST Path: "C:/Users/tf2ma/Documents/Ableton/VST Plugins"