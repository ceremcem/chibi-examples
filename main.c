#include "hal.h"
#include "ch.h"
#include "test.h"

int main(void)
{
	halInit();
	chSysInit();
	
	
	while(TRUE){
	
		palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
		chThdSleepMilliseconds(500);
		palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
		chThdSleepMilliseconds(500);
 
	}
	return 0;
}
