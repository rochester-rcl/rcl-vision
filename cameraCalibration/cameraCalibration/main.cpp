//
//  main.cpp
//  cameraCalibration
//
//  Created by JoshR on 4/27/15.
//  Copyright (c) 2015 JoshR. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

std::vector<cv::Point3f> Create3DChessboardCorners(cv::Size boardSize, float squareSize);

int main()
{
    
   

    Mat image = imread("/Volumes/disk1s2/openCVTest/chessboard.jpg", 1);
    
    if (image.data == 0) {
        cerr << "Image not found!" << endl;
        return -1;
    }
    
    //Vectors of image points
    std::vector<std::vector<cv::Point2f> > imageCorners(1);
    //Set number of inner* corners on the chessboard
    cv::Size boardSize(9,6);
    cv::Size imageSize = image.size();
    float squareSize = 1.f;
    //Find the chessboard corners
    bool found = cv::findChessboardCorners(image, boardSize, imageCorners[0]);
    
    if (!found) {
        cerr << "No corners detected" << endl;
    
    }
    
    //Draw corners
    //cv::drawChessboardCorners(image, boardSize, cv::Mat(imageCorners[0]), found);
    
    std::vector<std::vector<cv::Point3f> > objectPoints(1);
    objectPoints[0] = Create3DChessboardCorners(boardSize, squareSize);
    
    //Set up rotation and translation vectors
    std::vector<cv::Mat> rotationVectors;
    std::vector<cv::Mat> translationVectors;
    
    cv::Mat distortionCoefficients = cv::Mat::zeros(8, 1, CV_64F); // 8 distortion coefficients
    cv::Mat cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    
    int flags = 0;
    double rms = calibrateCamera(objectPoints, imageCorners, imageSize, cameraMatrix, distortionCoefficients, rotationVectors, translationVectors, flags|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);
    
    std::cout << "RMS: " << rms << std::endl;
    
    std::cout << "Camera matrix: " << cameraMatrix << std::endl;
    std::cout << "Distortion _coefficients: " << distortionCoefficients << std::endl;
    
    cv::Mat undistorted;
    cv::undistort(image, undistorted, cameraMatrix, distortionCoefficients);
    
    
    

    
    
    imshow("image", undistorted);
    
    waitKey();
    
}

std::vector<cv::Point3f> Create3DChessboardCorners(cv::Size boardSize, float squareSize)
{
    // This function creates the 3D points of your chessboard in its own coordinate system
    
    std::vector<cv::Point3f> corners;
    
    for( int i = 0; i < boardSize.height; i++ )
    {
        for( int j = 0; j < boardSize.width; j++ )
        {
            corners.push_back(cv::Point3f(float(j*squareSize),
                                          float(i*squareSize), 0));
        }
    }
    
    return corners;
}

