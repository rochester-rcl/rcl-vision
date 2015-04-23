//
//  main.cpp
//  OpenCVKinectSnapshot
//
//  Created by JoshR on 1/7/15.
//  Copyright (c) 2015 JoshR. All rights reserved.
//

#include "libfreenect/libfreenect.hpp"
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <pthread.h>
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

int boardHeight = 6;
int boardWidth = 10;

Size cbSize = Size(boardHeight,boardWidth);


class myMutex {
public:
    myMutex() {
        pthread_mutex_init( &m_mutex, NULL );
    }
    void lock() {
        pthread_mutex_lock( &m_mutex );
    }
    void unlock() {
        pthread_mutex_unlock( &m_mutex );
    }
private:
    pthread_mutex_t m_mutex;
};


class MyFreenectDevice : public Freenect::FreenectDevice {
public:
    MyFreenectDevice(freenect_context *_ctx, int _index)
    : Freenect::FreenectDevice(_ctx, _index), m_buffer_depth(FREENECT_DEPTH_11BIT),
    m_buffer_rgb(FREENECT_VIDEO_RGB), m_gamma(2048), m_new_rgb_frame(false),
    m_new_depth_frame(false), depthMat(Size(640,480),CV_16UC1),
    rgbMat(Size(640,480), CV_8UC3, Scalar(0)),
    ownMat(Size(640,480),CV_8UC3,Scalar(0)) {
        
        for( unsigned int i = 0 ; i < 2048 ; i++) {
            float v = i/2048.0;
            v = std::pow(v, 3)* 6;
            m_gamma[i] = v*6*256;
        }
    }
    
    // Do not call directly even in child
    void VideoCallback(void* _rgb, uint32_t timestamp) {
       // std::cout << "RGB callback" << std::endl;
        m_rgb_mutex.lock();
        uint8_t* rgb = static_cast<uint8_t*>(_rgb);
        rgbMat.data = rgb;
        m_new_rgb_frame = true;
        m_rgb_mutex.unlock();
    };
    
    // Do not call directly even in child
    void DepthCallback(void* _depth, uint32_t timestamp) {
        //std::cout << "Depth callback" << std::endl;
        m_depth_mutex.lock();
        uint16_t* depth = static_cast<uint16_t*>(_depth);
        depthMat.data = (uchar*) depth;
        m_new_depth_frame = true;
        m_depth_mutex.unlock();
    }
    
    bool getVideo(Mat& output) {
        m_rgb_mutex.lock();
        if(m_new_rgb_frame) {
            cvtColor(rgbMat, output, CV_RGB2BGR);
            m_new_rgb_frame = false;
            m_rgb_mutex.unlock();
            return true;
        } else {
            m_rgb_mutex.unlock();
            return false;
        }
    }
    
    bool getDepth(Mat& output) {
        m_depth_mutex.lock();
        if(m_new_depth_frame) {
            depthMat.copyTo(output);
            m_new_depth_frame = false;
            m_depth_mutex.unlock();
            return true;
        } else {
            m_depth_mutex.unlock();
            return false;
        }
    }
private:
    std::vector<uint8_t> m_buffer_depth;
    std::vector<uint8_t> m_buffer_rgb;
    std::vector<uint16_t> m_gamma;
    Mat depthMat;
    Mat rgbMat;
    Mat ownMat;
    myMutex m_rgb_mutex;
    myMutex m_depth_mutex;
    bool m_new_rgb_frame;
    bool m_new_depth_frame;
};


int main(int argc, char **argv) {
    
    //Get intrinsics and distortion coefficients of kinect
    
    FileStorage file;
    Mat intrinsics, distortion;
    file.open("/Volumes/disk1s2/kinectCalib/intrinsics.yml", FileStorage::READ);
    //Read the matrices from the file
    file["M1"] >> intrinsics;
    file["D1"] >> distortion;
    
    file.release();
    
	bool die(false);
	string filename("/Users/joshr/Desktop/kinectSnapshot");
	string suffix(".png");
	int i_snap(0),iter(0);
	
  	Mat depthMat(Size(640,480),CV_16UC1);
	Mat depthf (Size(640,480),CV_8UC1);
	Mat gray, rgbMat(Size(640,480),CV_8UC3,Scalar(0));
	Mat ownMat(Size(640,480),CV_8UC3,Scalar(0));
    
    Mat rvec = Mat(Size(3,1), CV_64F);
    Mat tvec = Mat(Size(3,1), CV_64F);
    
    vector<Point2d> imagePoints, imageFramePoints, imageOrigin;
    vector<Point3d> boardPoints, framePoints;
    
    //Generate vectors for points on the board
    
    for (int i=0; i<boardWidth; i++)
    {
        for (int j=0; j<boardHeight; j++)
        {
            boardPoints.push_back( Point3d( double(i), double(j), 0.0));
        }
    }
    
    //generate points in reference frame
    
    framePoints.push_back(( Point3d(0.0, 0.0, 0.0 )));
    framePoints.push_back(( Point3d(5.0, 0.0, 0.0 )));
    framePoints.push_back(( Point3d(0.0, 5.0, 0.0 )));
    framePoints.push_back(( Point3d(0.0, 0.0, 5.0 )));
	
	// The next two lines must be changed as Freenect::Freenect
	// isn't a template but the method createDevice:
	// Freenect::Freenect<MyFreenectDevice> freenect;
	// MyFreenectDevice& device = freenect.createDevice(0);
	// by these two lines:
	
	Freenect::Freenect freenect;
	MyFreenectDevice& device = freenect.createDevice<MyFreenectDevice>(0);
	
	namedWindow("rgb",CV_WINDOW_AUTOSIZE);
	namedWindow("depth",CV_WINDOW_AUTOSIZE);
	device.startVideo();
	device.startDepth();
	while (!die) {
		device.getVideo(rgbMat);
		device.getDepth(depthMat);
        cvtColor(rgbMat,gray,COLOR_BGR2GRAY);
        bool found = findChessboardCorners(gray, cbSize, imagePoints, CALIB_CB_FAST_CHECK);
        
        //Find camera's orientation based on corners
        
        if (found)
        {
            //find extrinsic parameters
            solvePnP( Mat(boardPoints), Mat (imagePoints), intrinsics, distortion, rvec, tvec, false);
            
            //Project reference point onto the frame
            projectPoints(framePoints, rvec, tvec, intrinsics, distortion, imageFramePoints);
            
            //Now we draw the points
            
            //circle(rgbMat, (Point) imagePoints[0], 4 ,CV_RGB(255,0,0) );
            
            Point one, two, three;
            one.x=10; one.y=10;
            two.x = 60; two.y = 10;
            three.x = 10; three.y = 60;
            
            line(rgbMat, one, two, CV_RGB(255,0,0));
            line(rgbMat, one, three, CV_RGB(0,255,0) );
            
            
            line(rgbMat, imageFramePoints[0], imageFramePoints[1], CV_RGB(255,0,0), 2 );
            line(rgbMat, imageFramePoints[0], imageFramePoints[2], CV_RGB(0,255,0), 2 );
            line(rgbMat, imageFramePoints[0], imageFramePoints[3], CV_RGB(0,0,255), 2 );
            
            
            
            //show the pose estimation data
            cout << fixed << "rvec = ["
            << rvec.at<double>(0,0) << ", "
            << rvec.at<double>(1,0) << ", "
            << rvec.at<double>(2,0) << "] \t" << "tvec = ["
            << tvec.at<double>(0,0) << ", "
            << tvec.at<double>(1,0) << ", "
            << tvec.at<double>(2,0) << "]" << endl;
            
            
			
            
            
            
        }
		cv::imshow("rgb", rgbMat);
		depthMat.convertTo(depthf, CV_8UC1, 255.0/2048.0);
		//cv::imshow("depth",depthf);
		char k = cvWaitKey(5);
        
        if( k == 's' ) {
			std::ostringstream file;
			file << filename << i_snap << suffix;
			imwrite(file.str(),rgbMat);
			i_snap++;
		}
 
		if( k == 27){
			cvDestroyWindow("rgb");
			cvDestroyWindow("depth");
			break;
		}
		
		if(iter >= 1000) break;
		iter++;
	}
	
	device.stopVideo();
	device.stopDepth();
	return 0;
}


