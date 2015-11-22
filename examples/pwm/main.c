#include "hal.h"
#include "ch.h"
#include "test.h"

static PWMConfig pwmcfg = {
	200000, /* 200Khz PWM clock frequency*/
	1024, /* PWM period of 1024 ticks ~ 0.005 second */
	NULL, /* No callback */

	/* Only channel 1 enabled */
	{
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL}
	}
};

static THD_WORKING_AREA(waThread1, 128);

static THD_FUNCTION(Thread1, arg) {
	(void)arg;

	pwmcnt_t duty = 1024;
	bool direction = true;
	while(true) {
		pwmEnableChannel(&PWMD4, 0, (pwmcnt_t)duty);
		chThdSleepMilliseconds(1);

		if (direction) {
			duty -= 1;
		}
		else {
			duty 	+= 1;
		}

		if (duty < 1) {
			direction = false;

		}

		if (duty > 1023) {
			direction = true;
		}



	}


}
int main(void)
{
	halInit();
	chSysInit();

	pwmStart(&PWMD4, &pwmcfg);
  palSetPadMode(GPIOD, GPIOD_LED4, PAL_MODE_ALTERNATE(2));      /* Green.   */

	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

	while(true){
		chThdSleepMilliseconds(500);

	}
	return 0;
}
