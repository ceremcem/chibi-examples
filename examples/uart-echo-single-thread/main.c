#include "hal.h"
#include "ch.h"



int main(void)
{
	halInit();
	chSysInit();

    uint8_t buffer[2];
	palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(7)); // used function : USART3_TX
	palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(7)); // used function : USART3_RX
	/* Start the serial driver(change it's state to ready) pointed by arg1 with configurations in arg2
     * if arg2 is NULL then use default configuration in halconf.h*/
	sdStart(&SD3, NULL);

	while(true){
		/* Read arg3 bytes from device pointed by arg1(SD3 for this example)
		 * and write data to arg2(buffer for this example). Type of buffer
		 * should be (uint8_t *)*/
        sdRead(&SD3, buffer, 2);
		chThdSleepMilliseconds(100);/*Wait for an arbitrary time*/
		/* Write arg3 bytes from arg2 to device pointed by arg1(SD3 for this example).
		 * Type of arg2 should be (uint8_t *) otherwise only first 8 bit will be send.*/
		sdWrite(&SD3, buffer,2);
		chThdSleepMilliseconds(100);/*Wait for an arbitrary time*/
	}


	return 0;
}
