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


// Define Parameters
#define epsilon 	0.01
#define dt 			0.01 //100mslooptime
#define MAX 		4 //ForCurrent Saturation
#define MIN 		-4
#define Kp 			0.1
#define Kd 			0.01
#define Ki 			0.005

// Modes of Operation
#define bang_bang 	0x0
#define PID 		0x1
#define extraCred 	0x2
#define inVsOut 	0x3


#define bbOn 		99
#define bbOff 		1

#define maxVolts 	3.3



/***************** Debug Flag ************************************************/					
int debugen = 0;		

/***************** Function Prototypes ***************************************/
int do_init(void);
int readVal(void);
void delay_msecs(unsigned int);
void update_lcd(int, int, u32);
float PID_I_D(float, float);


/***************** Global Variables ******************************************/	
volatile unsigned long	timestamp;			// timestamp since the program began
volatile float 			setPointVolts;


int main(void){
	XStatus 	status;
	u16			sw, oldSw =0xFFFF;		// 0xFFFF is invalid - makes sure the PWM freq is updated the first time
	int			rotcnt, oldRotcnt = 0x1000;	
	bool		done = false;

	init_platform();

	// initialize devices and set up interrupts, etc.
 	status = do_init();
 	if (status != XST_SUCCESS) { 
 		PMDIO_LCD_setcursor(1,0);
 		PMDIO_LCD_wrstring("****** ERROR *******");
 		PMDIO_LCD_setcursor(2,0);
 		PMDIO_LCD_wrstring("INIT FAILED- EXITING");
 		exit(XST_FAILURE);
 	}
 	
	// initialize the global variables
	timestamp = 0;							
	pwm_freq = INITIAL_FREQUENCY;
	pwm_duty = INITIAL_DUTY_CYCLE;
	clkfit = 0;
	new_perduty = false;
	inHardware = false;
    
	// start the PWM timer and kick of the processing by enabling the Microblaze interrupt
	PWM_SetParams(&PWMTimerInst, pwm_freq, pwm_duty);	
	PWM_Start(&PWMTimerInst);
    microblaze_enable_interrupts();
    
	// display the greeting   
    PMDIO_LCD_setcursor(1,0);
    PMDIO_LCD_wrstring(" PROJECT TWO CLC");
	PMDIO_LCD_setcursor(2,0);
	PMDIO_LCD_wrstring("  Jordan & Paul");
	NX4IO_setLEDs(0x0000FFFF);
	delay_msecs(2000);
	NX4IO_setLEDs(0x00000000);
		
    // write the static text to the display
    PMDIO_LCD_clrd();
    PMDIO_LCD_setcursor(1,0);
    PMDIO_LCD_wrstring("FR:      DCY:   ");
    PMDIO_LCD_setcursor(2,0);
    PMDIO_LCD_wrstring("FR:      DCY:   ");

    // turn off the LEDs and clear the seven segment display
    NX4IO_setLEDs(0x00000000);
    NX410_SSEG_setAllDigits(SSEGLO, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
    NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
      
    // main loop
	do { 
		// check rotary encoder pushbutton to see if it's time to quit
		if (PMDIO_ROT_isBtnPressed()) {
			done = true;
		}
		else {
			new_perduty = false;
			sw = NX4IO_getSwitches();
			if (sw != oldSw) {
				switch (sw & 0x3) {
					case 0x00:	Mode = bang_bang;	break;
					case 0x01:	Mode = PID;			break;
					case 0x02:	Mode = extraCred;	break;
					case 0x03:	Mode = inVsOut;		break;
				}

				oldSw = sw;
				new_perduty = true;
				NX4IO_setLEDs(sw); 			// show the switches selected with LEDs
			}
		
			// read rotary count and handle duty cycle changes
			// limit duty cycle to 1% to 99%
			PMDIO_ROT_readRotcnt(&rotcnt);
			if (rotcnt != oldRotcnt) {
				// show the rotary count in hex on the seven segment display
				NX4IO_SSEG_putU16Hex(SSEGLO, rotcnt);

				// change the duty cycle
				pwm_duty = MAX(1, MIN(rotcnt, 99));
				oldRotcnt = rotcnt;
				new_perduty = true;
			}

			// update generated frequency and duty cycle	
			if (new_perduty) {
				u32 freq, dutycycle;
			
				// set the new PWM parameters - PWM_SetParams stops the timer
				status = PWM_SetParams(&PWMTimerInst, pwm_freq, pwm_duty);

				if (status == XST_SUCCESS) {
					PWM_GetParams(&PWMTimerInst, &freq, &dutycycle);
					update_lcd(freq, dutycycle, 1);
										
					PWM_Start(&PWMTimerInst);

				}
			}
		}

	} while (!done);
	
	// wait until rotary encoder button is released		
	do {
		delay_msecs(10);
	} while (PMDIO_ROT_isBtnPressed());

	// we're done,  say goodbye
	xil_printf("\nThat's All Folks!\n\n");
	PMDIO_LCD_wrstring("-- Goodbye --");
	NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_B, CC_LCY, CC_E, DP_NONE);
	NX410_SSEG_setAllDigits(SSEGLO, CC_B, CC_LCY, CC_E, CC_BLANK, DP_NONE);
	delay_msecs(5000);
	PMDIO_LCD_clrd();
	cleanup_platform();
	exit(0);
}

/**************************** HELPER FUNCTIONS ******************************/
		
/****************************************************************************/
/**
* initialize the system
* 
* This function is executed once at start-up and after resets.  It initializes
* the peripherals and registers the interrupt handler(s)
*****************************************************************************/
int do_init(void)
{
	int status;				// status from Xilinx Lib calls
	
	// initialize the Nexys4IO and Pmod544IO hardware and drivers
	// rotary encoder is set to increment from 0 by DUTY_CYCLE_CHANGE 
 	status = NX4IO_initialize(NX4IO_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	
	status = PMDIO_initialize(PMDIO_BASEADDR);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	
	// successful initialization.  Set the rotary encoder
	// to increment from 0 by DUTY_CYCLE_CHANGE counts per
	// rotation.
 	PMDIO_ROT_init(DUTY_CYCLE_CHANGE, true);
	PMDIO_ROT_clear();
	
	
	// initialize the GPIO instance
	status = XGpio_Initialize(&GPIOInst, GPIO_DEVICE_ID);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	// GPIO channel 1 is an 8-bit input port.  bit[7:1] = reserved, bit[0] = PWM output (for duty cycle calculation)
	// GPIO channel 2 is an 8-bit output port.  bit[7:1] = reserved, bit[0] = FIT clock
	XGpio_SetDataDirection(&GPIOInst, GPIO_OUTPUT_CHANNEL, 0xFE);

	// initialize the GPIO instance
	status = XGpio_Initialize(&GPIOInstTime, GPIO_DEVICE_ID2);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	// Two 32b input GPIO's for the high time and the low time
	XGpio_SetDataDirection(&GPIOInstTime, GPIO_HIGHT_CHANNEL, 0xFF);
	XGpio_SetDataDirection(&GPIOInstTime, GPIO_LOWT_CHANNEL, 0xFF);

	// initialize the PWM timer/counter instance but do not start it
	// do not enable PWM interrupts.  Clock frequency is the AXI clock frequency
	status = PWM_Initialize(&PWMTimerInst, PWM_TIMER_DEVICE_ID, false, AXI_CLOCK_FREQ_HZ);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	
	// initialize the interrupt controller
	status = XIntc_Initialize(&IntrptCtlrInst, INTC_DEVICE_ID);
    if (status != XST_SUCCESS)
    {
       return XST_FAILURE;
    }

	// connect the fixed interval timer (FIT) handler to the interrupt
    status = XIntc_Connect(&IntrptCtlrInst, FIT_INTERRUPT_ID,
                           (XInterruptHandler)FIT_Handler,
                           (void *)0);
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }
 
	// start the interrupt controller such that interrupts are enabled for
	// all devices that cause interrupts.
    status = XIntc_Start(&IntrptCtlrInst, XIN_REAL_MODE);
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

	// enable the FIT interrupt
    XIntc_Enable(&IntrptCtlrInst, FIT_INTERRUPT_ID);

    // set the duty cycles for RGB1.  The channels will be enabled/disabled
    // in the FIT interrupt handler.  Red and Blue make purple
    NX4IO_RGBLED_setDutyCycle(RGB1, 64, 0, 64);
    NX4IO_RGBLED_setChnlEn(RGB1, false, false, false);

	return XST_SUCCESS;
}


/****************************************************************************
*
****************************************************************************/
float bangBang(float setPoint, float realPoint) {

	float output;

	// setPoint = read to get value from the driver
	while (1) {
		delay_msecs(100);
		// realPoint = read to get value from the driver
		setPointVolts = ((setPoint * maxVolts) / 100);

		if (realPoint < setPointVolts) {
			// drive signal bbOn
		}
		else if (realPoint > setPointVolts) {
			// drive signal bbOff
		}
	}
	return output;
}


/****************************************************************************
*
****************************************************************************/
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

/****************************************************************************
*
****************************************************************************/
int readVal(void) {
	// get the value from the sensor

	// return value
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