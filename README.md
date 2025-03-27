# *`MetallC64`*

A <a href="https://en.wikipedia.org/wiki/Commodore_64">Commodore 64</a> emulator written in C.<br>

## about the Commodore 64  
The **Commodore 64 (C64)** is one of the most iconic home computers of all time. Released in **1982**, it became the **best-selling computer** ever, due to its powerful hardware, affordability, and massive software library. It featured:
```java
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
- **6510 CPU** a bit modified version of <a href="https://github.com/beddinao/MOS-6502-Emulator">**6502**</a>  `(instruction accurate independant emulator)`
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

some one line BASIC programs at `./programs/bas/10_print.bas`

`10 PRINT CHR$(205.5+RND(1)); : GOTO 10`

<div align="center" width="100%">
  <img  width="100%" src="./images/gifs/recording_1.gif" title="10_print 1" />
</div>

`10 PRINT CHR$(32+(INT(RND(1)+.5)*81)); : GOTO 10`

<div align="center" width="100%">
  <img  width="100%" src="./images/gifs/recording_3.gif" title="10_print 3" />
</div>

## shell interface  
The emulator has a **command shell** that runs in a separate thread for debugging<br> and general hardware behaviour observation:

### available commands:  

| **command**  | **description** |
|----------------|---------------|
| `LDP [path]` | Load a BASIC program (`.prg`) into memory. |
| `LDD [path]` | Load a (`.d64`) D64 disk image. |
|-|-|
| `DMP $st $en` | Dump memory from address `$st` to `$en`. |
| `SCR` | Show CPU status/registers. |
| `SVR` | Show VIC-II status/registers. |
| `SC1` | Show CIA#1 status/registers. |
| `SC2` | Show CIA#2 status/registers. |
|-|-|
| `BRD [1-16]` | Change the border color. |
| `BGR [1-16]` | Change the background color. |
| `TXT [1-16]` | Change the text color. |
|-|-|
| `CLR` | Clear the loaded program from memory **and perform a hard reset**. |
| `EXT` | Exit the emulator. |


## graphic libraries:
MetallC64 uses <a href="https://www.libsdl.org/">SDL3</a> as the main library.<br>
however the v1.0 branch still uses <a href="https://github.com/codam-coding-college/MLX42">MLX42<a>, a lightweight library on top of GLFW.

## system requirements
- **`Linux or MacOS`**
- **`readline (libreadline-dev)`**
- **`cmake >= 3.18`**
- if trying to compile the v1.0 branch mlx42 needs **`glfw (libglfw3-dev)`**

## installation
- clone this repository and cd to it
```bash
git clone git@github.com:beddinao/MetallC64.git && cd MetallC64
```
- compile everything including the SDL/MLX42 source and run the emulator
```bash
make && ./MetallC64
```

## screenshots

`animated demos`

<div align="left" width="100%">
<img  width="49%" src="./images/gifs/hole_vector-willy_the_wuzz.gif" title="hole-vector"/>
  <img  width="49%" src="./images/gifs/globe-willy_the_wuzz.gif" title="globe"/>
	<br><br>
  <img  width="49%" src="./images/gifs/the_Amiga_ball.gif" title="the AMIGA ball"/>
  <img  width="49%" src="./images/gifs/bigpixelnyan.gif" title="pixel nyan"/>
  

</div>
<br><br>

`basic games`

<div align="center" width="100%">
	<img width="49%" src="./images/screenshots/monopole.png" title="monopole" />
	<img width="49%" src="./images/screenshots/tetris.png" title="tetris" />
</div>
<br> <br>

`some art brought from the C64 demoScene`

<div align="center" width="100%">
  <img  width="49%" src="./images/screenshots/quiet_nights.png" title="quiet nights" />
  <img  width="49%" src="./images/screenshots/tetris_intro.png" title="tetris intro" />
  <img  width="49%" src="./images/screenshots/wool_on_her_mind.png" title="wool on her mind" />
  <img  width="49%" src="./images/screenshots/show_time.png" title="show time" />
  <img  width="49%" src="./images/screenshots/efucollab.png" title="eight feet under"/>
  <img  width="49%" src="./images/screenshots/chillin.png" title="chillin" />
  <img  width="49%" src="./images/screenshots/obey_the_machine.png" title="obey the machine" />
  <img  width="49%" src="./images/screenshots/honcho_of_the_seven_raging_coding_seas.png" title="honcho of the seven raging coding seas"/>
  <img  width="49%" src="./images/screenshots/Magic_Bytes.png" title="Magic Bytes" />
  <!-- <img  width="49%" src="./images/screenshots/dark_faces.png" title="dark faces" /> -->
  <img  width="49%" src="./images/screenshots/c64com_charged_mikeal_spiham.png" title="c64.com charged" />
  <img  width="49%" src="./images/screenshots/petscii_tracing.png" title="petscii tracing" />
  <img  width="49%" src="./images/screenshots/no_slackers_sky.png" title="no slackers sky" />
</div>

## **upcoming features:**  
- **Full hardware sprite support** (partial-progress)  
- **SID sound emulation** (partial-progress)

## useful-resources
- C64 Wiki: https://www.c64-wiki.com/wiki/Main_Page
- memory map: https://sta.c64.org/cbm64mem.html
- VIC-II ultimate resource: https://www.zimmers.net/cbmpics/cbm/c64/vic-ii.txt
- 6502 ultimate resource: https://www.masswerk.at/6502/6502_instruction_set.html
- 6502 BCD: http://www.6502.org/tutorials/decimal_mode.html

