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
unsigned char characterCursorPos;
unsigned char enemyCursorPos;

/*--------------------------- Task Scheduler ---------------------------------*/

task tasks[2];
const unsigned char tasksNum = 2;
const unsigned long periodJoystick = 25;
const unsigned long periodLCD_Output = 25;

const unsigned long tasksPeriodGCD = 25;

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
	switch(state) { // Transitions
		case STICK_INIT:
			state = STICK_WAIT;
			characterCursorPos = 1;
			enemyCursorPos = 16;
			break;
		case STICK_WAIT:
			if (coords[1] < JOYSTICK_INIT - DELTA) {
				if (characterCursorPos >= 17) {
					characterCursorPos = characterCursorPos - 16;
					state = STICK_UP;
				}
				else 
					state = STICK_WAIT;
			}
			else if (coords[1] > JOYSTICK_INIT + DELTA) {
				if (characterCursorPos <= 16) {
					characterCursorPos = characterCursorPos + 16;
					state = STICK_DOWN;
				}
				else
					state = STICK_WAIT;
			}
			else if (coords[0] < JOYSTICK_INIT - DELTA) {
				if (characterCursorPos > 1 && characterCursorPos != 17) {
					characterCursorPos--;
					state = STICK_LEFT;
				}
				else
					state = STICK_WAIT;
			}
			else if (coords[0] > JOYSTICK_INIT + DELTA) {
				if (characterCursorPos < 16 || (characterCursorPos > 16 && characterCursorPos < 32)) {
					characterCursorPos++;
					state = STICK_RIGHT;
				}
				else
					state = STICK_WAIT;
			}
			else {
				state = STICK_WAIT;
			}
			break;
		case STICK_UP:
			state = STICK_WAIT;
			break;
		case STICK_DOWN:
			state = STICK_WAIT;
			break;
		case STICK_LEFT:
			state = STICK_WAIT;
			break;
		case STICK_RIGHT:
			state = STICK_WAIT;
			break;
		default:
			state = STICK_WAIT;
	} // Transitions

	switch(state) { // State actions
		case STICK_WAIT:
			set_PWM(0);
			break;
		case STICK_UP:
			set_PWM(246.94);
			break;
		case STICK_DOWN:
			set_PWM(329.63);
			break;
		case STICK_LEFT:
			set_PWM(293.66);
			break;
		case STICK_RIGHT:
			set_PWM(220.00);
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	return state;
}

enum SCREEN_States { SCREEN_INIT, SCREEN_UPDATE } SCREEN_State;
int TickFct_LCD_Output(int state) {
	switch(state) { // Transitions
		case SCREEN_INIT:
			state = SCREEN_UPDATE;
			break;
		case SCREEN_UPDATE:
			break;
		default:
			state = SCREEN_INIT;
	} // Transitions

	switch(state) { // State actions
		case SCREEN_UPDATE:
			fetchAnalogStick();
			//joystickTest(); // converts analog input to X and Y for debugging
 			LCD_ClearScreen();
			LCD_DisplayString_NoClear(32, (const unsigned char *)(" "));	// needed for movement of characters to be seen
 			LCD_Cursor(characterCursorPos);
 			LCD_WriteData('>');
			//LCD_Cursor(enemyCursorPos);
			//LCD_WriteData('?');
			
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	SCREEN_State = state;
	return state;
}

int main() {
	DDRA = 0x00; PORTA = 0xFF; //Setting A to be input (Input from Analog Stick)
	DDRB = 0xFF; PORTB = 0x40; //Setting B to be output (Speaker at PB4)
	DDRC = 0xFF; PORTC = 0x00; //Setting C to be output (LCD Screen)
	DDRD = 0xFF; PORTD = 0x00; //Setting D to be output (LCD Screen)
	
	// Initializes the LCD display and ADC functionality
	LCD_init();
	ADC_init();
	PWM_on();

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