#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"

#include "app_remap_led.h"

//static void linkTo(void);

// void toggleLED(uint8_t);

static linkID_t sLinkID1 = 0;
volatile int msgcn=0;
volatile uint8_t msgr[2];

volatile int state=0;
char intro[] = {"press the button on both controllers to initiate the connection\n\r"};
char connected[] ={"connected\n\r"};
char yourBuffer[50], toPrint[] = "Instructions: Start by clicking sequence\n\rSymbol inside letter - wait up to 2 sec\n\rTo start new letter - wait up to 4 sec\n\rTo start new word - wait for more than 4 sec\n\rPress -.-.. to see results\n\rEnter a line for the Morse challenge: ", toPrint1[]= "Morse translation: ", toPrint2[]="You wrote: ", toPrint3[] = "Challenge line was: ";
volatile int index=0, counter=0, arr, yourIndex=0;
volatile uint8_t arrIndex=0;
volatile char start=0, WordOrLetterOrChar=0, flag=0;
volatile char letter;
static const char *morseArray[] = {"-----",".----","..---","...--", "....-", ".....", "-....", "--...", "---..", "----.",    ".-","-...","-.-.","-..", ".","..-.","--.","....", "..",".---","-.-", ".-..","--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."};
const char lettersE[] = {'E','A','I','W','R','U','S','J','P','?','L','?','F','V','H','1','?','?','?','?','?','2','?','?','?','3','?','4','5'};
const char lettersT[] = {'T','M','N','O','G','K','D','?','?','Q','Z','Y','C','X','B','0','9','?','8','?','?','?','7','?','?','?','?','?','?','6','?'};
volatile char buffer[50];
uint8_t received;



/* application Rx frame handler. */
static uint8_t sRxCallback(linkID_t);

#define SPIN_ABOUT_A_SECOND  NWK_DELAY(1000)
extern int tempOffset;


#pragma vector=PORT1_VECTOR
__interrupt void Port1_ISR(void)
{
    if (P1IFG)
    {
        P1IFG = 0;
        LPM1_EXIT;
    }
}

void main (void)
{
    int i=0;
    volatile int len;

    BSP_Init();
    BCSCTL3 |= LFXT1S_2 ;
    TACCR0 = ~0x0;
    TACCTL0 = CCIE;
    TACCR0 = 65535;
    P1DIR = 3;                     // set p1 as input
    P1REN |= BIT2;                  // pullup resistor on p1.3, the button
    P1IES |= BIT2;                  // interrupt on push button
    P1IE = BIT2;                    // enable interrupts on p1.3

    SMPL_Init(sRxCallback);

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    UCA0CTL0 = 0x0;                 //  Receive/transmit lsb first, no parity, 8 data bits, one stop bit, uart, Asynchronous.
    UCA0CTL1 |= UCSSEL0 + UCSSEL1;  //  BRCLK SMCLK
    UCA0BR0 = 104;                  //  Set Baud Rate to 9600
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS0;              //  USBRSx=0b001 mod for baud rate
    P3SEL |= 0x30;
    UCA0CTL1 &= ~UCSWRST;           //  USCI Software Reset
    IE2 |= UCA0RXIE;                //  enable receive interrupt


    for(i=0; i<65; i++)  // print the intro
    {
        while(UCA0STAT&UCBUSY);
        UCA0TXBUF = intro[i];
    }

    P1IFG = 0;
    /* wait for a button press... */
    _BIS_SR(LPM1_bits+GIE);
    /* turn on LEDs. */
/*
     never coming back...
    linkTo();
    */
    while (SMPL_SUCCESS != SMPL_Link(&sLinkID1))
    {
        /* blink LEDs until we link successfully */
        SPIN_ABOUT_A_SECOND;
    }

    /* we're linked. turn off LEDs */

    for(i =0; i<11; i++)  // print the connected
    {
        while(UCA0STAT&UCBUSY);
        UCA0TXBUF = connected[i];
    }

    /* turn on RX. default is RX off. */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);


    while(1)
    {
        for(i=0; i<sizeof(toPrint); i++) // print the instructions
        {
            while(UCA0STAT&UCBUSY);
            UCA0TXBUF = toPrint[i];
        }

        index=0;
        yourIndex=0;
        start=0;
        counter=0;

        while(!start);  // start = 1 after user has ended input
        TAR=0;

        while(1)
        {

            while(state == 0);
            state = 0;
            TACTL = TASSEL_1 + MC_0;        // STOP clock to check passed time since last character

            if (TAR < 24000 && !start)
            {
                if (received == 1)
                {
                    arrIndex = arrIndex*2 +1;
                    while(UCA0STAT&UCBUSY);
                    UCA0TXBUF = '-';
                }
                else
                {
                    arrIndex=arrIndex*2+2;               //take the right son of father
                    while(UCA0STAT&UCBUSY);
                    UCA0TXBUF = '.';
                }

                if(arr)
                    letter = lettersE[arrIndex];            //which array we are searching
                else
                    letter = lettersT[arrIndex];        //the other array

                if(arrIndex==26){
                          arrIndex++;
                          break;
                }

            }






            else if(TAR < 48000 || start) //4 sec
            {
                if(start)
                    start=0;
                else{
                    yourBuffer[yourIndex++]=letter;
                    while(UCA0STAT&UCBUSY);
                    UCA0TXBUF = '\\';
                }


                arrIndex=0;

                 if(received==1){
                     letter = 'T';
                     arr=0;    //which array we are searching at
                     while(UCA0STAT&UCBUSY);
                     UCA0TXBUF = '-';
                 }
                 else{
                     letter= 'E';
                     arr=1;
                     while(UCA0STAT&UCBUSY);
                     UCA0TXBUF = '.';
                 }

            }






            else //more than 4
            {
                 yourBuffer[yourIndex++]=letter;
                 yourBuffer[yourIndex++]=32;
                 arrIndex = 0;
                 while(UCA0STAT&UCBUSY);
                 UCA0TXBUF = '&';
                 if(received==1){
                     letter = 'T';
                     arr=0;
                     while(UCA0STAT&UCBUSY);
                     UCA0TXBUF = '-';
                 }
                 else{
                     letter= 'E';
                     arr=1;
                     while(UCA0STAT&UCBUSY);
                     UCA0TXBUF = '.';
                 }

            }


            TAR=0;
            TACTL = TASSEL_1 + MC_1;


        }// while

          while(UCA0STAT&UCBUSY);
             UCA0TXBUF = 0xA;
          while(UCA0STAT&UCBUSY);
             UCA0TXBUF = 0xD;

        for(i=0; i<sizeof(toPrint2); i++) // print the instructions
        {
            while(UCA0STAT&UCBUSY);
            UCA0TXBUF = *(toPrint2+i);
        }

        for(i=0; i<yourIndex; i++) // print the instructions
        {
            while(UCA0STAT&UCBUSY);
            UCA0TXBUF = yourBuffer[i];
        }

        while(UCA0STAT&UCBUSY);
        UCA0TXBUF = '\n';
        while(UCA0STAT&UCBUSY);
        UCA0TXBUF = '\r';
        for(i=0; i<sizeof(toPrint3); i++) // print the instructions
        {
            while(UCA0STAT&UCBUSY);
            UCA0TXBUF = *(toPrint3+i);
        }
        for(i=0; i<index; i++) // print the instructions
        {
            while(UCA0STAT&UCBUSY);
            UCA0TXBUF = buffer[i];
        }


         while(UCA0STAT&UCBUSY);
              UCA0TXBUF = 0xA;
         while(UCA0STAT&UCBUSY);
              UCA0TXBUF = 0xD;
         while(UCA0STAT&UCBUSY);
              UCA0TXBUF = 0xA;
         while(UCA0STAT&UCBUSY);
              UCA0TXBUF = 0xD;

    }

}

#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
    TACTL = TASSEL_1 + MC_0;
}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void interrupt_usci(void){

    int j, i,ind;
    if (IFG2 & UCA0RXIFG){

    if(UCA0RXBUF!=0xD){                         //if not enter we check or the press
          UCA0TXBUF = UCA0RXBUF; //writing rx data to tx data on screen
          buffer[index++]=UCA0TXBUF; //save to buffer our result
    }//if

   else{

     while(UCA0STAT&UCBUSY);
        UCA0TXBUF = 0xA;
     while(UCA0STAT&UCBUSY);
        UCA0TXBUF = 0xD;

       for(i=0;i<sizeof(toPrint1);i++){
         while(UCA0STAT&UCBUSY);
         UCA0TXBUF = *(toPrint1+(char)i);
       }

      while(UCA0STAT&UCBUSY);
         UCA0TXBUF = 0xA;
      while(UCA0STAT&UCBUSY);
         UCA0TXBUF = 0xD;

       for(i=0;i<index;i++){

           if(buffer[i]>=48 && buffer[i]<=57){ //numbers
               ind = buffer[i]-'0';
           }

           else if(buffer[i]>=65 && buffer[i]<=90){ //caps lock
               ind = buffer[i] - 55;
           }
           else if(buffer[i]>=97 && buffer[i]<=122){//letters no caps lock
               ind = buffer[i] - 87;
           }
           else if(buffer[i]==32){//space
               while(UCA0STAT&UCBUSY);
               UCA0TXBUF = 38;
               continue;
           }
           j=0;
           while(morseArray[ind][j]!='\0'){
                  while(UCA0STAT&UCBUSY);
                  UCA0TXBUF = morseArray[ind][j++];
           }

           if(buffer[i+1]!=32 && i+1!=index){
           while(UCA0STAT&UCBUSY);
           UCA0TXBUF = 92;
           }

        }

       start = 1;

           while(UCA0STAT&UCBUSY);
              UCA0TXBUF = 0xA;
           while(UCA0STAT&UCBUSY);
              UCA0TXBUF = 0xD;
       }

    }
} // rx interrupt vector


/*void toggleLED(uint8_t which)
{
    if (1 == which)
    {
        BSP_TOGGLE_LED1();
    }
    else if (2 == which)
    {
        BSP_TOGGLE_LED2();
    }
    return;
}
*/

/* handle received frames. */
static uint8_t sRxCallback(linkID_t port)
{
    uint8_t len, tid;

    /* is the callback for the link ID we want to handle? */
    if (port == sLinkID1)
    {
        /* yes. go get the frame. we know this call will succeed. */
        if ((SMPL_SUCCESS == SMPL_Receive(sLinkID1, &received, &len)) && len)
        {
           state = 1;
           return 1;
        }
    }
    /* keep frame for later handling. */
    return 0;
}
