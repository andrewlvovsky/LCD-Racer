/*
 * alvov001_final_project.c
 *
 * Created: 8/23/2018 3:42:44 PM
 * Author : Andrew
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "io.h"
#include "useful.h"

#define REF_AVCC (1 <<REFS0) // Reference = AVCC = 5 V
#define REF_INT (1 << REFS0) | (1 << REFS1) //Internal reference 2.56 V
#define UPAxisInit 543
#define LRAxisInit 551

void wait(long numOP);

/*Define user variables and functions for your state machines here.*/
unsigned short message;
unsigned short val1;
unsigned short val2;
unsigned short temp;
unsigned char temp_array[32];
unsigned char bVal;

task tasks[2];

const unsigned char tasksNum = 2;
const unsigned long periodJoystick = 50;
const unsigned long periodLCD_Output = 50;

const unsigned long tasksPeriodGCD = 50;

unsigned char processingRdyTasks = 0;
void TimerISR() {
	unsigned char i;
	if (processingRdyTasks) {
		printf("Period too short to complete boobs\n");
	}
	processingRdyTasks = 1;
	for (i = 0; i < tasksNum; ++i) { // Heart of scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
	processingRdyTasks = 0;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

int TickFct_Joystick(int state);
int TickFct_LCD_Output(int state);

enum STICK_States { STICK_INIT, STICK_WAIT, STICK_UP, STICK_DOWN, STICK_LEFT, STICK_RIGHT } STICK_State;
int TickFct_Joystick(int state) {
	bVal = 0x00;
	
	//ADMUX = REF_AVCC | 0x00;
	//wait(300);
	//val1 = ADC;
	//ADMUX = REF_AVCC | 0x01;
	//wait(300);
	//val2 = ADC;
	
	switch(STICK_State) { // Transitions
		case STICK_INIT:
			//LCD_DisplayString(1, "I'm here");
			state = STICK_WAIT;
			//LCD_DisplayString(1, "You are my puppyGive me the bone");
			break;
		case STICK_WAIT:
			//LCD_DisplayString(1, "You are cool");
			//if (val2 < LRAxisInit-10) {
				//state = STICK_UP;
				//message = val2;
				//bVal |= 0x08;
				//temp = val2;
			//}
			//else if (val2 > LRAxisInit+10) {
				//state = STICK_DOWN;
				//message = val2;
				//bVal |= 0x04;
				//temp = val2;
			//}
			//else if (val1 > UPAxisInit+10) {
				//state = STICK_LEFT;
				//message = val1;
				//bVal |= 0x01;
				//temp = val1;
			//}
			//else if (val1 < UPAxisInit-10) {
				//state = STICK_RIGHT;
				//message = val1;
				//bVal |= 0x02;
				//temp = val1;
			//}
			break;
		case STICK_UP:
			if (temp != val2) {
				state = STICK_WAIT;
			}
			break;
		case STICK_DOWN:
			if (temp != val2) {
				state = STICK_WAIT;
			}
			break;
		case STICK_LEFT:
			if (temp != val1) {
				state = STICK_WAIT;
			}
			break;
		case STICK_RIGHT:
			if (temp != val1) {
				state = STICK_WAIT;
			}
			break;
		default:
			state = STICK_WAIT;
	} // Transitions

	switch(STICK_State) { // State actions
		case STICK_WAIT:
			break;
		case STICK_UP:
			PORTB = bVal;
			break;
		case STICK_DOWN:
			PORTB = bVal;
			break;
		case STICK_LEFT:
			PORTB = bVal;
			break;
		case STICK_RIGHT:
			PORTB = bVal;
			break;
		default: // ADD default behaviour below
			break;
	} // State actions

}

enum SCREEN_States { SCREEN_INIT, SCREEN_WAIT } SCREEN_State;
int TickFct_LCD_Output(int state) {
	switch(state) { // Transitions
		case SCREEN_INIT:
			state = SCREEN_WAIT;
			break;
		case SCREEN_WAIT:
			break;
		default:
			state = SCREEN_INIT;
	} // Transitions

	switch(state) { // State actions
		case SCREEN_WAIT:
			LCD_DisplayString(1, LCD_To_String(ADC, temp_array, 4));
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	SCREEN_State = state;
	return state;
}

int main() {
	DDRA = 0x00; PORTA = 0xFF; //Setting A to be input (Input from Analog Stick)
	DDRB = 0xFF; PORTB = 0x00; //Setting B to be output (Debug LEDs for Analog Stick)
	DDRC = 0xFF; PORTC = 0x00; //Setting C to be output (LCD Screen)
	DDRD = 0xFF; PORTD = 0x00; //Setting D to be output (LCD Screen)
	
	// Initializes the LCD display and ADC functionality
	LCD_init();
	ADC_init();

	STICK_State = STICK_INIT; // Initial state
	SCREEN_State = SCREEN_INIT;
	
	PORTB = PORTC = 0; // Init outputs
	
	// Priority assigned to lower position tasks in array
	unsigned char i=0;
	tasks[i].state = STICK_INIT;
	tasks[i].period = periodJoystick;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Joystick;

	++i;
	tasks[i].state = SCREEN_INIT;
	tasks[i].period = periodLCD_Output;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_LCD_Output;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();

	while(1) {
	} // while (1)
	
	return 0;
}

void wait(long numOP){
	for( long i = 0; i < numOP; i++){
		asm("nop");
	}
}