/* A C R I S   P R O J E C T ********
 * LED Controller                   *
 * http://jwcxz.com/projects/acris  *
 *                                  *
 * J. Colosimo -- http://jwcxz.com/ *
 *                                  *
 * LED controller main loop         *
 ************************************/

#include "main.h"

#include "dbgled.h"
#include "eeprom.h"
#include "led.h"
#include "tlc.h"
#include "uart.h"

volatile uint8_t uart_rxbuf[UART_RX_BUFSZ];
volatile uint8_t *uart_rxbuf_iptr = uart_rxbuf;
volatile uint8_t *uart_rxbuf_optr = uart_rxbuf;
volatile uint8_t uart_rxbuf_count = 0;
volatile uint8_t rxen = 0;

uint8_t instaddr = 0;

volatile uint8_t tlc[24];

uint8_t action;         // current action
uint8_t numargs;        // number of arguments to expect
uint8_t args[16];       // array to store arguments
uint8_t* argptr = args; //   ... associated pointer

/* COMMAND PROCESSOR STATE MACHINE */
#define CST_IDLE    0
#define CST_SYNC    1
#define CST_ARGS    2
static uint8_t cmdstate;

int main(void) {
    // initialize debug LEDs
    dbg_init();

    // initialize TLC
    tlc_init();
    // set everything off
    tlc_drive();

    // get the address of the device
    instaddr = get_addr();

    // initialize UART
    uart_init();

    // show address of the device on the debug LEDs
    dbg_set(0x0);
    //_delay_ms(100);
    //show_addr();
    
    // enable interrupts
    sei();
    rxen = 1;

    uint8_t i = 0;

    while (1) {
        if ( UCSR0A & (_BV(FE0) | _BV(DOR0) | _BV(UPE0)) ) {
            cli();
            // reset the UART
            UCSR0B = 0;
            UCSR0B = _BV(RXCIE0) | _BV(RXEN0);
            sei();
        }

        if ( rxen == 0 && uart_rxbuf_count < UART_RX_BUFSZ/2 ) {
            // buffer has been partially depleted, so we can start accepting
            // data again
            dbg_off(DBG_OVFLWERR);
            rxen = 1;
            sei();
        }

        if ( uart_data_rdy() ) {
            receive_data();
        }
    }

	return 0;
}

void receive_data(void) {
    unsigned char inbyte;

    inbyte = uart_rx();

    if ( inbyte == CMD_SYNC )  {
        // the sync byte is always treated as a trigger to reset the state
        // machine -- never send it as an argument
        cmdstate = CST_SYNC;
    } else {
        switch (cmdstate) {
            case CST_IDLE:
                cmdstate = CST_IDLE;
                break;
            case CST_SYNC:
                // save command for later processing
                action = inbyte;
                argptr = args;

                if ( inbyte == instaddr ) {
                    numargs = 16;
                    cmdstate = CST_ARGS;
                } else if ( (inbyte >= 0xF0 && inbyte <= 0xFE) && 
                            (instaddr >= (inbyte&0x0F)*16 && 
                                instaddr <= (inbyte&0x0F)*16+15) ) {
                    numargs = 16;
                    cmdstate = CST_ARGS;
                } else if ( inbyte == CMD_DOALL ) {
                    numargs = 16;
                    cmdstate = CST_ARGS;
                } else {
                    cmdstate = CST_IDLE;
                }
                break;

            case CST_ARGS:
                *argptr++ = inbyte;

                if ( argptr - args == numargs ) {
                    cmdstate = CST_IDLE;
                    led_action();
                } else {
                    cmdstate = CST_ARGS;
                }
                break;

            default:
                // the hell!?
                cmdstate = CST_IDLE;
                break;
        }
    }
}
