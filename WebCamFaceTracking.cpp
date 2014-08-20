/*
 * WebCamFaceTracking.cpp
 *
 *  Created on: Jul 31, 2014
 *      Author: internshipdude
 */


#include <iostream>
#include <stdio.h>
#include <fstream>
#include<sstream>
#include <string>
#include <ctime> // for time measurement

#include <math.h> // use M_PI from here and sin

//OPENCV
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// EIGEN
#include <Eigen/Dense> // in eclipse project setting, include all of Eigen

// my Class
#include "WebCamFaceTracking.h"


using namespace std;
using namespace cv; // opencv
using namespace Eigen;


//namespace WebCamFaceTracking {
//
//WebCamFaceTracking::WebCamFaceTracking() {
//	// TODO Auto-generated constructor stub
//
//}
//
//WebCamFaceTracking::~WebCamFaceTracking() {
//	// TODO Auto-generated destructor stub
//}
//
//} /* namespace WebCamFaceTracking */

//=================================================================================================
//=================================================================================================
//=================================================================================================
/*
 * This is a facetracking class by the use of a webcam that is set directly above the screen.
 * Used to return the estimated Position of the eyes in 3D space
 *
 * The origin of its coordinate system is the center of the screen, unless you calibrate, then it is perpendicuar on the screen to where you calibrated it
 * -Z = Depth
 * +X = Right
 * +Y = Up
 * Everything is measured in milimeters
 *
 * The constructor reads the settings from the WebCamFaceTracking.cfg file
 * To update the Tracking Position, do "Vector3d Objectname.getNewTrackData();"
 *
 *
 * */
//=================================================================================================
//=================================================================================================
//=================================================================================================


//=================================================================================================
//=================================================================================================
//=================================================================================================
// public methods
//=================================================================================================
//=================================================================================================
//=================================================================================================
// note: replace cfg file with set methods


WebCamFaceTracking::WebCamFaceTracking(){
	configFile = "WebCamFaceTracking.cfg";
	readConfigFile();

	setUpSmoothing(); // necessary for dynamic Matrices
	resetSmoothers();
	faceFound=false;
	profileFound=false;
	calibrate=false;
	smoothonly=false;

	eyeXrelation=0.5;
	eyeYrelation=0.4;

	//	faceTrackingFile = "haarcascade_frontalface_alt2.xml"; // TODO send this to config file
	//	faceTrackingFile = "haarcascade_frontalface_alt_tree.xml";
	faceTrackingFile = "haarcascade_frontalface_default.xml";
	//	profileTrackingFile = "haarcascade_profileface.xml";

	face_cascade.load(faceTrackingFile);
	//	profile_cascade.load(profileTrackingFile);
	captureDevice.open(0);

	// read resolutions
	webcamYresolution = captureDevice.get(CV_CAP_PROP_FRAME_HEIGHT);
	webcamXresolution = captureDevice.get(CV_CAP_PROP_FRAME_WIDTH);

}

Vector3d WebCamFaceTracking::getNewTrackData(){
	// updates the frame and returns you the absolute position
	clock_t before = clock();
	trackUpdate();
	clock_t after = clock();
	//cout << ceil(double(after - before)*1000/ CLOCKS_PER_SEC) << " ms = for trackupdate"<<endl;
	return humanPosition;
}

Vector3d WebCamFaceTracking::getHumanPosition(){
	return humanPosition;
}


void WebCamFaceTracking::setScreenHeigth(double heigth){
	realLifeScreenHeight = heigth;
}

void WebCamFaceTracking::setScreenWidth(double width){
	realLifeScreenWidth = width;
}

void WebCamFaceTracking::setFaceHeigth(double faceheigth){
	realLifeFaceHeight = faceheigth;
}

void WebCamFaceTracking::setCamXResolution(int res){
	webcamXresolution = res;
}

void WebCamFaceTracking::setCamYResolution(int res){
	webcamYresolution = res;
}

void WebCamFaceTracking::setCamVerticalFOV(double fov){
	webcamVerticalFOV = fov;
}

void WebCamFaceTracking::setImageTrackMargin(int margin){
	imageMargin = margin;
}

void WebCamFaceTracking::setTrackDataSmoothingSteps(int cols){
	previousVectorDataCols = cols;
	setUpSmoothing();
}

void WebCamFaceTracking::setTrackPixelDataSmoothingSteps(int cols){
	previousPixelDataCols = cols;
	setUpSmoothing();
}

void WebCamFaceTracking::setErrorTolerance(double err){
	errorTolerance = err;
}

void WebCamFaceTracking::setShowWebCamStream(bool show ){
	showwebcam = show;
}

void WebCamFaceTracking::setCurrentConfigFile(const char* name){
	configFile = name;
}

void WebCamFaceTracking::findFaceAgain(){
	faceFound = false;
}

/* LINE BY LINE STRUCTURE OF CFG:
	realLifeScreenHeight in mm
	realLifeScreenWidth
	realLifeFaceHeight
	webcamVerFOV degrees
	imageMargin int TODO: should depend on res,FOV
	previousDataCols int
	previousPixelDataCols
	errorTolerance double
	showwebcam bool
	faceTrackingFile the char* needed to load the .xml // TODO

 */

void WebCamFaceTracking::readConfigFile(){
	// all parameters/Global variables are stored in a separate config file to be manipulated by the application

	// open file for input
	ifstream inputFile;
	inputFile.open(configFile);

	string line;
	inputFile >> line >> realLifeScreenHeight;
	inputFile >> line >> realLifeScreenWidth;
	inputFile >> line >> realLifeFaceHeight;
	inputFile >> line >> webcamVerticalFOV;
	inputFile >> line >> imageMargin;
	inputFile >> line >> previousVectorDataCols;
	inputFile >> line >> previousPixelDataCols;
	inputFile >> line >> errorTolerance;
	inputFile >> line >> showwebcam;

	//	string line;
	//	getline(configfile,line);
	//	getline(configfile,line);
	//	cout << line << endl;
	//
	//    faceTrackingFile = (const char*)line.c_str();
	////	faceTrackingFile = line;
	//
	//	cout << "SUCCEDEFULL READING A CHAR* " << faceTrackingFile << endl;

	//close file
	inputFile.close();
}

void WebCamFaceTracking::saveToConfigFile(){ // TODO currently not working because of misterious eigen messages sneaking in.
	// open file for output
	ofstream outputfile;
	outputfile.open(configFile);

	// write everything

	outputfile << "realLifeScreenHeightMilimeters= ";
	outputfile << realLifeScreenHeight << endl;

	outputfile << "realLifeScreenWidthMilimeters= ";
	outputfile << realLifeScreenWidth << endl;

	outputfile << "realLifeFaceHeightMilimeters= ";
	outputfile << realLifeFaceHeight<< endl;

	outputfile << "webcamVerticalFOVDegrees= ";
	outputfile << webcamVerticalFOV << endl;

	outputfile << "imageMarginPixels= ";
	outputfile << imageMargin << endl;

	outputfile << "previousVectorDataColsSmootheningIterations= ";
	outputfile << previousVectorDataCols << endl;

	outputfile << "previousPixelDataColsSmootheningIterations= ";
	outputfile << previousPixelDataCols << endl;

	outputfile << "errorTolerancePixels= ";
	outputfile << errorTolerance<< endl;

	outputfile << "showwebcamBoolean= ";
	outputfile << showwebcam << endl;

	//close it
	outputfile.close();
}

void WebCamFaceTracking::reportStatus(){
	cout << " It should be working, hopefully " << endl;
}

void WebCamFaceTracking::updateTrackData(){
	trackUpdate();
}

void WebCamFaceTracking::updateTrackDataSmoothOnly(){
	smoothonly=true;
	trackUpdate();
	smoothonly=false;
}

double WebCamFaceTracking::getXHumanPosition(){
	return humanPosition(0);
}

double WebCamFaceTracking::getYHumanPosition(){
	return humanPosition(1);
}

double WebCamFaceTracking::getZHumanPosition(){
	return humanPosition(2);
}

void WebCamFaceTracking::calibrateCenter(){
	calibrate = true;
}

void WebCamFaceTracking::resetCalibration(){
	calibrationX=0;
	calibrationY=0;
}

//=================================================================================================
//=================================================================================================
//=================================================================================================
/////////////////////// private METHODS /////////////////////////////
//=================================================================================================
//=================================================================================================
//=================================================================================================


void WebCamFaceTracking::setUpSmoothing(){
	// needed because of the dynamic sizes of the smoothening storage
	previousVectorData.conservativeResize(3,previousVectorDataCols);
	previousPixelData.conservativeResize(3,previousPixelDataCols);
}

void WebCamFaceTracking::resetSmoothers(){

	for( int i=0 ; i < previousVectorData.cols() ; i++){
		previousVectorData(0,i)=0;
		previousVectorData(1,i)=0;
		previousVectorData(2,i)=0;
	}
	for( int i=0 ; i < previousPixelData.cols() ; i++){
		previousPixelData(0,i)=0;
		previousPixelData(1,i)=0;
		previousPixelData(2,i)=1; // faceheigth
	}
	lastTrackPixelData(0) = 0;
	lastTrackPixelData(1) = 0;
	lastTrackPixelData(2) = 1;
}



void WebCamFaceTracking::trackPixelDataSmoother(int faceX, int faceY, int faceHeight){

	// Method: weighted averaging with previous values, and ignore rawData with minor changes: TODO adjust errorTolerance to facehight
	if(fabs(faceX-previousPixelData(0,0)) < errorTolerance) faceX = previousPixelData(0,0);
	if(fabs(faceY-previousPixelData(1,0)) < errorTolerance) faceY = previousPixelData(1,0);
	if(fabs(faceHeight-previousPixelData(2,0)) < errorTolerance*1.5) faceHeight = previousPixelData(2,0);

	Vector3d pixelData(faceX, faceY , faceHeight);
	pixelData = pixelData*(double)1/(previousPixelData.cols()+1);

	for( int i=0 ; i < previousPixelData.cols() ; i++){
		pixelData = pixelData + previousPixelData.col(i)*((double)1/( previousPixelData.cols() + 1 ));
	}

	//push all vectors around like a queue
	for( int i = previousPixelData.cols()-1 ; i > 0 ; i--){ // backwards loop
		previousPixelData.col(i) = previousPixelData.col(i-1);
	}
	previousPixelData.col(0)=pixelData;

}


Vector3d WebCamFaceTracking::trackVectorSmoother(Vector3d data){
	// smoothen out the offsetvector

	// Method: weighted averaging with previous values:
	data = data*(double)1/(previousVectorData.cols()+1);

	for( int i=0 ; i < previousVectorData.cols() ; i++){
		data = data + previousVectorData.col(i)*((double)1/( previousVectorData.cols() + 1 ));
	}

	//push all vectors around like a queue
	for( int i = previousVectorData.cols()-1 ; i > 0 ; i--){ // backwards loop
		previousVectorData.col(i) = previousVectorData.col(i-1);
	}
	previousVectorData.col(0)=data;
	return data;

}


void WebCamFaceTracking::trackFilter(double faceX, double faceY, double faceHeight){
	// here we interprete the trackdata to the cameraoffset. 0,0 pixel at top left

	// FILTER IT FIRST
	trackPixelDataSmoother( faceX, faceY, faceHeight);
	faceX = previousPixelData(0,0);
	faceY = previousPixelData(1,0);
	faceHeight = previousPixelData(2,0);
	//		cout << faceX << " = FaceX , " << faceY << " = FaceY , " << faceHeight << " = FaceHeight ,  AFTER SMOOTHY " << endl;

	double faceWidth = faceHeight; // keep this just in case


	// first, create pixel to mm scale by comparing to realLifeFaceHeigth, // TODO, we may need a horizontal imagescale
	double imageScale = realLifeFaceHeight/faceHeight;

	// estimate the center between the eyes in pixels, the center is the origin;
	double eyeX = faceX + faceWidth*eyeXrelation - (double)webcamXresolution/2; // eyeX=.5 for center of eye, .7 for right eye
	double eyeY = faceY + faceHeight*eyeYrelation - (double)webcamYresolution/2;

	// estimate the distance from camera to center of camera plane (only using y hight, we guess the camera does not distort
	// Camera plane := your face plane normal to the webcam's direction
	double distanceToCamPlane = (webcamYresolution*imageScale/2)/(tan(webcamVerticalFOV*(M_PI/180)/2)); // TODO -100 gives resonable results, WHY?!?!!??!
	//cout << "camplaneheigth = " << webcamYresolution*imageScale << ", distance calculation = " << (webcamYresolution*imageScale/2)/(tan(webcamVerticalFOV*(M_PI/180)/2))<< endl;
	// now estimate the position of your eyes (to the webcam)
	//// -Z is depth, +X is right, +Y is up

	Vector3d data( // WARNING: we are in the webcame Frame coords
			-eyeX*imageScale, // minus X because webcam image is mirrored @ y-axis
			-eyeY*imageScale, // I have to flip y to have +y up. maybe because the eye coords origin is top left of webcam frame
			distanceToCamPlane //old: negative sign because i needed to flip it, maybe because of webcamVerFOV
	);

	// NEW TRY, since camera parralel to screen, just shift the result down, cam is 1cm over screen
	data(1) = data(1) + (realLifeScreenHeight/2+10);


	// smooth out data by extrapolation and update cameraOffset
	data = trackVectorSmoother(data);

	humanPosition = data;

	if(calibrate){ // calibrate the data for the x,y coords, maybe later also the z s.t. the initial FOV is as demanded.
		calibrate = false;
		calibrationX = humanPosition(0);
		calibrationY = humanPosition(1);
	}
	humanPosition(0) -= calibrationX;
	humanPosition(1) -= calibrationY;

}




void WebCamFaceTracking::trackUpdate(){
	//create a vector array to store the face found
	std::vector<Rect> faces(1);


	if(!smoothonly){ // smoothonly asks for smoothend data without opencv
		clock_t begin = clock();
		//capture a new image frame, this is the image in the showwebcam, the grayscale is used for face detection
		captureDevice>>captureFrame;

		if(faceFound){ // if true, work on a cropped version for performance boost

			int ancorX = lastTrackPixelData(0) - imageMargin;
			int ancorY = lastTrackPixelData(1) - imageMargin;
			int ancorHeight = lastTrackPixelData(2) + 2*imageMargin;
			int ancorWidth = lastTrackPixelData(2) + 2*imageMargin;

			// make sure we do not cross the boarders
			if( ancorX < 0 ) ancorX=0;
			if( ancorY < 0 ) ancorY=0;
			if( ancorX + ancorWidth >= webcamXresolution ) ancorWidth = webcamXresolution - ancorX;
			if( ancorY + ancorHeight >= webcamYresolution ) ancorHeight = webcamYresolution - ancorY;

			//convert captured image to gray scale and equalize
			cvtColor(captureFrame( Rect(ancorX , ancorY , ancorWidth , ancorHeight ) ), grayscaleFrame, CV_BGR2GRAY);
			equalizeHist(grayscaleFrame, grayscaleFrame);

			if(showwebcam){ // show what i am tracking
				imshow("CROP", grayscaleFrame);
				waitKey(1);
			}

			// estimate the size of the face
			double min_face_size = lastTrackPixelData(2)*0.8;
			double max_face_size = lastTrackPixelData(2)*1.2;

			//		clock_t begindetect = clock();
			//		if(!profileFound){ // distinguish between tracking the front or the side of the face
			// mistery note: seting 1.1 to 1.5 make it much faster, but looses faceHeight accuracy. use this if only the x,y data is needed
			face_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 0, CV_HAAR_FIND_BIGGEST_OBJECT| CV_HAAR_SCALE_IMAGE, Size(min_face_size, min_face_size),Size(max_face_size, max_face_size));

			//			if(faces.size()==0){
			//				profile_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 0, CV_HAAR_FIND_BIGGEST_OBJECT| CV_HAAR_SCALE_IMAGE, Size(min_face_size, min_face_size),Size(max_face_size, max_face_size));
			//				if(faces.size()==1) profileFound=true;
			//			}
			//		} else {
			//			cout << "profile found" << endl;
			//			profile_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 0, CV_HAAR_FIND_BIGGEST_OBJECT| CV_HAAR_SCALE_IMAGE, Size(min_face_size, min_face_size),Size(max_face_size, max_face_size));
			//
			//			if(faces.size()==0){
			//				profileFound=false;
			//				face_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 0, CV_HAAR_FIND_BIGGEST_OBJECT| CV_HAAR_SCALE_IMAGE, Size(min_face_size, min_face_size),Size(max_face_size, max_face_size));
			//			}
			//		}
			//		clock_t endDetect = clock();
			//		cout << ceil(double(endDetect - begindetect)*1000/ CLOCKS_PER_SEC) << " ms = time cropped detection"<<endl;

			// now recalibrate the data back to the big image frame
			if(faces.size()==1) {
				faces[0].x += ancorX;
				faces[0].y += ancorY;
			}

		}else{ // find the face again using the entire image

			//convert captured image to gray scale and equalize
			cvtColor(captureFrame, grayscaleFrame, CV_BGR2GRAY);
			equalizeHist(grayscaleFrame, grayscaleFrame);
			//find faces and store them in the vector array, BIGGEST OBJECT makes sure we only have one!
			//		clock_t begindetect = clock();
			face_cascade.detectMultiScale(grayscaleFrame, faces, 1.1, 2, CV_HAAR_FIND_BIGGEST_OBJECT| CV_HAAR_SCALE_IMAGE, Size(30,30));
			//		clock_t endDetect = clock();
			//		cout << ceil(double(endDetect - begindetect)*1000/ CLOCKS_PER_SEC) << " msec = time full detection"<<endl;
		}

		clock_t end = clock();
		//	cout << ceil(double(end - begin)*1000/ CLOCKS_PER_SEC) << " msec = time per facedetection"<<endl;

		if(showwebcam){
			//draw a rectangle for all found faces in the vector array on the original image
			if(faces.size()==1)	{
				Point pt1(faces[0].x + faces[0].width, faces[0].y + faces[0].height);
				Point pt2(faces[0].x, faces[0].y);
				int rec2size = 8;
				Point pt1eye(faces[0].x + faces[0].width*eyeXrelation -rec2size/2, faces[0].y + faces[0].height*eyeYrelation - rec2size/2);
				Point pt2eye(faces[0].x + faces[0].width*eyeXrelation +rec2size/2, faces[0].y + faces[0].height*eyeYrelation + rec2size/2);
				Point pt1track(faces[0].x + faces[0].width + imageMargin, faces[0].y + faces[0].height + imageMargin);
				Point pt2track(faces[0].x - imageMargin, faces[0].y - imageMargin);

				rectangle(captureFrame, pt1, pt2, cvScalar(0, 255, 0, 0), 1, 8, 0);
				rectangle(captureFrame, pt1eye, pt2eye, cvScalar(0,0,0, 0), 3, 8, 0);
				rectangle(captureFrame, pt1track, pt2track, cvScalar(0,0,255, 0), 3, 8, 0);
			}
			//show the output
			imshow("outputCapture", captureFrame);
			//pause for 33ms, i replaced it with 1 ms, without it it wont show the image
			waitKey(1);
		}

	} else {  // if only smoothing, pass previous info, future idea: predict movement
		faces[0].x=lastTrackPixelData(0);
		faces[0].y=lastTrackPixelData(1);
		faces[0].height=lastTrackPixelData(2);
	}// end smoothonly



	//if tracking found, read and interpret it
	if(faces.size()==1) {
		faceFound=true;
		// update last actual position
		lastTrackPixelData(0) = faces[0].x;
		lastTrackPixelData(1) = faces[0].y;
		lastTrackPixelData(2) = faces[0].height;

		trackFilter(faces[0].x, faces[0].y, faces[0].height);

	}else{
		// if no face detected, remember to try the entire image again!
		faceFound = false;
	}
	clock_t postsmooth = clock();
	//	cout << ceil(double(postsmooth - end)*1000/ CLOCKS_PER_SEC) << " msec = time for smoothening"<<endl;
}




