# Stereo Camera Basler and Ultrasound acquisition

Acquisition of a stereo system composed of two USB cameras acA1300-200um and an ultrasound machine Biocare iS 20 using the Pylon API and OpenCV.
Stereo system acquisition is made with the Pylon 5 API and images are converted to Mat OpenCV. Ultrasound acquisition is made through [AV.io HD](https://www.epiphan.com/products/avio-hd/) that allows us to handle the ultrasound machine as a webcam, so ultrasound images are acquired with VideoCapture of OpenCV. Finally, images are concatenated and shown in a window.

## Usage

Path to image storage is defined in root variable, within this folder three subfolders with default names R, L and US are created, one for each device. To acquire an image on each device press C, with D you can delete the last acquired image of each device. To exit or end the acquisition press ESC.