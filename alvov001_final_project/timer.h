/*
 * timer.h
 *
 * Created: 8/27/2018 12:50:28 PM
 *  Author: Andrew
 */ 


#ifndef TIMER_H_
#define TIMER_H_


typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

void TimerOn();
void TimerOff();
void TimerSet(unsigned long M);
void TimerISR();


#endif /* TIMER_H_ */