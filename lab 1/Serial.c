
//#include "msp432.h"
#include <Serial.h>
#include <Hardware.h>




void setupSerial(void);
void readInput(char *string);
void writeOutput(char *string);

void setupSerial()
{
    PC_RXD_PSEL0 |=  (PC_RXD_PIN|PC_TXD_PIN);           // P1.2 and P1.3 are EUSCI_A0 RX
    PC_RXD_PSEL1 &= ~(PC_RXD_PIN|PC_TXD_PIN);           // and TX respectively.

    PC_UART_CTL |= EUSCI_Reset;                         // EUSCI in reset mode
    PC_UART_MCTLW = EUSCI_Oversample;                   // disable oversampling
    PC_UART_CTL = EUSCI_CTL_Reg;                        // no parity, 1 stop bit, 8-bit, SMCLK
    PC_UART_BRW = EUSCI_BRW;                            // SMCLK / 115200 = 26
    PC_UART_CTL &= ~EUSCI_Reset;                        // EUSCI out of reset mode
    PC_UART_IE |= EUSCI_Interupt;                       // enable read interrupt

    NVIC_EnableIRQ(EUSCIA0_IRQn);                       // enable EUSCI interrupt handler
}

//read input from the serial port
void readInput(char *string)
{
    int i = 0;  // Location in the char array "string" that is being written to

    // One of the few do/while loops I've written, but need to read a character before checking to see if a \n has been read
    do
    {
        // If a new line hasn't been found yet, but we are caught up to what has been received, wait here for new data
        while(read_location == storage_location && INPUT_BUFFER[read_location] != '\n');
        string[i] = INPUT_BUFFER[read_location];  // Manual copy of valid character into "string"
        INPUT_BUFFER[read_location] = '\0';
        i++; // Increment the location in "string" for next piece of data
        read_location++; // Increment location in INPUT_BUFFER that has been read
        if(read_location == BUFFER_SIZE)  // If the end of INPUT_BUFFER has been reached, loop back to 0
            read_location = 0;
    }
    while(string[i-1] != '\n'); // If a \n was just read, break out of the while loop

    string[i-1] = '\0'; // Replace the \n with a \0 to end the string when returning this function
    read_location++;    //TODO: needed this to keep aligned with "storage_location", need to test when BUFFER_SIZE rolls over
}

//sends information to the serial port
void writeOutput(char *string)
{
    int i = 0;  // Location in the char array "string" that is being written to

    while(string[i] != '\0') {
        PC_UART_TXD = string[i];
        i++;
        while(!(PC_UART_IFG & BIT1));
    }
}
