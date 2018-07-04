******** analogComp.h  *******

analogComp

This little library can be used to set and manage the analog
comparator that is integrated in a wide variety of
Atmel microcontrollers

Written by Leonardo Miliani <leonardo AT leonardomiliani DOT com>

Add Teensy 3.x/LC support

Written by Hisashi Ito <info AT mewpro DOT cc>
Additional improvements by Samy Kamkar <code@samy.pl> / https://samy.pl

***********************
Version history

v. 1.3.3:  MK64FX512VMD12 (Teensy 3.5) and MK66FX1M0VMD18 (Teensy 3.6) support
v. 1.3.2:  ATtiny441/841 and ATtiny1634 support
           fixed a bug in handling digital buffer (changed to no touch at all)
v. 1.3.1:  MKL26Z64 (Teensy LC) support
v. 1.3.0:  MK20DX128/256 (Teensy 3.x) support
v. 1.2.1:  fixed a bug that let the ADC off after a comparison
v. 1.2.0:  fixed a bug into the conversion from analog to phisical pins
v. 1.1.1:  now it calls the correct interrupt vector for the MCU in use
v. 1.1.0:  fixed several issues with ATtinyx313 and ATtinyx5
v. 1.0.1:  fixed Atmega8/A support
v. 1.0.0:  new methods and bug fixing
v. 0.1.0:  the library can enable/disable an interrupt on AC
v. 0.0.1:  early release - prototype of the library


***********************
How to use it - Methods

Unpack the library and copy it into your /libraries folder, that usually
is in your sketchs' folder. Then include the library by adding the
following code at the top of your sketch:

#include "analogComp.h"


Now you can set what has to be connected to the inverting (AIN-) and
non-inverting (AIN+) inputs of the analog comparator. Usually, the
AIN+ is connected to external pin AIN0 and the AIN- is connected to
external pin AIN1. The AIN+ can be connected to the internal voltage
reference (usually 1V1), while the AIN- can be connected to any of the
analog input pins of the microcontroller. (see "Supported Microcontrollers"
for specific limitations)


To choose the analog comparator inputs use the method setOn():

analogComparator.setOn([AIN+, AIN-]);

For the AIN+ you can choose between the following:
AIN0: set the AIN0 pin as input
INTERNAL_REFERENCE: set the internal voltage reference as input

For the AIN- you can choose between the following:
AIN1: set the AIN1 pin as input
A0..Ax: set the Analog Input Channel (max number depends of the MCU)

AIN+ and AIN- are optionals, if not set, AIN0 and AIN1 will be used.

You can enable an interrupt routine to be executed when an event occurs:

analogComparator.enableInterrupt(myFunction[, event]);

myFunction is the name of the function to be called when the event
occurs. event can be:
CHANGE: when the comparation changes between AIN+>AIN- and AIN+<AIN-
RISING: when the voltage on AIN+ becomes greater than the voltage on AIN-
FALLING: when AIN+ becomes smaller than AIN-
event is optional, if not set CHANGE will be choosen.


You can disable the interrupt by calling this method:

analogComparator.disableInterrupt();


You can wait for a comparation to occur:

analogComparator.waitComp([timeout]);

timeout is optional and rapresents the number of milliseconds to wait
before to return (default is 5000).
This method will return false (0) if voltage on AIN- will remain
greater than the voltage on AIN+ for the whole interval specified by
timeout; it will return true (1) if AIN+ will become greater than
AIN- during the interval.

If the analog comparator won't be set up before to call the waitComp
method, by default the library will use AIN0 and AIN1 pins.


To switch off the analog comparator, call this method:

analogComparator.setOff();

It will switch off the analog comparator and will disable any enabled
interrupt.


***********************
How it works

The Analog Comparator is an operational amplifier (op-amp) integrated
into the microcontroller. An op-amp has 2 inputs: a non-inverting input,
AIN+, and an inverting input, AIN-. When the voltage that is present on
the non-inverting input is greater than the voltage that is present on
the inverting input, the op-amp will set to high its output. The signal
can be used to raise an interrupt to do an automatic operation.
Different combinations of inverting and non-inverting inputs can be
selected by manipulating the internal registers.


***********************
Supported microcontrollers

Actually the library works with a wide variety of Atmel microcontrollers
and Arduino boards:
- Attiny2313/4313*
- Attiny24/44/84
- Attiny25/45/85
- Atmega344/644/1284
- Atmega8
- Atmega48/88/168/328 (Arduino UNO)
- Atmega640/1280/1281/2560/2561 (Arduino MEGA)
- Atmega32U4** (Arduino Leonardo)
And also works with freescale microcontrollers on Teensy 3.x boards:
- MK20DX128/256***
- MK64FX512VMD12
- MK66FX1M0VMD18

Specific limitations:
* Attiny2313/4313:
due to the fact that these MCUs don't have an integrated ADC, only AIN1
is allowed for AIN-.

** Atmega32U4:
the Atmega32U4 AIN- can only be connected to an analog input pin because
it has no AIN1 pin.

*** MK20DX128/256
There are two analog comparators on MK20DX128 and three on MK20DX256.
Second or third can be accessed by the object analogComparator1 or
analogComparator2 respectively.
AIN- and AIN+ are respectively called INM and INP in freescale's terminology
and both of these can be assigned to following signals.
analogComparator (CMP0):
- 0. CMP0_IN0 (K20Pin 51 = TeensyPin 11)
- 1. CMP0_IN1 (K20Pin 52 = TeensyPin 12)
- 2. CMP0_IN2 (K20Pin 53 = TeensyPin 28)
- 3. CMP0_IN3 (K20Pin 54 = TeensyPin 27)
- 4. CMP0_IN4 (K20Pin 55 = TeensyPin 29)
- 5. VREF Output/CMP0_IN5 (K20Pin 17)
- 6. Bandgap
- 7. 6b DAC0 Reference
analogComparator1 (CMP1):
- 0. CMP1_IN0 (K20Pin 45 = TeensyPin 23)
- 1. CMP1_IN1 (K20Pin 46 = TeensyPin 9)
- 3. 12-bit DAC0_OUT/CMP1_IN3 (K20Pin 18 = TeensyPin 40/A14/DAC (Teensy 3.[12] only))
- 5. VREF Output/CMP1_IN5 (K20Pin 17) 
- 6. Bandgap
- 7. 6b DAC1 Reference
analogComparator2 (CMP2 Teensy 3.[12] only):
- 0. CMP2_IN0 (K20Pin 28 = TeensyPin 3)
- 1. CMP2_IN1 (K20Pin 29 = TeensyPin 4)
- 6. Bandgap
- 7. 6b DAC2 Reference

*** MKL26Z64
AIN- and AIN+ are respectively called INM and INP in freescale's terminology
and both of these can be assigned to following signals.
analogComparator (CMP0):
- 0. CMP0_IN0 (KL26Pin 39 = TeensyPin 11)
- 1. CMP0_IN1 (KL26Pin 40 = TeensyPin 12)
- 4. CMP0_IN4 (KL26Pin 14 = TeensyPin 26)
- 5. VREF Output/CMP0_IN5 (KL26Pin 13)
- 6. Bandgap
- 7. 6b DAC0 Reference

*** MK64FX512VMD12/MK66FX1M0VMD18
There are three analog comparators on MK64FX512VMD12/MK66FX1M0VMD18.
Second or third can be accessed by the object analogComparator1 or
analogComparator2 respectively.
AIN- and AIN+ are respectively called INM and INP in freescale's terminology
and both of these can be assigned to following signals.
analogComparator (CMP0):
- 0. CMP0_IN0                 (K64/66Pin C8  = TeensyPin 11)
- 1. CMP0_IN1                 (K64/66Pin B8  = TeensyPin 12)
- 2. CMP0_IN2                 (K64/66Pin A8  = TeensyPin 35)
- 3. CMP0_IN3                 (K64/66Pin D7  = TeensyPin 36)
- 4. 12-bit DAC1_OUT/CMP0_IN4 (K64/66Pin L4  = TeensyPin 67/A22/DAC1)
- 5. VREF Output/CMP0_IN5     (K64/66Pin M3)
- 6. Bandgap
- 7. 6b DAC0 Reference
analogComparator1 (CMP1):
- 0. CMP1_IN0                 (K64/66Pin A12 = TeensyPin 23)
- 1. CMP1_IN1                 (K64/66Pin A11 = TeensyPin 9)
- 2. ADC0_SE16/CMP1_IN2       (K64/66Pin J3  = N/C)
- 3. 12-bit DAC0_OUT/CMP1_IN3 (K64/66Pin L3  = TeensyPin 66/A21/DAC0)
- 5. VREF Output/CMP1_IN5     (K64/66Pin M3) 
- 6. Bandgap
- 7. 6b DAC1 Reference
analogComparator2 (CMP2):
- 0. CMP2_IN0                 (K64/66Pin K9  = TeensyPin 3)
- 1. CMP2_IN1                 (K64/66Pin J9  = TeensyPin 4)
- 2. ADC1_SE16/CMP2_IN2       (K64/66Pin K3  = N/C)
- 3. 12-bit DAC1_OUT/CMP2_IN3 (K64/66Pin L4  = TeensyPin 67/A22/DAC1)
- 6. Bandgap
*********************
Licence

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3.0 of the License, or (at your
option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


***********************
Document revision

9th revision: 2016/12/10
8th revision: 2014/10/20
7th revision: 2013/07/30
