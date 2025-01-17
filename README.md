<div align="center" width="100%" >
  <img align="center" src="./images/Commodore_64_logo.png" />
</div>

# *`MetallC64`*

An instruction-accurate <a href="https://en.wikipedia.org/wiki/Commodore_64">Commodore 64</a> emulator written in C.<br>

## main components

- 6510 CPU, a derivative of the <a hred="https://github.com/beddinao/MOS-6502-Emulator">6502</a>.
- VIC-II (Video Interface Chip).
- 2x 6526 CIA (Complex Interface Adapters).
- 6581 SID (Sound Interface Device) (not-implemented-yet).

## main loop

## shell interface
this emulator has a dedicated thread to run a shell interface with a set of builin commands to manage the emulator.
commands are:

```
• LDP [path to .prg]: load BASIC program to memory.
• LDD [path to .d64]: load D64 disk image.

• BRD [int:1-16]: change default border color.
• BGR [int:1-16]: change default background color.
• TXT [int:1-16]: change default text color.

• DMP $st $en: dump memory from address $st to $en.
• SCR: show CPU status.
• SVR: show VIC-II status.
• SC1: show CIA#1 status.
• SC2: show CIA#2 status.

• CLR: clear loaded program from memory:
  • also performs a hard reset.
• EXT: exit emulation.
```

## upcoming features
- SID (Sound Interface) support
- Full hardware Sprites support (partial progress)
