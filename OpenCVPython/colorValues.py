#!/usr/bin/env python

import numpy as np
import cv2
from matplotlib import pyplot as plt

img = cv2.VideoCapture('/Users/joshr/Documents/opencv/1SungraphTokyo15FujiNaraKyoto1_2.mp4')

while(img.isOpened()):
    ret, frame = img.read()
    if ret==True:
        red = np.uint8([[[255,0,0]]])
        hsv_red = cv2.cvtColor(red,cv2.COLOR_BGR2HSV)
        
        print hsv_red
        
        
        
        if cv2.waitKey(100) & 0xFF == ord('q'):
            break
    else:
        break
img.release()
cv2.destroyAllWindows()