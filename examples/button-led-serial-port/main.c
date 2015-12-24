#include "hal.h"
#include "ch.h"
#include "test.h"

//#define MAILBOXSIZE 16

static THD_WORKING_AREA(serialListen, 128);/*Thread area for rxListen*/
static THD_WORKING_AREA(serialTalk, 128);/*Thread area for txWrite*/

//int32_t txMailboxArea[MAILBOXSIZE];/*Mailbox area for sending (uint8_t) objects */
//mailbox_t serialMbox;/*mailbox name*/

static THD_FUNCTION (rxListen, arg)
{
	(void)(arg);
	uint8_t buffer;
	while(!0)
	{
		/* Read arg3 bytes from device pointed by arg1(SD3 for this example)
		 * and write data to arg2(buffer for this example). Type of buffer
		 * should be (uint8_t *). This function blocks this thread until
		 * desired number of bytes have read*/
		sdRead(&SD3, &buffer, 1);
		/* Post contents of arg2 to mailbox pointed by arg1, wait to post maximum arg3 mSeconds
		 * arg3 may be TIME_IMMEDIATE(post if you can) or TIME_INFINITE(wait until post).
		 * If mailbox object is full, function waits for a free slot for arg3 mSeconds */

		//chMBPost(&serialMbox, buffer, TIME_INFINITE);

		if(buffer == 0xaa) {
			palSetPad(GPIOD, GPIOD_LED3);
		}
		else if (buffer == 0xbb){
			palClearPad(GPIOD, GPIOD_LED3);
		}

	}
}



static THD_FUNCTION(txWrite, arg)
{
	(void)(arg);
	uint8_t toSend;
	while(!0)
	{
		/* Get contents of next mail in mailbox pointed by arg1 to arg2
		 * and wait mail arg3 mSeconds if there is no message is present.
		 *
		 * Returns MSG_OK if succesfully a mail fetched from mailbox*/
		if(palReadPad(GPIOA, GPIOA_BUTTON))
		{
			/* Write arg3 bytes from arg2 to device pointed by arg1(SD3 for this example).
			 * Type of arg2 should be (uint8_t *) otherwise only first 8 bit will be send.*/
			toSend = 1;
			sdWrite(&SD3, &toSend, 1);
		}
		else {
			toSend = 0;
			sdWrite(&SD3, &toSend, 1);
		}
	}
}


int main(void)
{
	halInit();
	chSysInit();

	thread_t *listener;

	palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(7)); // used function : USART3_TX
	palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(7)); // used function : USART3_RX

	sdStart(&SD3, NULL);

//	chMBObjectInit(&serialMbox, txMailboxArea, MAILBOXSIZE);

	listener = chThdCreateStatic(serialListen, sizeof(serialListen), NORMALPRIO, rxListen, NULL);

  chThdCreateStatic(serialTalk, sizeof(serialTalk), NORMALPRIO, txWrite, NULL);


	chThdWait(listener);


	return 0;
}
