// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// HMC5883
// This code is designed to work with the HMC5883_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Compass?sku=HMC5883_I2CS#tabs-0-product_tabset-2#tabs-0-product_tabset-2

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
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


// Create I2C bus
int i2cCreate ();


//Change data to rigister you want
void i2cSetRigister(int file, char Rigister_to_set, char Data_for_set_to_rigister);


//setup sensor and set rigisters
void setup_magnet(int file);

//read data from sensor and return what we want
// if select_output == x return raw x 
// if select_output == y return raw y 
// if select_output == z return raw z 
float magnet_data(int file, char select_output);




int main() 
{
	int file;
		
	// Create I2C bus
	file = i2cCreate();
	
	//Setup magnitometer
	setup_magnet(file);	
	
	//this while show data for ever
	while(1)
	{
      	//print angel of head of device to magnetic north
		printf("head = %f \n",magnet_data(file, 'h'));

		//delay for show data
		//sleep 10 milisecond
		usleep(100000);
	}
	return 0;
} 

//#######################################################################


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



//Change data to rigister you want
void i2cSetRigister(int file, char Rigister_to_set, char Data_for_set_to_rigister)
{
	char config[2] = {0};
	config[0] = Rigister_to_set;
	config[1] = Data_for_set_to_rigister;
	write(file, config, 2);
}


//setup sensor and set rigisters
void setup_magnet(int file)
{
	// Get I2C device, HMC5883 I2C address is 0x1E(30)
	ioctl(file, I2C_SLAVE, 0x1E);

	//Rigister 0x00
	/*
	CRA7  CRA6    CRA5    CRA4     CRA3     CRA2     CRA1     CRA0 
	(0)   MA1(0)  MA0(0)  DO2_(1)  DO1_(0)  DO0_(0)  MS1_(0)  MS0_(0)
	_____________________________________________
	|DO2 |DO1 |DO0 |Typical Data Output Rate (Hz)|
	|____|____|____|_____________________________|
	|0   |0   |0   |0.75                         |
	|0   |0   |1   |1.5                          |
	|0   |1   |0   |3                            |
	|0   |1   |1   |7.5                          |
	|1   |0   |0   |15 (Default)                 |
	|1   |0   |1   |30                           |
	|1   |1   |0   |75                           |
	|1   |1   |1   |Reserved                     |
	|____|____|____|_____________________________|



   |MS1|MS0|Measurement Mode 
   |___|___|____________________________
   |0  |0 
   |   |   |Normal measurement configuration (Default). In normal measurement 
   |   |   |configuration the device follows normal measurement flow. The positive and 
   |   |   |negative pins of the resistive load are left floating and high impedance. 
   |___|___|__________________________________________________
   |0   1 
   |		Positive bias configuration for X, Y, and Z axes. In this configuration, a positive 
   |		current is forced across the resistive load for all three axes. 
   |______________________________________________________________________
   |1   0 
   |		Negative bias configuration for X, Y and Z axes. In this configuration, a negative 
   |		current is forced across the resistive load for all three axes.. 
   |___________________________________________________________
   |1   1 
   |		This configuration is reserved. 
   |____________________________________________________________
	*/
	
	// Select Configuration register A(0x00)
	// Normal measurement configuration, data rate o/p = 0.75 Hz(0x60)
	i2cSetRigister(file, 0x00 , 0x34);
	
	//Rigister 0x02
	/*
	
	MR7   MR6  MR5  MR4  MR3  MR2  MR1     MR0 
    HS(0) (0)  (0)  (0)  (0)  (0)  MD1_(0) MD0_(1) 

    HS ->> 1 => Set this pin to enable High Speed I2C, 3400kHz. 


    MD1 MD0 Operating Mode 
    
	0   0 
        Continuous-Measurement Mode. In continuous-measurement mode, 
        the device continuously performs measurements and places the 
        result in the data register. RDY goes high when new data is placed 
        in all three registers. After a power-on or a write to the mode or 
        configuration register, the first measurement set is available from all 
        three data output registers after a period of 2/fDO and subsequent 
        measurements are available at a frequency of fDO, where fDO is the 
        frequency of data output. 
    
	0   1 
        Single-Measurement Mode (Default). When single-measurement 
        mode is selected, device performs a single measurement, sets RDY 
        high and returned to idle mode. Mode register returns to idle mode 
        bit values. The measurement remains in the data output register and 
    	RDY remains high until the data output register is read or another 
        measurement is performed. 
    
    1   0 
	    Idle Mode. Device is placed in idle mode. 
	    
    1   1 
	    Idle Mode. Device is placed in idle mode. 


	*/
	// Select Mode register(0x02)
	// Continuous measurement mode(0x00)
	i2cSetRigister(file, 0x02, 0x00);
		
	//sleep for set rigister and reboot sensor
	//sleep 1 sec
	sleep(1);

}


//read data from sensor and return what we want
// if select_output == x return raw x 
// if select_output == y return raw y 
// if select_output == z return raw z 
float magnet_data(int file, char select_output)
{			
	// Read 6 bytes of data from register(0x03)
	// xMag msb, xMag lsb, zMag msb, zMag lsb, yMag msb, yMag lsb
	char reg[1] = {0x03};
	write(file, reg, 1);
	char data[6] ={0};
		
	int xMag;
	int yMag;
	int zMag;
	float heading=0;
	if(read(file, data, 6) != 6)
	{
		printf("Erorr : Input/output Erorr \n");
	}
	else
	{
		// Convert the data
		xMag = (data[0] * 256 + data[1]);
		if(xMag > 32767)
		{
			xMag -= 65536;
		}
	
		zMag = (data[2] * 256 + data[3]);
		if(zMag > 32767) 
		{
			zMag -= 65536;
		}
	
		yMag = (data[4] * 256 + data[5]);
		if(yMag > 32767) 
		{
			yMag -= 65536;
		}
	
		// Output raw data to screen
		//for debug uncommment it
		//printf("Magnetic field in X-Axis : %d \n", xMag);
		//printf("Magnetic field in Y-Axis : %d \n", yMag);
		//printf("Magnetic field in Z-Axis : %d \n", zMag);

	}
		
	//calculate head of sensor
	heading = 180 * atan2(yMag,xMag)/3.141592;
		
	//set heading of device
	if(heading < 0)
	{
     		heading += 360;
    }
	 	
    //for select bettwin inputs
	switch(select_output) {
		//show x raw data
		case 'x' :
    		//for debug uncommment it
			//printf("Magnetic field in X-Axis : %d \n", xMag);
			return (float) xMag;
    		break; 
      		
		//show y raw data
		case 'y' :
    		//for debug uncommment it
			//printf("Magnetic field in Y-Axis : %d \n", yMag);
			return (float) yMag;
    		break; 
      		
		//show z raw data
		case 'z' :
    		//for debug uncommment it
			//printf("Magnetic field in Z-Axis : %d \n", zMag);
			return (float) zMag;
    		break; 
      	
	  	//show head angel
		case 'h' :
    		//for debug uncommment it
			//printf("head x-y: %f \n", heading);
			return (float)heading;
    		break; 
      		
    	//anything else
    	default :
      		printf("Enter x, y, z or h \n"); 	
		  
	}
	
}
