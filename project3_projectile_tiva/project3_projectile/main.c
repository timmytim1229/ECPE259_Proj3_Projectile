/*
 * project3_projectile.c
 *
 * Created: 11/14/2016 11:58:19 AM
 * Author : krackletopwin
 */ 

#define F_CPU 16000000L // Specify oscillator frequency
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "minmea.h"


void gpsInit()
{
	
}

void readGPS()
{

}

int main(void)
{
    DDRB = 0b00100000; // configure pin 7 of PORTB as output (digital pin 13 on the Arduino Mega2560)
	DDRD = 0b00000010; // configure pin 1 of PORTD as output
	DDRD =
     
    while(1)
    {
        PORTB = 0b00100000; // set 7th bit to HIGH
        _delay_ms(500);
        PORTB = 0b00000000; // set 7th bit to LOW
        _delay_ms(500);
    }
}

