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

def imageDenoise(): #Find all the files in the working directory -- in this case looking for .tif extension only
    for dirname, dirnames, filenames in os.walk(directory):
        for file in filenames:
            if '.tif' in file:
                
                filePath =  os.path.abspath(os.path.join(dirname, file))
            
                outputDirectory = output + file
                
                workingMessage = 'Successfully read image file, denoising ...'
                
                completeMessage = outputDirectory + ' written'
            
                img = cv2.imread(filePath)
                
                if img is not None:
                    #Print a message once the image matrix is loaded
                    print workingMessage
                    #track how long it takes to denoise
                    start_time = time.time()
                    #Input matrix, output matrix, strength for luminance component, strength for chrominance component, template patch in pixels, window size for weighted average of pixels
                    dst = cv2.fastNlMeansDenoisingColored(img,None,3,10,7,21)
                    #write the denoised file and report on it
                    cv2.imwrite(outputDirectory, dst)

                    print completeMessage

                    print("%s seconds" % (time.time() - start_time))

    return outputDirectory
    
    print 'Starting ffmpeg'
#Handler for input and output directory arguments being passed to ffmpegTranscode function
def outputInfo(outputDirectory):
    
    os.chdir(output)
    strippedDirectory = outputDirectory[:-9]
    inputExtension = '%5d.tif'
    outputExtension = 'access.mp4'
    newDirectory = strippedDirectory + inputExtension
    newOutput = strippedDirectory + outputExtension

    return (newDirectory, newOutput)
#Transcode the sequence using libx264
def ffmpegTranscode(newDirectory, newOutput):

    ffmpegCommand = ['ffmpeg', '-i', newDirectory , '-c:v', 'libx264', '-crf', '18', '-r', '18', '-an', '-pix_fmt', 'yuv420p', newOutput ]

    subprocess.call(ffmpegCommand)


#The program


denoiser = imageDenoise()

newDirectory, newOutput = outputInfo(denoiser)

ffmpegTranscode(newDirectory, newOutput)






#stitch w/ffmpeg using command line from os library 
#remove temp directory
