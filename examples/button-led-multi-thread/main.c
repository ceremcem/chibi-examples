#include "hal.h"
#include "ch.h"
#include "test.h"

static THD_WORKING_AREA(listenerWorkingArea, 128);
static THD_WORKING_AREA(blinkerWorkingArea, 128);


int32_t mailboxArea[16];/*this should be minimum uint32_t not less*/
mailbox_t threadComm;

static THD_FUNCTION(buttonListener, arg)
{
	(void)(arg);
	int32_t msg;
	while(!0)
	{
		if(palReadPad(GPIOA, GPIOA_BUTTON)) 
		{
			msg = 1;
			chMBPost(&threadComm, msg, TIME_INFINITE);
			
		}
		else
		{	
			msg = 0;
			chMBPost(&threadComm, msg, TIME_INFINITE);
		}
	}
	//return NULL;
}

static THD_FUNCTION(ledBlinker, arg)
{
	(void)(arg);
	int32_t temp = 0;
	while(!0)
	{
		if(chMBFetch(&threadComm, &temp, TIME_INFINITE) == MSG_OK)
		{
			if(temp)
			{
				palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
				//temp = 0U;			
			}

			else
				palClearPad(GPIOD, GPIOD_LED3);
		}
	}
	//return NULL;
}

int main(void)
{
	halInit();
	chSysInit();
	
	thread_t *listener;
	chMBObjectInit(&threadComm, mailboxArea, 16);
	
    listener = chThdCreateStatic(listenerWorkingArea, sizeof(listenerWorkingArea), HIGHPRIO, buttonListener, NULL);
    chThdCreateStatic(blinkerWorkingArea, sizeof(blinkerWorkingArea), HIGHPRIO, ledBlinker, NULL);
    
    /*while(!0)
    {
		//chThdSleepMilliseconds(100);
	}*/
    
    chThdWait(listener);
    
	return 0;
}
