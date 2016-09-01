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
#include "shell.h"
#include "chprintf.h"

/*
 * Red LED blinker thread, times are in milliseconds.
 */
#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

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
/*
static void cmd_temp(BaseSequentialStream *chp, int argc, char *argv[]) {
    chprintf(chp, "Temp: %.2f \n\r", pc0_temp);
}

static void cmd_volt(BaseSequentialStream *chp, int argc, char *argv[]) {
    chprintf(chp, "PC0 DCV: %.2f \n\r", pc0_volt);
    chprintf(chp, "PC1 DCV: %.2f \n\r", pc1_volt);
}
*/
static void cmd_test(BaseSequentialStream *chp) {
    chprintf(chp, "LED ON \n\r");
    //palSetPad(GPIOA, GPIOA_LED_GREEN; // led on
}

static const ShellCommand commands[] = {
    {"test",  &cmd_test},
    {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *)&SD1,
    commands
};



/*
 * Application entry point.
 */
static thread_t *shelltp = NULL;
/*
static void ShellHandler(eventid_t id) {

  (void)id;
  if (chThdTerminatedX(shelltp)) {
    chThdWait(shelltp);                 /* Returning memory to heap.
    shelltp = NULL;
  }
}
*/
int main(void) {
/*
  static const evhandler_t evhndl[] = {
    ShellHandler
  };
  event_listener_t el2;
*/
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  shellInit();


  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  //chEvtRegister(&shell_terminated, &el2, 2);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */

 while (true) {
    if (!shelltp) {
      shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                    "shell", NORMALPRIO + 1,
                                    shellThread, (void *)&shell_cfg1);
    }

    //chEvtDispatch(evhndl, chEvtWaitOneTimeout(ALL_EVENTS, MS2ST(500)));
  }
}
