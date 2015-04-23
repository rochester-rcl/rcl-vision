#!/usr/bin/env python
import cv2
import numpy as np

cap = cv2.VideoCapture('/Users/joshr/Documents/opencv/fuji2.mp4')
codec = cv2.cv.CV_FOURCC(*'WRLE')
output = cv2.VideoWriter('/Users/joshr/Documents/opencv/tokyoTest.mov',codec, 24.0, (1920,1080), False)
while(1):

    # Take each frame
    _, frame = cap.read()

    # Convert BGR to HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # define range of blue color in HSV
    lower_magenta = np.array([0,0,0])
    upper_magenta = np.array([130,255,255])

    # Threshold the HSV image to get only blue colors
    mask = cv2.inRange(hsv, lower_magenta, upper_magenta)

    # Bitwise-AND mask and original image
    res = cv2.bitwise_and(frame,frame, mask= mask)

    #cv2.imshow('frame',frame)
    #cv2.imshow('mask',mask)
    cv2.imshow('res',res)
    output.write(res)
    k = cv2.waitKey(5) & 0xFF
    if k == 27:
        break
output.release
cv2.destroyAllWindows()
