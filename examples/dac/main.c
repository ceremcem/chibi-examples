#include "hal.h"
#include "ch.h"

/* DAC Related */

#define DAC_BUF_DEPTH 1
static dacsample_t samples_buf[DAC_BUF_DEPTH];

static const DACConfig dac1cfg1 = {
  init:         1000U,/*başlangıç değeri*/
  datamode:     DAC_DHRM_12BIT_RIGHT/*datasheette yazıyor*/
};


static const DACConversionGroup dacgrpcfg1 = {
  num_channels: 1U,/*Kaç kanala birden yazacağı*/
  end_cb:       NULL,/*Conversion bittiğinde çağırılacak fonksiyon*/
  error_cb:     NULL,/*Hata durumunda çalışacak fonksiyon*/
  trigger:      DAC_TRG(0)
};

/*Timer 6 için config bunu kullanıyoruz çünkü dac bu timer ile tetikleniyor.
 * Ayrıntısı büyük datasheette*/
static const GPTConfig gpt6cfg1 = {
  frequency:    1000000U,
  callback:     NULL,
  cr2:          TIM_CR2_MMS_1,  /* MMS = 010 = TRGO on Update Event.        */
  dier:         0U
};
/* DAC Related */

int main(void)
{
	halInit();
	chSysInit();
	samples_buf[0] = 0;
	uint8_t dir = 1;
	/*Port for analog output Mode is confusing but it's output*/
    palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);
    
    /*Start timer and dac driver*/
    dacStart(&DACD1, &dac1cfg1);
    gptStart(&GPTD6, &gpt6cfg1);
    /*Start timer in continuous mode to trigger dac*/
	gptStartContinuous(&GPTD6, 2U);
	
    /*Oscilating */
    while(!0)
    {
		/*write first arg4 values from arg3 to driver pointed in arg1 
		 * with configuration pointed by arg2 */
		dacStartConversion(&DACD1, &dacgrpcfg1, samples_buf, 1);
		/*Maximum value for dac*/
		if(samples_buf[0] == 0x0FFF)
			dir = 0;
		else if(samples_buf[0] == 0x0000)/*Minimum value for dac*/
			dir = 1;
			
		if(dir)
			samples_buf[0]++;
		else
			samples_buf[0]--;
		
		/*Wait a while*/
		chThdSleepMilliseconds(1);
	}
	return 0;
}
