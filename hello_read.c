#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../ftd2xx.h"

#define DATARATE1 9600 
#define DATARATE2 19200
#define DATARATE3 38400
#define DATARATE4 57600
#define DATARATE5 115200

int main(){
    int datarate = DATARATE2;
    FT_HANDLE handle;
    FT_STATUS ftstatus;
    DWORD read;
    unsigned char buff[36962];

    mode_t old_mask;
    old_mask = umask(011);
    int fd = open("sample_image.jpg", O_WRONLY | O_TRUNC | O_CREAT , 0666);
    umask(old_mask);

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

    //FT_SetFlowControl(handle, FT_FLOW_RTS_CTS, 0, 0);

    fprintf(stderr, "start reading\n");

    if(FT_Read(handle, buff, 36962, &read) != FT_OK){
        fprintf(stderr, "could not read from usb\n");
    }
    

    else{
        write(fd, buff, 36962);
        close(fd);
        for(DWORD i=0; i<read; ++i){
            fprintf(stdout, "%02x ", buff[i]);
        }
        fprintf(stdout, "\n");
    }

    FT_Close(handle);
    sleep(2);
    fprintf(stderr, "closed the device\n");

}
