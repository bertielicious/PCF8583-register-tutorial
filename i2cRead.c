#include "config.h"
uchar i2cRead(void)
{
    uchar bcd = 0;
    SSPCON2bits.RCEN = 1;   // receive the data byte from the PCF8583 slave
    while(!SSPSTATbits.BF);  //wait for all 8 bits to be received
    bcd = SSPBUF;         // save received data in temp_data 
    SSPCON2bits.ACKDT = 1;      // prepare to send NACK
    SSPCON2bits.ACKEN = 1;      // initiate NACK
    while(ACKEN);               // wait for NACK to repeat
   
    return (((bcd>>4)*10) + (bcd & 0x0f)); // convert BCD to binary
}
