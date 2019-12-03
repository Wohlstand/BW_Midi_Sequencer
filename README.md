# A header-only MIDI sequencer for C++

## Features
* Support for streaming of MIDI, XMI (via XMI2MIDI), MUS (via MUS2MIDI)
* For case of OPL3 emulator using, it's possible to play IMF and CMF files with OPL instruments shipped on aboard
* Built-in loops support:
  * Final-Fantasy VII loops via "loopStart" and "loopEnd" markers
  * RPG Maker way via CC111 as a "loopStart"
  * XMI native loops (for XMI files only)
  * Extended variant of Final Fantasy VII loops to support nested loops and loop counts ("loopStart=XXX" and "loopEnd=0")
* Multiple devices support per track and the "device switch" event
* Full seekability (ability to see the current position, jump to certain point in any moment, see the full time length, see a range of loop)
* Ability to load a file via file path or via memory
  * NOTE: Acceptable file paths should be in UTF-8 even on Windows
* Compatibility with C++98 standard which can work on a bunch of older C++ compilers

## License
* The code of sequencer is under MIT license
* The code of "fraction.hpp" is in a public domain
* MUS2MIDI and XMI2MIDI are taken from WildMIDI project and there are licensed under LGPLv2
  * If you want to use this MIDI sequencer with a MIT license, you should disable MUS and XMI suppory by removing of "cvt_*.hpp" flies and declaring of next macros:
    * "-DBWMIDI_DISABLE_MUS_SUPPORT"
    * "-DBWMIDI_DISABLE_XMI_SUPPORT"

## Usage tips
When you are using a sequencer, you should rename a class by declaring a next macro:
```cpp
#define BW_MidiSequencer MyVeryCoolMidiSequencer
```
This is required to avoid a collision of ABI when using the same MIDI sequencer in multiple code files or when you are using this in a library project.

