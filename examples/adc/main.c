#include "hal.h"
#include "ch.h"
#include "test.h"


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
	
	palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_ANALOG); // this is 6th channel
	adcStart(&ADCD1, &adccfg);//start adcdriver 1 with configure adccfg
	
	while(TRUE)
	{
		/* Reading from adc driver pointed by arg1, with configure arg2
		 * to buffer pointed by arg3, arg4 times*/
		 
        adcStartConversion(&ADCD1, &adccg, samples_buf, ADC_BUF_DEPTH);
	}
	return 0;
}
