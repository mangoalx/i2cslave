//#include <HardWire.h>

/*
#include <Wire.h>
#include <TinyWireM.h>
#include <USI_TWI_Master.h>

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}*/


/****************************************************************************************
 * MCP9808 simulator with Arduino feather 32u4
 *          by John Xu, Videri
 *          
 * Version 0.1
 *    - Basic version
 *    - I2C slave device 
 *    - Command line allow setting values and changing I2C address
 *    - Support repeat-start reading (write 1 byte register No. then restart to read data), new wire library
 * 
 ***************************************************************************************/

 

#include <Wire.h>
//#include <HardWire.h>
 
/*******************************************************************
 * MCP9808 Register map
 * 
 * addr   len   name
 * 0      2     RFU
 * 1      2     Config
 * 2      2     Tupper
 * 3      2     Tlower
 * 4      2     Tcrit
 * 5      2     Ta
 * 6      2     ManufacturerID
 * 7      2     DeviceID/revision
 * 8      1     Resolution
 */
/********************************************************************/
// If we need to use interrupt then un-comment these
//#define  UseInterrupt
//#define  INTERRUPT_PIN           2      //I don't need this pin 

#define  SLAVE_ADDRESS           0x1C  //slave address,any number from 0x01 to 0x7F
#define  SLAVE_MASK              0x4   //This value is put in TWAMR, the bit will be ignored, so 0x1E will also
                                        //be accepted as slave address (shifted left for 1 so it is 4 instead of 2)
//twi_slaaddr = TWDR >> 1;
#define  REG_MAP_SIZE            18     //9 registers of 2 bytes data, last one Resolution is single byte, check later

#define  MAX_SENT_BYTES          3      //3 bytes can be written in - reg address, 2 bytes of data maximum

#define  MANUFACTUREID           0x0054

#define DEVICEID                0X0400

#define RFU                     0x001F

const byte defaultReg[REG_MAP_SIZE] = {0x0,0x1F,      //RFU
                                       0x0,0x0,       //config
                                       0x0,0x0,       //Tupper
                                       0x0,0x0,      //Tlower
                                       0x0,0x0,      //Tcrit
                                       0x0,0x0,      //Ta
                                       0x0,0x54,      //ManufactureID
                                       0x04,0x0,      //DeviceID
                                       0x03,0x0     //Resolution, 1 byte, fill 0 the last byte
};

const byte TaRegister=5;             //no.5 register is Temperature ambient, will be used as default pointer value                                       
                                       

/********* Global  Variables  ***********/

byte registerMap[REG_MAP_SIZE];

byte registerMapTemp[REG_MAP_SIZE];

byte receivedCommands[MAX_SENT_BYTES];

byte regPointer=TaRegister;           //write only, to specify the register to access,default to Ta register
byte dataReceived=0;                  //how many byte data received 
byte mapPointer=2*regPointer;         //index for registerMap

#include "commandline.h"

void setup()

{

#ifdef UseInterrupt

  {

    pinMode(INTERRUPT_PIN,OUTPUT);

    digitalWrite(INTERRUPT_PIN,HIGH);

  }
#endif
  for(byte i=0;i<REG_MAP_SIZE;i++)         //copy deault data into registers
    registerMap[i]=defaultReg[i];

  Wire.begin(SLAVE_ADDRESS); 
#ifdef SLAVE_MASK
  TWAMR = SLAVE_MASK;
#endif                                      //if SLAVE_MASK is defined, it should be written to TWAMR

  Wire.onRequest(requestEvent);

  Wire.onReceive(receiveEvent);

//  Wire.onRequestData(requestData);          // New from hardwire library, when was read by master, return a byte
  

}

 

void loop()

{
  processDataReceved();               //Check if we received data to be written to registers
  if(getCommandLineFromSerialPort(CommandLine))      //global CommandLine is defined in CommandLine.h
      DoMyCommand(CommandLine);
  delay(1);  

}
//This handler needs to be short,there is just several uS to get ready for sending data. Maybe it holds the scl if not ready
void requestEvent()

{

  Wire.write(registerMap+regPointer*2, 2);   //Send 2 bytes data only

}

 
//This handler for slave was read by master, and return a byte data to be send
//byte requestData(void)
//{
//  return registerMap[mapPointer++];
//}


//This handler needs to short, cause it hold the i2c until quit
void receiveEvent(int bytesReceived)

{
  //LED_ON();
  
  //if only 1 byte is received, it is register address, put it into register pointer
  if(bytesReceived==1) regPointer=Wire.read();

  else{
    for (dataReceived = 0; dataReceived < bytesReceived && dataReceived<MAX_SENT_BYTES; dataReceived++)
  
        receivedCommands[dataReceived] = Wire.read();
  
  // if we receive more data than allowed just throw it away, I just leave them un-read here, should not need to read the rest
  
    if(dataReceived == 1 && (receivedCommands[0] < REG_MAP_SIZE/2)) 
  // if only 1 byte received, this is the reg address, and a read operation is coming
    {
      regPointer = receivedCommands[0];         //if the reg address is valid, then set the pointer
      dataReceived = 0;                         //clear dataReceived, no more process is needed
    }
  }
 
}

void processDataReceved(void)
{
  if(dataReceived != 0){
    int i = receivedCommands[0];
    if (i>0 && i<5 && dataReceived == 3) {                         //Register 1 to 4 
      registerMap[i*2]=receivedCommands[1];
      registerMap[i*2+1]=receivedCommands[2];
    }
    else if (i==8 && dataReceived == 2) {
      registerMap[i*2]=receivedCommands[1];                       //Register 8 resolution, 1 byte
    }
    dataReceived=0;                                               //Clear dataReceived
  }
}

