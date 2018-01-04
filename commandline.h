//Name this tab: CommandLine.h

#include <string.h>
#include <stdlib.h>

#include "debug.h"

//this following macro is good for debugging, e.g.  print2("myVar= ", myVar);
#define print2(x,y) (Serial.print(x), Serial.println(y))
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))    //This is used to decide the length of an array

#define CR '\r'
#define LF '\n'
#define BS '\b'
#define NULLCHAR '\0'
#define SPACE ' '

#define COMMAND_BUFFER_LENGTH        25                        //length of serial buffer for incoming commands
char   CommandLine[COMMAND_BUFFER_LENGTH + 1];                 //Read commands into this buffer from Serial.  +1 in length for a termination char
bool nullToken=true;                                           //Mark end of the commandline when calling readnum, readhex,readword
const char *delimiters            = ", \n";                    //commands can be separated by return, space or comma

/****************************************************
   Add your commands prototype here
***************************************************/
void helpCommand(void);
void ledonCommand(void);
void ledoffCommand(void);
void ledCommand(void);
void setaddrCommand(void);
void setbinCommand(void);
void setvalCommand(void);
void nullCommand(void);
void listCommand(void);
void getvalCommand(void);
/*************************************************************************************************************
     your Command Names Here
*/
void (*g_pFunctionCmd[])(void)  =  {
  helpCommand,
  ledonCommand,
  ledoffCommand,
  ledCommand,
  setbinCommand,
  setvalCommand,
  setaddrCommand,
  listCommand,
  getvalCommand,

  nullCommand                     //Keep this extra nullCommand always the last, to process unknown command
                                  //Do not put this command in the Commandlist, it will only be used when unknown command is received
};

const char *Commandlist[] = {
  "help",
  "ledon",                        //these led commands for debug 
  "ledoff",
  "led",

  "setbin",                       //set register value in binary, can access full register map
  "setval",                       //set register value in decimal (program converts it to the format register needed)
                                  //can only access temperature registers 2,3,4,5
  "setaddr",                      //set i2c slave address, is it ok to re-begin with a new address?
  "list",
  "getval"
};

const char *CommandHelp[] = {
  "help [command] - to see the help about using command \n command list:",
//  "command list:",
  "ledon - to turn on LED",
  "ledoff - to turn off LED",
  "led <0/1> - to turn off/on LED",
  "setbin <regAddr> byte0 byte1 ... - fill the registers with data byte0, byte1 ... starting from regAddr, data in HEX format",
  "setval <decVal> [regAddr] - set the temperature registers (2,3,4,5) with the decimal value",
  "setaddr <addr> - set the slave device's address with addr (in HEX format)",
  "list - list the registers content in HEX format",
  "getval [regAddr] - read the temperature registers (2,3,4,5) in decimal format",
  "unknown command - use help without a parameter to see the valid command list"
};

const int CommandLength = ARRAY_LENGTH(Commandlist);
/*************************************************************************************************************
    getCommandLineFromSerialPort()
      Return the string of the next command. Commands are delimited by return"
      Handle BackSpace character
      Make all chars lowercase
*************************************************************************************************************/

bool
getCommandLineFromSerialPort(char * commandLine)
{
  static uint8_t charsRead = 0;                      //note: COMAND_BUFFER_LENGTH must be less than 255 chars long
  //read asynchronously until full command input
  while (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case CR:      //likely have full command in buffer now, commands are terminated by CR and/or LS
      case LF:
        commandLine[charsRead] = NULLCHAR;       //null terminate our command char array
        if (charsRead > 0)  {
          charsRead = 0;                           //charsRead is static, so have to reset
          Serial.println(commandLine);
          return true;
        }
        break;
      case BS:                                    // handle backspace in input: put a space in last char
        if (charsRead > 0) {                        //and adjust commandLine and charsRead
          commandLine[--charsRead] = NULLCHAR;
          Serial << byte(BS) << byte(SPACE) << byte(BS);  //no idea how this works, found it on the Internet
        }
        break;
      default:
        // c = tolower(c);
        if (charsRead < COMMAND_BUFFER_LENGTH) {
          commandLine[charsRead++] = c;
        }
        commandLine[charsRead] = NULLCHAR;     //just in case
        break;
    }
  }
  return false;
}


/* ****************************
   readNumber: return a 16bit (for Arduino Uno) signed integer from the command line
   readWord: get a text word from the command line

*/
int
readNumber () {
  char * numTextPtr = strtok(NULL, delimiters);         //K&R string.h  pg. 250
  if(numTextPtr==NULL) nullToken=true;
  else nullToken=false;
  return atoi(numTextPtr);                              //K&R string.h  pg. 251
}
int
readHex () {
  char * numTextPtr = strtok(NULL, delimiters);         //K&R string.h  pg. 250

  if(numTextPtr==NULL) nullToken=true;
  else nullToken=false;
  
  return (int)strtol(numTextPtr,NULL,16);                              //K&R string.h  pg. 251
}

char * readWord() {
  char * numTextPtr = strtok(NULL, delimiters);               //K&R string.h  pg. 250
  if(numTextPtr==NULL) nullToken=true;
  else nullToken=false;
  
  return numTextPtr;
}

void
nullCommand(void) {
  Serial.println("Command not found... use help to see the command list");      //
}

//This is used to print hex data with leading 0 if it is only 1 digit
void serialPrintHex(byte b){
  if(b<16)
    Serial.print("0");
  Serial.print(b,HEX);
}


/****************************************************
   DoMyCommand
*/
bool
DoMyCommand(char * commandLine) {
  //  print2("\nCommand: ", commandLine);
  int result,cmdNo;

  char * ptrToCommandName = strtok(commandLine, delimiters);
  //  print2("commandName= ", ptrToCommandName);

  for(cmdNo=0;cmdNo<CommandLength;cmdNo++){
    if (strcmp(ptrToCommandName, Commandlist[cmdNo]) == 0)                    //find out the command number
      break;
  } 

  (*g_pFunctionCmd[cmdNo])();                                                 //call the command function
}

/****************************************************
   Add your commands functions here
***************************************************/
void helpCommand(){
  int i=0;
  char *pcCmd=readWord();                                         //get the parameter after help command
  
  if(!nullToken)
    for(i=0;i<CommandLength;i++)
      if (strcmp(pcCmd, Commandlist[i]) == 0)                    //find out the command number corresponding the parameter,
        break;                                                   //if not found, use nullCommand number, if no parameter, use 0 (for help command)

  Serial.println(CommandHelp[i]);                                 //print help text 
  if(i==0)
    for(int i=0;i<CommandLength;i++)
      Serial.println(Commandlist[i]);                             //if not specified which command to help,list all commands
}

void ledonCommand() {
  LED_ON();
}
void ledoffCommand() {
  LED_OFF();
}
void ledCommand() {
  int firstOperand = readNumber();
  if (firstOperand == 0)
      LED_OFF();
  else LED_ON();
}

void setaddrCommand(){
  byte addr;
  addr = readHex();
//  ina219.setAddress((uint8_t)addr);
  Serial.print("Set to new address: 0x");
  Serial.println(addr,HEX);
  Wire.begin(addr);                               // To change the slave address, re-run begin with new address
  
}

void setbinCommand(){
  byte regAddr = readHex()*2;
  byte binData = readHex();

  while((!nullToken)&&(regAddr<REG_MAP_SIZE)){
    registerMap[regAddr++]=binData;
    binData = readHex();
  }
}

/****************************************************
 * set temperature value of a temperature register (2,3,4,5)
 * Usage: setval value register  
 * The value should be integar, and between -256 to 255
 * if register not exists, by default use 5 (Ta)
 ******************************************************/
void setvalCommand(){
  byte UpperByte,LowerByte;
  byte regAddr;
  int Temperature;

  Temperature=readNumber();
  if((nullToken)||(Temperature>255)||(Temperature<-100)){
    Serial.println("Temperature out of range,valid range is -100 to 255");
    return;
  }
  regAddr = readHex();
  if(nullToken) regAddr = TaRegister;            //if ommitted,read Ta register 
  if((regAddr<2)||(regAddr>5)) {
    Serial.println("out of range,valid range is 2 to 5");
  }
  else {
    UpperByte=0;
    if(Temperature<0) {
      UpperByte = 0x10;
      Temperature=Temperature&0xFF;
    }
    UpperByte|=(Temperature/16)&0x0F;
    LowerByte=(Temperature*16)&0xF0;

    registerMap[regAddr*2]=UpperByte;
    registerMap[regAddr*2+1]=LowerByte;
  }

}

// list command to print out full register map in HEX
void listCommand(void){
  for(int i=0;i<REG_MAP_SIZE;i++){
    serialPrintHex(registerMap[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

void getvalCommand(void){
  byte UpperByte,LowerByte;
  byte regAddr = readHex();
  int Temperature;

  if(nullToken) regAddr = TaRegister;            //if ommitted,read Ta register 
  if((regAddr<2)||(regAddr>5)) {
    Serial.println("out of range,valid range is 2 to 5");
  }
  else {
    
    UpperByte=registerMap[regAddr*2];
    LowerByte=registerMap[regAddr*2+1];

    UpperByte = UpperByte & 0x1F; //Clear flag bits
    if ((UpperByte & 0x10) == 0x10){ //T A < 0°C
      UpperByte = UpperByte & 0x0F;                   //Clear SIGN

      Temperature = (UpperByte * 16 + LowerByte / 16);
      Temperature -= 256;
    }else
      Temperature = (UpperByte * 16 + LowerByte / 16);

    Serial.print("The value is: ");
    Serial.println(Temperature);
  }
}
/* This is the sample code from data sheet of MCP9808
i2c_start(); // send START command
i2c_write (AddressByte & 0xFE); //WRITE Command (see Section 4.1.4 “Address Byte”)
i2c_write(0x05); // Write T A Register Address
//also, make sure bit 0 is cleared ‘0’
i2c_start(); //Repeat START
i2c_write(AddressByte | 0x01); // READ Command (see Section 4.1.4 “Address Byte”)
UpperByte = i2c_read(ACK); // READ 8 bits
LowerByte = i2c_read(NAK); // READ 8 bits
i2c_stop(); // send STOP command
//also, make sure bit 0 is Set ‘1’
//and Send ACK bit
//and Send NAK bit
//Convert the temperature data
//First Check flag bits
if ((UpperByte & 0x80) == 0x80){
//T A 3 T CRIT
}
if ((UpperByte & 0x40) == 0x40){
//T A > T UPPER
}
if ((UpperByte & 0x20) == 0x20){
//T A < T LOWER
}
UpperByte = UpperByte & 0x1F; //Clear flag bits
if ((UpperByte & 0x10) == 0x10){ //T A < 0°C
UpperByte = UpperByte & 0x0F;
//Clear SIGN
Temperature = 256 - (UpperByte x 16 + LowerByte / 16);
}else
//T A
3 0°C
Temperature = (UpperByte x 16 + LowerByte / 16);
//Temperature = Ambient Temperature (°C)
*/
