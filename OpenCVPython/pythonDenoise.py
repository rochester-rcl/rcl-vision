import numpy as np
import cv2
import os
import fnmatch
import argparse
import subprocess
import time

True = True

cv2.setUseOptimized(True)

parser = argparse.ArgumentParser(description="Run a script to correct image noise from RetroScan and create an MP4")#Setting up our Argument Parser
parser.add_argument('-d', '--directory', help="Absolute Path of Directory You Want to Scan", required="true", type=str)#Adding a directory argument
parser.add_argument('-o', '--output', help="Path of Output File", required="true")#Adding an output argument
args = vars(parser.parse_args())#Get some variables from parse_args dictionary (directory, output)
directory = args['directory']
output = args['output']


def imageDenoise():
    for dirname, dirnames, filenames in os.walk(directory):
        for file in filenames:
            if '.tif' in file:
            
                filePath =  os.path.abspath(os.path.join(dirname, file))
                
                global outputDirectory
            
                outputDirectory = output + file
                
                workingMessage = 'Successfully read image file, denoising ...'
                
                completeMessage = outputDirectory + ' written'
            
                img = cv2.imread(filePath)
                
                if img is not None:
                    
                    print workingMessage
                    
                    start_time = time.time()

                    dst = cv2.fastNlMeansDenoisingColored(img,None,3,10,7,21)

                    cv2.imwrite(outputDirectory, dst)

                    print completeMessage

                    print("%s seconds" % (time.time() - start_time))
    
    print 'Starting ffmpeg'

def outputInfo():
    
    os.chdir(output)
    global newDirectory
    global newOutput
    strippedDirectory = outputDirectory[:-9]
    inputExtension = '%5d.tif'
    outputExtension = 'access.mp4'
    newDirectory = strippedDirectory + inputExtension
    newOutput = strippedDirectory + outputExtension

    return newDirectory, newOutput

def ffmpegTranscode():
    subprocess.call(['ffmpeg', '-i', newDirectory, '-c:v', 'libx264', '-crf', '18', '-r', '18', '-an', '-pix_fmt', 'yuv420p', newOutput ])


#The program


imageDenoise()

outputInfo()

ffmpegTranscode()






#stitch w/ffmpeg using command line from os library 
#remove temp directory
