/*
 * Thanks to the original developers "goebish" and "bikemike".
 * https://github.com/bikemike/nrf24_multipro/blob/rx_mode/nRF24_multipro/Bayang.ino
*/

#include <util/atomic.h>
#include <EEPROM.h>
#include "iface_nrf24l01.h"
#include <Servo.h>

Servo servo1, servo2, servo3, servo4;

#define RF_POWER TX_POWER_5mW
//#define RF_POWER TX_POWER_20mW
//#define RF_POWER TX_POWER_80mW 
//#define RF_POWER TX_POWER_158mW

//free pins
//pin                       0
//pin                       1
//pin                       10
//pin                       11
//pin                       12
//pin                       13
//pin                       A6
//pin                       A7

//pins for servos
#define pin_servo1          2
#define pin_servo2          3
#define pin_servo3          4
#define pin_servo4          5

//output pins of buttons
#define pin_output_flip     6
#define pin_output_rth      7
#define pin_output_headless 8
#define pin_output_invert   9

//pin LED RX on/off
#define pin_LED             A5

//SPI Comm.pins with nRF24L01
//#define pin_CE              A0 //comment if you connect the pin CE to 3.3V
#define pin_CSN             A1
#define pin_SCK             A2
#define pin_MOSI            A3
#define pin_MISO            A4

// SPI outputs
#define CE_on    //PORTC |= _BV(0)  //PC0, comment if you connect the pin CE to 3.3V
#define CE_off   //PORTC &= ~_BV(0) //PC0, comment if you connect the pin CE to 3.3V
#define CS_on    PORTC |= _BV(1)  //PC1
#define CS_off   PORTC &= ~_BV(1) //PC1
#define SCK_on   PORTC |= _BV(2)  //PC2
#define SCK_off  PORTC &= ~_BV(2) //PC2
#define MOSI_on  PORTC |= _BV(3)  //PC3
#define MOSI_off PORTC &= ~_BV(3) //PC3
// SPI input
#define  MISO_on (PINC & _BV(4))  //PC4

uint8_t transmitterID[4];
uint8_t packet[32];
static bool reset = true;

//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void setup()
{
//  Serial.begin(9600); //print value ??????on a serial monitor
  Serial.println("Start");

  pinMode(pin_LED, OUTPUT);
  digitalWrite(pin_LED, LOW); //start LED off
  
//  pinMode(pin_CE, OUTPUT); //comment if you connect the pin CE to 3.3V
  pinMode(pin_CSN, OUTPUT);
  pinMode(pin_SCK, OUTPUT);
  pinMode(pin_MOSI, OUTPUT);
  pinMode(pin_MISO, INPUT);
  
  pinMode(pin_output_flip, OUTPUT);
  pinMode(pin_output_rth, OUTPUT);
  pinMode(pin_output_headless, OUTPUT);
  pinMode(pin_output_invert, OUTPUT);
  
  servo1.attach(pin_servo1);
  servo2.attach(pin_servo2);
  servo3.attach(pin_servo3);
  servo4.attach(pin_servo4);
}

//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void loop()
{
  uint32_t timeout = 0;
  
  //reset/rebind
  if(reset)
  {
    reset = false;
    
    NRF24L01_Reset();
    NRF24L01_Initialize();
    Bayang_init();
    Bayang_bind();
  }
  
  timeout = process_Bayang();
  
  // wait before sending next packet
  while(micros() < timeout);
}
 
