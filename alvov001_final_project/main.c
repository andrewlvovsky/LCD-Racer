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
#include "joystick.h"
#include "pwm.h"
#include "timer.h"

/*-------------------------- Global Variables --------------------------------*/
unsigned short temp;
/*--------------------------- Task Scheduler ---------------------------------*/
task tasks[2];

const unsigned char tasksNum = 2;
const unsigned long periodJoystick = 50;
const unsigned long periodLCD_Output = 100;

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

/*-------------------------------------------------------------------------*/

int TickFct_Joystick(int state);
int TickFct_LCD_Output(int state);

enum STICK_States { STICK_INIT, STICK_WAIT, STICK_UP, STICK_DOWN, STICK_LEFT, STICK_RIGHT } STICK_State;
int TickFct_Joystick(int state) {
	
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
		
			break;
		case STICK_DOWN:
		
			break;
		case STICK_LEFT:
		
			break;
		case STICK_RIGHT:
		
			break;
		default:
			state = STICK_WAIT;
	} // Transitions

	switch(STICK_State) { // State actions
		case STICK_WAIT:
			break;
		case STICK_UP:
			//PORTB = bVal;
			break;
		case STICK_DOWN:
			//PORTB = bVal;
			break;
		case STICK_LEFT:
			//PORTB = bVal;
			break;
		case STICK_RIGHT:
			//PORTB = bVal;
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	return state;
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
			fetchAnalogStick();
			joystickTest(); // converts analog input to X and Y for debugging
// 			LCD_ClearScreen();
// 			LCD_Cursor(1);
// 			LCD_WriteData('>');
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