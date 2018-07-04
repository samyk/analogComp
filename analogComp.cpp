/* This file is part of the analogComp library.
   Please check the README file and the notes
   inside the analogComp.h file
*/

//include required libraries
#include "analogComp.h"

#if !defined(__MK20DX256__) && !defined(__MK20DX128__) && !defined(__MKL26Z64__) && !defined(__MK64FX512__) && !defined(__MK66FX1M0__)

analogComp::analogComp(volatile uint8_t *acsrxa, volatile uint8_t *acsrxb) :
 _initialized(0), _interruptEnabled(0), _ACSRXA(acsrxa), _ACSRXB(acsrxb) {
} 

//setting and switching on the analog comparator
uint8_t analogComp::setOn(uint8_t tempAIN0, uint8_t tempAIN1) {
    if (_initialized) { //already running
        return 1;
    }

    //initialize the analog comparator (AC)
    *_ACSRXA &= ~(1<<ACIE); //disable interrupts on AC
    *_ACSRXA &= ~(1<<ACD); //switch on the AC

    //choose the input for non-inverting input
    if (tempAIN0 == INTERNAL_REFERENCE) {
        *_ACSRXA |= (1<<ACBG); //set Internal Voltage Reference (1V1)
    } else {
        *_ACSRXA &= ~(1<<ACBG); //set pin AIN0
    }

//for Atmega32U4, only ADMUX is allowed as input for AIN-
#ifdef ATMEGAxU
    if (tempAIN1 == AIN1) {
        tempAIN1 = 0; //choose ADC0
    }
#endif

//AtTiny2313/4313 don't have ADC, so inputs are always AIN0 and AIN1
#ifndef ATTINYx313
    // allow for channel or pin numbers
#if defined(ATMEGAx0)
    if (tempAIN1 >= 54) tempAIN1 -= 54;
#elif defined(ATMEGAxU)
    if (tempAIN1 >= 18) tempAIN1 -= 18;
#elif defined(ATMEGAx4)
    if (tempAIN1 >= 24) tempAIN1 -= 24;
#elif defined(CORE_ANALOG_FIRST) && (defined(ATTINYx5) || defined(ATTINYx4) || defined(ATTINYx41) || defined(ATTINY1634))
    if (tempAIN1 >= CORE_ANALOG_FIRST) {
        tempAIN1 -= CORE_ANALOG_FIRST;
    }
#else
    if (tempAIN1 >= 14) tempAIN1 -= 14;
#endif
    //choose the input for inverting input
    oldADCSRA = ADCSRA;
    if ((tempAIN1 >= 0) && (tempAIN1 < NUM_ANALOG_INPUTS)) { //set the AC Multiplexed Input using an analog input pin
        ADCSRA &= ~(1<<ADEN);
        ADMUX &= ~31; //reset the first 5 bits
        ADMUX |= tempAIN1; //choose the ADC channel (0..NUM_ANALOG_INPUTS-1)
        *_ACSRXB |= (1<<ACME);
    } else {
        *_ACSRXB &= ~(1<<ACME); //set pin AIN1
    }
#endif

    _initialized = 1;
    return 0; //OK
}


//enable the interrupt on comparations
void analogComp::enableInterrupt(void (*tempUserFunction)(void), uint8_t tempMode) {
    if (_interruptEnabled) { //disable interrupts
        SREG &= ~(1<<SREG_I);
        *_ACSRXA &= ~(1<<ACIE);
    }

    if (!_initialized) {
        setOn(AIN0, AIN1);
    }

    //set the interrupt mode
    userFunction = tempUserFunction;
    if (tempMode == CHANGE) {
        *_ACSRXA &= ~((1<<ACIS1) | (1<<ACIS0)); //interrupt on toggle event
    } else if (tempMode == FALLING) {
        *_ACSRXA &= ~(1<<ACIS0);
        *_ACSRXA |= (1<<ACIS1);
    } else { //default is RISING
        *_ACSRXA |= ((1<<ACIS1) | (1<<ACIS0));

    }
    //enable interrupts
    *_ACSRXA |= (1<<ACIE);
    SREG |= (1<<SREG_I);
    _interruptEnabled = 1;
}


//disable the interrupt on comparations
void analogComp::disableInterrupt(void) {
    if ((!_initialized) || (!_interruptEnabled)) {
        return;
    }
    *_ACSRXA &= ~(1<<ACIE); //disable interrupt
    _interruptEnabled = 0;
}


//switch off the analog comparator
void analogComp::setOff() {
    if (_initialized) {
        if (_interruptEnabled) {
            *_ACSRXA &= ~(1<<ACIE); //disable interrupts on AC events
            _interruptEnabled = 0;
        }
        *_ACSRXA |= (1<<ACD); //switch off the AC

#ifndef ATTINYx313
        if (oldADCSRA & (1<<ADEN)) { //ADC has to be powered up
            ADCSRA |= (1<<ADEN);
        }
#endif
        _initialized = 0;
    }
}


//wait for a comparation until the function goes in timeout
uint8_t analogComp::waitComp(unsigned long _timeOut) {
    //exit if the interrupt is on
    if (_interruptEnabled) {
        return 0; //error
    }

    //no timeOut?
    if (_timeOut == 0) {
        _timeOut = 5000; //5 secs
    }

    //set up the analog comparator if it isn't
    if (!_initialized) {
        setOn(AIN0, AIN1);
        _initialized = 0;
    }

    //wait for the comparation
    unsigned long _tempMillis = millis() + _timeOut;
    do {
        if ((*_ACSRXA && (1<<ACO)) == 1) { //event raised
            return 1;
        }
    } while ((long)(millis() - _tempMillis) < 0);

    //switch off the analog comparator if it was off
    if (!_initialized) {
        setOff();
    }
    return 0;
}


//ISR (Interrupt Service Routine) called by the analog comparator when
//the user choose the raise of an interrupt
#if defined(ANA_COMP1_vect)
ISR(ANA_COMP0_vect) {
    analogComparator.userFunction(); //call the user function
}
ISR(ANA_COMP1_vect) {
    analogComparator1.userFunction(); //call the user function
}
#elif defined(ANALOG_COMP_vect)
ISR(ANALOG_COMP_vect) {
    analogComparator.userFunction(); //call the user function
}
#else
ISR(ANA_COMP_vect) {
    analogComparator.userFunction(); //call the user function
}
#endif

#else

#define CMP_SCR_DMAEN   ((uint8_t)0x40) // DMA Enable Control
#define CMP_SCR_IER     ((uint8_t)0x10) // Comparator Interrupt Enable Rising
#define CMP_SCR_IEF     ((uint8_t)0x08) // Comparator Interrupt Enable Falling
#define CMP_SCR_CFR     ((uint8_t)0x04) // Analog Comparator Flag Rising
#define CMP_SCR_CFF     ((uint8_t)0x02) // Analog Comparator Flag Falling
#define CMP_SCR_COUT    ((uint8_t)0x01) // Analog Comparator Output
#define CMPx_CR0        0 // CMP Control Register 0
#define CMPx_CR1        1 // CMP Control Register 1
#define CMPx_FPR        2 // CMP Filter Period Register
#define CMPx_SCR        3 // CMP Status and Control Register
#define CMPx_DACCR      4 // DAC Control Register
#define CMPx_MUXCR      5 // MUX Control Register

analogComp::analogComp(volatile uint8_t *base, void(*seton_cb)(uint8_t, uint8_t)) :
 _initialized(0), _interruptEnabled(0), CMP_BASE_ADDR(base), _setonCb(seton_cb) {
} 

//setting and switching on the analog comparator
uint8_t analogComp::setOn(uint8_t tempAIN0, uint8_t tempAIN1) {
    if (_initialized) { //already running
        return 1;
    }

    // clock gate: comparator clock set on. it is off by default for conserving power.
    SIM_SCGC4 |= SIM_SCGC4_CMP;
    _setonCb(tempAIN0, tempAIN1);
    CMP_BASE_ADDR[CMPx_CR1] = 0; // set CMPx_CR1 to a known state
    CMP_BASE_ADDR[CMPx_CR0] = 0; // set CMPx_CR0 to a known state
    CMP_BASE_ADDR[CMPx_CR1] = B00000001; // enable comparator
    CMP_BASE_ADDR[CMPx_MUXCR] = tempAIN0 << 3 | tempAIN1; // set comparator input PSEL=tempAIN0 / MSEL=tempAIN1

    _initialized = 1;
    return 0; //OK
}


//enable the interrupt on comparations
void analogComp::enableInterrupt(void (*tempUserFunction)(void), uint8_t tempMode) {
    if (_interruptEnabled) { //disable interrupts
        CMP_BASE_ADDR[CMPx_SCR] = 0;
    }

    if (!_initialized) {
        setOn(0, 1);
    }

    //set the interrupt mode
    userFunction = tempUserFunction;
    if (tempMode == CHANGE) {
        CMP_BASE_ADDR[CMPx_SCR] = CMP_SCR_IER | CMP_SCR_IEF;
    } else if (tempMode == FALLING) {
        CMP_BASE_ADDR[CMPx_SCR] = CMP_SCR_IEF;
    } else { //default is RISING
        CMP_BASE_ADDR[CMPx_SCR] = CMP_SCR_IER;
    }
    _interruptEnabled = 1;
}


//disable the interrupt on comparations
void analogComp::disableInterrupt(void) {
    if ((!_initialized) || (!_interruptEnabled)) {
        return;
    }
    CMP_BASE_ADDR[CMPx_SCR] = 0; //disable interrupt
    _interruptEnabled = 0;
}


//switch off the analog comparator
void analogComp::setOff() {
    if (_initialized) {
        if (_interruptEnabled) {
            CMP_BASE_ADDR[CMPx_SCR] = 0; //disable interrupts on AC events
            _interruptEnabled = 0;
        }
        CMP_BASE_ADDR[CMPx_CR1] = 0; //switch off the AC
        _initialized = 0;
    }
}


//wait for a comparation until the function goes in timeout
uint8_t analogComp::waitComp(unsigned long _timeOut) {
    //exit if the interrupt is on
    if (_interruptEnabled) {
        return 0; //error
    }

    //no timeOut?
    if (_timeOut == 0) {
        _timeOut = 5000; //5 secs
    }

    //set up the analog comparator if it isn't
    if (!_initialized) {
        setOn(0, 1);
        _initialized = 0;
    }

    //wait for the comparation
    unsigned long _tempMillis = millis() + _timeOut;
    do {
        if ((CMP_BASE_ADDR[CMPx_SCR] & CMP_SCR_COUT)) { //event raised
            return 1;
        }
    } while ((long)(millis() - _tempMillis) < 0);

    //switch off the analog comparator if it was off
    if (!_initialized) {
        setOff();
    }
    return 0;
}

// fast response from comparator
uint8_t analogComp::getOutputFast() {
  return CMP_BASE_ADDR[CMPx_SCR] & CMP_SCR_COUT;
}

// get output of comparator
uint8_t analogComp::getOutput() {
	
	uint8_t _output = 0;
	
    //set up the analog comparator if it isn't
    if (!_initialized) {
        setOn(0, 1);
        _initialized = 0;
    }

    if ((CMP_BASE_ADDR[CMPx_SCR] & CMP_SCR_COUT)) { //check if output flag is set
        _output = 1;
	}

    //switch off the analog comparator if it was off
    if (!_initialized) {
        setOff();
    }
    return _output;
}

void analogComp::configureDac(uint8_t val)
{
	uint8_t tmp8 = 0U;
	
	if (val == 0)
	{
		CMP0_DACCR = 0U;
		return;
	}

    /* CMPx_DACCR. */
    tmp8 |= CMP_DACCR_DACEN; /* Enable the internal DAC. */
    tmp8 |= CMP_DACCR_VRSEL; /* Vin2 */
    tmp8 |= CMP_DACCR_VOSEL(val);

    CMP0_DACCR = tmp8;
}


//ISR (Interrupt Service Routine) called by the analog comparator when
//the user choose the raise of an interrupt
void cmp0_isr() {
    CMP0_SCR |= CMP_SCR_CFR | CMP_SCR_CFF; // clear CFR and CFF
    analogComparator.userFunction(); //call the user function
}

static void CMP0_Init(uint8_t inp, uint8_t inm) {
    NVIC_SET_PRIORITY(IRQ_CMP0, 64); // 0 = highest priority, 255 = lowest
    NVIC_ENABLE_IRQ(IRQ_CMP0); // handler is now cmp0_isr()
    if (inp == 0 || inm == 0) { // CMP0_IN0 (Teensy 3.x, Teensy LC: pin11)
        CORE_PIN11_CONFIG = PORT_PCR_MUX(0); // set pin11 function to ALT0
    }
    if (inp == 1 || inm == 1) { // CMP0_IN1 (Teensy 3.x, Teensy LC: pin12)
        CORE_PIN12_CONFIG = PORT_PCR_MUX(0); // set pin12 function to ALT0
    }
    if (inp == 2 || inm == 2) { // CMP0_IN2 (Teensy 3.[012]: pin28; Teensy 3.[56]: pin35; Teensy LC: N/A)
#if defined(__MK20DX256__) || defined(__MK20DX128__) // Teensy 3.[012]
        CORE_PIN28_CONFIG = PORT_PCR_MUX(0); // set pin28 function to ALT0
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__) // Teensy 3.[56]
        CORE_PIN35_CONFIG = PORT_PCR_MUX(0); // set pin35 function to ALT0
#else // Teensy LC
        ; // do nothing
#endif
    }
    if (inp == 3 || inm == 3) { // CMP0_IN3 (Teensy 3.[012]: pin27; Teensy 3.[56]: pin 36; Teensy LC: N/A)
#if defined(__MK20DX256__) || defined(__MK20DX128__) // Teensy 3.[012]
        CORE_PIN27_CONFIG = PORT_PCR_MUX(0); // set pin27 function to ALT0
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__) // Teensy 3.[56]
        CORE_PIN36_CONFIG = PORT_PCR_MUX(0); // set pin36 function to ALT0
#else // Teensy LC
        ; // do nothing
#endif
    }
    if (inp == 4 || inm == 4) { // CMP0_IN4 (Teensy 3.[012]: pin29; Teensy 3.[56]: pin 67; Teensy LC: pin26)
#if defined(__MK20DX256__) || defined(__MK20DX128__) // Teensy 3.[012]
        CORE_PIN29_CONFIG = PORT_PCR_MUX(0); // set pin29 function to ALT0
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__) // Teensy 3.[56]
        // CORE_PIN67_CONFIG = PORT_PCR_MUX(0); // set pin67 function to ALT0
#else // Teensy LC
        CORE_PIN26_CONFIG = PORT_PCR_MUX(0); // set pin26 function to ALT0
#endif
    }
    if (inp == 5 || inm == 5) { // VREF Output/CMP0_IN5
        ; // do nothing
    }
    if (inp == 6 || inm == 6) { // Bandgap
        ; // do nothing
    }
    if (inp == 7 || inm == 7) { // 6b DAC0 Reference
        ; // do nothing
    }
}

#if CMP_INTERFACES_COUNT > 1 // Teensy 3.x
void cmp1_isr() {
    CMP1_SCR |= CMP_SCR_CFR | CMP_SCR_CFF; // clear CFR and CFF
    analogComparator1.userFunction(); //call the user function
}

static void CMP1_Init(uint8_t inp, uint8_t inm) {
    NVIC_SET_PRIORITY(IRQ_CMP1, 64); // 0 = highest priority, 255 = lowest
    NVIC_ENABLE_IRQ(IRQ_CMP1); // handler is now cmp1_isr()
    if (inp == 0 || inm == 0) { // CMP1_IN0 (pin23)
        CORE_PIN23_CONFIG = PORT_PCR_MUX(0); // set pin23 function to ALT0
    }
    if (inp == 1 || inm == 1) { // CMP1_IN1 (pin9)
        CORE_PIN9_CONFIG = PORT_PCR_MUX(0); // set pin9 function to ALT0
    }
    if (inp == 3 || inm == 3) { // 12-bit DAC0_OUT/CMP1_IN3 (Teensy 3.[012]: pin 40; Teensy 3.[56]: pin 66)
#if defined(__MK20DX256__) || defined(__MK20DX128__) // Teensy 3.[012]
        // CORE_PIN40_CONFIG = PORT_PCR_MUX(0); // set pin40 function to ALT0
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__) // Teensy 3.[56]
        // CORE_PIN66_CONFIG = PORT_PCR_MUX(0); // set pin66 function to ALT0
#else // Teensy LC
        ; // never reach here
#endif
    }
    if (inp == 5 || inm == 5) { // VREF Output/CMP1_IN5
        ; // do nothing
    }
    if (inp == 6 || inm == 6) { // Bandgap
        ; // do nothing
    }
    if (inp == 7 || inm == 7) { // 6b DAC1 Reference
        ; // do nothing
    }
}

#if CMP_INTERFACES_COUNT > 2
void cmp2_isr() {
    CMP2_SCR |= CMP_SCR_CFR | CMP_SCR_CFF; // clear CFR and CFF
    analogComparator2.userFunction(); //call the user function
}

static void CMP2_Init(uint8_t inp, uint8_t inm) {
    NVIC_SET_PRIORITY(IRQ_CMP2, 64); // 0 = highest priority, 255 = lowest
    NVIC_ENABLE_IRQ(IRQ_CMP2); // handler is now cmp2_isr()
    if (inp == 0 || inm == 0) { // CMP2_IN0 (pin3)
        CORE_PIN3_CONFIG = PORT_PCR_MUX(0); // set pin3 function to ALT0
    }
    if (inp == 1 || inm == 1) { // CMP2_IN1 (pin4)
        CORE_PIN4_CONFIG = PORT_PCR_MUX(0); // set pin4 function to ALT0
    }
    if (inp == 3 || inm == 3) { // CMP2_IN3 (Teensy 3.[56]: pin 67)
#if defined(__MK20DX256__) || defined(__MK20DX128__) // Teensy 3.[012]
// this setting exists at the CMP section of the manual but
// non-existent in "K20 Signal Multiplexing and Pin Assignments"
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__) // Teensy 3.[56]
        // CORE_PIN67_CONFIG = PORT_PCR_MUX(0); // set pin67 function to ALT0
#else // Teensy LC
        ; // never reach here
#endif
    }
    if (inp == 6 || inm == 6) { // Bandgap
        ; // do nothing
    }
    if (inp == 7 || inm == 7) { // 6b DAC2 Reference
        ; // do nothing
    }
}
#endif
#endif

#endif

#ifndef CMP_INTERFACES_COUNT
#  if defined(SFIOR)
analogComp analogComparator(&ACSR, &SFIOR);
#  elif defined(ACSRA)
analogComp analogComparator(&ACSRA, &ADCSRB);
#  elif defined(ACSR0A)
analogComp analogComparator(&ACSR0A, &ACSR0B);
#  else
analogComp analogComparator(&ACSR, &ADCSRB);
#  endif
#  if defined(ANA_COMP1_vect)
analogComp analogComparator1(&ACSR1A, &ACSR1B);
#  endif
#else
analogComp analogComparator(&CMP0_CR0, CMP0_Init);
#  if CMP_INTERFACES_COUNT > 1
analogComp analogComparator1(&CMP1_CR0, CMP1_Init);
#    if CMP_INTERFACES_COUNT > 2
analogComp analogComparator2(&CMP2_CR0, CMP2_Init);
#    endif
#  endif
#endif
