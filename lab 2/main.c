#include "msp.h"
#include <stdio.h>
#include "Serial.h"
#include "hardware.h"

#define WRSR    1      //write status register
#define WRITE   2      //Write memory code
#define READ    3      //Read memory code
#define WRDI    4      //Reset write enable launch
#define RDSR    5      //Read status register
#define WREN    6      //Set write enable launch
#define RDID    159    //Read device ID

void GPIO_pins_init();
void initialize_SysTick();
void SysTick_delay(uint16_t delay);
void SPI_init();
void FRAM_write(uint8_t command);
void readData(uint8_t command);

//lab 1
char action;
uint8_t SerialFlag = 0;     //flag that indicates a complete serial read
serbuf A;                   //circular buffer for UART storage

//lab 2
uint8_t data;
uint8_t dataBack;
uint16_t address = 0;

void main(void){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    setupSerial();
    GPIO_pins_init();
    SPI_init();             //set up SPI to communicate with 7 seg display
    initialize_SysTick();   //set up SysTick timer
    __enable_interrupt();   //enable interrupts

    while(1){
        if(SerialFlag){
            SerialFlag = 0;
            //STORE
            if(action == 'w'){
                FRAM_write(WRITE);
            }
            //DIR
            if(action == 'f'){
                //display_files()
            }
            //MEM
            if(action == 'm'){
                //show_memory()
            }
            //DELETE
            if(action == 'd'){
                //delete_file()
            }
            //READ
            if(action == 'r'){
                readData(READ);
            }
            //CLEAR
            if(action == 'c'){
                //clear_all()
            }
            printf("action: %c\n", action);
        }
    }
}

void GPIO_pins_init(){
    //P2.4 = CS, P2.5 = WP, P2.6 = HOLD
    P2->SEL0 &= ~(BIT4 | BIT5 | BIT6);
    P2->SEL1 &= ~(BIT4 | BIT5 | BIT6);
    P2->DIR  |=  (BIT4 | BIT5 | BIT6);
    P2->OUT  &= ~(BIT4 | BIT5 | BIT6);
}

void SPI_init(){
    //initializes EUSCI_B0 as SPI with baud rate 3MHz
    EUSCI_B0->CTLW0 = 0x0001;
    EUSCI_B0->CTLW0 = 0xAD83;
    //                0110 1101 1000 0011
    EUSCI_B0->BRW = 5;
    EUSCI_B0->CTLW0 &= ~0x0001;

    //P1.4 = STE, P1.5 = CLK, P1.6 = SIMO, P1.7 = SOMI
    P1->SEL0 |=  (BIT5 | BIT6 | BIT7);
    P1->SEL1 &= ~(BIT5 | BIT6 | BIT7);
}

void initialize_SysTick(){
    SysTick->CTRL = 0;          //disable while configuring
    SysTick->LOAD = 3000000;    //set a countdown of 1 second
    SysTick->VAL = 0;           //reset the count
    SysTick->CTRL = 5;          //enable SysTick with SMCLK
}

void SysTick_delay(uint16_t delay){
    SysTick->LOAD = delay * 3000 - 1;   //convert delay to ms
    SysTick->VAL = 0;
    while((SysTick->CTRL & 0x00010000) == 0);   //stays here while counting down
}

void FRAM_write(uint8_t command){
    P2->OUT &= ~BIT4;       //CS low
    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = command;

    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = address;

    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = data;

    SysTick_delay(10);
    P2->OUT |=  BIT4;       //CS high
    SysTick_delay(10);
}

void readData(uint8_t command){
    P2->OUT &= ~BIT4;       //CS low
    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = command;

    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        dataBack = EUSCI_B0->RXBUF;

    SysTick_delay(10);
    P2->OUT |=  BIT4;       //CS high
    SysTick_delay(10);
}

void setupSerial()
{
    P1->SEL0 |=  (BIT2 | BIT3); // P1.2 and P1.3 are EUSCI_A0 RX
    P1->SEL1 &= ~(BIT2 | BIT3); // and TX respectively.

    EUSCI_A0->CTLW0  |= BIT0; // Disables EUSCI. Default configuration is 8N1
    EUSCI_A0->CTLW0 |= BIT7; // Connects to SMCLK BIT[7:6] = 10
    /*******************************
     * Baud Rate Configuration in MCTLW register
     * UCBRSx:Fractional portion of baud rate integer(table 22-4)
     * UCBRFx:First stage modulation(table 22-3)
     * RESERVED: Bit 1-3  dont change ever
     * UCOS16: Bit 0 enables oversampling (if integer N is greater than 16 use oversampling)
     *
     * -----------------Configuration process for 3MHz SMCLK with baud rate 115200---------------------------
     * UCBRSx = 3000000/115200 = N = 26.041666 > 16 (use oversample) --> 3000000/(16*115200) = 1.6276
     * UCBRSx table value: 0.6276 -> (0xB5)
     * UCBRF = 0.628 * 16 = 10 (0x0A) (Remainder of the divide)
     * UCOS16 = 1 (oversampling enabled)
     */

    EUSCI_A0->BRW = 1;          //integer portion of baud rate division 3000000/(16*115200) = 1.628 -> N = 1
    EUSCI_A0->MCTLW = 0xB5A1;   //UCBRS (Bits 15-8) & UCBRF (Bits 7-4) & UCOS16 (Bit 0)
    EUSCI_A0->CTLW0 &= ~BIT0;   // Enable EUSCI
    EUSCI_A0->IFG &= ~BIT0;     // Clear interrupt
    EUSCI_A0->IE |= BIT0;       // Enable interrupt
    NVIC_EnableIRQ(EUSCIA0_IRQn);
    //__enable_interrupt();
}

//sends information to the serial port

void writeOutput(int send)
{
    EUSCI_A0->TXBUF = send;             //load the integer into the UART buffer
    while(!(EUSCI_A0->IFG & BIT1));     //wait until the send flag is seen
}

//interrupt for serial port
void EUSCIA0_IRQHandler(void)
{
    if (EUSCI_A0->IFG & BIT0)  // Interrupt on the receive line
    {
        action = EUSCI_A0->RXBUF;        //newest character goes to the head of the buffer
        //A.buf[A.head] = bpm;
        A.head = (A.head + 1) % BUFFER_SIZE;    //keeps the size of the buffer within a specific range
        EUSCI_A0->IFG &= ~BIT0;                 //Clear the interrupt flag right away in case new data is ready
        //if(A.buf[A.head - 1] == '\0'){          //set the flag that indicates a full read when a null character is read
            SerialFlag = 1;
        //}
    }
}

