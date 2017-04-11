#include <iostream>
#include <fstream>
#include <math.h>
#include <unistd.h>
using namespace std;

class Turret
{
    private :
        int xHome;
        int yHome;
        int d;
		FILE *arduino;
    public :
        Turret();
        void sendAngles(double panAngle, double tiltAngle);
		void sendPosition(int xLoc, int yLoc);
        void readAngles(double &panAngle, double &tiltAngle);
        void turnOnLaser();
        void turnOffLaser();
};

Turret::Turret(){
	ifstream calFile("calibrationFile.txt");
	calFile >> xHome >> yHome >> d;
	//cout << "xHome: " << xHome << endl;
	//cout << "yHome: " << yHome << endl;
	//cout << "d: " << d << endl;
	calFile.close();
	arduino = fopen("/dev/ttyUSB0", "w");
	usleep(2000000);	
}

void Turret::sendAngles(double panAngle, double tiltAngle){
	// Angles should be given in degrees 
	int pan = (int)panAngle;
	int tilt = (int)tiltAngle;
	fprintf(arduino, "ANGLE;%d;%d;\n", pan, tilt);
}

void Turret::sendPosition(int xLoc, int yLoc){
	// Positions should be given in pixels (written in the camera frame)

	// Convert to turret frame
	xLoc -= xHome;
	yLoc -= yHome;

	// Convert to turret angles (inverse kinematics)
	double panAngle = 0;
	double tiltAngle = 0;
	
	// Command turrt angles
	this->sendAngles(panAngle, tiltAngle);
}

void Turret::readAngles(double &panAngle, double &tiltAngle){
	// Angles should be returned in degrees 
}

void Turret::turnOnLaser(){
	fprintf(arduino, "ON;\n");
}

void Turret::turnOffLaser(){
	fprintf(arduino, "OFF;\n");
}
