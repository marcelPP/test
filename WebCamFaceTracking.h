/*
 * WebCamFaceTracking.h
 *
 *  Created on: Jul 31, 2014
 *      Author: internshipdude
 */



#ifndef WEBCAMFACETRACKING_H_
#define WEBCAMFACETRACKING_H_

//OPENCV, keep them just in case. WARNING: we need opencv_ highgui, core, objdetect, imgproc libraries for this to work
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// EIGEN
#include <Eigen/Dense> // in eclipse project setting, include all of Eigen

//namespace WebCamFaceTracking {

class WebCamFaceTracking {
public:
	// the consstructor
	WebCamFaceTracking();

private: // all variables private, they are all specified in the cfg file or

	// opencv headtracking init
	cv::CascadeClassifier face_cascade;
//	cv::CascadeClassifier profile_cascade;
	cv::VideoCapture captureDevice;
	cv::Mat captureFrame;
	cv::Mat grayscaleFrame;

	Eigen::Vector3d humanPosition; // this is the tracker data being streamed,

	// sizes in mm
	double realLifeScreenHeight;
	double realLifeScreenWidth; // unused here, will be needed for perspective distortion
	double realLifeFaceHeight; // regarding the rectangle in the videostream

	// web cam tech specs
	int webcamXresolution;
	int webcamYresolution;
	double webcamVerticalFOV; // wikipedia kinect

	// track details
	bool faceFound; // Used for reset "tracking" without recalibrating.
	bool profileFound; // used to Track the side of a face
	int imageMargin; //image margin for tracking TODO, should be dependant on webcam

	// SMOOTHENING Iterations is the nr. of cols in this verctor: NEWEST VECTOR TO .col(0)
	int previousVectorDataCols;
	int previousPixelDataCols;
	Eigen::Matrix<double, 3, Eigen::Dynamic> previousVectorData;
	Eigen::Matrix<double, 3, Eigen::Dynamic> previousPixelData;
	Eigen::Vector3d lastTrackPixelData; // need for the tracking part, to crop the image
	double errorTolerance;

	// relative eye position to the facewidth/height
	double eyeXrelation;
	double eyeYrelation;

	// calibration
	bool calibrate;
	double calibrationX;
	double calibrationY;

	// booleans
	bool showwebcam;
	bool smoothonly;

	// location of files:
	const char* configFile;
	const char* faceTrackingFile;
//	const char* profileTrackingFile; // side of face

//
//	// extra
//	int framecounter=0;
//	double lengthConversionScale = 0.0393701; // mm to inch
//	bool convertCoordinates = false;


	// method declarations
public:


	// the main thing for the user
	Eigen::Vector3d getNewTrackData();
	Eigen::Vector3d getHumanPosition();
	void saveToConfigFile();
	void readConfigFile();
	void reportStatus();
	void calibrateCenter();
	void resetCalibration();

	// Eigen independant methods
	void updateTrackData();
	void updateTrackDataSmoothOnly();
	double getXHumanPosition();
	double getYHumanPosition();
	double getZHumanPosition();

	//set methods
	void setScreenHeigth(double);
	void setScreenWidth(double);
	void setFaceHeigth(double);
	void setCamXResolution(int);
	void setCamYResolution(int);
	void setCamVerticalFOV(double);
	void setImageTrackMargin(int);
	void setTrackDataSmoothingSteps(int);
	void setTrackPixelDataSmoothingSteps(int);
	void setErrorTolerance(double);
	void setShowWebCamStream(bool);
	void setCurrentConfigFile(const char*);
	void findFaceAgain();



private:

	void setUpSmoothing();
	void resetSmoothers();
	void trackPixelDataSmoother(int , int , int);
	Eigen::Vector3d trackVectorSmoother(Eigen::Vector3d);
	void trackFilter(double , double , double );
	void trackUpdate();

	//public:
	//	WebCamFaceTracking();
	//	virtual ~WebCamFaceTracking();
};

//} /* namespace WebCamFaceTracking */

#endif /* WEBCAMFACETRACKING_H_ */
