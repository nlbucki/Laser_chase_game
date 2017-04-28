#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Turret.cpp"
#include <unistd.h>
#include <ctime>

using namespace cv;
using namespace std;

int game_mode;
int game_difficulty;
int collisionDist = 20;
double k_effort = 5000; // Proportional gain to push red dot away from green dot
double k_circle = 0.3; // Proportional gain to push red dot back into boundary
int r = 100; // Radius of boundary circle
double xControl, yControl, lastXInt, lastYInt, xError, yError, lastXError, lastYError;
double k_i = 30;
clock_t lastTime;
clock_t start, end;

// HSV ranges for green laser
Scalar lowHSVgreen = Scalar(40, 90, 150);
Scalar highHSVgreen = Scalar(110, 255, 255);

// HSV ranges for red laser (Hue is split between start and end of range for red)
Scalar lowHSVred1 = Scalar(0, 70, 150);
Scalar highHSVred1 = Scalar(8, 255, 255);
Scalar lowHSVred2 = Scalar(172, 70, 150);
Scalar highHSVred2 = Scalar(179, 255, 255);

Mat imgOrig; // Image captured by camera
Mat imgHSV;
Mat imgThreshGreen;
Mat imgThreshRed1;
Mat imgThreshRed2;
Mat imgThreshRed;

// Used to find center of green/red dot
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
Moments m;
int cxGreen, cyGreen, cxRed, cyRed;
bool redDotFound, greenDotFound;

int main( int argc, char** argv ){
	cout << "Opening Camera... " << endl;
    VideoCapture cam(1); // Get camera 
    if (!cam.isOpened()){
        cout << "error: failed to open camera" << endl;
        return -1;
    }
	
	cout << "Moving to home position... " << endl;
	Turret turret;
	int xHome, yHome;
	turret.getHomePos(xHome, yHome);
	turret.sendAngles(0, 0);
	turret.turnOnLaser();

	cout << "Enter game mode (chase = 1, run = 2): ";
	cin >> game_mode;
	cout << "Enter difficulty (1-3): ";
	cin >> game_difficulty;
    while (waitKey(1) != 27){
        bool imgReadSuccess = cam.read(imgOrig); // Capture Image
		
        if (!imgReadSuccess){
            cout << "error: failed to read image frame" << endl;
            break;
        }

        // Convert original image to HSV
		cvtColor(imgOrig, imgHSV, COLOR_BGR2HSV);

		// Threshold images
		inRange(imgHSV, lowHSVgreen, highHSVgreen, imgThreshGreen);
		inRange(imgHSV, lowHSVred1, highHSVred1, imgThreshRed1);
		inRange(imgHSV, lowHSVred2, highHSVred2, imgThreshRed2);
		imgThreshRed = imgThreshRed1 + imgThreshRed2;

		// Perform morphological operations to reduce background noise
		erode(imgThreshGreen, imgThreshGreen, getStructuringElement(MORPH_ELLIPSE, Size(1, 1)));
		dilate(imgThreshGreen, imgThreshGreen, getStructuringElement(MORPH_ELLIPSE, Size(10, 10))); 
		erode(imgThreshRed, imgThreshRed, getStructuringElement(MORPH_ELLIPSE, Size(1, 1))); 
		dilate(imgThreshRed, imgThreshRed, getStructuringElement(MORPH_ELLIPSE, Size(10, 10))); 	

		// Find center of each dot
 		findContours(imgThreshGreen, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
		if(contours.size() > 0){
			m = moments(contours[0], false);
			cxGreen = m.m10/m.m00;
			cyGreen = m.m01/m.m00;
			circle(imgOrig, Point2f(cxGreen, cyGreen), 5, Scalar(255, 255, 0), -1, 8, 0);
			stringstream locText;
			locText << "(" << cxGreen << "," << cyGreen << ")";
			putText(imgOrig, locText.str(), Point2f(0,imgOrig.rows), FONT_HERSHEY_PLAIN, 5, Scalar(255, 255, 255));
			greenDotFound = true;
		}else{
			greenDotFound = false;
		}
		findContours(imgThreshRed, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
		if(contours.size() > 0){
			m = moments(contours[0], false);
			cxRed = m.m10/m.m00;
			cyRed = m.m01/m.m00;
			circle(imgOrig, Point2f(cxRed, cyRed), 5, Scalar(255, 0, 255), -1, 8, 0);
			redDotFound = true;
		}else{
			redDotFound = false;
		}
 	
		// Display images
        imshow("Original", imgOrig);
        //imshow("Thresholded Image (Green)", imgThreshGreen);
   		//imshow("Thresholded Image (Red)", imgThreshRed);
		//turret.readPosition(cxRed, cyRed);
		xError = cxGreen - cxRed;
		yError = cyGreen - cyRed;

		// Update control law based on game mode and difficulty selected
		if(game_mode == 1){ // Chase player mode
			switch(game_difficulty){
				case 1:{
					collisionDist = 20;
					xControl = 0.5*xError;
					yControl = 0.5*yError;
					break;
					}
				case 2:
					collisionDist = 35;
					xControl = 0.75*xError;
					yControl = 0.75*yError;
					break;
				case 3:{
					/*clock_t time = clock();
					double T = double(time - lastTime)/CLOCKS_PER_SEC;
					//cout << T << endl;
					double xInt = k_i*T/2*(xError + lastXError);
					double yInt = k_i*T/2*(yError + lastYError);
					xControl = lastXInt + xInt + xError;
					yControl = lastYInt + yInt + yError;;
					turret.sendPosition(cxRed + xControl, cyRed + yControl);
					lastXError = xError;
					lastYError = yError;
					lastXInt = xInt;
					lastYInt = yInt;
					lastTime = time;*/
					collisionDist = 50;
					xControl = xError;
					yControl = yError;
					break;
					}
				default:
					cout << "error: invalid difficulty selection" << endl;
					return -1;
			}
			if(greenDotFound){
				turret.sendPosition(cxRed + xControl, cyRed + yControl);
			}
		} else if(game_mode == 2){ // Run from player mode
			switch(game_difficulty){
				case 1:
					collisionDist = 50;
					k_effort = 1000;
					break;
				case 2:
					collisionDist = 35;
					k_effort = 3000;
					break;
				case 3:{
					collisionDist = 20;
					k_effort = 5000;
					break;
					}
				default:
					cout << "error: invalid difficulty selection" << endl;
					return -1;
			}
			double r_vect = sqrt(pow(cxRed - cxGreen, 2) + pow(cyRed - cyGreen, 2));
			double x_effort = k_effort*(cxRed - cxGreen)/pow(r_vect,2);
			double y_effort = k_effort*(cyRed - cyGreen)/pow(r_vect,2);
			double x_m = cxRed - xHome;
			double y_m = -(cyRed - yHome);
			double r_m = sqrt(pow(x_m,2) + pow(y_m,2)); // Measured radius
			double r_mGreen = sqrt(pow(cxGreen - xHome, 2) + pow(-(cyGreen - yHome), 2));
			//cout << "r_vect: " << r_vect << endl;
			//cout << "x_m: " << x_m << "\t\t y_m: " << y_m << endl;
			//cout << "x_effort: " << x_effort << "\t\t y_effort: " << y_effort << endl;
			//cout << "x_mGreen: " << cxGreen - xHome << "\t\t y_mGreen: " << -(cyGreen - yHome) << endl;
			//cout << "x_mRed: " << x_m << "\t\t y_mRed: " << y_m << endl;
			if(r_m >= r && r_mGreen <= r){ // If on or outside boundary circle
				double x_r = x_m*r/r_m;
				double y_r = y_m*r/r_m;
				double tanForce = (-x_effort*y_r - y_effort*x_r)/r; // Dot product of tangent vector and effort vector
				//cout << "tanForce: " << tanForce << endl;
				xControl = tanForce*(-y_r/r) + k_circle*(x_r - x_m);
				yControl = -(tanForce*(x_r/r) + k_circle*(y_r - y_m));
				//cout << "Green: IN	Red: OUT" << endl;
			}else if(r_m > r && r_mGreen > r){ xControl = 0.5*(xHome - cxRed);
				yControl = 0.5*(yHome - cyRed);
				//cout << "Green: OUT	Red: OUT" << endl;
			}else{ // If inside boundary circle
				xControl = x_effort;
				yControl = y_effort;
				//cout << "Green: ??	Red: IN" << endl;
			}
			// Set saturation limits
			double limit = 100;
			if(xControl > limit){
				xControl = limit;
			}else if(xControl < -limit){
				xControl = -limit;
			}
			if(yControl > limit){
				yControl = limit;
			}else if(yControl < -limit){
				yControl = -limit;
			}
			if(greenDotFound && redDotFound){
				turret.sendPosition(cxRed + xControl, cyRed + yControl);
			}else if(!redDotFound){
				turret.sendPosition(xHome, yHome);
			}
			//cout << "cxRed: " << cxRed << "\t\t cyRed: " << cyRed << endl;
			//cout << "xControl: " << xControl << "\t\t yControl: " << yControl << endl << endl;

		} else {
			cout << "error: invalid game mode selection" << endl;
			return -1;
		}

		// Detect whether a collision has occured
		if(sqrt(pow(xError, 2) + pow(yError, 2)) <= collisionDist && redDotFound && greenDotFound){
			//cout << "Collision detected!" << endl;
			system("(speaker-test -t sine -f 2000 )& pid=$! ; sleep 0.2s ; kill -9 $pid");
			system("(speaker-test -t sine -f 1000 )& pid=$! ; sleep 0.2s ; kill -9 $pid");
			turret.turnOffLaser();
			turret.sendAngles(0, 0);
			usleep(2000000);
			turret.turnOnLaser();
			system("(speaker-test -t sine -f 1000 )& pid=$! ; sleep 0.2s ; kill -9 $pid");
			system("(speaker-test -t sine -f 2000 )& pid=$! ; sleep 0.2s ; kill -9 $pid");
			cxGreen = -100;
			cyGreen = -100;
        	imgReadSuccess = cam.read(imgOrig); // Capture Image
        	imgReadSuccess = cam.read(imgOrig); // Capture Image
       		imgReadSuccess = cam.read(imgOrig); // Capture Image
        	imgReadSuccess = cam.read(imgOrig); // Capture Image
		}
	}

    return 0;
}
