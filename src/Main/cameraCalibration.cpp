/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//
// Copyright (C) 2012, Takuya MINAGAWA.
// Third party copyrights are property of their respective owners.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//M*/

#include "cameraCalibration.h"

#include <stdio.h>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "commonCvFunctions.h"

using namespace std;
using namespace cv;
using namespace cvar;


cameraCalibration::cameraCalibration(void)
{
	max_img_num = 25;
	pat_row = 7;
	pat_col = 10;
	chess_size = 23.0;
	camera_matrix.create(3, 3, CV_32FC1);
	distortion.create(1, 5, CV_32FC1);
}

cameraCalibration::~cameraCalibration(void)
{
}

void cameraCalibration::setMaxImageNum(int num)
{
	max_img_num = num;
}

void cameraCalibration::setBoardColsAndRows(int r, int c)
{
	pat_row = r;
	pat_col = c;
}


void cameraCalibration::setChessSize(float size)
{
	chess_size = size;
}


bool cameraCalibration::addCheckerImage(Mat& img)
{
	if(checker_image_list.size() >= max_img_num)
		return false;
	else
		checker_image_list.push_back(img);

	return true;
}


void cameraCalibration::releaseCheckerImage()
{
	checker_image_list.clear();
}


bool cameraCalibration::doCalibration()
{
	int i, j, k;
	bool found;
	int image_num = checker_image_list.size();
//	int pat_size = pat_row * pat_col;
//	int all_points = image_num * pat_size;
	if(image_num < 3){
		cout << "please add checkker pattern image!" << endl;
		return false;
	}
//	int *p_count = new int[image_num];

	rotation.clear();
	translation.clear();

	Size pattern_size(pat_col,pat_row);
//	Point3f *objects = new Point3f[all_points];
//	Point2f *corners = new Point2f[all_points];
	Point3f obj;
	vector<Point3f> objects;
	vector<vector<Point3f>> object_points;
	// 3D set of spatial coordinates
	for (j = 0; j < pat_row; j++) {
		for (k = 0; k < pat_col; k++) {
			obj.x = j * chess_size;
			obj.y = k * chess_size;
			obj.z = 0.0;
			objects.push_back(obj);
		}
	}

	vector<Point2f>	corners;
	vector<vector<Point2f>> image_points;

	int found_num = 0;
	cvNamedWindow ("Calibration", CV_WINDOW_AUTOSIZE);
	vector<Mat>::iterator img_itr = checker_image_list.begin();
	i = 0;
	while (img_itr != checker_image_list.end()) {
		// Corner detection of chess board (calibration pattern)
		found = cv::findChessboardCorners(*img_itr, pattern_size, corners);

		cout << i << "...";
		if (found) {
			cout << "ok" << endl;
			found_num++;
		}
		else {
			cout << "fail" << endl;
	    }
		// Fixed a corner position in the sub-pixel accuracy, drawing
		Mat src_gray(img_itr->size(), CV_8UC1, 1);
		cvtColor(*img_itr, src_gray, CV_BGR2GRAY);
//		cvCvtColor (src_img[i], src_gray, CV_BGR2GRAY);
		cornerSubPix(src_gray, corners, Size(3,3), Size(-1,-1), TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
//		cvFindCornerSubPix (src_gray, &corners[i * PAT_SIZE], corner_count,
//                       cvSize (3, 3), cvSize (-1, -1), cvTermCriteria (CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));
		drawChessboardCorners(*img_itr, pattern_size, transPointVecToMat2D(corners), found);
//		cvDrawChessboardCorners (src_img[i], pattern_size, &corners[i * PAT_SIZE], corner_count, found);
//		p_count[i] = corner_count;
		if(found){
			image_points.push_back(corners);
			object_points.push_back(objects);
		}
		corners.clear();

		cvShowImage ("Calibration", &((IplImage)(*img_itr)));
		cvWaitKey (0);
		i++;
		img_itr++;
	}
	cvDestroyWindow ("Calibration");

	if (found_num < 3){
		return false;
	}

//  cvInitMatHeader (&image_points, ALL_POINTS, 1, CV_32FC2, corners);
//  cvInitMatHeader (&point_counts, IMAGE_NUM, 1, CV_32SC1, p_count);

	// Internal parameters, distortion factor, the estimation of the external parameters
//  cvCalibrateCamera2 (&object_points, &image_points, &point_counts, cvSize (640, 480), intrinsic, distortion);
	calibrateCamera(object_points, image_points, checker_image_list[0].size(), camera_matrix, distortion, rotation, translation);

/*  CvMat sub_image_points, sub_object_points;
  int base = 0;
  cvGetRows (&image_points, &sub_image_points, base * PAT_SIZE, (base + 1) * PAT_SIZE);
  cvGetRows (&object_points, &sub_object_points, base * PAT_SIZE, (base + 1) * PAT_SIZE);
  cvFindExtrinsicCameraParams2 (&sub_object_points, &sub_image_points, intrinsic, distortion, rotation, translation);
  
  // (7) Export to XML file
  CvFileStorage *fs;
  fs = cvOpenFileStorage ("camera.xml", 0, CV_STORAGE_WRITE);
  cvWrite (fs, "intrinsic", intrinsic);
  cvWrite (fs, "rotation", rotation);
  cvWrite (fs, "translation", translation);
  cvWrite (fs, "distortion", distortion);
  cvReleaseFileStorage (&fs);
  */
	return true;
}

void cameraCalibration::saveCameraMatrix(const string& filename)
{
	FileStorage fs(filename, FileStorage::WRITE);
	writeCameraMatrix(fs, "camera_matrix");
}


void cameraCalibration::writeCameraMatrix(FileStorage& cvfs, const string& name)
{
	cvfs << name << camera_matrix;
}