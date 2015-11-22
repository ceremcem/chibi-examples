#include "hal.h"
#include "ch.h"

#define MAILBOXSIZE 16

static THD_WORKING_AREA(serialListen, 128);/*Thread area for rxListen*/
static THD_WORKING_AREA(serialTalk, 128);/*Thread area for txWrite*/

int32_t txMailboxArea[MAILBOXSIZE];/*Mailbox area for sending (uint8_t) objects */
mailbox_t serialMbox;/*mailbox name*/


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
		chMBPost(&serialMbox, buffer, TIME_INFINITE);
		chThdSleepMilliseconds(100);
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
		if(chMBFetch(&serialMbox, (msg_t *)&toSend, TIME_INFINITE) == MSG_OK)
		{
			/* Write arg3 bytes from arg2 to device pointed by arg1(SD3 for this example).
			 * Type of arg2 should be (uint8_t *) otherwise only first 8 bit will be send.*/
			sdWrite(&SD3, &toSend,1);
		}
		chThdSleepMilliseconds(500);
	}
}

int main(void)
{
	halInit();
	chSysInit();
	
	thread_t *listener;
	
	palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(7)); // used function : USART3_TX
	palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(7)); // used function : USART3_RX
	/* Start the serial driver(change it's state to ready) pointed by arg1 with configurations in arg2
     * if arg2 is NULL then use default configuration in halconf.h*/
	sdStart(&SD3, NULL);
	
	/*Initialize mailbox object pointed by arg1, which uses buffer pointed by arg2 and have 
	 * arg3 objects*/
	chMBObjectInit(&serialMbox, txMailboxArea, MAILBOXSIZE);
	
	/* Create a thread uses arg2 of area arg1 with priority of 
	 * arg3(LOWPRIO,NORMALPRIO,HIGHPRIO or any value between 0-255), thread function is arg4
	 * 
	 * Returns pointer to thread object*/
    listener = chThdCreateStatic(serialListen, sizeof(serialListen), LOWPRIO, rxListen, NULL);
    
    chThdCreateStatic(serialTalk, sizeof(serialTalk), LOWPRIO, txWrite, NULL);
    
    /*Wait until thread pointed in arg1 finish*/
	chThdWait(listener);
	
	return 0;
}
