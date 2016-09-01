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
#include "shell/shell.h"
#include "streams/chprintf.h"

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread2, 1024);

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
static void cmd_ledOn(BaseSequentialStream *chp);
static void cmd_ledOff(BaseSequentialStream *chp);

static const ShellCommand shCmds[] = {
    {"ledon",  cmd_ledOn},
    {"ledoff", cmd_ledOff},
    {NULL, NULL}
};

static const ShellConfig shCfg = {
    (BaseSequentialStream *)&SD1,
    shCmds
};



/*
 * Application entry point.
 */
static void cmd_ledOn(BaseSequentialStream *chp) {
    chprintf(chp, "LED ON \n\r");
    //palSetPad(GPIOA, GPIOA_LED_GREEN; // led on
}

static void cmd_ledOff(BaseSequentialStream *chp) {
    chprintf(chp, "LED OFF \n\r");
    //palClearPad(GPIOA, GPIOA_LED_GREEN); // led off
}

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
  sdStart(&SD1, NULL);
  palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
  palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing except
   * sleeping in a loop and check the button state.
   */
  thread_t *sh = NULL;
  while (true) {
    if (TRUE) {  //(!palReadPad(GPIOC, GPIOC_BUTTON))
      //TestThread(&SD1);
      sh = shellCreate(&shCfg, waThread2, NORMALPRIO);
    }
    else if (chThdTerminated(sh)) {
      chThdRelease(sh);
      sh = NULL;
    }

    chThdSleepMilliseconds(1000);
    }
}
