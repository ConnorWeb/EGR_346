#ifndef PC_UART_H
#define PC_UART_H



#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <signal.h>
#include <math.h>


#define _RX_BUFFER_SIZE 256
#define UART_TIMEOUT 600
#define UART_PORT 3
#define UART_BAUD 115200
#define UART_PARITY NOPARITY
#define UART_BYTESIZE 8
#define UART_STOPBITS ONESTOPBIT


//leave these defined if you want to see the TX and RX on console
//comment out the preprocessor directive to quiet outputs
#define DEBUG_SHOW_WRITE
#define DEBUG_SHOW_READ

HANDLE hMasterCOM;
DCB dcbMasterInitState;
DCB dcbMaster;

volatile int UARTexitFlag;
typedef struct{
    char buf[_RX_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
}serbuf;

//Interrupt Vector
void intHandler(int dummy);

//Function Prototypes
void openComm(HANDLE* hComm, int comInt);
BOOL setSerialParams(HANDLE* phComm, DCB dcbMaster);
void setTimeouts(HANDLE*, COMMTIMEOUTS*);
BOOL writeComm(HANDLE* phComm, char* writeString);
void readComm(HANDLE* phComm, char* readString);
bool WriteData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten);
bool ReadData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwRead, UINT timeout);
void initialize_comm(HANDLE* hComm,int comInt);
void CloseCom(HANDLE* hComm, DCB dcbMasterInitState);
void send_command(BYTE command[], DWORD length);


#endif // PC_UART_H_INCLUDED
