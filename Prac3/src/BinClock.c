/*
 * BinClock.c
 * Jarrod Olivier
 * Modified for EEE3095S/3096S by Keegan Crankshaw
 * August 2019
 *
 * RDXNIC008 HYSMAT002
 * 12/08/2019
*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions
#include <errno.h>
#include <math.h>

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int hours, mins, secs;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance

int HH,MM,SS;


// Function declarations
void turnOffOscillator(void);
void turnOnOscillator(void);
// Write functions
int write12Hour(int value, int pm);
int writeMin(int value);
int writeSec(int Value);
// Get Functions
int getHour(void);
int getMin(void);
int getSec(void);

void initGPIO(void){
	/*
	 * Sets GPIO using wiringPi pins. see pinout.xyz for specific wiringPi pins
	 * You can also use "gpio readall" in the command line to get the pins
	 * Note: wiringPi does not use GPIO or board pin numbers (unless specifically set to that mode)
	 */
	printf("Setting up\n");
	wiringPiSetup(); //This is the default mode. If you want to change pinouts, be aware

	RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

	//Set up the LEDS
	for(int i = 0; i < sizeof(HOURPINS)/sizeof(HOURPINS[0]); i++){
	    pinMode(HOURPINS[i], OUTPUT);
	}


    for(int i = 0; i < sizeof(MINPINS)/sizeof(MINPINS[0]); i++){
        pinMode(MINPINS[i], OUTPUT);
    }

    //Set Up the Seconds LED for PWM
	pinMode(SECS, PWM_OUTPUT);
	printf("LEDS done\n");

	//Set up the Buttons
	for(int j = 0; j < sizeof(BTNS)/sizeof(BTNS[0]); j++){
		pinMode(BTNS[j], INPUT);
		pullUpDnControl(BTNS[j], PUD_UP);
	}

	//Attach interrupts to Buttons
	wiringPiISR(BTNS[0], INT_EDGE_FALLING, &hourInc);
	wiringPiISR(BTNS[1], INT_EDGE_FALLING, &minInc);
	printf("BTNS done\n");
	printf("Setup done\n");
}

void initRTC()
{
	// set to 12 hour time am
	wiringPiI2CWriteReg8(RTC,HOUR, 0b01000000);
	// start clock
	wiringPiI2CWriteReg8(RTC,SEC, 0b10000000);
}

void cleanUpGPIO(void){
    printf("Cleaning up GPIO");

	//Reset LED lines to input
	for(int i = 0; i < sizeof(HOURPINS)/sizeof(HOURPINS[0]); i++){
	    pinMode(HOURPINS[i], INPUT);
	}


    for(int i = 0; i < sizeof(MINPINS)/sizeof(MINPINS[0]); i++){
        pinMode(MINPINS[i], INPUT);
    }

    //Reset SECS line to input
    pinMode(SECS, INPUT);
}

/*
 * The main function
 * This function is called, and calls all relevant functions we've written
 */
int main(void){


	initGPIO();
	cleanUpGPIO();
    	initGPIO();
    	printf("begin RTC init");
	initRTC();
    	printf("finish RTC init");

	// get the current time
	getCurrentTime();
	secs = getSecs();
	mins = getMins();
	hours = getHours();
	// compensate for if hours is in 24 hour time.
	if (hours > 12)
	{
		hours = hours - 12;
	}
	// write the current time to the RTC.
	write12Hour(hours, 0);
	writeMin(mins);
	writeSec(secs);
	// Repeat this until we shut down
	for (;;){
    	//Fetch the time from the RTC
	secs = getSec();
    	mins = getMin();
    	hours = getHour();
	    //Write your logic here

	    //Function calls to toggle LEDs
	    lightHours(hours);
        lightMins(mins);

    	//max value for pwm is 1042
    	//secs = wiringPiI2CReadReg8(RTC, SEC) & 0b01111111;
	    //secs = getSec();
    	// Print out the time we have stored on our RTC
    	printf("The current time is: %d:%d:%d\n", hours, mins, secs);
    	secPWM(secs);
	    //using a delay to make program less cpu hungry
	    delay(1000); //milliseconds
    	}
	return 0;
}

/*
 * Turns on corresponding LED's for hours
 */
void lightHours(int units){
	for(int i = 0; i < 4; i++){
        int binPow = pow(2, 3-i);
        digitalWrite(HOURPINS[i], units/binPow);
        units %= binPow;
    }
}

/*
 * Turn on the Minute LEDs
 */
void lightMins(int units){
	for(int i = 0; i < 6; i++){
        int binPow = pow(2, 5-i);
        digitalWrite(MINPINS[i], units/binPow);
        units %= binPow;
    }
}

/*
 * PWM on the Seconds LED
 * The LED should have 60 brightness levels
 * The LED should be "off" at 0 seconds, and fully bright at 59 seconds
 */
void secPWM(int units){
	// Write your logic here
        pwmWrite (SECS, units/60.0*1042) ;     /* provide PWM value for duty cycle */
}

/*
 * hourInc
 * Fetch the hour value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 23 hours in a day
 * Software Debouncing should be used
 */
void hourInc(void){
	//Debounce
	long interruptTime = millis();
	//printf("Hours");
	if (interruptTime - lastInterruptTime>200){
		//Fetch RTC Time
		hours = getHour();
		hours++;
		//Increase hours by 1, ensuring not to overflow
		if(hours > 12)
		{
			hours = 1;
		}
		printf("%d", hours);
		//Write hours back to the RTC
		write12Hour(hours, 0);
		lightHours(hours);
		//printf("Hour = %x\n", hours);
	}
	lastInterruptTime = interruptTime;
}

/*
 * minInc
 * Fetch the minute value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 60 minutes in an hour
 * Software Debouncing should be used
 */
void minInc(void){
	long interruptTime = millis();
	//printf("Minutes");
	if (interruptTime - lastInterruptTime>200){
		//Fetch RTC Time
		mins = getMin();
		//Increase minutes by 1, ensuring not to overflow
		mins++;
                //Increase hours by 1, ensuring not to overflow
                if(mins >= 60)
                {
                        mins = 0;
                }
		//Write minutes back to the RTC
		writeMin(mins);
		lightMins(mins);
		//printf("Minutes = %x\n", mins);
	}
	lastInterruptTime = interruptTime;
}

//Added functions to make things easyer.


void turnOffOscillator(void)
{
	int currentTime = wiringPiI2CReadReg8(RTC, 0x00);
	wiringPiI2CWriteReg8(RTC, 0x00, (currentTime & 0b01111111));
	//printf("Waiting for oscilator to stop");
	while ((wiringPiI2CReadReg8(RTC, 0x03) & 0b00100000) == 0b00100000)
	{ }
}

void turnOnOscillator(void)
{
	int currentTime = wiringPiI2CReadReg8(RTC, 0x00);
	wiringPiI2CWriteReg8(RTC, 0x00, (currentTime | 0b10000000));
}

/*
 * Write 12 Hour
 * Writes the hour to the RTC, taking into account that bits 6,5 hold hour info and
 * you soulden't write to bit 7.
 *
 * Defult is set to am. To set pm pass 1 for pm.
 */
int write12Hour(int value, int pm)
{
	// check if hour is valid
	if (value > 12)
		return 1;
	turnOffOscillator();
	int units = (value % 10);
	int tens = value/10 << 4;
	int regValue = 0b00000000 | (units+ tens);
	if (pm)
		regValue |= 0b00100000;
	else
		regValue &= 0b00011111;
	//end if (pm)

	regValue &= 0b01111111;
	wiringPiI2CWriteReg8(RTC, 0x02, regValue);
	turnOnOscillator();
	return 0;
} // end Write Hour


/*
 * Write min
 * Writes the minutes to the RTC.
 */
int writeMin(int value)
{
	if (value > 59)
		return 1;
	turnOffOscillator();
        int units = (value % 10);
        int tens = (value/10) << 4;
        int regValue = 0b00000000 | (units +tens) ;
	regValue &= 0b01111111;
        wiringPiI2CWriteReg8(RTC, 0x01, regValue);
	turnOnOscillator();
	return 0;
} // end writeMin

/*
 * Write sec
 * Writes the seconds to the RTC.
 */
int writeSec(int value)
{
        if (value > 59)
                return 1;
        turnOffOscillator();
        int units = (value % 10);
        int tens = (value/10) << 4;
        int regValue = 0b00000000 | (units +tens) ;
        regValue &= 0b01111111;
        wiringPiI2CWriteReg8(RTC, 0x00, regValue);
        turnOnOscillator();
        return 0;
} // end writeSec


/*
 *Get Hour
 *Fetches and converts the current hour from the RTC
 */
int getHour(void)
{
	//Fetch the hour and accomidate for the other information stored in the hours register.
        int num = wiringPiI2CReadReg8(RTC, 0x02) & 0b00011111;
        int result =((num & 0b01110000)>>4)*10 + (num & 0x0F);
        return (int)result;
} // end getHour

/*
 *Get Min
 *Fetches and converts the current min from the RTC
 */
int getMin(void)
{
        //Fetch the minute  and accomidate for the other information stored in the hours register.
        int num = wiringPiI2CReadReg8(RTC, 0x01) & 0b01111111;
        int result =((num & 0b01110000)>>4)*10 + (num & 0x0F);
        return (int)result;
}

/*
 *Get sec
 *Fetches and converts the current second from the RTC
 */
int getSec(void)
{
        //Fetch the second and accomidate for the other information stored in the hours register.
        int num = wiringPiI2CReadReg8(RTC, 0x00) & 0b01111111;
        int result =((num & 0b01110000)>>4)*10 + (num & 0x0F);
        return result;
} // end getSec

