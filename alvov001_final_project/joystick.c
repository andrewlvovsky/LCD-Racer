/*
 * joystick.c
 *
 * Created: 8/27/2018 12:39:13 PM
 *  Author: Andrew
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "joystick.h"

#define REF_AVCC (1 <<REFS0) // Reference = AVCC = 5 V
#define REF_INT (1 << REFS0) | (1 << REFS1) //Internal reference 2.56 V
#define UPAxisInit 543
#define LRAxisInit 551

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	// in Free Running Mode, a new conversion will trigger whenever
	// the previous conversion completes.
}

void wait(long numOP){
	for( long i = 0; i < numOP; i++){
		asm("nop");
	}
}