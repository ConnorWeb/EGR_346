/*
 * Serial.c
 *
 *  Created on: Feb 8, 2021
 *      Author: dakot
 */
//#include <msp.h>
//#include <stdio.h>
#include "Serial.h"

uint8_t SerialFlag = 0;     //flag that indicates a complete serial read
uint8_t read_length;        //length of the most recent read
//int storage_location = 0;   // used in the interrupt to store new data
serbuf A;                   //circular buffer for UART storage
//uint16_t read_location = A.head;      // used in the main application to read valid data that hasn't been read yet


int check_read(){
    if(SerialFlag){
        SerialFlag = 0;
        return 1;
    }
    else{return 0;}
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
    __enable_interrupt();
}

//read the values that are stored in the input buffer
//this function starts by starting at an offset read position and works backwards
//through the buffer until it sees a null value. when null is read, the values are
//then converted into a single integer that is returned to the main function.
void readBuffer(char str[BUFFER_SIZE]){
   // int i = 0;                          //incrementing value for converting buffer values into ints
    int k = 0;                          //incrementing value for reading from the buffer into a temporary array
   // char read;                          //variable used to store a single buffer value for conversion
   // int tempo = 0;                      //integer that is sent back as the blink speed
   // int temp = 0;                       //temporarily holds the integer value of the buffer character when it is converted
    int read_offset = 2;                //this offset value sets the read point in the UART buffer that should be the last character before null
    do{
        str[k] = A.buf[A.head - read_offset - k];   //read UART buffer value into temporary buffer
        k++; // Increment location in INPUT_BUFFER that has been read
    }
    while(str[k-1] != '\0');        //stop storing values when a null character is read
    /*while(Buffer_Read[i] != '\0'){          //convert values from the temporary buffer while they are not null
        read = Buffer_Read[i];              //take single character from the UART buffer
        temp = read - '0';                  //convert the value to an integer
        tempo = tempo + (temp * pow(10,i)); //add each value read from the temporary buffer into an integer
        i++;                                //increment read position
    }
    return(tempo);      //send back the tempo value*/
}

//interrupt for serial port
void EUSCIA0_IRQHandler(void)
{
    if (EUSCI_A0->IFG & BIT0)  // Interrupt on the receive line
    {
        A.buf[A.head] = EUSCI_A0->RXBUF;        //newest character goes to the head of the buffer
        A.head = (A.head + 1) % BUFFER_SIZE;    //keeps the size of the buffer within a specific range
        EUSCI_A0->IFG &= ~BIT0;                 //Clear the interrupt flag right away in case new data is ready
        if(A.buf[A.head - 1] == '\0'){          //set the flag that indicates a full read when a null character is read
            SerialFlag = 1;
        }
    }
}

int get_head(){
    return A.head;
}

int get_tail(){
    return A.tail;
}

int get_length(){
    return read_length;
}
