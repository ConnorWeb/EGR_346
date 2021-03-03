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
void seperate();


uint8_t UART_read;      //used to check if there has been a UART communication

//lab 2
uint8_t data;
uint8_t dataBack;
uint16_t address = 0;
char pc_string[BUFFER_SIZE];
char command[BUFFER_SIZE];
char extra[BUFFER_SIZE];
char action;

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
            //seperate();
            printf("whole string from pc is: %s\ncommand is: %s\n extra is: %s\n\n", pc_string, command, extra);  //uncomment this line if the debugger is connected and you want to see the value

            //STORE
            /*if(strcmp(command, "store") == 0){
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
            }
            //CLEAR
            if(strcmp(command, "clear") == 0){
                //clear_all()
            }*/
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

//needs work. mostly copied from EGR326 SPI lab
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

//needs work. mostly copied from EGR326 SPI lab
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

//seperates the command from pc
//ex. command is "STORE testfile.txt" -> command array will have "STORE" and extra array will have "testfile.txt"
void seperate(){
    int i, j = 0, got_command = 0;
    for(i = get_head() - get_length(); i < get_head; i++){
        if(command[j] == ' '){
            got_command = 1;
            j = 0;
            continue;
        }
        else if(command[j] == '\0'){
            extra[0] = '\0';
            return;
        }
        if(!got_command){
            command[j] = tolower(pc_string[i]);
        }
        else{
            extra[j] = tolower(pc_string[i]);
        }
        j++;
    }
}

