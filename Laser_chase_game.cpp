#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int game_mode;
int game_difficulty;

// HSV ranges for green laser
Scalar lowHSVgreen = Scalar(40, 50, 150);
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
int cxGreen;
int cyGreen;
int cxRed;
int cyRed;

int pan_error;
int tilt_error;

int main( int argc, char** argv ){
    VideoCapture cam(1); // Get camera 
    if (!cam.isOpened()){
        cout << "error: failed to open camera" << endl;
        return -1;
    }
	
	//FILE *arduino;
	//arduino = fopen("/dev/ttyACM0", "w");

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
		}
		findContours(imgThreshRed, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
		if(contours.size() > 0){
			m = moments(contours[0], false);
			cxRed = m.m10/m.m00;
			cyRed = m.m01/m.m00;
			circle(imgOrig, Point2f(cxRed, cyRed), 5, Scalar(255, 0, 255), -1, 8, 0);
		}
 
		// Display images
        //imshow("Original", imgOrig);
        //imshow("Thresholded Image (Green)", imgThreshGreen);
   		//imshow("Thresholded Image (Red)", imgThreshRed);
		
		// Chase player mode
		if(game_mode == 1){
			pan_error = cyGreen - cyRed;
			tilt_error = cxGreen - cxRed;
			switch(game_difficulty){
				case 1:
				case 2:
				case 3:
					
					break;
				default:
					cout << "error: invalid difficulty selection" << endl;
					return -1;
			}
		} else if(game_mode == 2){

		} else {
			cout << "error: invalid game mode selection" << endl;
			return -1;
		}
	}

    return 0;
}
