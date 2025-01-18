<div align="center" width="100%" >
  <img align="center" src="./images/Commodore_64_logo.png" />
</div>

# *`MetallC64`*

A <a href="https://en.wikipedia.org/wiki/Commodore_64">Commodore 64</a> emulator written in C.<br>

## about the Commodore 64  
The **Commodore 64 (C64)** is one of the most iconic home computers of all time. Released in **1982**, it became the **best-selling computer** ever, due to its powerful hardware, affordability, and massive software library. It featured:
```
- 6510 CPU, a modified version of the 6502.  
- VIC-II (Video Interface Chip) for graphics and sprites.  
- 6581 SID (Sound Interface Device) for advanced sound synthesis.  
- Two 6526 CIA (Complex Interface Adapters) for I/O operations.
```

The C64 was popular for **gaming, programming, and productivity**, with thousands of titles released for it.  

## about this emulator  
MetallC64 is a work-in-progress C64 emulator that aims to run **BASIC programs**. However, it is **not cycle-accurate** and **not highly precise**, meaning some programs **may not work as expected or as in real hardware**.  

### current limitations:  
- **VIC-II (Graphics) is not fully accurate** and may have **visual glitches**.  
- **SID (Sound) is not implemented yet**.  
- Some **hardware features are missing or incomplete**.

### main components implemented:  
- **<a href="https://github.com/beddinao/MOS-6502-Emulator">6510 CPU</a>**  `(instruction accurate independant emulator)`
- **VIC-II**  `(Video Interface Chip)`
  ```
    • 16 KB address space for screen, character, and sprite memory
    • 320 × 200 pixels video resolution (160 × 200 in multicolor mode)
    • 40 × 25 characters text resolution
    • 8 hardware sprites, each 24 × 21 pixels (12 × 21 in multicolor mode)
    • 16 colors
    • Raster interrupts for advanced effects and timing control
    • Smooth scrolling
    • Three character display modes and two bitmap modes:
      - Standard Character Mode
      - Multicolor Character Mode
      - Extended Background Color Mode
      - Standard Bitmap Mode
      - Multicolor Bitmap Mode
  ```
- **CIA-1 & CIA-2**  `(Complex Interface Adapters)`
  ```
    • keyboard input scanning for detecting key presses
    • timer A & Timer B in each chip for generating delays and timing events
    • memory management registers for I/O mapping and bank switching
    • time of Day (TOD) clock
  ```

## basic functionality test

10 PRINT one liners at `./programs/bas/10_print.bas`

`10 PRINT CHR$(205.5+RND(1)); : GOTO 10`

`10 PRINT CHR$(32+(INT(RND(1)+.5)*81)); : GOTO 10`

<div align="center" width="100%">
  <img  width="49%" src="./images/gifs/recording_1.gif" title="10_print 1" />
  <img  width="49%" src="./images/gifs/recording_3.gif" title="10_print 3" />
</div>


## shell interface  
The emulator has a **command shell** that runs in a separate thread:

### available commands:  

| **command**  | **description** |
|----------------|---------------|
| `LDP [path]` | Load a BASIC program (`.prg`) into memory. |
| `LDD [path]` | Load a (`.d64`) D64 disk image. |
|-|-|
| `BRD [1-16]` | Change the border color. |
| `BGR [1-16]` | Change the background color. |
| `TXT [1-16]` | Change the text color. |
|-|-|
| `DMP $st $en` | Dump memory from address `$st` to `$en` in the range `0-FFFF`. |
| `SCR` | Show CPU status. |
| `SVR` | Show VIC-II status. |
| `SC1` | Show CIA#1 status. |
| `SC2` | Show CIA#2 status. |
|-|-|
| `CLR` | Clear the loaded program from memory **and perform a hard reset**. |
| `EXT` | Exit the emulator. |

## graphics library:
MetallC64 uses <a href="https://github.com/codam-coding-college/MLX42">MLX42</a>, a lightweight graphics library based on GLFW and OpenGL.

## screenshots

`animated demos`

<div align="center" width="100%">
  <img  width="40%" src="./images/gifs/the_Amiga_ball.gif" title="the AMIGA ball"/>
  <img  width="40%" src="./images/gifs/bigpixelnyan.gif" title="pixel nyan"/>
  <img  width="40%" src="./images/gifs/hellas_guys.gif" title="hellas guys"/>
</div>

`some art brought from the C64 demoScene`

<div align="center" width="100%">
  <img  width="40%" src="./images/screenshots/wool_on_her_mind.png" title="wool on her mind" />
  <img  width="40%" src="./images/screenshots/c64com_charged_mikeal_spiham.png" title="c64.com charged" />
  <img  width="40%" src="./images/screenshots/show_time.png" title="show time" />
  <img  width="40%" src="./images/screenshots/obey_the_machine.png" title="obey the machine" />
  <img  width="40%" src="./images/screenshots/no_slackers_sky.png" title="no slackers sky" />
  <img  width="40%" src="./images/screenshots/dark_faces.png" title="dark faces" />
  <img  width="40%" src="./images/screenshots/highlander.png" title="highlander" />
  <img  width="40%" src="./images/screenshots/petscii_tracing.png" title="petscii tracing" />
  <img  width="40%" src="./images/screenshots/efucollab.png" title="eight feet under"/>
</div>


## system requirements
- **`Linux or Macos`**
- **`glfw (libglfw3-dev)`**
- **`readline (libreadline-dev)`**
- **`cmake >= 3.18`**

## **upcoming features:**  
- **Full hardware sprite support** (partial-progress)  
- **SID sound emulation**

## useful-resources
- C64 Wiki: https://www.c64-wiki.com/wiki/Main_Page
- memory map: https://sta.c64.org/cbm64mem.html
- VIC-II ultimate resource: https://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt
- 6502 ultimate resource: https://www.masswerk.at/6502/6502_instruction_set.html
- 6502 BCD: http://www.6502.org/tutorials/decimal_mode.html
