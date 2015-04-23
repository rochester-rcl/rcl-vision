#!/usr/bin/env python

import numpy as np
import cv2
from matplotlib import pyplot as plt

img = cv2.VideoCapture('/Users/joshr/Documents/opencv/joyCut3.mp4')
codec = cv2.cv.CV_FOURCC(*'WRLE')
output = cv2.VideoWriter('/Users/joshr/Documents/opencv/tokyoTest.mov',codec, 24.0, (1920,1080), False)

while(img.isOpened()):
    ret, frame = img.read()
    if ret==True:
        n = input("Enter A Quantization Value: ")
        
        indices = np.arange(0,256)
        
        divider = np.linspace(0,255,n+1)[1]
        
        quantiz = np.int0(np.linspace(0,255,n))
        
        color_levels = np.clip(np.int0(indices/divider),0,n-1)
        
        palette = quantiz[color_levels]
        
        im2 = palette[frame]
        
        im2 = cv2.convertScaleAbs(im2)
        
    
        cv2.imshow('frame', im2)
    
    
        if cv2.waitKey(100) & 0xFF == ord('q'):
            break
    else:
        break
img.release()
output.release
cv2.destroyAllWindows()

