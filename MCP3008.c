#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "MCP3008.h"

#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])

static const char *DEVICE="/dev/spidev0.0";
static uint8_t MODE = SPI_MODE_0;
static uint8_t BITS = 8;
static uint32_t CLOCK = 1000000;
static uint16_t DELAY = 5;

static int Prepare(int fd);
//uint8_t ControlBitsDifferential(uint8_t cannel);
//uint8_t ControlBits(uint8_t channel);
//int ReadADC(int fd, uint8_t channel);
//void RunMCP();

// the Main methode changed to int[]
int * RunMCP()
{
    int fd = open(DEVICE, O_RDWR);
    static int value[8];
    if(fd <= 0)
    {
        printf("Device %s nicht gefunden\n", DEVICE);
        return value;
    }
    if(Prepare(fd)==-1)
        return value;
    uint8_t i;
    for(i = 0; i < 8; i++)
    {
        //printf("Channel %d: %d\n", i+1,ReadADC(fd, i));
        value[i] = ReadADC(fd, i);
    }
    //for(i = 0; i < 8; i++)
    //{
        //if(value[i]>0)
        //printf("Channel %d: %d\n", 6+1,value[6]);
    //}
    close(fd);
    return value;
}

// Given a prep'd descriptor, and an ADC channel, fetch the raw ADC value for the given channel
int ReadADC(int fd, uint8_t channel)
{
    uint8_t tx[] = {1, ControlBits(channel),0};
    uint8_t rx[3];
    struct spi_ioc_transfer tr =
    {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = ARRAY_SIZE(tx),
        .delay_usecs = DELAY,
        .speed_hz = CLOCK,
        .bits_per_word = BITS,
    };
    if(ioctl(fd, SPI_IOC_MESSAGE(1), &tr)==1)
    {
        perror("IO Error");
        abort();
    }
    return ((rx[1]<<8)&0x300) | (rx[2] & 0xFF);
}

// (SGL/DIF = 1, D2=D1=D0)
uint8_t ControlBits(uint8_t channel)
{
    return 0x8 | ControlBitsDifferential(channel);
}

// (SGL/DIF = 0, D2=D1=D0)
uint8_t ControlBitsDifferential(uint8_t channel)
{
    return (channel & 7) << 4;
}

// Ensure all settings are correct for the ADC
static int Prepare(int fd)
{
    if(ioctl(fd, SPI_IOC_WR_MODE, &MODE)==-1)
    {
        perror("Can't set MODE");
        return -1;
    }
    if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS)==-1)
    {
        perror("Can't set number of BITS");
        return -1;
    }
    if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK)==-1)
    {
        perror("Can't set write CLOCK");
        return -1;
    }
    if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK)==-1)
    {
        perror("Can't set read CLOCK");
        return -1;
    }
    return 0;
}