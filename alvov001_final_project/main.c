/*
 * LCD Racer - CS 120B Final Project
 *
 * Created: 8/23/2018 3:42:44 PM
 * Author : Andrew Lvovsky
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>	// for random generator
#include "io.h"
#include "joystick.h"
#include "pwm.h"
#include "timer.h"
#include "character.h"

/*------------------------ Button Initialization -----------------------------*/

unsigned char buttonOne = 0x00;
unsigned char buttonTwo = 0x00;
unsigned char buttonThree = 0x00;
unsigned char buttonFour = 0x00;

/*-------------------------- Global Variables --------------------------------*/

#define TASKS_NUM 4
#define MAX_ENEMIES 4
#define MENU_REFRESH_TIME 15

const unsigned short NON_SHOOTABLE_SPRITE = 270;
const unsigned short SHOOTABLE_SPRITE = 250;
const unsigned short BULLET_SPRITE = 45;

unsigned char characterCursorPos;
unsigned char enemyMovementTime, enemyMovementFactor = 5;
unsigned char localTime, menuTime = 0;
unsigned short scoreTime, globalTime;
unsigned char dontSpawnTop = 0; 
unsigned char dontSpawnBottom = 0;
unsigned char bulletOnScreen = 0;
unsigned char twoPlayerEnabled = 0;

npc enemies[MAX_ENEMIES];
npc bullet;

/*-------------------------- Helper Functions --------------------------------*/

void titleDisplay() {
	LCD_ClearScreen();
	LCD_DisplayString_NoClear(1, (const unsigned char *)("====== LCD ====="));
	LCD_DisplayString_NoClear(17, (const unsigned char *)("===== RACER ===="));
}

void menuDisplay() {
	LCD_ClearScreen();
	LCD_DisplayString_NoClear(1, (const unsigned char *)("R for One Player"));
	LCD_DisplayString_NoClear(17, (const unsigned char *)("Y for Two Player"));
}

void refreshDisplay() {
	LCD_ClearScreen();
	LCD_DisplayString_NoClear(32, (const unsigned char *)(" "));	// needed for movement of characters to be seen
	LCD_Cursor(characterCursorPos);
	LCD_WriteData('>');
	LCD_Cursor(bullet.cursorPos);
	LCD_WriteData(bullet.sprite);
	for (unsigned char i = 0; i < MAX_ENEMIES; i++) {
		LCD_Cursor(enemies[i].cursorPos);
		LCD_WriteData(enemies[i].sprite);
	}
	
}

void gameOverDisplay() {
	unsigned char temp_array[6];
	
	LCD_ClearScreen();
	LCD_DisplayString_NoClear(1, (const unsigned char *)("GAME OVER :("));
	LCD_DisplayString_NoClear(17, (const unsigned char *)("SCORE: "));
	LCD_DisplayString_NoClear(24, LCD_To_String(scoreTime, temp_array, 6));
}

void initCharacters() {
	for(unsigned char i = 0; i < MAX_ENEMIES; i++) {
		if(i % 2 == 0)
			enemies[i].sprite = NON_SHOOTABLE_SPRITE;
		else
			enemies[i].sprite = SHOOTABLE_SPRITE;
		enemies[i].cursorPos = 0;
	}
	
	bullet.cursorPos = 0;
	bullet.sprite = BULLET_SPRITE;
}

/*--------------------------- Task Scheduler ---------------------------------*/

task tasks[TASKS_NUM];
const unsigned long periodJoystick = 50;
const unsigned long periodShoot = 25;
const unsigned long periodLCD_Output = 50;
const unsigned long periodEnemy_Generator = 50;
const unsigned long tasksPeriodGCD = 25;

void TimerISR() {
	unsigned char i;
	for (i = 0; i < TASKS_NUM; ++i) { // Heart of scheduler code
		if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
}

/*-------------------------------------------------------------------------*/

int TickFct_Joystick(int state);
int TickFct_Shoot(int state);
int TickFct_LCD_Output(int state);
int TickFct_Enemy_Generator(int state);

enum STICK_States { STICK_INIT, STICK_BUTTON_RELEASE, STICK_WAIT, STICK_UP, STICK_DOWN, STICK_LEFT, STICK_RIGHT, STICK_GAME_OVER } STICK_State;
int TickFct_Joystick(int state) {
	switch(state) { // Transitions
		case STICK_INIT:
			characterCursorPos = 1;
			if(buttonOne)
				state = STICK_WAIT;
			else
				state = STICK_INIT;
			break;
		case STICK_BUTTON_RELEASE:
			if(buttonOne)
				state = STICK_BUTTON_RELEASE;
			else
				state = STICK_INIT;
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
			else
				state = STICK_WAIT;
			for (unsigned char i = 0; i < MAX_ENEMIES; i++) {
				if (characterCursorPos == enemies[i].cursorPos) {
					state = STICK_GAME_OVER;
					break;
				}
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
			if(buttonOne)
				state = STICK_BUTTON_RELEASE;
			else
				state = STICK_GAME_OVER;
			break;
		default:
			state = STICK_WAIT;
	} // Transitions

	switch(state) { // State actions
		case STICK_INIT:
			set_PWM(0);
			break;
		case STICK_WAIT:
			set_PWM(0);
			break;
		case STICK_BUTTON_RELEASE:
			set_PWM(400.00);
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

enum SHOOT_States { SHOOT_INIT, SHOOT_BUTTON_RELEASE, SHOOT_WAIT, SHOOT_GO, SHOOT_GAME_OVER } SHOOT_State;
int TickFct_Shoot(int state) {
	switch(state) { // Transitions
		case SHOOT_INIT:
			if(buttonOne)
				state = SHOOT_WAIT;
			else
				state = SHOOT_INIT;
			break;
		case SHOOT_BUTTON_RELEASE:
			if(buttonOne)
				state = SHOOT_BUTTON_RELEASE;
			else
				state = SHOOT_INIT;
			break;
		case SHOOT_WAIT:
			if (buttonFour || bulletOnScreen)
				state = SHOOT_GO;
			else
				state = SHOOT_WAIT;
			break;
		case SHOOT_GO:
			state = SHOOT_WAIT;
			break;
		case SHOOT_GAME_OVER:
			if(buttonOne)
				state = STICK_BUTTON_RELEASE;
			else
				state = STICK_GAME_OVER;
			break;
		default:
			break;
	} // Transitions
	
	switch(state) { // State actions
		case SHOOT_INIT:
			break;
		case SHOOT_BUTTON_RELEASE:
			break;
		case SHOOT_WAIT:
			break;
		case SHOOT_GO:
			if(bullet.cursorPos == 0) {
				set_PWM(500.00);
				bulletOnScreen = 1;
				bullet.cursorPos = characterCursorPos + 1;	//spawn bullet in front of character
			}
			else if (bullet.cursorPos != 0) {
				if (bullet.cursorPos != 16 && bullet.cursorPos != 32 && bulletOnScreen)
					bullet.cursorPos++;
				else {
					bullet.cursorPos = 0;	// bullet didn't hit any enemies, so put bullet back
					bulletOnScreen = 0;
				}
				for (unsigned char i = 0; i < MAX_ENEMIES; i++) {
					if (bullet.cursorPos == enemies[i].cursorPos && enemies[i].sprite == NON_SHOOTABLE_SPRITE) {
						bullet.cursorPos = 0;
						bulletOnScreen = 0;
						break;
					}
					if (bullet.cursorPos == enemies[i].cursorPos && enemies[i].sprite == SHOOTABLE_SPRITE) {
						enemies[i].cursorPos = 0;
						bullet.cursorPos = 0;
						bulletOnScreen = 0;
						break;
					}
				}
			}
			break;
		case SHOOT_GAME_OVER:
			break;
		default:
			break;
	} // State actions
	
	SHOOT_State = state;
	return state;
}

enum SCREEN_States { SCREEN_INIT, SCREEN_MENU, SCREEN_BUTTON_RELEASE, SCREEN_UPDATE, SCREEN_GAME_OVER } SCREEN_State;
int TickFct_LCD_Output(int state) {
	switch(state) { // Transitions
		case SCREEN_INIT:
			scoreTime = 0;
			if(buttonOne && menuTime < MENU_REFRESH_TIME) {
				menuTime = 0;
				state = SCREEN_UPDATE;
			}
			else if (!buttonOne && menuTime < MENU_REFRESH_TIME)
				state = SCREEN_INIT;
			else if (!buttonOne && menuTime == MENU_REFRESH_TIME) {
				menuTime = 0;
				state = SCREEN_MENU;
			}
			break;
		case SCREEN_MENU:
			if(buttonOne && menuTime < MENU_REFRESH_TIME) {
				menuTime = 0;
				state = SCREEN_UPDATE;
			}
			else if (!buttonOne && menuTime < MENU_REFRESH_TIME)
				state = SCREEN_MENU;
			else if (!buttonOne && menuTime == MENU_REFRESH_TIME) {
				menuTime = 0;
				state = SCREEN_INIT;
			}
			break;
		case SCREEN_BUTTON_RELEASE:
			if(buttonOne)
				state = SCREEN_BUTTON_RELEASE;
			else
				state = SCREEN_INIT;
			break;
		case SCREEN_UPDATE:
			for (unsigned char i = 0; i < MAX_ENEMIES; i++) {
				if(characterCursorPos == enemies[i].cursorPos)
					state = SCREEN_GAME_OVER;
			}
			break;
		case SCREEN_GAME_OVER:
			if(buttonOne)
				state = SCREEN_BUTTON_RELEASE;
			else
				state = SCREEN_GAME_OVER;
			break;
		default:
			state = SCREEN_INIT;
	} // Transitions

	switch(state) { // State actions
		case SCREEN_INIT:
			globalTime++;
			menuTime++;
			titleDisplay();
			break;
		case SCREEN_MENU:
			globalTime++;
			menuTime++;
			menuDisplay();
			break;
		case SCREEN_BUTTON_RELEASE:
			break;
		case SCREEN_UPDATE:
			scoreTime++;
			globalTime++;
			fetchAnalogStick();
			refreshDisplay();
			//joystickTest(); // converts analog input to X and Y coords on LCD for debugging
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

enum ENEMY_States { ENEMY_INIT, ENEMY_BUTTON_RELEASE, ENEMY_UPDATE, ENEMY_MOVE, ENEMY_GAME_OVER } ENEMY_State;
int TickFct_Enemy_Generator(int state) {
	switch(state) { // Transitions
		case ENEMY_INIT:
			localTime = 0;
			enemyMovementTime = 10;
			if(buttonOne) {
				initCharacters();
				srand(globalTime);
				state = ENEMY_UPDATE;
			}
			else
				state = ENEMY_INIT;
			break;
		case ENEMY_BUTTON_RELEASE:
			if(buttonOne)
				state = ENEMY_BUTTON_RELEASE;
			else
				state = ENEMY_INIT;
			break;
		case ENEMY_UPDATE:
			if (localTime < enemyMovementTime)
				state = ENEMY_UPDATE;
			else if (localTime == enemyMovementTime) {
				localTime = 0;
				state = ENEMY_MOVE;
			}
			for (unsigned char i = 0; i < MAX_ENEMIES; i++) {
				if (characterCursorPos == enemies[i].cursorPos) {
					state = ENEMY_GAME_OVER;
					break;
				}
			}
			break;
		case ENEMY_MOVE:
			state = ENEMY_UPDATE;
			break;
		case ENEMY_GAME_OVER:
			if(buttonOne)
				state = ENEMY_BUTTON_RELEASE;
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
		case ENEMY_BUTTON_RELEASE:
			break;
		case ENEMY_MOVE:
			if (scoreTime % enemyMovementFactor == 0 && enemyMovementTime > 1) {
				enemyMovementTime--;
			}
			
			for(unsigned char i = 0; i < MAX_ENEMIES; i++) {
				if ((enemies[i].cursorPos > 1 && enemies[i].cursorPos < 18) || (enemies[i].cursorPos >  17 && enemies[i].cursorPos <= 33))
					enemies[i].cursorPos--;
				else
					enemies[i].cursorPos = 0;
			}
			
			for(unsigned char i = 0; i < MAX_ENEMIES; i++) {
				if (!dontSpawnBottom && (enemies[i].cursorPos == 31 || enemies[i].cursorPos == 32 || enemies[i].cursorPos == 33 || enemies[i].cursorPos == 16 || enemies[i].cursorPos == 17)) {
					dontSpawnBottom = 1;
				}
				if (!dontSpawnTop && (enemies[i].cursorPos == 15 || enemies[i].cursorPos == 16 || enemies[i].cursorPos == 17 || enemies[i].cursorPos == 32 || enemies[i].cursorPos == 33)) {
					dontSpawnTop = 1;
				}
			}
			for(unsigned char i = 0; i < MAX_ENEMIES && rand() % 2; i++) {
				if (enemies[i].cursorPos == 0 && dontSpawnBottom == 0) {
					enemies[i].cursorPos = 33;
				}
				if (enemies[i].cursorPos == 0 && dontSpawnTop == 0) {
					enemies[i].cursorPos = 17;
				}=
			}
			
			dontSpawnBottom = dontSpawnTop = 0;
			
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
	
	globalTime = 0;

	STICK_State = STICK_INIT; // Initial state
	SCREEN_State = SCREEN_INIT;
	ENEMY_State = ENEMY_INIT;
	SHOOT_State = SHOOT_INIT;
	
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
	tasks[i].state = SHOOT_INIT;
	tasks[i].period = periodShoot;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_Shoot;

	++i;
	tasks[i].state = SCREEN_INIT;
	tasks[i].period = periodLCD_Output;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_LCD_Output;
	
	TimerSet(tasksPeriodGCD);
	TimerOn();

	while(1) {
		buttonOne = (~PINA) & 0x20;		//PA5
		buttonTwo = (~PINA) & 0x10;		//PA4
		buttonThree = (~PINA) & 0x08;	//PA3
		buttonFour = (~PINA) & 0x04;	//PA2
	} // while (1)
	
	return 0;
}