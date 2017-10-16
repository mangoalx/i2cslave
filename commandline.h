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

  nullCommand                     //Keep this extra nullCommand always the last, to process unknown command
};

const char *Commandlist[] = {
  "help",
  "ledon",                        //these led commands for debug 
  "ledoff",
  "led",

  "setbin",                       //set register value in binary, can access full register map
  "setval",                       //set register value in decimal (program converts it to the format register needed)
                                  //can only access temperature registers 2,3,4,5
  "setaddr"                       //set i2c slave address, is it ok to re-begin with a new address?
};

const char *CommandHelp[] = {
  "help command - to see the help about using command \n command list: \n",
  "ledon - to turn on LED",
  "ledoff - to turn off LED",
  "led <0/1> - to turn off/on LED",
  "setbin regAddr byte0 byte1 ... - fill the registers with data byte0, byte1 ... starting from regAddr, data in HEX format",
  "setval regAddr decVal - set the temperature registers (2,3,4,5) with the decimal value",
  "setaddr addr - set the slave device's address with addr (in HEX format)"
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
  return atoi(numTextPtr);                              //K&R string.h  pg. 251
}
int
readHex () {
  char * numTextPtr = strtok(NULL, delimiters);         //K&R string.h  pg. 250
  return (int)strtol(numTextPtr,NULL,16);                              //K&R string.h  pg. 251
}

char * readWord() {
  char * word = strtok(NULL, delimiters);               //K&R string.h  pg. 250
  return word;
}

void
nullCommand(void) {
  Serial.println("Command not found... ");      //
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
  Serial.println(CommandHelp[0]);
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
  
}

void setbinCommand(){
  
}

void setvalCommand(){
  
}

