#!/usr/bin/env python

import numpy as np
import cv2
from matplotlib import pyplot as plt

img = cv2.VideoCapture('/Users/joshr/Documents/opencv/leader4.mp4')
codec = cv2.cv.CV_FOURCC(*'WRLE')
output = cv2.VideoWriter('/Users/joshr/Documents/opencv/leader4.mov',codec, 24.0, (1920,1080), False)

while(img.isOpened()):
    ret, frame = img.read()
    if ret==True:
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        edges = cv2.Canny(gray,200,300)
    
        
        #Write the grayscale edge images to a new file
        color = cv2.cvtColor(edges, cv2.cv.CV_GRAY2RGB)
        output.write(color)
        cv2.imshow('frame', color)
    
    
        if cv2.waitKey(100) & 0xFF == ord('q'):
            break
    else:
        break
img.release()
output.release
cv2.destroyAllWindows()

