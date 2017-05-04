#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "../ftd2xx.h"

#define DATARATE1 9600 
#define DATARATE2 19200
#define DATARATE3 38400
#define DATARATE4 57600
#define DATARATE5 115200

//#define HELLO_WORLD

int main(){
    int datarate = DATARATE1;
    FT_HANDLE handle;
    FT_STATUS ftstatus;
    DWORD written;
    DWORD index=0;
    unsigned char buff[9574] = "hello world!\n";
    struct stat st;

    if((ftstatus = FT_ListDevices((PVOID)index, buff, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER)) != FT_OK){
        fprintf(stderr, "could not get the device list\n");
        return -1;
    }

    fprintf(stdout, "serial number: %s\n", buff);

    //ftstatus = FT_OpenEx("FTG4EWXL", FT_OPEN_BY_SERIAL_NUMBER, &handle);
    ftstatus = FT_Open(0, &handle);
    if(ftstatus != FT_OK){
        fprintf(stderr, "could not open usb: %d \n", ftstatus);
        return -1;
    }

    if((ftstatus=FT_ResetDevice(handle))!=FT_OK){
        fprintf(stderr, "could not reset usb:%d\n", ftstatus);
        return -1;
    }

    if(FT_SetBaudRate(handle, datarate) != FT_OK){
        fprintf(stderr, "could not set baud rate\n");
        return -1;
    }

    ftstatus = FT_SetDataCharacteristics(handle, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE);
    if(ftstatus != FT_OK){
        fprintf(stderr, "could not set data characteristics\n");
        return -1;
    }

    //FT_SetBaudRate(&handle, 115200);
    int fd = open("./sample_tx.txt", O_RDONLY);
    if(fd<0){
        fprintf(stderr, "could not open the file\n");
        return -1;
    }
    fstat(fd, &st);
    read(fd, buff, st.st_size);
    close(fd);

#ifdef HELLO_WORLD   
    //HELLO WORLD VERSION"
    for(int i=0; i<10000; ++i){
        if(FT_Write(handle, buff, 14, &written) != FT_OK){
            fprintf(stderr, "could not write to usb\n");

        }
    

        else{
            for(int i=0; i<14; ++i){
                fprintf(stderr, "%02x ", buff[i]);
            }
            fprintf(stderr, "\n");
        }
    }
#endif   
#ifndef HELLO_WORLD
    //FT_SetFlowControl(handle, FT_FLOW_RTS_CTS, 0, 0);

    ftstatus = FT_Write(handle, buff, st.st_size, &written);
    if (ftstatus != FT_OK){
        fprintf(stderr, "could not write to usb\n");
    }

    else{
        for(int i=0; i<st.st_size; ++i){
            fprintf(stderr, "%02x ", buff[i]);
        }
        fprintf(stderr, "\n");
    }
#endif

    FT_Close(handle);
    fprintf(stderr, "closed the device\n");

}
