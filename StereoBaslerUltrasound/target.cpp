#include "target.h"

#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;


double median(InputArray v)
{
	vector<double> vec;
	v.copyTo(vec);

	size_t n = vec.size() / 2;
	nth_element(vec.begin(), vec.begin() + n, vec.end());
	if (vec.size() % 2 == 1)
		return vec[n];
	else
	{
		double vn = vec[n];
		nth_element(vec.begin(), vec.begin() + n - 1, vec.end());
		return 0.5*(vn + vec[n - 1]);
	}
}


void detect(Mat& im, bool global_th, bool th_im)
{
	Mat bw;
	if (global_th)
		threshold(im, bw, 0, 255, THRESH_BINARY + THRESH_OTSU);
	else
		adaptiveThreshold(im, bw, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 61, 20);

	// Create structuring element, apply morphological opening operation
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(bw, bw, MORPH_OPEN, kernel);

	// Compute contours
	vector<vector<Point>> contours;
	findContours(bw, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);


	// Loop over the contours, approximate the contour with a reduced set of
	// points and save contour if meets certain conditions with its centroid
	// area and perimeter
	double perimeter, area;
	vector<Point> approx;

	vector<vector<Point>> conts;
	Mat2d c;
	Mat1d areas, perimeters;
	Moments M;
	for (auto& cnt : contours)
	{
		// Compute perimeter and approximate contour
		perimeter = arcLength(cnt, true);
		approxPolyDP(cnt, approx, 0.01*perimeter, true);

		// Compute area
		area = contourArea(cnt);

		// Check if approximated contour is stored with its area, perimeter
		// and centroid
		if ((approx.size() > 5) & (area > 30) & (area < 4000))
		{
			conts.push_back(cnt);
			areas.push_back(area);
			perimeters.push_back(perimeter);

			M = moments(cnt);
			c.push_back(Vec2d(M.m10 / M.m00, M.m01 / M.m00));
		}
	}


	// As targets are concentric circles, both circles have the same coordinate
	// center, and distance between these centers should be zero
	Mat1d d(c.rows - 1, 1);
	for (int i = 0; i < c.rows - 1; i++)
		d(i) = norm(c(i) - c(i + 1));

	// Take the first 5 contours with smaller neighboring centers distances,
	// which would be potential circles
	Mat ind;
	sortIdx(d, ind, SORT_EVERY_COLUMN + SORT_ASCENDING);
	ind.convertTo(ind, CV_32F);
	ind = ind.rowRange(0, 5);

	remap(areas, areas, Mat::zeros(ind.size(), ind.type()), ind, INTER_LINEAR);
	remap(perimeters, perimeters, Mat::zeros(ind.size(), ind.type()), ind, INTER_LINEAR);


	// Evaluate circularity criteria. For a circle R = 1
	Mat1d R = 4 * CV_PI * areas / (perimeters.mul(perimeters));

	// Adjust a circle in the contours and save the radius
	Point2f cen;
	float radius;
	Mat1d r(ind.rows, 1);
	vector<vector<Point>> circ(ind.rows);
	for (int i = 0; i < ind.rows; i++)
	{
		circ[i] = conts[static_cast<size_t>(ind.at<float>(i))];
		minEnclosingCircle(circ[i], cen, radius);
		r(i) = radius;
	}


	// To take the three circles between the five contours, area, circularity
	// and the adjusted radius in three of the five contours should have
	// approximately the same values.
	// Subtracting and dividing by the median in each feature measured and
	// adding them, the three smaller values are the three circles.
	Mat1d v = abs(median(areas) - areas) / median(areas) + \
		abs(median(R) - R) / median(R) + abs(median(r) - r) / median(r);

	// Take the three smaller elements of v
	sortIdx(v, ind, SORT_EVERY_COLUMN + SORT_ASCENDING);


	// Draw bouding boxes in detections
	Mat ch[3] = { im, im, im };
	merge(ch, 3, im); // Convert image to three channel

	Rect lim;
	for (int i = 0; i < 3; i++)
	{
		lim = boundingRect(circ[ind.at<int>(i)]);
		rectangle(im, lim, Scalar(0, 255, 0), 3);
	}
	
}