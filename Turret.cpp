#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <unistd.h>
#include <deque>
#include <iostream>
#include <cstdlib>
using namespace std;

class Turret
{
    private :
        int xHome;
        int yHome;
        int d;
		const static double PI = 3.14159265;
		FILE* arduinoRead;
		FILE* arduinoWrite;
		double map(double val, double in_min, double in_max, double out_min, double out_max);
    public :
        Turret();
        void sendAngles(double panAngle, double tiltAngle);
		void sendPosition(int xLoc, int yLoc);
        void readAngles(double &panAngle, double &tiltAngle);
		void readPosition(int &xLoc, int &yLoc);
        void turnOnLaser();
        void turnOffLaser();
};

Turret::Turret(){
	// Read in calibration data
	ifstream calFile("calibrationFile.txt");
	calFile >> xHome >> yHome >> d;
	//cout << "xHome: " << xHome << endl;
	//cout << "yHome: " << yHome << endl;
	//cout << "d: " << d << endl;
	calFile.close();

	// Initialize serial port read/write
	arduinoRead = fopen("/dev/ttyUSB0", "r");
	arduinoWrite = fopen("/dev/ttyUSB0", "w");
	if(arduinoRead == NULL || arduinoWrite == NULL){
		cout << "error: failed to open serial port to arduino" << endl;
	}
	usleep(3000000);// Wait in case Arduino resets when serial port opens	
}

void Turret::sendAngles(double panAngle, double tiltAngle){
	// Angles should be given in degrees 

	// Map desired angle to motor command
	int panPWM = map(panAngle, -60, 60, 0, 255);
	int tiltPWM = map(tiltAngle, -60, 60, 0, 255);
	fprintf(arduinoWrite, "ANGLE\n%d\n%d\n", panPWM, tiltPWM);
}

void Turret::sendPosition(int xLoc, int yLoc){
	// Calculates required angles to achieve desired position and calls sendAngles(2)
	// Positions should be given in pixels (written in the camera frame)

	// Convert to turret frame (i.e. (xLoc,yLoc) is relative to home position)
	double x = xLoc-xHome;
	double y = -(yLoc-yHome); // Invert y-coordinate b/c (0,0) is the top left corner in camera frame instead of the bottom left corner (which is the normal convention for 2D coordiantes)
	
	// Convert to turret angles in radians (inverse kinematics)
	double panAngle = atan2(x,d);
	double tiltAngle = atan2(y*cos(panAngle), d);

	// Convert from radians to degrees
	panAngle = panAngle*180.0/PI;
	tiltAngle = tiltAngle*180.0/PI;
	
	// Command turret angles
	this->sendAngles(panAngle, tiltAngle);
}

void Turret::readAngles(double &panAngle, double &tiltAngle){
	// Reads angle values from arduino and outputs pan and tilt angles in degrees

	char str [50];
	// Read until the end of the file is reached so that the most recent angles are read (instead of reading old data)
	while(fgets(str, 100, arduinoRead) != NULL){}
	// Extract joint angles
	string s = str;
	string panAngStr = s.substr(0, s.find(","));
	string tiltAngStr = s.substr(s.find(",") + 1, string::npos);

	// Convert from string to float/double
	panAngle = atof(panAngStr.c_str());
	tiltAngle = atof(tiltAngStr.c_str());
	
	// Convert angles to degrees
	panAngle = map(panAngle, 0, 1023, -60, 60);
	tiltAngle = map(tiltAngle, 0, 1023, -60, 60);
}

void Turret::readPosition(int &xLoc, int &yLoc){
	// Reads angle values using readAngles(2) and outputs x,y location in camera frame

	// Read angles in degrees
	double panAngle, tiltAngle;
	readAngles(panAngle, tiltAngle);
	
	// Convert angles to radians
	panAngle = panAngle*PI/180.0;
	tiltAngle = tiltAngle*PI/180.0;

	// Convert angles to position in turret frame (forward kinematics)
	double x = d*tan(panAngle);
	double y = d*tan(tiltAngle)/cos(panAngle);

	// Convert to position in camera frame
	xLoc = x + xHome;
	yLoc = yHome - y; 
}

void Turret::turnOnLaser(){
	// Sends command to turn on laser to arduino
	fprintf(arduinoWrite, "%s", "ON\n");
}

void Turret::turnOffLaser(){
	// Sends command to turn off laser to arduino
	fprintf(arduinoWrite, "%s", "OFF\n");
}

double Turret::map(double val, double in_min, double in_max, double out_min, double out_max){
	// Linear map from one range of values to another
	return (val - in_min)*(out_max - out_min)/(in_max - in_min) + out_min;
}
