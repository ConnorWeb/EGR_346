#ifndef __SERIAL_H_
#define __SERIAL_H_


void setupSerial(void);
void readInput(char*);
void writeOutput(char*);

#define EUSCI_Reset                   0x0001  // EUSCI in reset mode
#define EUSCI_Oversample              0x00    // disable oversampling
#define EUSCI_CTL_Reg                 0x0081  // no parity, 1 stop bit, 8-bit, SMCLK
#define EUSCI_BRW                     0x1A    // SMCLK / 115200 = 26
#define EUSCI_Interupt                0x0001  // enable read interrupt
#define     BUFFER_SIZE               100     // Serial RX/TX buffer size


char INPUT_BUFFER[BUFFER_SIZE];
char output[100];
uint8_t storage_location;   // used in the interrupt to store new data
uint8_t read_location;      // used in the main application to read valid data that hasn't been read yet

#endif  //__SERIAL_H_
