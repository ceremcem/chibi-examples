#include "hal.h"
#include "ch.h"
#include "test.h"

static PWMConfig pwmcfg = {
	2000000, /* PWM Driver frequency 2 MHz */
	4, /* Period of output wave */
	NULL, /* No callback */

	/* Only channel 1 enabled */
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL}
	}
};

static THD_WORKING_AREA(waThread1, 128);

static THD_FUNCTION(Thread1, arg) {
	(void)arg;

	pwmcnt_t duty = 2, period = 40;
	pwmEnableChannel(&PWMD1, 0, (pwmcnt_t)duty);

	while(true) {

 		pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH (&PWMD1, 5000)); /* 50% */
		pwmChangePeriod(&PWMD1, (pwmcnt_t)period);
		period += 1;
		chThdSleepMilliseconds(200);

	}


}
int main(void)
{
	halInit();
	chSysInit();

	pwmStart(&PWMD1, &pwmcfg);
  palSetPadMode(GPIOD, GPIOD_LED4, PAL_MODE_ALTERNATE(2));      /* Green.   */

	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	while(true){
		chThdSleepMilliseconds(500);

	}
	return 0;
}
