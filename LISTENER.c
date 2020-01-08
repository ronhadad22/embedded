#include <msp430.h>
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"

volatile linkID_t sLinkID2 = 0;
volatile int state=0;

static uint8_t sRxCallback(linkID_t);

#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A1(void)
{
    TACTL = TASSEL_1 + MC_0;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port1_ISR1(void)
{
    uint8_t in;
    P1IFG = 0;
    if (state == 0)
    {
        state++;
        LPM1_EXIT;
    }
    else
    {
        if (P1IES & BIT2) //press
        {
            TACTL = TASSEL_1 + MC_0; //stop
            if(TAR>1000||state==1){
                if(state==1)
                    state++;
                P1IES &= ~BIT2;
            }
            TAR = 0;
            TACTL = TASSEL_1 + MC_1; //start
        }
        else
        {

            TACTL = TASSEL_1 + MC_0;        // STOP clock to check press time

                P1IES |= BIT2;
                if(TAR>7000)                    //7:12 sec = line
                {
                    in = 1;
                }
                else
                {
                    in = 0;                   //less = dot
                }
                SMPL_Send(sLinkID2,&in,1);
            TAR=0;
            TACTL = TASSEL_1 + MC_1; //start
            P1REN |= BIT2;
        }

    }
}




int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    BCSCTL3 |= LFXT1S_2 ;
    TACCR0 = ~0x0;
    TACCTL0 = CCIE;
    TACCR0 = 65535;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    P1DIR = 3;                     // set p1.1 and p1.2 as output and p1.3 as input
    P1REN = BIT2;                  // pullup resistor on p1.3, the button
    P1IES = BIT2;                  // enable interrupts on p1.3
    P1IE = BIT2;                    // get the interrupt from pressing the button
    _BIS_SR(GIE);

    SMPL_Init(sRxCallback);

    _BIS_SR(LPM1_bits+GIE);


    while (SMPL_SUCCESS != SMPL_LinkListen(&sLinkID2));

    _BIS_SR(LPM1_bits+GIE);

    return 0;
}

static uint8_t sRxCallback(linkID_t port)
{
  uint8_t len;
  len =5;
  return 0;
}