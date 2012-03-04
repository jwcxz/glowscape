/* A C R I S   P R O J E C T ********
 * LED Controller                   *
 * http://jwcxz.com/projects/acris  *
 *                                  *
 * J. Colosimo -- http://jwcxz.com/ *
 *                                  *
 * LED actions                      *
 ************************************/

#include "led.h"
#include "tlc.h"

void led_action(void) {
    // this is just a wrapper function that decides what to do based on the
    // current command
    
    led_set();

    /* 
    // delete the default call to led_set() above and replace it with the
    // following switch statement to support multiple types of actions
    switch (action) {

        default:
            led_set();
            break;
    }
    */

    tlc_drive();
}

void led_set(void) {
    uint8_t i;

    for ( i=0 ; i<16 ; i++ ) {
        set(tlc, i, ser2led(args[i]));
    }
}
