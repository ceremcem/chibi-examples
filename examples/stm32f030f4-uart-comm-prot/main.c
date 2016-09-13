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

/*PACKET:
[SOH, PROTOCOL_ID, HOW_MANY_BYTE_THIS_PAYLOAD, COMPLEMENT_OF_PREVOUS_INDEX, PAYLOAD, CRC1, CRC2] or
[SOH, PROTOCOL_ID, HOW_MANY_BYTE_THIS_PAYLOAD, COMPLEMENT_OF_PREVOUS_INDEX, PAYLOAD, EOT]
*/

/* PROTOCOL DEFINITION:
 * SOH
 * Protocol ID (HEX, UTF-8 like scheme (if frame[x] is part of Protocol_ID and frame[x] > 127 then frame[x+1] belongs to Protocol_ID  ))
 * == BEGIN PAYLOAD ==
 * == END PAYLOAD ==
 * EOT
 *
 *
 * Protocol ID: 0xAA
 * -----------------
 * - Big Endian :
 * 		(0xA0B0C0D0 ->
 * 			Byte[0] = 0xA0,
 * 			Byte[1] = 0xB0,
 * 			Byte[2] = 0xC0,
 * 			Byte[3] = 0xD0
 * - 16 Digital Input
 * - 16 Digital Output
 * - 4 Analog Input (16 bits per channel)
 * - 2 Analog Output (16 bits per channel)
 * -------------------------------------------
 * 	DETAILS OF PAYLOAD
 * -------------------------------------------
 * SET DIGITAL OUTPUTS 15..8    //HERE IS REMOTE IO's YOU CAN SET THIS BLOCK with set_digital func.
 * SET DIGITAL OUTPUTS 7..0     //HERE IS LOCAL IO's
 * --------------------------
 * SEND DIGITAL INPUTS 15..8
 * SEND DIGITAL INPUTS 7..0
 * --------------------------
 * SET ANALOG OUTPUT 0 15..8
 * SET ANALOG OUTPUT 0 7..0
 * SET ANALOG OUTPUT 1 15..8
 * SET ANALOG OUTPUT 1 7..0
 * --------------------------
 * SEND ANALOG INPUT 0 15..8
 * SEND ANALOG INPUT 0 7..0
 * SEND ANALOG INPUT 1 15..8
 * SEND ANALOG INPUT 1 7..0
*/


#define SOH 0x01
#define EOT 0x04
#define GET_BYTE(VALUE, BYTE_INDEX) ((VALUE & (0xFF << (BYTE_INDEX * 8))) >> (BYTE_INDEX * 8))
#define PAYLOAD_LENGTH 23
#define PACKET_SIZE 5 + PAYLOAD_LENGTH
#ifndef TRUE
#define TRUE 1
#define FALSE !TRUE
#endif

binary_semaphore_t lock;

void shift_array(uint8_t *arr, int i) {
    int j = 0;
    while(i < PACKET_SIZE) {
        arr[j] = arr[i];
        j++;
        i++;
    }

}

void set_physical_output(uint8_t mcu_outputs) {
    //this function use B group
    //you must set the B GROUP as output in main.
    uint8_t mask = 1;
    int state = 0;
    int i = 0;
    for(i = 0; i < 7;) {
        state = mask & mcu_outputs;
        if(state) {
            palSetPad(GPIOB, i);
        }
        else {
            palClearPad(GPIOB, i);
        }
        i++;
        mask <<= i;
    }
}

//COMM CLASS
typedef struct Comm {

    //Comm functions
    void (*send) (struct Comm *);
    void (*read) (struct Comm *);
    void (*set_digital) (struct Comm *, int, int);
    int (*get_digital) (struct Comm *, int);
    void (*set_analog) (struct Comm *, int, uint16_t);
    void (*on_receive) (struct Comm *);
    //#Comm functions

    //Protocol variables
    uint8_t frame_start;
    uint8_t protocol_id;
    uint8_t length; 		//length of payload
    uint8_t comp_of_length;	//complement of length
    uint32_t digital_io;
    uint16_t analog_io[4];
    uint8_t virtual_tx_start;
    uint8_t virtual_tx[5];
    uint8_t virtual_rx[5];
    uint8_t frame_end;
    //#Protocol variables

    //Other variables
    expchannel_t interrupt_channel;

} Comm;

void comm_protocol_send(Comm *c) {
    uint8_t packet[PACKET_SIZE];

    packet[0] = c->frame_start;
    packet[1] = c->protocol_id;

    //TODO: calc_payload_length??? but not ready payload here
    packet[2] = c->length;
    packet[3] = ~(c->length);

    //Payload
    packet[4] = GET_BYTE(c->digital_io, 3);
    packet[5] = GET_BYTE(c->digital_io, 2);
    packet[6] = GET_BYTE(c->digital_io, 1);
    packet[7] = GET_BYTE(c->digital_io, 0);

    packet[8] = GET_BYTE(c->analog_io[3], 1);
    packet[9] = GET_BYTE(c->analog_io[3], 0);
    packet[10] = GET_BYTE(c->analog_io[2], 1);
    packet[11] = GET_BYTE(c->analog_io[2], 0);
    packet[12] = GET_BYTE(c->analog_io[1], 1);
    packet[13] = GET_BYTE(c->analog_io[1], 0);
    packet[14] = GET_BYTE(c->analog_io[0], 1);
    packet[15] = GET_BYTE(c->analog_io[0], 0);

    packet[16] = 0x01; //virtual_tx_start
    //virtual_tx
    packet[17] = 0x16;
    packet[18] = 0x17;
    packet[19] = 0x18;
    packet[20] = 0x19;
    packet[21] = 0x19;
    //#virtual_tx

    //virtual_rx
    packet[22] = 0x16;
    packet[23] = 0x17;
    packet[24] = 0x18;
    packet[25] = 0x19;
    packet[26] = 0x19;
    //#virtual_rx

    //#Payload
    packet[27] = c->frame_end;

    sdWrite(&SD1, packet, PACKET_SIZE);

}

void comm_protocol_set_digital(Comm *c, int pin, int state) {
    //This function use for set the remote machine IO.
    //This function set the digital_io 31..15. Eg. if pin is 0 then this function set the digital_io's 15.bit.
    //This function use 3rd byte of digital_io array. (according to BIG ENDIAN)
    uint32_t mask = 0 << 31;
    if(pin < 0 || pin > 7) //avoid overflow
    	return;

    if(state) {
    	mask |= (1<<pin);
    	mask <<= 24;
    	c->digital_io = c->digital_io | mask;
    }
    else {
    	mask |= (1 << pin);
    	mask = ~mask;
    	mask <<= 24;
    	uint32_t new_mask = 0 << 31;
    	new_mask |= 1 << 23;
    	mask |= new_mask;
    	c->digital_io &= mask;
    }

}

int comm_protocol_get_digital(Comm *c, int pin) {
    //Must be modify this function after seperate MCU and ESP IO blocks
    uint32_t mask = 0 << 31;
    mask |= (1<<pin);
    mask <<= 16;
    mask &= c->digital_io;
    uint8_t result = mask >> (16+pin);
    if(result == 0x01) {
        return TRUE;
    }
    else return FALSE;

}

void comm_protocol_set_analog(Comm *c, int pin, uint16_t value) {
    if(pin >= 0 && pin <= 3) {
	c->analog_io[pin] = value;
    }
}

void comm_protocol_read(Comm *c) {
    uint8_t rxbuffer[PACKET_SIZE];
    int frame_detected = FALSE;
    int read_n_byte = PACKET_SIZE;
    int i = 0;
    int read_flag = TRUE;

    while(!frame_detected) {

        if(read_flag) {
            sdRead(&SD1, rxbuffer + PACKET_SIZE - read_n_byte, read_n_byte);
            read_flag = FALSE;
        }

        if(rxbuffer[i] == c->frame_start) {
            if(i+3 < PACKET_SIZE) { //rxbuffer[i+3] is exist?
                if( (rxbuffer[i+2] ^ rxbuffer[i+3]) == 0xFF) { //length xor length complement
                    if(i == 0) {
                        if(rxbuffer[PACKET_SIZE-1] == c->frame_end) {
                            //telegram is valid
                            //clear the IO's

                            c->digital_io = 0;
                            c->analog_io[0] = 0;
                            c->analog_io[1] = 0;
                            c->analog_io[2] = 0;
                            c->analog_io[3] = 0;

                            //#clear

                            //updating variables
                            c->digital_io |= rxbuffer[4] << 24;
                            c->digital_io |= rxbuffer[5] << 16;
                            c->digital_io |= rxbuffer[6] << 8;
                            c->digital_io |= rxbuffer[7] << 0;

                            c->analog_io[0] |= rxbuffer[8] << 8;
                            c->analog_io[0] |= rxbuffer[9] << 0;
                            c->analog_io[1] |= rxbuffer[10] << 8;
                            c->analog_io[1] |= rxbuffer[11] << 0;
                            c->analog_io[2] |= rxbuffer[12] << 8;
                            c->analog_io[2] |= rxbuffer[13] << 0;
                            c->analog_io[3] |= rxbuffer[14] << 8;
                            c->analog_io[3] |= rxbuffer[15] << 0;

                            c->virtual_tx_start = rxbuffer[16];

                            int j;
                            for(j = 0; j < 5; j++) {
                                c->virtual_tx[j] = rxbuffer[17+j];
                            }

                            for(j = 0; j < 5; j++) {
                                c->virtual_rx[j] = rxbuffer[22+j];
                            }

                            //sdWrite(&SD1, rxbuffer, sizeof(rxbuffer));
                            uint8_t mcu_outputs = GET_BYTE(c->digital_io, 2);

                            set_physical_output(mcu_outputs);
                            c->on_receive(c);
                            //if virtual tx start is true than call virtual serial tx start

                            frame_detected = TRUE;
                        }
                        else {
                            i++;
                            continue;
                        }
                    }
                    else {
                        shift_array(rxbuffer, i);
                        read_n_byte = i;
                        i = 0;
                        read_flag = TRUE;
                    }
                }
                else {
                    i++;
                    continue;
                }
            }
            else {
                shift_array(rxbuffer, i);
                read_n_byte = i;
                i = 0;
                read_flag = TRUE;
            }

        }
        else if(i < PACKET_SIZE-1){
            i++;

        }
        else {
            read_n_byte = PACKET_SIZE;
            i = 0;
            read_flag = TRUE;
        }
    }



}

void comm_protocol_on_receive(Comm *c) {
    //This function invoked when the valid telegram is received.
    //You can write here, what ever do you want.
    //Example if virtual_tx_start is true then start second serial tx with virtual_tx
    /*
    if(c->virtual_tx_start) {
        sdStart(&SD2, NULL);
        sdWrite(&SD2, virtual_tx, 5);

    }
    else {
        sdStop(&SD2);
    }
    */
    sdWrite(&SD1, (c->virtual_rx), 5);
    return;
}

/*
void comm_protocol_read_virtual_rx(Comm *c) {
    //you must start this function in any thread
    uint8_t virtual_rx_buffer[5]; //sizeof c->virtual_rx_buffer
    int i = 0;
    //sdRead(&SD2, virtual_rx_buffer, 5);
    //update c->virtual_rx_buffer
    for(i = 0; i < 5; i++) {
        c->virtual_rx[i] = virtual_rx_buffer[i];

    }
    //send new telegram here

}
*/
Comm* new_Comm(Comm* cp, uint8_t id){
    cp->protocol_id = id;
    cp->frame_start = SOH;
    cp->length = 0x1B;
    cp->frame_end = EOT;

    cp->send = comm_protocol_send;
    cp->read = comm_protocol_read;
    cp->set_digital = comm_protocol_set_digital;
    cp->get_digital = comm_protocol_get_digital;
    cp->set_analog = comm_protocol_set_analog;
    cp->on_receive = comm_protocol_on_receive;

    return cp;
};

//Define global comm obj.
static Comm __c, *c;
//#COMM CLASS

//INTERRUPT thread
static THD_WORKING_AREA(comm_protocol_on_interrut, 128);
static THD_FUNCTION(on_interrupt, c) {
    //Catch interrupt this channel
    //do what ever you want
    Comm *cp = (Comm*)c;
    uint32_t mask = 0 << 31;

    while(true) {
        chBSemWait(&lock);
        //set the digital input pin
        mask |= (1<<cp->interrupt_channel);
        cp->digital_io |= mask;

        //send the new IO's
        cp->send(cp);
        chThdSleepMilliseconds(500);
    }
}

//SERIAL2 virtual_rx
static THD_WORKING_AREA(comm_protocol_read_virtual_rx, 128);
static THD_FUNCTION(read_virtual_rx, c) {
    //Catch interrupt this channel
    //do what ever you want
    Comm *cp = (Comm*)c;
    uint8_t virtual_rx_buffer[5]; //sizeof cp->virtual_rx
    int i = 0;
    while(true) {
        //sdRead(&SD2, virtual_rx_buffer, 5);
        //update cp->virtual_rx_buffer
        for(i = 0; i < 5; i++) {
            cp->virtual_rx[i] = virtual_rx_buffer[i];

        }
        //send new telegram here
        //cp->send(cp);
        chThdSleepMilliseconds(500);
    }
}


//RELATED FOR INTERRUPT
//Interrupt callback
static void extcb1(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  extChannelDisable(&EXTD1, channel);
  c->interrupt_channel = channel;
  //sdWrite(&SD1, &(c->interrupt_channel), sizeof(c->interrupt_channel) );
  chSysLockFromISR();
  chBSemSignalI(&lock);
  chSysUnlockFromISR();
  extChannelEnable(&EXTD1, channel);

}
//Interrupt config
//Channel 0 -- Edge catching | start mode | Which Pad Group
static const EXTConfig extcfg = {
  {
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extcb1},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL},
    {EXT_CH_MODE_DISABLED, NULL}
  }
};
//#INTERRUPT



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
    chBSemObjectInit(&lock, true);
    extStart(&EXTD1, &extcfg);
    extChannelEnable(&EXTD1, 0);

    palSetPadMode(GPIOB, 1, PAL_MODE_OUTPUT_PUSHPULL);      //button
    palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(1));       /* USART1 TX.       */
    palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(1));      /* USART1 RX.       */


    palClearPad(GPIOB, 1);
    c = new_Comm(&__c, 0xAA);
    //this thread must be create after defined c.
    chThdCreateStatic(comm_protocol_on_interrut, sizeof(comm_protocol_on_interrut), NORMALPRIO, on_interrupt, (void*)c);
    chThdCreateStatic(comm_protocol_read_virtual_rx, sizeof(comm_protocol_read_virtual_rx), NORMALPRIO, read_virtual_rx, (void*)c);

    //Example
    int i;
    for(i = 0; i < 8; i++) {
        c->set_digital(c, i, 1);
    }
    c->send(c);

    sdWrite(&SD1, "\n", 1);
    for(i = 0; i < 8; i++) {
        c->set_digital(c, i, 0);
        sdWrite(&SD1, "\n", 1);
        c->send(c);
    }

    //c->set_digital(c, 0, 1);
    //#Example
    //c->send(c);
    /*
    //Example
    c->read(c);
    int result = c->get_digital(c, 0);
    if(result) {
      palSetPad(GPIOA, 1);
      chThdSleepMilliseconds(1000);
      palClearPad(GPIOA, 1);

    }
    else {
      palSetPad(GPIOA, 1);
      chThdSleepMilliseconds(5000);
      palClearPad(GPIOA, 1);

    }
    //#Example
    */
    while (1) {
      if(true) {
          c->read(c);
          chThdSleepMilliseconds(200);
          //chThdSleepMilliseconds(5000);
      }
    }
}
