#include "Turret.cpp"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv){
	Turret turret;
	double panAngle, tiltAngle;
	//turret.turnOnLaser();
	int waitTime = 500000;
	while(1){
		turret.readAngles(panAngle, tiltAngle);
		//cout << "Pan Angle: " << panAngle << endl;
		//cout << "Tilt Angle: " << tiltAngle << endl;
		for(int i=-60; i < 60; i++){
			cout << i << endl;
			turret.sendPosition(i,0);
			usleep(10000);
		}
		for(int i=60; i > -60; i--){
			cout << i << endl;
			turret.sendPosition(i,0);
			usleep(10000);
		}
		usleep(waitTime);
		turret.turnOnLaser();
		usleep(waitTime);
		turret.turnOffLaser();
		usleep(waitTime);
	}
	return 0;
}
