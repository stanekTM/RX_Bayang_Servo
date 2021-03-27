/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License.
 If not, see <http://www.gnu.org/licenses/>.
*/

#define BAYANG_BIND_COUNT       1000
#define BAYANG_PACKET_PERIOD    2000
#define BAYANG_PACKET_SIZE      15
#define BAYANG_RF_NUM_CHANNELS  4
#define BAYANG_RF_BIND_CHANNEL  0
#define BAYANG_ADDRESS_LENGTH   5

static uint8_t Bayang_rf_chan = 0;
static uint8_t Bayang_rf_channels[BAYANG_RF_NUM_CHANNELS] = {0};
static uint8_t Bayang_rx_tx_addr[BAYANG_ADDRESS_LENGTH];

//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
uint32_t process_Bayang()
{
  uint32_t timeout = micros() + BAYANG_PACKET_PERIOD;
  
  Bayang_receive_packet();
  
  return timeout;
}

//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void Bayang_init()
{
  const uint8_t bind_address[BAYANG_ADDRESS_LENGTH] = {0, 0, 0, 0, 0};
  
  uint8_t i;
  for(i = 0; i < BAYANG_ADDRESS_LENGTH; i++)
  {
    Bayang_rx_tx_addr[i] = random() & 0xff;
  }
  
  Bayang_rf_channels[0] = 0x00;
  
  for(i = 1; i < BAYANG_RF_NUM_CHANNELS; i++)
  {
    Bayang_rf_channels[i] = random() % 0x42;
  }

/*
    //pevná adresa goebish
    const uint8_t bind_address[] = {0, 0, 0, 0, 0};
    memcpy(Bayang_rx_tx_addr, transmitterID, 4);
    Bayang_rx_tx_addr[4] = Bayang_rx_tx_addr[0] ^ 0xFF;
    Bayang_rf_channels[0] = 0x00;
    uint8_t i;
    for(i = 1; i < BAYANG_RF_NUM_CHANNELS; i++)
    {
      Bayang_rf_channels[i] = transmitterID[i] % 0x42;
    }
*/

  NRF24L01_Initialize();
  NRF24L01_SetTxRxMode(TX_EN);
  NRF24L01_SetTxRxMode(RX_EN);
  XN297_SetTXAddr(bind_address, BAYANG_ADDRESS_LENGTH);
  XN297_SetRXAddr(bind_address, BAYANG_ADDRESS_LENGTH);
  NRF24L01_FlushTx();
  NRF24L01_FlushRx();
  NRF24L01_WriteReg(NRF24L01_01_EN_AA, 0x00);      // No Auto Acknowldgement on all data pipes
  NRF24L01_WriteReg(NRF24L01_02_EN_RXADDR, 0x01);
  NRF24L01_WriteReg(NRF24L01_03_SETUP_AW, 0x03);
  NRF24L01_WriteReg(NRF24L01_11_RX_PW_P0, BAYANG_PACKET_SIZE); // rx pipe 0
  NRF24L01_WriteReg(NRF24L01_04_SETUP_RETR, 0x00); // no retransmits
  NRF24L01_SetBitrate(NRF24L01_BR_1M);             // 1Mbps
  NRF24L01_SetPower(RF_POWER);
  NRF24L01_Activate(0x73);                         // Activate feature register
  NRF24L01_WriteReg(NRF24L01_1C_DYNPD, 0x00);      // Disable dynamic payload length on all pipes
  NRF24L01_WriteReg(NRF24L01_1D_FEATURE, 0x01);
  NRF24L01_Activate(0x73);
  
  XN297_Configure(_BV(NRF24L01_00_EN_CRC) | _BV(NRF24L01_00_CRCO) | _BV(NRF24L01_00_PWR_UP) | _BV(NRF24L01_00_PRIM_RX));
  NRF24L01_WriteReg(NRF24L01_07_STATUS, 0x70);
  NRF24L01_FlushRx();
  
  delay(150);
}

//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void Bayang_bind()
{
  int bind_count = 0;
  uint8_t bind_packet[BAYANG_PACKET_SIZE] = {0};
  uint32_t timeout;
  
  digitalWrite(pin_LED, LOW);
  
  NRF24L01_WriteReg(NRF24L01_05_RF_CH, BAYANG_RF_BIND_CHANNEL);
  
  while(bind_count < 10)
  {
  timeout = millis() + 5;
    while(millis() < timeout)
    {
      delay(1);
      
      // data received from tx
      if(NRF24L01_ReadReg(NRF24L01_07_STATUS) & _BV(NRF24L01_07_RX_DR))
      {
        XN297_ReadPayload(packet, BAYANG_PACKET_SIZE);
        NRF24L01_WriteReg(NRF24L01_07_STATUS, 0x70); // Clear data ready, data sent, and retransmit
        NRF24L01_FlushRx();
        
        if(packet[0] == 0xA4)
        {
          if (0 == bind_count)
          {
            memcpy(bind_packet, packet, BAYANG_PACKET_SIZE);
            ++bind_count;
          }
          else
          {
            if (0 == memcmp(bind_packet, packet, BAYANG_PACKET_SIZE))
            ++bind_count;
          }
        }
        break;
      }
    }
  }
  
  memcpy(Bayang_rx_tx_addr, &  packet[1], 5);
  memcpy(Bayang_rf_channels, & packet[6], 4);
  transmitterID[0] = packet[10];
  transmitterID[1] = packet[11];

  XN297_SetTXAddr(Bayang_rx_tx_addr, BAYANG_ADDRESS_LENGTH);
  XN297_SetRXAddr(Bayang_rx_tx_addr, BAYANG_ADDRESS_LENGTH);

  NRF24L01_WriteReg(NRF24L01_05_RF_CH, Bayang_rf_channels[Bayang_rf_chan++]);
  NRF24L01_WriteReg(NRF24L01_07_STATUS, 0x70);
  NRF24L01_FlushRx();
  
  digitalWrite(pin_LED, HIGH);
}

//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
//************************************************************************************************************************************************************************
void Bayang_receive_packet()
{
  static bool is_bound = false;
  static uint16_t failsafe_counter = 0;

  // data received from tx
  if(NRF24L01_ReadReg(NRF24L01_07_STATUS) & _BV(NRF24L01_07_RX_DR))
  {
    is_bound = true;
    failsafe_counter = 0;
    
    int sum = 0;
    
    uint16_t aileron, elevator, rudder, throttle, flip, rth, headless, invert;
    
    XN297_ReadPayload(packet, BAYANG_PACKET_SIZE);
    
    if (packet[0] == 0xA4) //0xA4, 0xA3 telemetry
    {
      Serial.println("bind packet");
    }
    else if (packet[0] == 0xA5) //0xA5
    {
      // data packet
      for (int i = 0; i < 14; i++)
      {
        sum += packet[i];
      }
      
      if ((sum & 0xFF) == packet[14])
      {
        // checksum OK
        aileron  = (packet[4]  & 3) * 256 + packet[5];
        elevator = (packet[6]  & 3) * 256 + packet[7];
        rudder   = (packet[10] & 3) * 256 + packet[11];
        throttle = (packet[8]  & 3) * 256 + packet[9];
        flip     = (packet[2]  & 0x08);
        rth      = (packet[2]  & 0x01);
        headless = (packet[2]  & 0x02);
        invert   = (packet[3]  & 0x80);
        
//        Serial.println(aileron,  DEC);
//        Serial.println(elevator, DEC);
//        Serial.println(rudder,   DEC);
//        Serial.println(throttle, DEC);
//        Serial.println(flip,     DEC);
//        Serial.println(rth,      DEC);
//        Serial.println(headless, DEC);
//        Serial.println(invert,   DEC);

        int value_servo1 = 0, value_servo2 = 0, value_servo3 = 0, value_servo4 = 0;
        
        value_servo1 = map(aileron,  0, 1023, 1000, 2000);
        value_servo2 = map(elevator, 0, 1023, 1000, 2000);
        value_servo3 = map(rudder,   0, 1023, 1000, 2000);
        value_servo4 = map(throttle, 0, 1023, 1000, 2000);
        
        servo1.writeMicroseconds(value_servo1);
        servo2.writeMicroseconds(value_servo2);
        servo3.writeMicroseconds(value_servo3);
        servo4.writeMicroseconds(value_servo4);
        
        digitalWrite(pin_output_flip,     flip);
        digitalWrite(pin_output_rth,      rth);
        digitalWrite(pin_output_headless, headless);
        digitalWrite(pin_output_invert,   invert);
      }
      else
      {
        Serial.println("checksum FAIL");
      }
      NRF24L01_WriteReg(NRF24L01_05_RF_CH, Bayang_rf_channels[Bayang_rf_chan++]);
      Bayang_rf_chan %= sizeof(Bayang_rf_channels);
      NRF24L01_WriteReg(NRF24L01_07_STATUS, 0x70);
      NRF24L01_FlushRx();
    }
    else
    {
      Serial.print("Unrecognized packet: ");
      Serial.println(packet[0], HEX);
    }
  }
  else if (is_bound)
  {
    ++failsafe_counter;
    
    if (failsafe_counter > 1000)
    {
      is_bound = false;
      failsafe_counter = 0;
      
      // neutral servo position
      servo1.write(90);
      servo2.write(90);
      servo3.write(90);
      servo4.write(90);
      
      // enable rebinding:
      extern bool reset;
      reset = true;
      Serial.println("reset");
    }
  }
}
 
