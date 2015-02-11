/*****************************************************************************
*
*	Authors: Jordan and Paul
* 	ECE 544 Project Two
*
******************************************************************************/

/************************ Include Files **************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xparameters.h"
#include "xintc.h"
#include "xtmrctr.h"
#include "xgpio.h"
#include "mb_interface.h"
#include "platform.h"
#include "Nexys4IO.h"
#include "PMod544IOR2.h"
#include "pwm_tmrctr.h"

/***************** Debug Flag ************************************************/					
int debugen = 0;		

/***************** Function Prototypes ***************************************/
int readVal(void);
void delay_msecs(unsigned int);
void update_lcd(int, int, u32);


/***************** Global Variables ******************************************/	
volatile unsigned long	timestamp;			// timestamp since the program began



int main(void){



}

float PID_I_D(float setPoint, float realPoint) {

	static float pre_error = 0;
	static float integral = 0;
	float error;
	float derivative;
	float output;

	// Calculate P-I-D
	error = setPoint - realPoint;

	// If error is too small stop integration
	if(abs(error) > epsilon) 
		integral = integral + (error * dt);

	derivative = (error - pre_error)/dt;
	output = Kp*error + Ki*integral + Kd*derivative;

	//Saturation Filter
	if(output > MAX) 
		output= MAX;


	else if(output < MIN)
		output= MIN;

	//Update error
	pre_error= error;

	return output;
}

int readVal(void) {
	// get the value from the sensor

	//return value
}
 
/****************************************************************************/
/**
* delay execution for "n" msecs
* 
* Uses a busy-wait loop to delay execution.  Timing is approximate but we're not 
*  looking for precision here, just a uniform delay function.  The function uses the 
*  global "timestamp" which is incremented every msec by FIT_Handler().
*
* @note
* Assumes that this loop is running faster than the fit_interval ISR 
*
* @note
* If your program seems to hang it could be because the function never returns
* Possible causes for this are almost certainly related to the FIT timer.  Check
* your connections...is the timer clocked?  is it stuck in reset?  is the interrupt 
* output connected? You would not be the first student to face this...not by a longshot 
*****************************************************************************/
void delay_msecs(unsigned int msecs) {
	
	unsigned long target;

	if ( msecs == 0 )
		return;
	
	target = timestamp + msecs;
	while ( timestamp != target ) {
		// spin until delay is over
	}
}

/****************************************************************************/
/**
 * update the frequency/duty cycle LCD display
 * 
 * writes the frequency and duty cycle to the specified line.  Assumes the
 * static portion of the display is already written and the format of each
 * line of the display is the same.
 *
 * @param	freq is the  PWM frequency to be displayed
 *
 * @param	dutycycle is the PM duty cycle to be displayed
 *
 * @param	linenum is the line (1 or 2) in the display to update
 *****************************************************************************/
void update_lcd(int freq, int dutycycle, u32 linenum) {
	PMDIO_LCD_setcursor(linenum, 3);
	PMDIO_LCD_wrstring("    ");
	PMDIO_LCD_setcursor(linenum, 3);
	
	// display Hz if frequency < 1Khz
	if (freq < 1000) 
		PMDIO_LCD_putnum(freq, 10);

	// display frequency in KHz
	else  {
		PMDIO_LCD_putnum((freq / 1000), 10);
		PMDIO_LCD_wrstring("K");
	}

	PMDIO_LCD_setcursor(linenum, 13);
	PMDIO_LCD_wrstring("  %");
	PMDIO_LCD_setcursor(linenum, 13);
	PMDIO_LCD_putnum(dutycycle, 10);
}