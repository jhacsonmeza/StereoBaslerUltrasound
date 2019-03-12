# Stereo Camera Basler and Ultrasound acquisition

Acquisition of a stereo system composed of two USB cameras acA1300-200um and an ultrasound machine Biocare iS 20 using the Pylon API and OpenCV.
Stereo system acquisition is made with the Pylon 5 API and images are converted to Mat OpenCV. Ultrasound acquisition is made through [AV.io HD](https://www.epiphan.com/products/avio-hd/) that allows us to handle the ultrasound machine as a webcam, so ultrasound images are acquired with VideoCapture of OpenCV. Finally, images are concatenated a displayed in a window.