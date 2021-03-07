/*
Author: Andrew Kornacki
Date: 2/18/21
Class: EGR 436 10
Title: Lab 2 FRAM
*/

#include "pc_uart.h"

char user[100] = "";   //user input variable

int main(void)
{
    initialize_comm(&hMasterCOM,UART_PORT);
    printf("\n---------------------------------------------------------------------------------------\n");
    printf("STORE <filename> - store file as an entry to FRAM.\n");
    printf("DIR - show a list of the files stored in FRAM.\n");
    printf("MEM - display how much memory is used and how much is available.\n");
    printf("DELETE <number> - delete entry from FRAM and defragment FRAM to effectively fill space.\n");
    printf("READ <number> - display title and text of the entry.\n");
    printf("CLEAR - erase all memory from FRAM.\n");
    printf("---------------------------------------------------------------------------------------\n\n");

    while(1){
        while(user[0] == '\0'){  //wait for input
            gets(user);
            fflush(stdin);
        }
        if(strcmp(user, "q") == 0){      //close communication port
            CloseHandle(&hMasterCOM);
            break;
        }
        else{
            send_command(user, strlen(user));    //send command to msp
            user[0] = '\0';
        }
    }
}

void send_command(BYTE command[256], DWORD length){

    DWORD dww;
    DWORD dwr;

    BYTE temp[256];

    int i;
    for(i = 0; i < 256; i++){
        temp[i] = command[i];        //copy into local array. This fixed an issue with data not going through correctly
    }

    strcat(command,"\0");  //write buffer string is appended with null character to indicate end of write

    if(WriteData(hMasterCOM,&temp,length + 1,&dww)){
        printf("wrote %s successfully\n", temp);
    }
    else{
        printf("failed to write %s\n", temp);
    }
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
