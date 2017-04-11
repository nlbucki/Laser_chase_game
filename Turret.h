#ifndef TURRET_H
#define TURRET_H

class Turret
{
	private :
		int xHome;
		int yHome;
		int d;
	public :
		Turret();
		void sendAngles(int &tilt_angle, int &pan_angle);
		void readAngles(int &tilt_angle, int &pan_angle);
};

#endif
