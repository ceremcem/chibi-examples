#include "hal.h"
#include "ch.h"
#include "test.h"

int main(void)
{
	halInit();
	chSysInit();


	while(TRUE){
		/* Read the Button */
		if(palReadPad(GPIOA, GPIOA_BUTTON)) {
			palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */

		}
		else {
			palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
		}

		chThdSleepMilliseconds(100);

	}
	return 0;
}
