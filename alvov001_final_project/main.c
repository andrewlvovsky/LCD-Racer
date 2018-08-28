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

unsigned char buttonOne = 0x00;
unsigned char buttonTwo = 0x00;
unsigned char buttonThree = 0x00;
unsigned char buttonFour = 0x00;

/*-------------------------- Global Variables --------------------------------*/

#define ENEMY_ASCII_SPRITE 270

unsigned char characterCursorPos;
unsigned char enemyCursorPos;
unsigned char enemyMovementTime;
unsigned char localTime;
unsigned short globalTime;

/*-------------------------- Helper Functions --------------------------------*/

void menuDisplay() {
	LCD_ClearScreen();
	LCD_DisplayString_NoClear(1, (const unsigned char *)("====== LCD ====="));
	LCD_DisplayString_NoClear(17, (const unsigned char *)("===== RACER ===="));
}

void refreshDisplay() {
	LCD_ClearScreen();
	LCD_DisplayString_NoClear(32, (const unsigned char *)(" "));	// needed for movement of characters to be seen
	LCD_Cursor(characterCursorPos);
	LCD_WriteData('>');
}

void gameOverDisplay() {
	unsigned char temp_array[6];
	
	LCD_ClearScreen();
	LCD_DisplayString_NoClear(1, "GAME OVER :(");
	LCD_DisplayString_NoClear(17, "SCORE: ");
	LCD_DisplayString_NoClear(24, LCD_To_String(globalTime, temp_array, 6));
}

/*--------------------------- Task Scheduler ---------------------------------*/

task tasks[3];
const unsigned char tasksNum = 3;
const unsigned long periodJoystick = 50;
const unsigned long periodLCD_Output = 50;
const unsigned long periodEnemy_Generator = 50;
const unsigned long tasksPeriodGCD = 50;

unsigned char processingRdyTasks = 0;

void TimerISR() {
	unsigned char i;
	if (processingRdyTasks) {
		printf("Period too short to complete tasks\n");
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
int TickFct_Enemy_Generator(int state);

enum STICK_States { STICK_INIT, STICK_WAIT, STICK_UP, STICK_DOWN, STICK_LEFT, STICK_RIGHT, STICK_GAME_OVER } STICK_State;
int TickFct_Joystick(int state) {
	switch(state) { // Transitions
		case STICK_INIT:
			characterCursorPos = 1;
			enemyCursorPos = 16;
			if(buttonFour)
				state = STICK_WAIT;
			else
				state = STICK_INIT;
			break;
		case STICK_WAIT:
			if (characterCursorPos == enemyCursorPos)
				state = STICK_GAME_OVER;	
			else if (coords[1] < JOYSTICK_INIT - DELTA) {
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
		case STICK_GAME_OVER:
			if(buttonFour)
				state = STICK_INIT;
			else
				state = STICK_GAME_OVER;
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
		case STICK_GAME_OVER:
			set_PWM(100.00);
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	return state;
}

enum SCREEN_States { SCREEN_INIT, SCREEN_UPDATE, SCREEN_GAME_OVER } SCREEN_State;
int TickFct_LCD_Output(int state) {
	switch(state) { // Transitions
		case SCREEN_INIT:
			globalTime = 0;
			menuDisplay();
			if(buttonFour)
				state = SCREEN_UPDATE;
			else
				state = SCREEN_INIT;
			break;
		case SCREEN_UPDATE:
			if(characterCursorPos == enemyCursorPos)
				state = SCREEN_GAME_OVER;
			break;
		case SCREEN_GAME_OVER:
			if(buttonFour)
				state = SCREEN_INIT;
			else
				state = SCREEN_GAME_OVER;
			break;
		default:
			state = SCREEN_INIT;
	} // Transitions

	switch(state) { // State actions
		case SCREEN_INIT:
			break;
		case SCREEN_UPDATE:
			globalTime++;
			
			fetchAnalogStick();
			refreshDisplay();
			//joystickTest(); // converts analog input to X and Y coords on LCD for debugging
			LCD_Cursor(enemyCursorPos);
			LCD_WriteData(ENEMY_ASCII_SPRITE);
			LCD_Cursor(enemyCursorPos + 18);
			LCD_WriteData(ENEMY_ASCII_SPRITE);
			break;
		case SCREEN_GAME_OVER:
			gameOverDisplay();
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	SCREEN_State = state;
	return state;
}

enum ENEMY_States { ENEMY_INIT, ENEMY_UPDATE, ENEMY_MOVE, ENEMY_GAME_OVER } ENEMY_State;
int TickFct_Enemy_Generator(int state) {
	switch(state) { // Transitions
		case ENEMY_INIT:
			localTime = 0;
			enemyMovementTime = 10;
			if(buttonFour)
				state = ENEMY_UPDATE;
			else
				state = ENEMY_INIT;
			break;
		case ENEMY_UPDATE:
			if (characterCursorPos == enemyCursorPos)
				state = ENEMY_GAME_OVER;
			else if (localTime < enemyMovementTime)
				state = ENEMY_UPDATE;
			else if (localTime == enemyMovementTime) {
				localTime = 0;
				state = ENEMY_MOVE;
			}
			break;
		case ENEMY_MOVE:
			state = ENEMY_UPDATE;
			break;
		case ENEMY_GAME_OVER:
			if(buttonFour)
				state = ENEMY_INIT;
			else
				state = ENEMY_GAME_OVER;
			break;
		default:
			state = ENEMY_INIT;
	} // Transitions

	switch(state) { // State actions
		case ENEMY_UPDATE:
			localTime++;
			break;
		case ENEMY_MOVE:
			if (globalTime % 5 == 0 && enemyMovementTime > 1)
				enemyMovementTime--;
			if (enemyCursorPos > 1)
				enemyCursorPos--;
			else
				enemyCursorPos = 16;
			break;
		case ENEMY_GAME_OVER:
			break;
		default: // ADD default behaviour below
			break;
	} // State actions
	ENEMY_State = state;
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
	ENEMY_State = ENEMY_INIT;
	
	PORTB = PORTC = 0; // Init outputs
	
	// Priority assigned to lower position tasks in array
	unsigned char i=0;
	tasks[i].state = STICK_INIT;
	tasks[i].period = periodJoystick;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Joystick;
	
	++i;
	tasks[i].state = ENEMY_INIT;
	tasks[i].period = periodEnemy_Generator;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Enemy_Generator;

	++i;
	tasks[i].state = SCREEN_INIT;
	tasks[i].period = periodLCD_Output;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_LCD_Output;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();

	while(1) {
		buttonOne = (~PINA) & 0x20;
		buttonTwo = (~PINA) & 0x10;
		buttonThree = (~PINA) & 0x08;
		buttonFour = (~PINA) & 0x04;
	} // while (1)
	
	return 0;
}