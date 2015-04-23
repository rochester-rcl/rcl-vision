//
//  main.cpp
//  OpenCV Optical Flow
//
//  Created by JoshR on 7/8/14.
//  Copyright (c) 2014 JoshR. All rights reserved.
//

#include <iostream>
//#include "Header.h"
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <math.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <sstream>

using namespace cv;

using namespace std;

static const double pi = 3.14159265358979323846; //Setting Pi as a static, constant variable

inline static double square(int a)
{
    return a * a;
}

/*every time the functions "square" and "allocateOnDemand are called, the compiler will place the body of the code there, reducing mess!*/

inline static void allocateOnDemand ( IplImage **img, CvSize size, int depth, int channels
                                     )
{
    if (*img != NULL) return;
    
    *img = cvCreateImage( size, depth, channels);
    if ( *img == NULL )
    {
        fprintf(stderr, "Error: Couldn't Allocate Image.\n");
        exit(-1);
    }
}

int main (void)
{
    /*This object decodes the input video stream*/
    CvCapture *input_video = cvCaptureFromFile(
                                               "/Volumes/disk1s2/openCVTest/Trimmed2.avi"
                                               );
    if (input_video == NULL)
    { //Can't find video file OR codec is unsupported by OpenCV
        fprintf(stderr, "Error: Can't Open Video.\n");
        return -1;
    }
    //cvQueryFrame( input_video );
    
    //Read the width and height of each video frame
    CvSize frame_size;
    frame_size.height =
    (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_HEIGHT );
    frame_size.width =
    (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_FRAME_WIDTH );
    
    //Get the number of frames
    
    long number_of_frames;
    //Move to the end of the file to get the total no. of frames
    cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_AVI_RATIO, 1. );
    number_of_frames = (int) cvGetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES);
    //Move to the beginning
    cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES, 0. );
    
    //Now we make 3 windows, "Frame N", "Frame N+1", and "Optical Flow"
    cvNamedWindow("Optical Flow", CV_WINDOW_AUTOSIZE);
    
    long current_frame = 0;
    while(true)
    {
        static IplImage *frame = NULL, *frame1 = NULL, *frame1_1C = NULL, *frame2_1C = NULL, *eig_image = NULL, *temp_image = NULL, *pyramid1 = NULL, *pyramid2 = NULL;
        
        //Traveling to the first frame
        cvSetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES, current_frame );
        //Move to the next frame
        frame = cvQueryFrame( input_video);
        if (frame == NULL)
        {
            fprintf(stderr, "Error: The End Happened Before We Thought.\n");
            return -1;
        }
        //Allocate another image
        allocateOnDemand( &frame1_1C, frame_size, IPL_DEPTH_8U, 1);//Analyzing the image through 8-bit, monocrhome color depth
        cvConvertImage(frame, frame1_1C);
        //Make a full color backup of the frame to draw optical flow on it
        allocateOnDemand( &frame1, frame_size, IPL_DEPTH_8U, 3 );//24 Bit color backup
        cvConvertImage(frame, frame1);
        
        //Get the second frame
        frame = cvQueryFrame( input_video );
        if (frame == NULL)
        {
            fprintf(stderr, "Error: The End Happened Before We Thought.\n");
            return -1;
        }
        allocateOnDemand( &frame2_1C, frame_size, IPL_DEPTH_8U, 1);
        cvConvertImage(frame, frame2_1C);
        
        /* Using Shi and Tomasi Corner Detection and Feature Tracking Algorithm */
        
        //First we need to allocate storage
        
        allocateOnDemand( &eig_image, frame_size, IPL_DEPTH_32F, 1 );
        allocateOnDemand( &temp_image, frame_size, IPL_DEPTH_32F, 1);
        
        //Store the features of frame 1 in an array
        CvPoint2D32f frame1_features[20];
        //Before the function is called this variable is set as the array size (max number of features). After the function call it's the number of features actually found
        int number_of_features;
        
        number_of_features = 20;
        
        /* Alright, time to run the Shi and Tomasi Algorithm
         * frame1_1C is the actual input image
         * eig_image and temp_image are the images the algorithm is working on
         * The first ".01" specifies the minimum quality of the features (based on the eigen values -- too much math for me!)
         * The second ".01" specifies the minimum Euclidian distance between features, ok this I understand
         * NULL means the algorithm is working on the entire input image - maybe we could also point to a specific area of the image
         * What the algorithm returns:
         *frame1_features contains feature points, which could possibly be stored or printed in stdout or xml or something?
         *number_of_features is a value of <= 400 because this is the threshold we set up there ^ */
        
        cvGoodFeaturesToTrack(frame1_1C, eig_image, temp_image, frame1_features, & number_of_features, .01, .05, NULL);
        
        /* Now some Lucas Kanade Optical Flow, which is used by OpenCV*/
        
        //The following array contains the locations of points from frame 1 in frame 2
        CvPoint2D32f frame2_features[20];
        
        /* "The i-th element of this array will be non-zero if and only if the i-th feature
         * of frame 1 was found in frame 2 (looking for matching points between frames 1 and 2)*/
        
        char optical_flow_found_feature[20];
        
        /* The i-th element of this array is the error in the optical flow for the i-th feature of frame 1 as found in frame 2
         * Again this is too much math for me... it's looking for the differences between the positions in 1 and 2 */
        float optical_flow_feature_error[20];
        
        // Set up the window size for the optical flow window
        
        CvSize optical_flow_window = cvSize(3,3);
        
        /* The algorithm is set to stop after 20 iterations or if epsilon is better that .3 - again, too much math */
        
        CvTermCriteria optical_flow_termination_criteria
        = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3); //can change these values
        
        // Lucas Kanade algorithm "cuts the image into pyramids of different resolutions"
        
        allocateOnDemand( &pyramid1, frame_size, IPL_DEPTH_8U, 1);
        allocateOnDemand( &pyramid2, frame_size, IPL_DEPTH_8U, 1);
        
        /* Run Lucas Kanade Optical Flow Algorithm
         * frame1_1C is the first frame with known features
         * frame2_1C is the second frame where we try to find the first frame's features
         * pyramid1 and pyramid2 are the workspace for the algorithm
         * frame1_features are the features returned from the first frame
         * frame2_features are the locations of the features in the second frame
         * number_of_features is the number of features in the frame1_features array
         * optical_flow_window is the size of the window
         * 5 is the max number of pyramids to use (0 is one level)
         * 0 - disable enhancements */
        
        cvCalcOpticalFlowPyrLK(frame1_1C, frame2_1C, pyramid1, pyramid2, frame1_features, frame2_features, number_of_features, optical_flow_window, 5, optical_flow_found_feature, optical_flow_feature_error, optical_flow_termination_criteria, 0);
        
        /*---------------------- Set Up File Saving Conventions--- Uncommenting this will create an XML file with all feature points for every frame ----
        
        int counter;
        
        string result;
        
        ostringstream convert;
        
        string filename;
        
       
        
        for (counter = 0; counter < current_frame; counter++)
        {
        
        convert << counter;
        
         result = convert.str();
         
        }
        filename = "featuresOutputFrame"+result+".xml";
        FileStorage fs(filename, FileStorage::WRITE);
        fs.open(filename, FileStorage::WRITE);
        
        ------------------------*/
 
        CvPoint p,q;
  
        // Draw the flow field
        for(int i = 0; i < number_of_features; i++)
        {
            // If LK didn't find the feature, it can be skipped
            
            if ( optical_flow_found_feature[i] == 0 ) continue;
            
            int line_thickness;
            
            line_thickness = 1;
            
            CvScalar line_color;
            line_color = CV_RGB(0,0,255); // blue
            
            p.x = frame1_features[i].x;
            p.y =  frame1_features[i].y;
            q.x =  frame2_features[i].x;
            q.y = frame2_features[i].y;
            
            cvCircle(frame1, p, 5, line_color);
      
        }
        
        //cvSetImageROI(frame1, cvRect(0,0, 500, 200));
        cvShowImage("Optical Flow", frame1);
 
        int frame_count;
        
        frame_count = cvGetCaptureProperty( input_video, CV_CAP_PROP_POS_FRAMES);
        
        int key_pressed;//Setting up a switch case for the controls
        key_pressed = cvWaitKey(100);
     
        switch (key_pressed)
        {
            {case 'f'://video forward
                current_frame++;
                /*FileStorage fs("frameOutput.xml", FileStorage::WRITE);
                fs << "frameCount" << frame_count;
                fs << "featuresXCoord" << p.x;
                fs << "featuresYCoord" << p.y;*/
                
                cout << "frameCount" << frame_count << "\n"; //For now we're just writing these values to the console because there is no elegant way to store them in an XML file.
                cout << "featuresXCoord" << p.x << "\n";
                cout << "featuresYCoord" << p.y << "\n";
                
                //fs.release();
                break;}
            {case 'b'://video back
                current_frame--;
                break;}
        }
        
        if (current_frame < 0) current_frame = 0;
        if (current_frame >= number_of_frames - 1) current_frame = number_of_frames - 2;
        

   
           }
    
}















