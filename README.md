This is project to simulate a mcp9808 sensor using an arduino feather 32u4 board, so we can easily get whatever temperature data we wang, -50, or 200, for test purpose.

Basically, it is an I2C slave device, its I2C address can be set either by pulling up/down the address pins (only read at initialization. feather board pin 6 for A0, 9 for A1, 10 for A2, base address 0x18. Ex. A0 is pulled high, the address will be 0x18 + 1 = 0x19) or sending setaddr command via serial port.

A simple command line interface is created for user to control the device via serial communication. The source code is in commandline.h. Use help to see a list of available commands.

The registers such as upper limit temperature, critical temperture etc. can be set via I2C like a real mcp9808. Anyway, I did not test these thoroughly, the flag bits are not set either (flags for over upper limit, lower than lower limit, over critical limit, etc.).

The temperature reading can be set via adjusting a potentiometer connected to analog input pin A0 of feather board. The 2 ends of the potentiometer are connected to Vref and Gnd respectively. Every 0.5s the analog input is read (0 - 1023) then mapped to range (-50, 200) and used as the temperature reading. You can use command setval the manually set the reading, however, do not forget to disable the automatic updating.




