#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <stdio.h>

using namespace std;


//############## i2c functions ##############
// Creates the i2c bus
int i2cCreate ();

// Writes data to specified register
void registerWrite(int bus, char address, char Register_to_set, char Data_for_set_to_register);

// Reads Register and returns value in different ways depending on the sensor need
int registerRead(int bus, unsigned char address, unsigned char reg);


//############## L3G4200D ##############
// Setup sensor and set registers
void setup_L3G4200D(int bus);
int l3g4200dRead(int bus, char axis);


int main()
{
	int bus;
	bus = i2cCreate();
	
	setupL3G4200D(bus);
	
	
	while(1)
	{
		// Print gyroscope data from L3G4200D
		cout<< l3g4200dRead(bus, 'x')<< endl;
		cout<< l3g4200dRead(bus, 'y')<< endl;
		cout<< l3g4200dRead(bus, 'z')<< endl;
	
		usleep(100000); //Just here to slow down the serial to make it more readable
	}
}



int i2cCreate ()
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-0";
	if ((file = open(bus, O_RDWR)) < 0) 
	{
		cout<< "Failed to open the bus."<< endl;;
		exit(1);
	}
	return file;
}


void registerWrite(int bus, char address, char Register_to_set, char Data_for_set_to_register)
{
	ioctl(bus, I2C_SLAVE, address);
	char config[2] = {Register_to_set, Data_for_set_to_register};
	write(bus, config, 2);
}


int registerRead(int bus, unsigned char address, unsigned char reg)
{
 	ioctl(bus, I2C_SLAVE, address);
    
	char config[1]={reg};
	write(bus, config, 1);
	
	// L3G4200D Gyroscope
	if(address == 0x69)
	{
		char data[1]={0};
		read(bus, data, 1);
		return (int16_t)data[0];
	}
}


void setup_L3G4200D(int bus)
{
	// Enable x, y, z and turn off power down:
	registerWrite(bus, 0x69, 0x20, 0b00001111);

	// If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
	registerWrite(bus, 0x69, 0x21, 0b00000000);
	
	// Configure CTRL_REG3 to generate data ready interrupt on INT2
	// No interrupts used on INT1, if you'd like to configure INT1
	// or INT2 otherwise, consult the datasheet:
	registerWrite(bus, 0x69, 0x22, 0b00001000);

	// CTRL_REG4 controls the full-scale range, among other things:
	registerWrite(bus, 0x69, 0x23, 0b00110000);

	// CTRL_REG5 controls high-pass filtering of outputs, use it
	// if you'd like:
	registerWrite(bus, 0x69, 0x24, 0b00000000);
}


int l3g4200dRead(int bus, char axis)
{
	int MSB, LSB;
	
	if (axis == 'x')
	{
		MSB = registerRead(bus, 0x69, 0x29);
		LSB = registerRead(bus, 0x69, 0x28);
		return (int16_t)((MSB << 8) | LSB);
	}
	else if (axis == 'y')
	{
		MSB = registerRead(bus, 0x69, 0x2B);
		LSB = registerRead(bus, 0x69, 0x2A);
		return (int16_t)((MSB << 8) | LSB);
	}
	else if (axis == 'z')
	{
		MSB = registerRead(bus, 0x69, 0x2D);
		LSB = registerRead(bus, 0x69, 0x2C);
		return (int16_t)((MSB << 8) | LSB);
	}
}



///////// /////////
