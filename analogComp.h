/*
	analogComp
	This library can be used to set and manage the analog comparator
    that is integrated in several Atmel microcontrollers

	Written by Leonardo Miliani <leonardo AT leonardomiliani DOT com>

        Modified for Teensy 3.x/LC and ATtiny841/1634 by Hisashi Ito <info AT mewpro DOT cc>
        Additional improvements by Samy Kamkar <code@samy.pl>

    The latest version of this library can be found at:
    http://www.leonardomiliani.com/

	Current version: 1.2.1 - 2013/07/30
    (for a complete history of the previous versions, see the README file)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public
	License as published by the Free Software Foundation; either
	version 3.0 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*/


#ifndef ANALOG_COMP_H
#define ANALOG_COMP_H


//Library is compatible both with Arduino <=0023 and Arduino >=100
#if defined(ARDUINO) && (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

//used to set the man number of analog pins
#ifdef NUM_ANALOG_INPUTS
#undef NUM_ANALOG_INPUTS
#endif

#ifdef CMP_INTERFACES_COUNT
#undef CMP_INTERFACES_COUNT
#endif

//check if the micro is supported
#if defined (__AVR_ATmega48__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega48P__) || defined (__AVR_ATmega88P__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega328P__)
#  define ATMEGAx8
#  define NUM_ANALOG_INPUTS 6
#elif defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)
#  define ATTINYx5
#  define NUM_ANALOG_INPUTS 4
#elif defined (__AVR_ATmega8__) || defined (__AVR_ATmega8A__)
#  define ATMEGA8
#  define NUM_ANALOG_INPUTS 6
#elif defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__)
#  define ATTINYx4
#  define NUM_ANALOG_INPUTS 8
#elif defined (__AVR_ATmega640__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega1281__) || defined (__AVR_ATmega2560__) || defined (__AVR_ATmega2561__)
#  define ATMEGAx0
#  define NUM_ANALOG_INPUTS 16
#elif defined (__AVR_ATmega344__) || defined (__AVR_ATmega344P__) || defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__) || defined (__AVR_ATmega644PA__) || defined (__AVR_ATmega1284P__)
#  define ATMEGAx4
#  define NUM_ANALOG_INPUTS 8
#elif defined (__AVR_ATtiny2313__) || defined (__AVR_ATtiny4313__)
#  define ATTINYx313
#  define NUM_ANALOG_INPUTS 0
#elif defined (__AVR_ATmega32U4__)
#  define ATMEGAxU
#  define NUM_ANALOG_INPUTS 12
#elif defined (__AVR_ATtiny1634__)
#  define ATTINY1634
#  define NUM_ANALOG_INPUTS 12
#elif defined (__AVR_ATtiny441__) || defined (__AVR_ATtiny841__)
#  define ATTINYx41
#  define NUM_ANALOG_INPUTS 12
// Analog Comparator Control and Status Register A
//                 // 0        1
#  define ACD    7 // ACD0     ACD1
#  define ACBG   6 // ACPMUX2  ACBG1
#  define ACO    5 // ACO0     ACO1
#  define ACIE   3 // ACIE0    ACIE1
#  define ACIS1  1 // ACIS01   ACIS11
#  define ACIS0  0 // ACIS00   ACIS10
// Analog Comparator Control and Status Register B
//                 // 0        1
#  define ACME   2 // ACNMUX0  ACME1
#elif defined (__MKL26Z64__)
#  define CMP_INTERFACES_COUNT 1
#elif defined (__MK20DX128__)
#  define CMP_INTERFACES_COUNT 2
#elif defined (__MK20DX256__) || (__MK64FX512__) || defined (__MK66FX1M0__)
#  define CMP_INTERFACES_COUNT 3
#else
#error Sorry, microcontroller not supported!
#endif

#ifndef ADMUX
#define ADMUX ADMUXA
#endif
#ifndef ACBG
#define ACBG ACPMUX2
#endif
#ifndef ACME
#define ACME ACNMUX0
#endif

const uint8_t AIN0 = 0;
const uint8_t INTERNAL_REFERENCE = 1;
const uint8_t AIN1 = 255;


class analogComp {
    public:
        //public methods
#ifdef CMP_INTERFACES_COUNT
        analogComp(volatile uint8_t *base, void(*seton_cb)(uint8_t, uint8_t));
#else
        analogComp(volatile uint8_t *acsrxa, volatile uint8_t *acsrxb);
#endif
        uint8_t setOn(uint8_t = AIN0, uint8_t = AIN1);
        void setOff(void);
        void enableInterrupt(void (*)(void), uint8_t tempMode = CHANGE);
        void disableInterrupt(void);
        uint8_t waitComp(unsigned long = 0);
        uint8_t getOutput(void);
        uint8_t getOutputFast(void);
        void configureDac(uint8_t val);
        void (*userFunction)(void);
    private:
        uint8_t _initialized;
        uint8_t _interruptEnabled;
#ifdef CMP_INTERFACES_COUNT
        volatile uint8_t *CMP_BASE_ADDR;
        void (*_setonCb)(uint8_t, uint8_t);
#else
        volatile uint8_t *_ACSRXA; // contains ACD, ACBG (or ACPMUX2), ACO, ACIE, ACIS1, ACIS0
        volatile uint8_t *_ACSRXB; // contains ACME (or ACNMUX0)
        uint8_t oldADCSRA;
#endif
};

extern analogComp analogComparator;
#ifdef ANA_COMP1_vect
extern analogComp analogComparator1;
#endif
#if CMP_INTERFACES_COUNT > 1
extern analogComp analogComparator1;
#  if CMP_INTERFACES_COUNT > 2
extern analogComp analogComparator2;
#  endif
#endif

#endif
