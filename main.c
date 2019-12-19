/*
 * File:   main.c
 * Author: Phil Glazzard
 *
 * Created on 19 November 2019, 18:49
 */
/* Software Construction
 1. Problem definition
 * Control of a motorised door to the chicken coup where the door will be:
 a)  Open if the light level is daylight or the clock timer has been 
 * triggered to open the door, or if the manual open button has been pressed.
 b) Closed if the light level is night level or the clock timer has been 
 * triggered to close the door or the manual close button has been 
 * pressed
 2. Display on LCD the date, time, door open time, door close time, timer ON/OFF
 * light sensor ON/ OFF, manual OPEN/ CLOSE ON/ OFF and battery charge level on 
 * an LCD with three button keypad to control all of above. Confirm key presses
 *  via buzzer.
 3. Via blu-tooth connection, transmit all LCD display info to phone app, and 
 * also allow control of all door functions from the app.
 4. Microcontroller to be in sleep mode except when:
 * a) door needs to move from closed to open or open to closed due to timer or 
 * light sensor inputs or manual close/ open button.
 * b) someone presses a keypad button (time out back to sleep mode after 
 * 2 minutes without button activity)
 * (c) blue tooth transmit and receive
 2. Sub systems
 * a) LCD
 * b) uC sleep
 * c) blue tooth
 * d) door
 * e) timer/ clock
 * f) light sensor
 * g) keypad
 * h) solar psu
 * i) motor
 * 
 3. Structs
 * a) LCD
 *  date(day, month, year)
 *  time (hour, minutes, seconds)
 *  door (open time, close time)
 *  timer(on, off)
 *  set time(door open time, door close time)
 *  light sensor (on, off)
 *  light sensor (adjust up time, adjust down time)
 *  manual door button (open, close)
 *  battery charge level display (o% - 100%)
 *  keypad with three buttons (up, down, enter)
 *  confirm key press with buzzer (key pressed, key not pressed)
 * 
 * b) uC sleep
 * if (blue tooth active or button pressed or door needs to open or close (light sensor or timer))
 *    wake up
 *    run required code
 *    else
 *    sleep
 * 
 * c) blu-tooth
 *    transmit
 *    receive
 *    run necessary code
 * 
 * d) door
 *    open 
 *    close
 *    open or close door
 * 
 * e) clock/ timer
 *      check open
 *      check close
 *      open or close door
 * 
 * f) light sensor
 *      check light
 *      check dark
 *      open or close door
 * 
 * g) keypad
 *    check button press
 *       which button pressed
 *       setup screen or exit to main loop
 *       take action or open / close door
 *  
 * 
 * h) solar psu
 *      charge LiPo battery
 *      display battery charge level
 * 
 * i) motor
 *    motor moves clockwise = open door
 *    motor moves anti-clockwise = close door
 * 
 * 
 *                  16f1459
 *                  ---------
 *   +5 Volts    1 |Vdd      | 20 0 Volts
        LCD D6   2 |RA5   RA0| 19   - PUSH BUTTON
 *    motor ACW  3 |RA4   RA1| 18   + PUSH BUTTON
       MCLR      4 |RA3      | 17  MOTOR DIRECTION
 *  ENT PBUTTON  5 |RC5   RC0| 16  LIGHT SENSOR (analog)
 *    RS         6 |RC4   RC1| 15  RTC INPUT
 *    EN         7 |RC3   RC2| 14  TOP LIMIT SWITCH
 *    LCD D4     8 |RC6   RB4| 13  SDA
 *    LCD D5     9 |RC7   RB5| 12  LCD D7
 *    TX        10 |RB7   RB6| 11  SCL
 *                  ---------
 motor CW and BOTTOM LIMIT SWITCH need to be allocated  uC pin each
 */

#include "config.h"
#include "configOsc.h"
#include "configPorts.h"
#include "configUsart.h"
#include "putch.h"
#include <stdio.h>
#include "configLCD.h"
#include "pulse.h"
#include "nibToBin.h"
#include "byteToBin.h"
#include "configI2c.h"
#include "i2cStart.h"
#include "i2cWrite.h"
#include "i2cRestart.h"
#include "PCF8583Read.h"
#include "PCF8583Write.h"
#include "setupTime.h"
#include "dateInput.h"
#include "timeInput.h"
#include "clearRow.h"
#include "setupDate.h"
#include "decToBcd.h"

void main()
{
    configOsc();        // configure internal clock to run at 16MHz
    configPorts();      // configure PORTS A/ B/ C
    configUsart();      // allow serial comms to PC for debugging
    configLCD();        // 20 x 4 LCD set up for 4 bit operation
    configI2c();        // I2C setup with PCF8583 RTC chip
    printf("Hello!\n"); // test serial port
    uchar result = 0;
    uchar day = 23;
    printf("day in decimal %d\n", day);
    char bcd = 0;
    bcd = decToBcd(day);
    printf("day in bcd %d\n", bcd);
    uchar Monday[12] = "Monday";
    uchar Tuesday[12] = "Tuesday";
    uchar Wednesday[12] = "Wednesday";
    uchar Thursday[12] = "Thursday";
    uchar Friday[12] = "Friday";
    uchar Saturday[12] = "Saturday";
    uchar Sunday[12] = "Sunday";
   
    while(1)
    {
        result = PCF8583Read(0xa0, 0x02);                   //read seconds
        printf("seconds = %d\t", result);                   //print seconds
        
        result = PCF8583Read(0xa0, 0x03);                   //read minutes
        printf("minutes = %d\t", result);                   //print minutes
        
        result = PCF8583Read(0xa0, 0x04);                   //read hours
        printf("hours = %d\t", result);                     //print hours
        
        result = PCF8583Read(0xa0, 0x05);             //read days
        printf("days = %d\t", result & 0x3f);                      //print days
        
        result = PCF8583Read(0xa0, 0x06);               // read weekdays
        
        if((result&0xe0)>>5 == 0)
        {
            result = 0;
            printf("%s\t", Monday);
        }
        else if((result&0xe0)>>5 == 1)
        {
            result = 1;
            printf("%s\t", Tuesday);
        }
        else if((result&0xe0)>>5 == 2)
        {
            result = 2;
            printf("%s\t", Wednesday);
        }
        else if((result&0xe0)>>5 == 3)
        {
            result = 3;
            printf("%s\t", Thursday);
        }
        else if((result&0xe0)>>5 == 4)
        {
            result = 4;
            printf("%s\t", Friday);
        }
        else if((result&0xe0)>>5 == 5)
        {
            result = 5;
            printf("%s\t", Saturday);
        }
        else if((result&0xe0)>>5 == 6)
        {
            result = 6;
            printf("%s\t", Sunday);
        }
        //printf("weekday %s\t", result);
        
        
       
        
        result = PCF8583Read(0xa0, 0x06);             //read months
        printf("months = %d\t", result & 0x0f);                      //print months
        
        result = PCF8583Read(0xa0,0x05);             //read years
        if((result&0xc0)>>6 == 0x00)
        {
            result = 19;
        }
        else if((result&0xc0)>>6 == 1)
        {
            result = 20;
        }
        else if((result&0xc0)>>6 == 2)
        {
            result = 21;
        }
        else if((result&0xc0)>>6 == 3)
        {
            result = 22;
        }
        printf("years = %d\n", result);                      //print years
         __delay_ms(1000);                      
    }

}