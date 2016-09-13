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
//#include "test.h"

/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
          /* Waiting for a queued message then retrieving it.*/
          thread_t *tp = chMsgWait();
          msg_t msg = chMsgGet(tp);

          /* Processing the message.*/
          if(msg == (msg_t)"ON")
            palSetPad(GPIOA, GPIOA_LED_GREEN);
          else if (msg == (msg_t)"OFF")
            palClearPad(GPIOA, GPIOA_LED_GREEN);
          /* Sending back an acknowledge.*/
          chMsgRelease(tp, MSG_OK);
  }
}


/*
 * Application entry point.
 */
static thread_t *console;
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
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD2, NULL);

  /*
   * Creates the blinker thread.
   */
  console = chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  while (true) {
    if (!palReadPad(GPIOC, GPIOC_BUTTON)) {
        palSetPad(GPIOA, GPIOA_LED_GREEN);
        (void)chMsgSend(console, (msg_t)"ON");    }
    else {
        palClearPad(GPIOA, GPIOA_LED_GREEN);
        (void)chMsgSend(console, (msg_t)"OFF");    }
    chThdSleepMilliseconds(50);
  }
}
