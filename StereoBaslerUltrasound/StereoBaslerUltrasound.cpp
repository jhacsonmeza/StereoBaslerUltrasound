#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCameraArray.h>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <filesystem>

using namespace Pylon;
using namespace cv;
using namespace std;
using namespace std::filesystem;


int main(int argc, char* argv[])
{
	// Root path to store images
	path root = "F:\\StereoBaslerUltrasound\\acquisition\\";

	// If there are no paths to each source, create them
	if (!is_directory(root / "L"))
	{
		if (!create_directory(root / "L"))
			return -1;
	}

	if (!is_directory(root / "R"))
	{
		if (!create_directory(root / "R"))
			return -1;
	}

	if (!is_directory(root / "US"))
	{
		if (!create_directory(root / "US"))
			return -1;
	}


	// The exit code of the sample application.
	int exitCode = 0;

	// Before using any pylon methods, the pylon runtime must be initialized. 
	PylonInitialize();

	try
	{
		// Get the transport layer factory.
		CTlFactory& tlFactory = CTlFactory::GetInstance();

		// Get all attached devices and exit application if no device is found.
		DeviceInfoList_t devices;
		if (tlFactory.EnumerateDevices(devices) == 0)
			throw RUNTIME_EXCEPTION("No camera present.");

		// Create an array of instant cameras for the found devices and avoid exceeding a maximum number of devices.
		CBaslerUsbInstantCameraArray cameras(2); //Equivalent to: CInstantCameraArray cameras(2); but for usb cameras

		// Create and attach all Pylon Devices.
		for (size_t i = 0; i < cameras.GetSize(); ++i)
		{
			cameras[i].Attach(tlFactory.CreateDevice(devices[i]));

			cameras[i].Open();
			cameras[i].ExposureAuto.SetValue(Basler_UsbCameraParams::ExposureAuto_Off);
			cameras[i].ExposureTime.SetValue(20000);
			cameras[i].Close();

			// Print the model name of the camera.
			cout << "Using device " << cameras[i].GetDeviceInfo().GetModelName() << endl;
		}
		cout << endl;


		// Create and open the default camera
		VideoCapture cap(0);
		if (!cap.isOpened())  // check if we succeeded
			return -1;
		cout << endl << endl;


		// Variables to use
		int iR, iL; // Index of cameras
		CGrabResultPtr ptrGrabResultL, ptrGrabResultR; // Store retrieve result as pointer of both cameras
		string DeviceSerNum[2]; // Store device serial number

		int cntImagesNum = -1; // Initialize counter of images to store them with index number
		string strFileName; // Filename string of images to store

		CPylonImage imgLeft, imgRight; // pylon images
		Mat imL, imR, imLrs, imRrs, frame, cat; // OpenCV matrices
		vector<Mat> matrices; // vector of Mat for image concatenation
		CImageFormatConverter formatConverter;


		// Check which camera is R and which L to assign the correct camera index
		for (int i = 0; i < cameras.GetSize(); i++)
		{
			DeviceSerNum[i] = cameras[i].GetDeviceInfo().GetSerialNumber();

			if (DeviceSerNum[i] == "22151646")
				iR = i;
			else if (DeviceSerNum[i] == "21953150")
				iL = i;
		}


		// Set up format convert to store pylon image as grayscale
		formatConverter.OutputPixelFormat = PixelType_Mono8;
		// Set up window to show acquisition
		namedWindow("Concatenation", WINDOW_NORMAL); resizeWindow("Concatenation", 640 * 3, 512);
		// Start grabbing cameras
		cameras.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByUser);


		while (cameras.IsGrabbing())
		{
			// Basler frame capture
			cameras[iR].RetrieveResult(5000, ptrGrabResultL, TimeoutHandling_ThrowException);
			cameras[iL].RetrieveResult(5000, ptrGrabResultR, TimeoutHandling_ThrowException);

			// US frame capture
			cap.read(frame);
			cvtColor(frame, frame, COLOR_BGR2GRAY);

			// If the image was grabbed successfully.
			if (ptrGrabResultL->GrabSucceeded() && ptrGrabResultR->GrabSucceeded() && !frame.empty())
			{
				// Convet left image to pylon image and then to Mat
				formatConverter.Convert(imgLeft, ptrGrabResultL);
				imL = Mat(ptrGrabResultL->GetHeight(), ptrGrabResultL->GetWidth(), CV_8UC1, (uint8_t *)imgLeft.GetBuffer());

				// Convet right image to pylon image and then to Mat
				formatConverter.Convert(imgRight, ptrGrabResultR);
				imR = Mat(ptrGrabResultR->GetHeight(), ptrGrabResultR->GetWidth(), CV_8UC1, (uint8_t *)imgRight.GetBuffer());

				// Resize basler images to US frame
				resize(imL, imLrs, Size(frame.cols, frame.rows));
				resize(imR, imRrs, Size(frame.cols, frame.rows));

				// Concatenate three images
				matrices = { imLrs, imRrs, frame };
				hconcat(matrices, cat);

				// show images
				imshow("Concatenation", cat);
				char c = waitKey(1);

				if (c == 27)
					break;
				else if (c == 'c')
				{
					cntImagesNum++;

					strFileName = root.string() + "L\\left" + to_string(cntImagesNum) + ".jpg";
					imwrite(strFileName, imL);

					strFileName = root.string() + "R\\right" + to_string(cntImagesNum) + ".jpg";
					imwrite(strFileName, imR);

					strFileName = root.string() + "US\\US" + to_string(cntImagesNum) + ".jpg";
					imwrite(strFileName, frame);

					cout << "Images with index " << cntImagesNum << " has been collected" << endl;
				}


			}
		}

		destroyAllWindows();

	}
	catch (const GenericException &e)
	{
		// Error handling
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}

	// Comment the following two lines to disable waiting on exit.
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	// Releases all pylon resources. 
	PylonTerminate();

	return exitCode;
}

