#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <time.h>
//Dabiri
#include <iostream>
#include <stdio.h>
//if you have bug remove this
using namespace std;

//############## i2c functions ##############
// Creates the i2c bus
int i2cCreate ();

// Writes data to specified register
void registerWrite(int bus, char address, char Register_to_set, char Data_for_set_to_register);

// Reads Register and returns value in different ways depending on the sensor need
int registerRead(int bus, unsigned char address, unsigned char reg);


//############## ADXL345 ##############
// Setup sensor and set registers
void setup_ADXL345(int bus);
double adxl345Read(int bus, char axis);

int main()
{
	
	int file;
	
	//open i2c bus
	bus = i2cCreate();
	
	//setup accl	
	setup_ADXL345(bus);
	
	
	while(1)
	{
		double x=adxl345Read('x',bus);
		double y=adxl345Read('y',bus);
		double z=adxl345Read('z',bus);

		cout << "x = " << x << endl;	
		cout << "y = " <<  y << endl;	
		cout << "z = " <<  z << endl;	

		cout << "############################" << endl;
		
		//sleep 100 milisec
		usleep(100000);
	}
	return 0;
}

//#############################################


// Create I2C bus
int i2cCreate ()
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-0";
	if ((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	return file;
}
 
 
// Change data to rigister you want
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
	
	// ADXL345 Accelometer
	if(address == 0x53)
	{
		char data[2]={0};
		read(bus, data, 2);
		return (int16_t)(data[1] << 8 | data[0]);
	}
}

 
void setup_ADXL345(int bus)
{       
	// Enable measuring    
    registerWrite(bus, 0x53, 0x2D, 0x08);
     
    // Select Bandwidth rate register(0x20)
    // normal mode ,Output Data Rate = 100Hz(0x0a)
    registerWrite(bus, 0x53, 0x2C, 0x0A);
	
    // Select Power Control register (0x2d)
    // Auto Sleep disable(0x88)
    registerWrite(bus, 0x53, 0x0D, 0x08);
	 
    // Select Data  format register(0x31)
    // Self test disabled, 4-wire interface,full resolution,range +/- 16g
    registerWrite(bus, 0x53, 0x31, 0x0A);
	 	
}


double adxl345Read(int bus, char axis)
{
    
    ioctl(bus, I2C_SLAVE, 0x53);
    //reads the raw data
    char reg[1] = {0x32};
    write(bus, reg, 1);
    char data[6] = {9};
    if (read(bus,data,6)!=6) cout<< "Problems with Accelerometer Data Readings"<< endl;
	
	if (axis == 'x')			return registerRead(bus, 0x53, 0x32)/256.0;
	else if (axis =='y')		return registerRead(bus, 0x53, 0x34)/256.0;
	else if (axis =='z')		return registerRead(bus, 0x53, 0x36)/256.0;
}
 
