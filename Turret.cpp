#include <iostream>
#include <fstream>
using namespace std;

class Turret
{
    private :
        int xHome;
        int yHome;
        int d;
    public :
        Turret();
        void sendAngles(double panAngle, double tiltAngle);
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
}

void Turret::sendAngles(double panAngle, double tiltAngle){
	// Angles should be given in degrees 
}

void Turret::readAngles(double &panAngle, double &tiltAngle){
	// Angles should be returned in degrees 
}

void Turret::turnOnLaser(){

}

void Turret::turnOffLaser(){

}
