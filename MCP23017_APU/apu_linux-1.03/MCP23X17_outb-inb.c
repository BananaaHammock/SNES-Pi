#include <stdint.h>
#include "parport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPiI2C.h>
#include "wiringPiSPI.h"
//#include "mcp23x0817.h"
#include <wiringPi.h>
#include <mcp23s17.h>
#include "MCP23X17_outb-inb.h"
#include <time.h>


#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


#define CHIPADDR 0x20
#define IOCON 0x0A
#define IODIRA 0x00
#define IPOLA 0x02
#define GPINTENA 0x04
#define DEFVALA 0x06
#define INTCONA 0x08
#define GPPUA 0x0C
#define INTFA 0x0E
#define INTCAPA 0x10
#define GPIOA 0x12
#define GPIOB 0x13
#define OLATA 0x14
#define IODIRB 0x01
#define IPOLB 0x03
#define GPINTENB 0x05
#define DEFVALB 0x07
#define INTCONB 0x09
#define GPPUB 0x0D
#define INTFB 0x0F
#define INTCAPB 0x11
#define OLATB 0x15



#define PA0_PIN 0x80
#define PA1_PIN 0x40
#define WR_PIN 0x20
#define RD_PIN 0x10
#define RESET_PIN 0x01
#define RESET_LED 0x08

#define ORIG_PA0_PIN 0x01
#define ORIG_PA1_PIN 0x02
#define ORIG_WR_PIN 0x04
#define ORIG_RD_PIN 0x08


#define GPIO_RD_PIN 4
#define GPIO_WR_PIN 5
#define GPIO_ADDR_0 6
#define GPIO_ADDR_1 10
#define GPIO_RESET  1
#define GPIO_LVLSFT_EN 11

/*
#define GPIO_RD_PIN 3
#define GPIO_WR_PIN 2
#define GPIO_ADDR_0 7
#define GPIO_ADDR_1 0
#define GPIO_RESET  5
#define GPIO_LVLSFT_EN 4
*/

#define GPIO_DATA_DIR  8

#define GPIO_DATA_D0      14
#define GPIO_DATA_D1      13
#define GPIO_DATA_D2      12
#define GPIO_DATA_D3      3
#define GPIO_DATA_D4      2
#define GPIO_DATA_D5      0
#define GPIO_DATA_D6      7
#define GPIO_DATA_D7      9

#define devId 0
#define CMD_WRITE         0x40
#define CMD_READ          0x41
#define USE_SPI_MODE      0
#define USE_I2C_MODE      0
#define USE_GPIO_CONTROL  1
#define USE_GPIO_DATA     1

#define DEBUG_MODE        0

//#define	MCP_SPEED	4000000

const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}



void nSleep(int nanosecs)
{
      struct timespec slptm;
    slptm.tv_sec = 0;
    slptm.tv_nsec = nanosecs;      //1000 ns = 1 us
    nanosleep(&slptm,NULL);

}



void write_GPIO_data(uint8_t data)
{
int i = 0;
int bits[8] = {0};

for (i=0;i<8;i++)
 if( (data & (1<<i) ) == (1<<i) )
   bits[i] = 1;   

digitalWrite(GPIO_DATA_D0, bits[0]); 
digitalWrite(GPIO_DATA_D1, bits[1]);
digitalWrite(GPIO_DATA_D2, bits[2]);
digitalWrite(GPIO_DATA_D3, bits[3]);
digitalWrite(GPIO_DATA_D4, bits[4]);
digitalWrite(GPIO_DATA_D5, bits[5]);
digitalWrite(GPIO_DATA_D6, bits[6]);
digitalWrite(GPIO_DATA_D7, bits[7]);
delayMicroseconds(40);//50);
}






void write_GPIO_CONTROL(uint8_t data)
 {
  init_MCP23S17();
  //digitalWrite (GPIO_LVLSFT_EN,0);

  static int  previous_PA0_PIN   = -1;
  static int  previous_PA1_PIN   = -1;
  static int  previous_WR_PIN    = -1;
  static int  previous_RD_PIN    = -1;
  static int  previous_RESET_PIN = -1;
  int current_PA0_PIN   = 0;
  int current_PA1_PIN   = 0;
  int current_WR_PIN    = 0;
  int current_RD_PIN    = 0;
  int current_RESET_PIN = 0;

  if ( (data & PA0_PIN) == PA0_PIN  )
    current_PA0_PIN = 1;
  if ( (data & PA1_PIN) == PA1_PIN  )
    current_PA1_PIN = 1;
  if ( (data &  WR_PIN) ==  WR_PIN  )
    current_WR_PIN = 1;
  if ( (data &  RD_PIN) ==  RD_PIN  )
    current_RD_PIN = 1;
  if ( (data & RESET_PIN) == RESET_PIN  )
    current_RESET_PIN = 1;

//printf("previous_PA1_PIN: %i\n", previous_PA1_PIN);
  if (  current_PA0_PIN != previous_PA0_PIN  )
  {
    digitalWrite (GPIO_ADDR_0,  current_PA0_PIN) ;
    previous_PA0_PIN = current_PA0_PIN;
  }
  if ( current_PA1_PIN != previous_PA1_PIN  )
  {
    digitalWrite (GPIO_ADDR_1,  current_PA1_PIN) ;
    previous_PA1_PIN = current_PA1_PIN;
  }
  if ( current_RD_PIN != previous_RD_PIN  )
  {
    digitalWrite (GPIO_RD_PIN,  current_RD_PIN) ;
    previous_RD_PIN = current_RD_PIN;
  }
  if ( current_WR_PIN != previous_WR_PIN  )
  {
    digitalWrite (GPIO_WR_PIN,  current_WR_PIN) ;
    previous_WR_PIN = current_WR_PIN;
  }
  if ( current_RESET_PIN != previous_RESET_PIN  )
  {
    digitalWrite (GPIO_RESET,  current_RESET_PIN) ;
    previous_RESET_PIN = current_RESET_PIN;
  }

delayMicroseconds(20);
 }




void write_MCP23017(int MCPregister, uint8_t data)
 {
  int fd = 0;
  fd = init_MCP23017(CHIPADDR);
  wiringPiI2CWriteReg8 (fd, MCPregister, (int)data);
 }



void write_MCP23S17(int MCPregister, uint8_t data)
 {
  initAll();
  uint8_t spiData [4] ;

  spiData [0] = CMD_WRITE | ((devId & 7) << 1) ;
  spiData [1] = (uint8_t)MCPregister ;
  spiData [2] = data ;
  wiringPiSPIDataRW (0, spiData, 3) ;
 }




uint8_t read_MCP23017_data()
 {
  int fd = 0;
  uint8_t data = 0;
  int value =0;
  fd = initAll();
  value = wiringPiI2CReadReg8 (fd, GPIOB);
  data = (uint8_t)value;
  return data;
 }



uint8_t read_MCP23S17_data()
 {
 initAll();
 uint8_t spiData [4] ;

  spiData [0] = CMD_READ | ((devId & 7) << 1) ;
  spiData [1] = GPIOB ;

  wiringPiSPIDataRW (0, spiData, 3) ;

  return spiData [2] ;
 }



uint8_t read_gpio_data()
{
uint8_t data = 0;

//delayMicroseconds(10);

  if (digitalRead (GPIO_DATA_D7) )
   data = data| 0x80;
  if (digitalRead (GPIO_DATA_D6) )
   data = data| 0x40;
  if (digitalRead (GPIO_DATA_D5) )
   data = data| 0x20;
  if (digitalRead (GPIO_DATA_D4) )
   data = data| 0x10;
  if (digitalRead (GPIO_DATA_D3) )
   data = data| 0x08;
  if (digitalRead (GPIO_DATA_D2) )
   data = data| 0x04;
  if (digitalRead (GPIO_DATA_D1) )
   data = data| 0x02;
  if (digitalRead (GPIO_DATA_D0) )
   data = data| 0x01;

//nSleep();
//usleep(1);
return data;
}


int changeDataPortDir(int direction)
{
static int currentDirection = -10;
if ( (direction == 1) | (direction == 0) )
 {
  if (direction != currentDirection)
   {
   
   if (USE_GPIO_DATA)
    change_GPIO_dir(direction);
   else if(USE_SPI_MODE)
    change_MCP23S17_dir(IODIRB, direction);
   else if (USE_I2C_MODE)
    change_MCP23017_dir(IODIRB, direction);

   currentDirection = direction;
   }
  }
 return currentDirection;
}


void change_GPIO_dir(int direction)
{

  if(direction == 1)
    {
    pinMode(GPIO_DATA_D0, INPUT);
    pinMode(GPIO_DATA_D1, INPUT);
    pinMode(GPIO_DATA_D2, INPUT);
    pinMode(GPIO_DATA_D3, INPUT);
    pinMode(GPIO_DATA_D4, INPUT);
    pinMode(GPIO_DATA_D5, INPUT);
    pinMode(GPIO_DATA_D6, INPUT);
    pinMode(GPIO_DATA_D7, INPUT);
    //delayMicroseconds(10);
    digitalWrite(GPIO_DATA_DIR,0);//A<B
    //usleep(2);
    }
  else if (direction == 0)
    {
    digitalWrite(GPIO_DATA_DIR,1);//A>B
//    delayMicroseconds(20);
    pinMode(GPIO_DATA_D0, OUTPUT);
    pinMode(GPIO_DATA_D1, OUTPUT);
    pinMode(GPIO_DATA_D2, OUTPUT);
    pinMode(GPIO_DATA_D3, OUTPUT);
    pinMode(GPIO_DATA_D4, OUTPUT);
    pinMode(GPIO_DATA_D5, OUTPUT);
    pinMode(GPIO_DATA_D6, OUTPUT);
    pinMode(GPIO_DATA_D7, OUTPUT);
    //usleep(2);
    }
//nSleep(100);
//usleep(1);
//delayMicroseconds(10);
}


void change_MCP23017_dir(int IODIRport, int direction)
{
initAll();
uint8_t value = 0;
if (direction == 1)
 value = 0xFF;
else
 value = 0;

write_MCP23017(IODIRport, value);
}

void change_MCP23S17_dir(int IODIRport, int direction)
{
 initAll();
 uint8_t value = 0;
 if (direction == 1)
  value = 0xFF;
 else
  value = 0;

 write_MCP23S17(IODIRport, value);
}






uint8_t remapSNESpins(uint8_t data)
{
uint8_t newData = 0x00;
int resetCounter = 0;

if ( (data & ORIG_PA0_PIN) == ORIG_PA0_PIN)
 newData = newData | PA0_PIN;
if ( (data & ORIG_PA1_PIN) == ORIG_PA1_PIN)
 newData = newData | PA1_PIN;

if ( (data & ORIG_RD_PIN) == ORIG_RD_PIN)
 newData = newData | RD_PIN;
else
 resetCounter++;

if ( (data & ORIG_WR_PIN) == ORIG_WR_PIN)
 newData = newData | WR_PIN;
else
 resetCounter++;

if ( ((data & ORIG_WR_PIN) == ORIG_WR_PIN) && ((data & ORIG_RD_PIN) == ORIG_RD_PIN) )
{
//newData = ((newData ^ 0xFF) | RD_PIN) ^ 0xFF; //clear RD Pin
}


if (resetCounter < 2)//Holds reset high, unless WR and RD are low
 newData = newData | (RESET_PIN + RESET_LED);

return newData;

}


void    outb_MCP23X17(uint8_t data,int port)
{
static uint8_t previous_data = 0;
initAll();


if (port == CONTROL)
 {
  if ( (data & 0x20) == 0x20 )//change direction bit
   changeDataPortDir(1);
  else
   changeDataPortDir(0);

  data = data ^ 0x0B;//Inverts control Pins 0, 1, and 3
  data = remapSNESpins(data);//Maps original pins of sch to GPIO pins

  if (previous_data != data)//Don't Rewrite control if not needed
   {
    if (USE_GPIO_CONTROL == 1)
      write_GPIO_CONTROL(data);
    else if (USE_SPI_MODE)
      write_MCP23S17(GPIOA, data ) ;
    else if (USE_I2C_MODE)
      write_MCP23017(GPIOA, data ) ;

    previous_data = data;
   }

  else
   return;
 }

else if (port == DATA)
{
 //changeDataPortDir(0);
 if (changeDataPortDir(-1) == 0)//If in output mode
  {
    if (USE_GPIO_DATA)
     write_GPIO_data(data);
    else if (USE_SPI_MODE)
     write_MCP23S17(GPIOB, data ) ;
    else if (USE_I2C_MODE)
     write_MCP23017(GPIOB, data ) ;
  }
 else 
  {
   printf("Nothing sent");
  }
}


//printf("%i ", hasBeenInit );
if (DEBUG_MODE)
{
printf("%s > OUT: ",KWHT);

if (port == DATA)
 printf("%s   DATA",KGRN );
else if (port == STATUS)
 printf("%s STATUS",KCYN );
else if (port == CONTROL)
 printf("%sCONTROL",KYEL );
else if (port == ECR)
 printf("%s   ECR",KMAG);
else
 printf("UNKNOWN PORT %u", port);

printf("| ");

printf("%03u-%s\n%s",data,byte_to_binary(data),KWHT);
}


}




uint8_t inb_MCP23X17(int port)
{
uint8_t data = 0;

changeDataPortDir(1);
if (changeDataPortDir(-1) == 1)//input mode
{
if (USE_GPIO_DATA)
 data = read_gpio_data();
else if (USE_SPI_MODE)
 data = read_MCP23S17_data();
else if (USE_I2C_MODE)
 data = read_MCP23017_data();

}
else
 data = 0;

if (DEBUG_MODE)
{
printf("%s <  IN: ",KWHT);

if (port == DATA)
 printf("%s   DATA",KGRN );
else if (port == STATUS)
 printf("%s STATUS",KCYN );
else if (port == CONTROL)
 printf("%sCONTROL",KYEL );
else if (port == ECR)
 printf("%s   ECR",KMAG);
else
 printf("UNKNOWN PORT %u", port);

printf("| ");

printf("%03u\n%s",data,KWHT);
}
 return  data;
}

int initAll()
{
static int fd = 0;
static int hasBeenInit = 0;

if (hasBeenInit)
 {
  if (USE_I2C_MODE)
   return fd;
  else
   return 0;
 } 

else
 {
   hasBeenInit = 1;
 
   if(USE_I2C_MODE)
    fd = init_MCP23017(CHIPADDR);
   else if (USE_SPI_MODE)
    init_MCP23S17();
   else
    wiringPiSetup () ;


   if(USE_GPIO_CONTROL == 1)
    {
     //pinMode (GPIO_LVLSFT_EN, OUTPUT) ;

    pinMode (GPIO_LVLSFT_EN, OUTPUT) ;
    digitalWrite (GPIO_LVLSFT_EN, 0) ;

     pinMode (GPIO_RD_PIN, OUTPUT) ;
     pinMode (GPIO_WR_PIN, OUTPUT) ;
     pinMode (GPIO_ADDR_0, OUTPUT) ;
     pinMode (GPIO_ADDR_1, OUTPUT) ;
     pinMode (GPIO_RESET,  OUTPUT) ;
     //digitalWrite (GPIO_LVLSFT_EN, 0) ;
    }

   if(USE_GPIO_DATA == 1)
    {
    //changeDataPortDir(1);
    pinMode(GPIO_DATA_DIR,OUTPUT);
    pinMode (GPIO_LVLSFT_EN, OUTPUT) ;
    digitalWrite (GPIO_LVLSFT_EN, 0) ;
    changeDataPortDir(1);

    }


     if(USE_I2C_MODE)
      return fd;
 }

return 0;
}


int init_MCP23017(int chipAddr)
{
static int fd = 0;
static int hasBeenInit = 0;
//int errCode = 0;
const char *device;
device = "/dev/i2c-0" ;

if (hasBeenInit == 1)//Don't Set Up
 return fd;

else//Set up
 {
 hasBeenInit = 1;
 fd = wiringPiI2CSetupInterface (device, chipAddr) ;

    if(USE_GPIO_CONTROL == 1)
     write_MCP23017(IODIRA, 0xFF);//Set Port A to inputs (CONTROL)
    else
     write_MCP23017(IODIRA, 0x00);//Set Port A to outputs (CONTROL)

 changeDataPortDir(1);//Set Port B to Inputs

 return fd;
 }

return -1;
}




int close_MCP23017(int chipAddr)
{
return 1;
}






void init_MCP23S17()
{
static int hasBeenInit = 0;

if (hasBeenInit == 1)//Don't Set Up
 return;

else//Set up
 {
 hasBeenInit = 1;

 wiringPiSetup () ;
 wiringPiSPISetup(0,10000000);

 if (USE_GPIO_CONTROL)
  change_MCP23S17_dir(IODIRA, 1);
 else
  change_MCP23S17_dir(IODIRA, 0);
 

 changeDataPortDir(1);//Set Port B to Inputs

 return;
 }

return;
}



int close_MCP23S17()
{
return 1;

}


