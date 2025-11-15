# A header-only MIDI sequencer for C++

## Features
* Support for streaming of MIDI, MUS, HMI, HMP, XMI (via built-in XMI2MID).
* For the OPL2/OPL3 emulator using, it's possible to play IMF, KLM, and CMF files with OPL instrumentsshipped on aboard or streaming raw OPL commands.
* Built-in loops support:
  * Final-Fantasy VII loops via "loopStart" and "loopEnd" markers.
  * RPG Maker way via CC111 as a "loopStart".
  * XMI native loops (for XMI files only).
  * Extended variant of Final Fantasy VII loops to support nested loops and loop counts ("loopStart=XXX" and "loopEnd=0").
* Multiple devices support per track and the "device switch" event.
* Full seekability (ability to see the current position, jump to certain point in any moment, see the full time length, see a range of loop).
* Ability to load a file via file path or via memory.
  * NOTE: Acceptable file paths should be in UTF-8 even on Windows.
* Compatibility with C++98 standard which can work on a bunch of older C++ compilers.

## License
* The code of sequencer is under MIT license
* The code of "fraction.hpp" is in a public domain
* XMI2MIDI is taken from WildMIDI project and there are licensed under LGPLv2:
  * If you want to use this MIDI sequencer with a MIT license, you should disable XMI suppory by removing of "impl/cvt_xmi2mus.hpp" file and declaring of next macros:
    * "-DBWMIDI_DISABLE_XMI_SUPPORT"

## Usage tips
When you embed this sequencer into a static library, you should rename a class by declaring a next macro:
```cpp
#define BW_MidiSequencer MyVeryCoolMidiSequencer
```
This is required to avoid a collision of ABI when using the same MIDI sequencer in multiple code files or when you are using this in a library project.

## Development and contribution notice
The most active development of this sequence is going at the repository of [libADLMIDI](https://github.com/Wohlstand/libADLMIDI) project
and gets sometimes synced to here too. Commits here are pretty large. If you interested to see more detailed changelog, you might want to
check out it [from here](https://github.com/Wohlstand/libADLMIDI/commits/master/src/midiseq). And therefore, And therefore the top state
of this repository considered to be pretty stable and free from the rest of incomplete and experimental/unstable updates. You still can
send pull-requests to this repository, and these changes will be synced back to the [libADLMIDI](https://github.com/Wohlstand/libADLMIDI),
[libOPNMIDI](https://github.com/Wohlstand/libOPNMIDI), [libEDMIDI](https://github.com/Wohlstand/libEDMIDI), and to the
[MixerX](https://github.com/WohlSoft/SDL-Mixer-X).
