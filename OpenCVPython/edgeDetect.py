#!/usr/bin/env python

import numpy as np
import cv2
from matplotlib import pyplot as plt

img = cv2.VideoCapture("/Users/joshr/Documents/opencv/irImage/Drone.mp4")
#edges = cv2.Canny(img,100,200)


cv2.imshow('edges', img)
key = cv2.waitKey(0)
if key == 27:
    cv2.destroyAllWindows()


