/*
Name: Nick Veltema & Cam Duce
Date: January 21, 2021
Class: EGR436
Assignment: Lab 1
Description:Establishes UART on COM Port 4 to transmit commands in beats per minute to a MSP432
microcontroller and receive confirmation of the command back.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <Windows.h>
#include <math.h>

#define BAUD_RATE   115200
#define BYTE_SIZE   8
#define BUFSIZE 4
#define READ_BUFSIZE    2
#define UART_TIMEOUT    60  //TODO: what should the timeout be



void init_UART(void);
void reset(void);
void inc_tempo(void);
void dec_tempo(void);
bool WriteData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten);
bool ReadData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwRead, UINT timeout);


int bpm;
int Buffer;

HANDLE hMasterCom;

int main()
{
    printf("Initialize UART\n");
    char u_input = '\0';

    init_UART();    // initialize UART serial communication
    reset();        // call reset to start tempo at 60bpm
    printf("u - Increase Tempo\nd - Decrease Tempo\nr - Reset Tempo\n");

    while(1)
    {
        while(u_input == '\0')  // wait for user input
        {
            scanf("%c", &u_input);
            fflush(stdin);
        }

        if(u_input == 'u')      // if "u" increase tempo
        {
            inc_tempo();
            u_input = '\0';
        }
        else if(u_input == 'd') // if "d" decrease tempo
        {
            dec_tempo();
            u_input = '\0';
        }
        else if(u_input == 'r') // if "r" reset tempo
        {
            reset();
            u_input = '\0';
        }
        else                    // notify user of incorrect input
        {
            u_input = '\0';
            printf("u - Increase Tempo\nd - Decrease Tempo\nr - Reset Tempo\n");
            printf("Please enter a valid command\n\n");
        }
    }
}

/*
Initialize UART on COM Port 4
*/
void init_UART(void)
{
    int purge_flag = 0;
    int dcb_flag = 0;
    int set_flag = 0;

    hMasterCom = CreateFile("\\\\.\\COM4",
       GENERIC_READ | GENERIC_WRITE,
       0,
       0,
       OPEN_EXISTING,
       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
       0);

    if(hMasterCom == INVALID_HANDLE_VALUE)
    {
        printf("Init COM port failed\n");
        return(1);
    }

    purge_flag = PurgeComm(hMasterCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR); //Purge open COM port
        if(!purge_flag)
        {
            printf("Purge COM port failed\n");
            return(1);
        }

    DCB dcbMasterInitState;
    dcb_flag = GetCommState(hMasterCom, &dcbMasterInitState);//Return non-zero if successful
        if(!dcb_flag)
        {
            printf("GetCommState failed\n");
            return(1);
        }

    DCB dcbMaster = dcbMasterInitState;

    dcbMaster.BaudRate = BAUD_RATE;
    dcbMaster.Parity = NOPARITY;
    dcbMaster.ByteSize = BYTE_SIZE;
    dcbMaster.StopBits = ONESTOPBIT;

    set_flag = SetCommState(hMasterCom, &dcbMaster);//Returns non-zero if successful
        if(!set_flag)
        {
            printf("SetCommState failed\n");
            return(1);
        }

    Sleep(60);  // delay 60ms
    printf("COM Initialization Successful\n");
    PurgeComm(hMasterCom, PURGE_RXABORT | PURGE_RXCLEAR); //Purge open COM port
}

void reset(void)
{

    DWORD dwW;
    DWORD dwR;

    bpm = 60;//initial BPM of the metronome

    //floor( log10(abs(bpm))) + 1  results in finding
    //the number of digits that a given decimal value has.
    // +2 is added to this for minimum buffer size for the writting
    Buffer = (floor( log10(abs(bpm))) + 1 ) + 2;

    char wbuf[Buffer];
    char rbuf[Buffer];
    rbuf[2] = '\0';

    itoa(bpm,wbuf,10);   // convert int to character string
    strcat(wbuf,"\n");

    WriteData(hMasterCom,wbuf,Buffer,&dwW);
    ReadData(hMasterCom,rbuf,Buffer-2,&dwR,UART_TIMEOUT);//the length of the buffer is exactly the length of the returned data when read. Hence the Buffer-2
    printf("Command Confirmed: %s BPM\n",rbuf);
}

void inc_tempo(void)
{

    DWORD dwW;
    DWORD dwR;

    bpm += 2;           // increase tempo by two beats per minute

    //floor( log10(abs(bpm))) + 1  results in finding
    //the number of digits that a given decimal value has.
    // +2 is added to this for minimum buffer size for the writting
    Buffer = (floor( log10(abs(bpm))) + 1 ) + 2;

    char wbuf[Buffer];
    char rbuf[Buffer];

    itoa(bpm,wbuf,10);   // convert int to character string
    strcat(wbuf,"\n");



    WriteData(hMasterCom,wbuf,Buffer,&dwW);
    ReadData(hMasterCom,rbuf,Buffer-2,&dwR,UART_TIMEOUT);//the length of the buffer is exactly the length of the returned data when read. Hence the Buffer-2
    printf("Command Confirmed: %s BPM\n",rbuf);
}

void dec_tempo(void)
{
    if(bpm <= 1)
    {
        printf("ERROR: The tempo can not be decreased below zero\n\n");
    }
    else
    {

        DWORD dwW;
        DWORD dwR;

        bpm -= 2;           // increase tempo by two beats per minute

        //floor( log10(abs(bpm))) + 1  results in finding
        //the number of digits that a given decimal value has.
        // +2 is added to this for minimum buffer size for the writting
        Buffer = (floor( log10(abs(bpm))) + 1 ) + 2;

        char wbuf[Buffer];
        char rbuf[Buffer];

        itoa(bpm,wbuf,10);   // convert int to character string
        strcat(wbuf,"\n");

        WriteData(hMasterCom,wbuf,Buffer,&dwW);
        ReadData(hMasterCom,rbuf,Buffer-2,&dwR,UART_TIMEOUT);//the length of the buffer is exactly the length of the returned data when read. Hence the Buffer-2
        printf("Command Confirmed: %s BPM\n",rbuf);
    }
}

bool WriteData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwWritten)
{
 bool success = false;
 OVERLAPPED o = {0};

 o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

 if (!WriteFile(handle, (LPCVOID)data, length, dwWritten, &o))
 {
     if(GetLastError() == ERROR_IO_PENDING)
        if(WaitForSingleObject(o.hEvent, INFINITE) == WAIT_OBJECT_0)
            if(GetOverlappedResult(handle, &o, dwWritten, FALSE))
            success = true;
 }
 else
    success = true;

 if(*dwWritten != length)
    success = false;

 CloseHandle(o.hEvent);
 PurgeComm(handle, PURGE_TXABORT | PURGE_TXCLEAR); //Purge open COM port
 return success;
}

bool ReadData(HANDLE handle, BYTE* data, DWORD length, DWORD* dwRead, UINT timeout)
{
 bool success = false;
 OVERLAPPED o = {0};

 o.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
 if(!ReadFile(handle, data, length, dwRead, &o))
 {
     if(GetLastError() == ERROR_IO_PENDING)
        if(WaitForSingleObject(o.hEvent, timeout) == WAIT_OBJECT_0)
            success = true;
        GetOverlappedResult(handle, &o, dwRead, FALSE);
 }
 else
    success = true;

 CloseHandle(o.hEvent);
 PurgeComm(handle, PURGE_RXABORT | PURGE_RXCLEAR); //Purge open COM port
 return success;
}





