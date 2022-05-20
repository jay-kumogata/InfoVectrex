# InfoVectrex

## Introduction
InfoVectrex is a Vectrex emulator that runs on Windows.

<img src="https://github.com/jay-kumogata/InfoVectrex/blob/master/screenshots/kousokusen_060717_1.PNG" width="200">

## How to run

- Put the BIOS file (rom.dat) in the same directory as the executable file (bin/InfoVectrex.exe).
- The BIOS file should be in the same format as vecx (without header).
- Double-click the executable file (bin/InfoVectrex.exe).

## How to play

### Joystick # 1

- [up-arrow]: Up button
- [down-arrow]: Down button
- [right-arrow]: Right button
- [left-arrow]: Left button
- [A]: Button # 1
- [S]: Button # 2
- [D]: Button # 3
- [F]: Button # 4

### System
- [0]: Maximum speed (line width: 0)
- [1]: High speed (line width: 1)
- [2]: Medium speed (line width: 2)

## Specification

### CPU
- 6809 opcodes
- FIRQ / IRQ / NMI

### VIA
- 6522 registers, port A and port B
- T1 timer (one-shot mode and free-run mode)
- T2 timer (one-shot mode)
- Shift register (modes #2 and #6)

### VG
- Integrator (X coordinate, Y coordinate, Offset and Joystick)
- Multiplexer
- \~RAMP signal and \~ZERO signal
- Joystick #1, Buttons #1, #2, #3 and #4

## Disclaimer
InfoVectrex is freeware. There is no guarantee. 
The author is not responsible for any damages caused by this software.
Vectrex is a trademark and brand of General Consumer Electronics Corporation.
