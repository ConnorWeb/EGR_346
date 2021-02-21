/*
 * Serial.h
 *
 *  Created on: Feb 8, 2021
 *      Author: dakot
 */
//included libraries
#include <msp.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#ifndef SERIAL_H_
#define SERIAL_H_

//Serial.c Function prototypes
void setupSerial(void);
int readBuffer(void);
void writeOutput(int send);

#define EUSCI_Reset                   0x0001  // EUSCI in reset mode
#define EUSCI_Interupt                0x0001  // enable read interrupt
#define     BUFFER_SIZE               255     // Serial RX/TX buffer size

//circular buffer that stores the input from UART
typedef struct{
    char buf[BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
}serbuf;


#endif /* SERIAL_H_ */
