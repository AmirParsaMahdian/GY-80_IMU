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


const unsigned char OSS = 0;  // Oversampling Setting
// Calibration values
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;
long b5;


// Create I2C bus
int i2cCreate();

// Change data to rigister you want
void registerWrite(int file, char Rigister_to_set, char Data_for_set_to_rigister);

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int registerRead(int file, unsigned char address);

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
short bmp085GetTemperature(unsigned int ut);

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up);

// Read the uncompensated temperature value
unsigned int bmp085ReadUT(int file);

// Read the uncompensated pressure value
unsigned long bmp085ReadUP(int file);

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration(int file);


int main()
{
	int file;
	file = i2cCreate();
	bmp085Calibration(file);
	
	while(1)
	{	
		double temperature, pressure, altitude;
		
		temperature = bmp085GetTemperature(bmp085ReadUT(file));
		cout<<"temp: "<<temperature<<endl;
		
  		pressure = bmp085GetPressure(bmp085ReadUP(file));
  		cout<<"pre: "<<pressure<<endl;
  		
		altitude = 44330 * ( 1 - pow((pressure / 101325) , ( 1 / 5.255 )));
		cout<<"alt: "<<altitude<<endl;
		
		usleep(1000000);
	}
}



int i2cCreate()
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


void registerWrite(int file, char Rigister_to_set, char Data_for_set_to_rigister)
{
	char config[2] = {0};
	config[0] = Rigister_to_set;
	config[1] = Data_for_set_to_rigister;
	write(file, config, 2);
}


int registerRead(int file, unsigned char address)
{
	char msb, lsb;
  
    ioctl(file,I2C_SLAVE,0x77);
    
	char config[1]={address};
	write(file,config,1);
	
	char data[2];
	read(file,data,2);
	msb = data[0];
	lsb = data[1];
  
	return ((int) msb<<8 | lsb);
}


short bmp085GetTemperature(unsigned int ut)
{
	long x1, x2;
	x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
	x2 = ((long)mc << 11)/(x1 + md);
	b5 = x1 + x2;
	
	return ((b5 + 8)>>4);
}


long bmp085GetPressure(unsigned long up)
{
	long x1, x2, x3, b3, b6, p;
	unsigned long b4, b7;
	
	b6 = b5 - 4000;
	
	// Calculate B3
	x1 = (b2 * (b6 * b6)>>12)>>11;
	x2 = (ac2 * b6)>>11;
	x3 = x1 + x2;
	b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
  
	// Calculate B4
	x1 = (ac3 * b6)>>13;
	x2 = (b1 * ((b6 * b6)>>12))>>16;
	x3 = ((x1 + x2) + 2)>>2;
	b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
  
	b7 = ((unsigned long)(up - b3) * (50000>>OSS));
	if (b7 < 0x80000000)
		p = (b7<<1)/b4;
	else
		p = (b7/b4)<<1;
    
	x1 = (p>>8) * (p>>8);
	x1 = (x1 * 3038)>>16;
	x2 = (-7357 * p)>>16;
	p += (x1 + x2 + 3791)>>4;
  
	return p;
}


unsigned int bmp085ReadUT(int file)
{
	unsigned int ut;
	unsigned char msb, lsb;
  
    ioctl(file,I2C_SLAVE,0x77);
  
	registerWrite(file, 0xF4, 0x2E);
  
	// Wait at least 4.5ms
	usleep(5000);
  
	// Read two bytes from registers 0xF6 and 0xF7
	ut = registerRead(file, 0xF6);
	return ut;
}


unsigned long bmp085ReadUP(int file)
{
	unsigned char msb, lsb, xlsb;
	unsigned long up = 0;

    ioctl(file,I2C_SLAVE,0x77);
    
	// Write 0x34+(OSS<<6) into register 0xF4
	// Request a pressure reading w/ oversampling setting
	registerWrite(file, 0xF4, 0x34+(OSS<<6));
  
	// Wait for conversion, delay time dependent on OSS
	usleep((2 + (3<<OSS))*1000);
  
	// Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
	char config[1]={0xF6};
	write(file,config,1);
	
	char data[3];
	read(file,data,3);
	msb = data[0];
	lsb = data[1];
	xlsb = data[2];
	
	up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
  	//test
	  cout << " msb = " << (unsigned long)msb << " lsb = "<< (unsigned long)lsb << " xlsb = " <<(unsigned long)xlsb <<  endl;
 	return up;
}


void bmp085Calibration(int file)
{
	ac1 = registerRead(file, 0xAA) ;
	ac2 = registerRead(file, 0xAC) - 65536;
	ac3 = registerRead(file, 0xAE) - 65536;
	ac4 = registerRead(file, 0xB0) ;
	ac5 = registerRead(file, 0xB2) ;
	ac6 = registerRead(file, 0xB4) ;
	b1 = registerRead(file, 0xB6) ;
	b2 = registerRead(file, 0xB8) ;
	mb = registerRead(file, 0xBA) - 65536;
	mc = registerRead(file, 0xBC) - 65536;
	md = registerRead(file, 0xBE) ;
}

//#######  #######
