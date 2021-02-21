/*
Author: Andrew Kornacki
Date: 2/18/21
Class: EGR 436 10
Title: Lab 2 FRAM
*/


#include "pc_uart.h"

int bpm;            //blink rate that is sent to the MSP
int buf;            //Serial buffer variable
char user = '\0';   //user input variable
int tempo;          //blink rate that is received from the MSP


int main(void)
{
    initialize_comm(&hMasterCOM,UART_PORT);
    printf("\n\n w - write\t f - display files\t m - show memory\t d - delete file\t r - read\t c - delete all files.\n\n");

    while(1){
        while(user == '\0'){  //wait for input
            scanf("%c",&user);
            fflush(stdin);
        }
        if(user == 'w' || user == 'f' || user == 'm' || user == 'd' || user == 'r' || user == 'c'){
            send_command(user);    //send command to msp
            user = '\0';
        }
        else if(user == 'q'){      //close communication port
            CloseHandle(&hMasterCOM);
            break;
        }
        else{
            printf("\n\n%c",user);
            printf(" is invalid\n\n");
            user = '\0';
        }
    }
}

void send_command(char command){

    DWORD dww;
    DWORD dwr;

    BYTE wbuf; //serial write buffer
    BYTE rbuf; //serial read buffer

    wbuf = command;

    if(WriteData(hMasterCOM,&wbuf,1,&dww)){
        printf("wrote %c successfully\n", wbuf);
    }
    else{
        printf("failed to write %c\n", wbuf);
    }
    /*if(ReadData(hMasterCOM,&rbuf,1,&dwr,UART_TIMEOUT)){
        printf("read back %c successfully\n", rbuf);
    }
    else{
        printf("failed to read back %c\n", rbuf);
    }*/
}

void initialize_comm(HANDLE* hComm, int comInt){
    int purge = 0;
    int comstate = 0;
    int params = 0;
    printf("\n opening com %d\n",comInt);
    openComm(hComm, comInt);
    printf("\n attempting to purge com\n");
    purge = PurgeComm(*hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    if(purge == 0){
        printf("\nCom could not be purged\n");
        exit(0);
    }
     printf("\n retrieving com state\n");
     comstate = GetCommState(*hComm, &dcbMasterInitState);
     if(comstate == 0 ){
        printf("\n could not retrieve com %d state \n", comInt);
        exit(0);
     }
    printf("\n Setting Serial Parameters\n");
    params = setSerialParams(hComm, dcbMaster);
    if(params == 0){
        printf("\n Serial Parameters Were Not Set\n");
        exit(0);
    }
}
void openComm(HANDLE* phComm, int comInt){

    printf("creating port string\n\n");
    char portStr[13] = "\\\\.\\COM3";

    *phComm = CreateFile("\\\\.\\COM3",
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        0,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                        0);

    if( *phComm == INVALID_HANDLE_VALUE){
        printf("Error in opening serial port... Exiting\n\n");
        exit(0);

    }
    else{
        printf(portStr);
        printf(" port has been connected");
    }
}

BOOL setSerialParams(HANDLE* phComm, DCB dcbMaster){
    bool status;

    dcbMaster.BaudRate = UART_BAUD;
    dcbMaster.Parity = UART_PARITY;
    dcbMaster.ByteSize = UART_BYTESIZE;
    dcbMaster.StopBits = UART_STOPBITS;

    status = SetCommState(*phComm, &dcbMaster);

    return(status);
}

bool WriteData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten){
     bool success = false;
     OVERLAPPED o = {0};
     o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
     if (!WriteFile(handle, (LPCVOID)data, length, dwWritten, &o)){
         if (GetLastError() == ERROR_IO_PENDING)
             if (WaitForSingleObject(o.hEvent, INFINITE) == WAIT_OBJECT_0)
                 if (GetOverlappedResult(handle, &o, dwWritten, FALSE))
                 success = true;
        }
     else
     success = true;
     if (*dwWritten != length)
     success = false;
     CloseHandle(o.hEvent);
     PurgeComm(handle, PURGE_TXABORT | PURGE_TXCLEAR);
     return success;
}

bool ReadData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwRead, UINT timeout){
     bool success = false;
     OVERLAPPED o = {0};

     o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
     if (!ReadFile(handle, data, length, dwRead, &o)){
         if (GetLastError() == ERROR_IO_PENDING)
             if (WaitForSingleObject(o.hEvent, timeout) == WAIT_OBJECT_0)
             success = true;
             GetOverlappedResult(handle, &o, dwRead, FALSE);
        }
     else
     success = true;
     CloseHandle(o.hEvent);
     PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR);
     return success;
}

void CloseCom(HANDLE* hComm, DCB dcbMasterInitState){
    SetCommState(*hComm, &dcbMasterInitState);

    CloseHandle(*hComm);
    *hComm = INVALID_HANDLE_VALUE;
}
