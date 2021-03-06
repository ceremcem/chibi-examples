/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include <string.h>
//#include "test.h"

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");

  while (true) {

    palClearPad(GPIOA, GPIOA_LED_GREEN);
    chThdSleepMilliseconds(1000);
    palSetPad(GPIOA, GPIOA_LED_GREEN);
    chThdSleepMilliseconds(1000);
  }
}
uint8_t rxbuffer[20];
uint8_t txbuffer[20];
uint8_t received;

static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread2, arg) {
  (void) arg;
  uint8_t start[] = "Hi from STM32F0:blinker \n";

  sdWrite(&SD1, start, sizeof(start));
  while (true) {
    sdRead(&SD1, rxbuffer, sizeof(rxbuffer));
    sdWrite(&SD1, rxbuffer, sizeof(rxbuffer));

}
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 1 using the driver default configuration.
   */
  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */
  /*
   * Creates the blinker thread.
   */
  //chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    if (TRUE)  //(!palReadPad(GPIOC, GPIOC_BUTTON))
      //TestThread(&SD1);
    chThdSleepMilliseconds(5000);
  }
}
