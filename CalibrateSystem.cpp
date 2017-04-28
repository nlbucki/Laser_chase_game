#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Turret.cpp"
#include "llsq.cpp"
#include <unistd.h>

#define PI 3.14159265

using namespace cv;
using namespace std;

int xHome, yHome;
double d;

double calibAngles [4] = {15, 15, -5, -15}; // {Top, Right, Bottom, Left}
bool dotReadSuccess;

// HSV ranges for red laser (Hue is split between start and end of range for red)
Scalar lowHSVred1 = Scalar(0, 90, 150);
Scalar highHSVred1 = Scalar(8, 255, 255);
Scalar lowHSVred2 = Scalar(172, 90, 150);
Scalar highHSVred2 = Scalar(179, 255, 255);

Mat imgOrig, imgHSV, imgThresh1, imgThresh2, imgThreshRed;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
Moments m;

bool getRedDot(int &xLoc, int &yLoc){
	VideoCapture cam(1); // Get camera
	if(!cam.isOpened()){
		cout << "error: failed to open camera" << endl;
		return false;
	}

	while(waitKey(1) != 10){
		// Read image from camera
		bool imgReadSuccess = cam.read(imgOrig);

		// Convert to HSV image and threshold to find red
		cvtColor(imgOrig, imgHSV, COLOR_BGR2HSV);
		inRange(imgHSV, lowHSVred1, highHSVred1, imgThresh1);
		inRange(imgHSV, lowHSVred2, highHSVred2, imgThresh2);
		imgThreshRed = imgThresh1 + imgThresh2;

		// Perform morphological operations (remove noise, etc.)
		erode(imgThreshRed, imgThreshRed, getStructuringElement(MORPH_ELLIPSE, Size(1, 1)));
        dilate(imgThreshRed, imgThreshRed, getStructuringElement(MORPH_ELLIPSE, Size(10, 10)));

		// Find location of center of red dot
		findContours(imgThreshRed, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
        if(contours.size() > 0){
            m = moments(contours[0], false);
            xLoc = m.m10/m.m00;
            yLoc = m.m01/m.m00;
            circle(imgOrig, Point2f(xLoc, yLoc), 5, Scalar(255, 0, 0), -1, 8, 0);
			stringstream sstm;
			sstm << "(" << xLoc << ", " << yLoc << ")";
			putText(imgOrig, sstm.str(), cvPoint(0,imgOrig.rows), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(255, 255, 255), 1, CV_AA);
        }
		
		// Display images
		imshow("Camera View", imgOrig);
		imshow("Thresholded Image", imgThreshRed);
	}
	return true;
}

int main( int argc, char** argv ){
	system("stty -F /dev/ttyUSB0 cs8 115200 ignbrk -brkint -icrnl -imaxbel -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts");
	cout << "Starting calibration routine..." << endl;
	Turret turret;
	turret.turnOnLaser();
	turret.sendAngles(0, 0);
	
	cout << "Press \"Enter\" to record (x, y) home position" << endl;
	dotReadSuccess = getRedDot(xHome, yHome);
	if(!dotReadSuccess){
		cout << "error: failed to read dot location" << endl;
		return -1;
	}
	cout << "xHome = " << xHome << endl;
	cout << "yHome = " << yHome << endl << endl;

	cout << "Starting distance estimation. Press \"Enter\" to record each point." << endl;

	int xLoc, yLoc;
	double panAng, tiltAng;
	double posData [4]; // In pixels
	double angData [4]; // In radians

	// Record data for top point
	cout << "Top" << endl;
	turret.sendAngles(0, calibAngles[0]); // Move turret to top edge, no pan
	dotReadSuccess = getRedDot(xLoc, yLoc); // Record y location of dot
	cout << "Recorded (" << xLoc << "," << yLoc << ")" << endl;
	if(!dotReadSuccess){
		cout << "error: failed to read dot location" << endl;
		return -1;
	}
	posData[0] = -(yLoc - yHome); // Convert y location to be relative to home position
	angData[0] = calibAngles[0] * PI/180.0; // Convert to radians

	// Record data for right point
	cout << "Right" << endl;
	turret.sendAngles(calibAngles[1], 0);
	dotReadSuccess = getRedDot(xLoc, yLoc);
	cout << "Recorded (" << xLoc << "," << yLoc << ")" << endl;
	if(!dotReadSuccess){
		cout << "error: failed to read dot location" << endl;
		return -1;
	}
	posData[1] = xLoc - xHome;
	angData[1] = calibAngles[1] * PI/180.0;

	// Record data for bottom point
	cout << "Bottom" << endl;
	turret.sendAngles(0, calibAngles[2]); 
	dotReadSuccess = getRedDot(xLoc, yLoc);
	cout << "Recorded (" << xLoc << "," << yLoc << ")" << endl;
	if(!dotReadSuccess){
		cout << "error: failed to read dot location" << endl;
		return -1;
	}
	posData[2] = -(yLoc - yHome);
	angData[2] = calibAngles[2] * PI/180.0;

	// Record data for left point
	cout << "Left" << endl;
	turret.sendAngles(calibAngles[3], 0);
	dotReadSuccess = getRedDot(xLoc, yLoc);
	cout << "Recorded (" << xLoc << "," << yLoc << ")" << endl;
	if(!dotReadSuccess){
		cout << "error: failed to read dot location" << endl;
		return -1;
	}
	posData[3] = xLoc - xHome;
	angData[3] = calibAngles[3] * PI/180.0;

	cout << "Calibration routine complete." << endl << endl;
	turret.turnOffLaser();

	// Perform least squares regression to find d
	double b;
	for (int i = 0; i < 4; i++){
		angData[i] = tan(angData[i]);
	}
	llsq(4, angData, posData, d, b);
	cout << "d = " << d << endl;
	
	// Store values in calibration file
	ofstream calFile;
	calFile.open("calibrationFile.txt");
	calFile << xHome << endl;
	calFile << yHome << endl;
	calFile << d << endl;
	calFile.close();	
	return 0;
}
