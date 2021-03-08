/***************
 * CLASS : EGR 436 EMBEDDED SYSTEMS INTERFACE
 * PROFESSORS : JIAO AND BRAKORA
 * DATE : 2/18/21
 * LAB GROUP MEMBERS: DAKOTA CULBERTSON  ANDREW KORNAKI  CONNOR WEBSTER
 * TITLE: LAB 2 UART SPI FRAM
 * DESCRIPTION: THIS CODE STORES AND RETRIEVES FILES FROM AN FRAM CHIP
 */

#include "msp.h"
#include <stdio.h>
#include "Serial.h"
#include "hardware.h"
#include <ctype.h>

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
uint8_t seperate();
void clear();


uint8_t UART_read;      //used to check if there has been a UART communication

//lab 2
uint8_t data;
char dataBack;
uint16_t address = 50;
char pc_string[BUFFER_SIZE];
char command[BUFFER_SIZE];
char extra[BUFFER_SIZE];

int main(){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;        // stop watchdog timer

    setupSerial();
    GPIO_pins_init();
    SPI_init();             //set up SPI to communicate with 7 seg display
    initialize_SysTick();   //set up SysTick timer
    __enable_interrupt();   //enable interrupts

    while(1){
        UART_read = check_read();
        if(UART_read){
            readBuffer(pc_string); //read in the value in the buffer to change the blink rate
            if(seperate()){
                printf("whole string from pc is: %s\ncommand is: %s\n extra is: %s\n\n", pc_string, command, extra);  //uncomment this line if the debugger is connected and you want to see the value

                //STORE
                if(strcmp(command, "store") == 0){
                    FRAM_write(WRITE);
                }
                //DIR
                if(strcmp(command, "dir") == 0){
                    //display_files()
                }
                //MEM
                if(strcmp(command, "mem") == 0){
                    //show_memory()
                }
                //DELETE
                if(strcmp(command, "delete") == 0){
                    //delete_file()
                }
                //READ
                if(strcmp(command, "read") == 0){
                    readData(READ);
                    printf("data back char: %c\n\n", dataBack);
                }
                //CLEAR
                if(strcmp(command, "clear") == 0){
                    //clear_all()
                }
            }
            else{
                printf("invalid command!!!\n");
            }
            clear();
        }
    }
}

void GPIO_pins_init(){
    //P2.4 = CS, P2.5 = WP, P2.6 = HOLD
    P2->SEL0 &= ~(BIT4 | BIT5 | BIT6);
    P2->SEL1 &= ~(BIT4 | BIT5 | BIT6);
    P2->DIR  |=  (BIT4 | BIT5 | BIT6);
    P2->OUT  |=  (BIT4 | BIT5 | BIT6);
}

void SPI_init(){
    //initializes EUSCI_B0 as SPI with baud rate 3MHz
    EUSCI_B0->CTLW0 = 0x0001;
    EUSCI_B0->CTLW0 = 0xAD83;   //maybe change this to get SPI communication with FRAM chip to work
    //                0b0110 1101 1000 0011 -> binary of above hex command
    EUSCI_B0->CTLW0 &= ~0x0001;

    //P1.4 = STE, P1.5 = CLK, P1.6 = SIMO, P1.7 = SOMI, what is P1.4 for? it's a pushbutton on the msp anyway
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

//needs work. mostly copied from EGR326 SPI lab
void FRAM_write(uint8_t command){
    P2->OUT &= ~BIT4;       //CS low

    //need to end op code first, then address, then data
    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = command;

    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = address;  //data sheet shows address is 13 bits but TXBUF is 8 bits...

    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = 'A';      //try to send one character to FRAM

    P2->OUT |=  BIT4;       //CS high
}

//needs work. mostly copied from EGR326 SPI lab
void readData(uint8_t command){
    P2->OUT &= ~BIT4;       //CS low

    //need to end op code first, then address, then read data
    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = command;

    while(!(EUSCI_B0->IFG & 2));    //wait until ready to transmit
        EUSCI_B0->TXBUF = address;  //data sheet shows address is 13 bits but TXBUF is 8 bits...

    while(!(EUSCI_B0->IFG & 1));    //wait until ready to receive. 1->go, 0->waiting in loop still
        dataBack = EUSCI_B0->RXBUF;

    P2->OUT |=  BIT4;       //CS high
}

//seperates the command from pc
//ex. command is "STORE testfile.txt" -> command array will have "STORE" and extra array will have "testfile.txt"
//returns 0 or 1 for if the command was determined to be valid or not. If the command had 2 or more spaces it's invalid,
//otherwise it's valid
uint8_t seperate(){
    int i, j = 0, got_command = 0;
    for(i = 0; i < get_head(); i++){
        if(pc_string[i] == ' '){
            if(got_command){
                return 0;       //invalid command from pc if there is more than one space char -> more than 2 words
            }
            got_command = 1;
            j = 0;
            continue;
        }
        else if(pc_string[i] == '\0'){
            if(!got_command){
                extra[0] = '\0';    //extra should be empty if command from pc is only one word
            }
            return 1;
        }
        if(!got_command){
            command[j] = tolower(pc_string[i]);
        }
        else{
            extra[j] = tolower(pc_string[i]);
        }
        j++;
    }
    return 1;
}

void clear(){
    int i;
    for(i = 0; i <= BUFFER_SIZE; i++){
        pc_string[i] = 0;
        command[i] = 0;
        extra[i] = 0;
    }
}

