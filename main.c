#include "hal.h"
#include "ch.h"
#include "math.h"

#define N 16



int main(void)
{
	halInit();
	chSysInit();
	palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
	palSetPadMode(GPIOD, 12, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOD, 13, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOD, 14, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOD, 15, PAL_MODE_OUTPUT_PUSHPULL);
	/*usart2 ve ledler kullanıma hazır*/

	while(true){
	}

 
	return 0;
}
