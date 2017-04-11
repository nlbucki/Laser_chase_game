#include <iostream>
#include <fstream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

int d, xHome, yHome;

int main( int argc, char** argv ){
	
	
	
	ofstream calFile;
	calFile.open("calibrationFile.txt");
	calFile << xHome << endl;
	calFile << yHome << endl;
	calFile << d << endl;
	calFile.close();	
	return 0;
}
