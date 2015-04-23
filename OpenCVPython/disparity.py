#!/usr/bin/env python
import cv2
import numpy as np
from matplotlib import pyplot as plt
cap1 = cv2.VideoCapture('/Users/joshr/Documents/opencv/fuji2.mp4')
cap2 = cv2.VideoCapture('/Users/joshr/Documents/opencv/tokyoTest.mov')
codec = cv2.cv.CV_FOURCC(*'WRLE')
output = cv2.VideoWriter('/Users/joshr/Documents/opencv/disparityTest.mov',codec, 24.0, (1920,1080), False)
while(1):

    # Take each frame
    _, frame1 = cap1.read()
    
    _, frame2 = cap2.read()
    
    gray1 = cv2.cvtColor(frame1, cv2.COLOR_BGR2GRAY)
    
    gray2 = cv2.cvtColor(frame2, cv2.COLOR_BGR2GRAY)
    
    stereo = cv2.StereoBM(cv2.STEREO_BM_BASIC_PRESET,ndisparities=16, SADWindowSize=15)
    
    disparity = stereo.compute(gray1,gray2)
    
    disparityImg = disparity.astype(np.uint8)
    
    disparity_color = cv2.cvtColor(disparityImg, cv2.COLOR_GRAY2RGB)
    
    output.write(disparity_color)
    
 


