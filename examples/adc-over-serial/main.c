#include "hal.h"
#include "ch.h"


// Create buffer to store ADC results. This is
// one-dimensional interleaved array
#define ADC_BUF_DEPTH 1 // We only read one by one
#define ADC_CH_NUM 1    // How many channel you use at same time
static adcsample_t samples_buf[ADC_BUF_DEPTH * ADC_CH_NUM]; // results array

/*dummy configure for adc*/
static const ADCConfig adccfg = {.dummy = 0};

static const ADCConversionGroup adccg = {
   // this 3 fields are common for all MCUs
      // set to TRUE if need circular buffer, set FALSE otherwise
      circular : FALSE,
      // number of channels
      num_channels : ADC_CH_NUM,
      // callback function when conversion ends
      end_cb : NULL,
      //callback function when error appears
      error_cb : NULL,
      //look to datasheet for information about the registers
      // CR1 register content
      cr1 : 0,
      // CR2 register content
      cr2 : ADC_CR2_SWSTART,//?
      // SMRP1 register content
      smpr1 : 0,
      // SMRP2 register content
      smpr2 : 0,
      // SQR1 register content
      sqr1 : ((ADC_CH_NUM - 1) << 20),
      // SQR2 register content
      sqr2 : 0,
      // SQR3 register content. We must select 1 channel
      // For example 6th channel
      // if we want to select more than 1 channel then simply
      // shift and logic or the values example (ch 15 & ch 10) : (15 | (10 << 5))
      sqr3 : 6
};



int main(void)
{
	halInit();
	chSysInit();
	
	uint8_t buffer[3];
	palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(7)); // used function : USART3_TX
	palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(7)); // used function : USART3_RX
	/* Start the serial driver(change it's state to ready) pointed by arg1 with configurations in arg2
     * if arg2 is NULL then use default configuration in halconf.h*/
	sdStart(&SD3, NULL);
	
	
	palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_ANALOG); // this is 10th channel
	adcStart(&ADCD1, &adccfg);//start adcdriver 1 with configure adccfg
	
	while(!0)
	{
		/* Reading from adc driver pointed by arg1, with configure arg2
		 * to buffer pointed by arg3, arg4 times*/
        adcStartConversion(&ADCD1, &adccg, samples_buf, ADC_BUF_DEPTH);
		/* Write arg3 bytes from arg2 to device pointed by arg1(SD3 for this example).
		 * Type of arg2 should be (uint8_t *) otherwise only first 8 bit will be send.*/
		buffer[0] = (uint8_t)(samples_buf[0]>>8);
		buffer[1] = (uint8_t)samples_buf[0];
		buffer[2] = (uint8_t)'\n';
		sdWrite(&SD3, (uint8_t *)buffer,3);
		chThdSleepMilliseconds(1000);/*Wait for an arbitrary time*/
	}


	return 0;
}
