#include "Turret.cpp"
#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char** argv){
	Turret turret;
	double panAngle, tiltAngle;
	turret.turnOnLaser();
	int waitTime = 40000;
	while(1){
		for(int i=-60; i < 60; i++){
			cout << i << endl;
			turret.sendAngles(i,i);
			//turret.readAngles(panAngle, tiltAngle);
			//cout << panAngle << "," << tiltAngle << endl;
			//cin.ignore();
			usleep(waitTime);
		}
		for(int i=60; i > -60; i--){
			cout << i << endl;
			turret.sendAngles(i,i);
			//cin.ignore();
			usleep(waitTime);
		}
		//turret.sendPosition(0,0);
		//usleep(waitTime);
		//turret.turnOnLaser();
		//usleep(waitTime);
		//cin.ignore();
		//turret.turnOffLaser();
		//cin.ignore();
		//usleep(waitTime);*/
	}
	return 0;
}
