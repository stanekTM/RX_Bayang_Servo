/*
 * Thanks to the original developers "goebish" and "bikemike".
 * https://github.com/bikemike/nrf24_multipro/blob/rx_mode/nRF24_multipro/Bayang.ino
*/

#include <util/atomic.h>
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
#define PIN_SERVO_1         2
#define PIN_SERVO_2         3
#define PIN_SERVO_3         4
#define PIN_SERVO_4         5

//output pins of buttons
#define PIN_OUT_FLIP        6
#define PIN_OUT_RTH         7
#define PIN_OUT_HEADLESS    8
#define PIN_OUT_INVERT      9

//pin LED RX on/off
#define PIN_LED             A5

//SPI Comm.pins with nRF24L01
//#define PIN_CE              A0 //comment if you connect the pin CE to 3.3V
#define PIN_CSN             A1
#define PIN_SCK             A2
#define PIN_MOSI            A3
#define PIN_MISO            A4

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
  //Serial.begin(9600); //print value on a serial monitor
  Serial.println("Start");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); //start LED off
  
//  pinMode(PIN_CE, OUTPUT); //comment if you connect the pin CE to 3.3V
  pinMode(PIN_CSN, OUTPUT);
  pinMode(PIN_SCK, OUTPUT);
  pinMode(PIN_MOSI, OUTPUT);
  pinMode(PIN_MISO, INPUT);
  
  pinMode(PIN_OUT_FLIP, OUTPUT);
  pinMode(PIN_OUT_RTH, OUTPUT);
  pinMode(PIN_OUT_HEADLESS, OUTPUT);
  pinMode(PIN_OUT_INVERT, OUTPUT);
  
  servo1.attach(PIN_SERVO_1);
  servo2.attach(PIN_SERVO_2);
  servo3.attach(PIN_SERVO_3);
  servo4.attach(PIN_SERVO_4);
}

//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void loop()
{
  uint32_t timeout = 0;
  
  //reset/rebind
  if (reset)
  {
    reset = false;
    
    NRF24L01_Reset();
    NRF24L01_Initialize();
    Bayang_init();
    Bayang_bind();
  }
  
  timeout = process_Bayang();
  
  // wait before sending next packet
  while (micros() < timeout);
}
 
