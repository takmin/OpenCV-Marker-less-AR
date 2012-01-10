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
#include "kltTrackingOBJ.h"
#include "commonCvFunctions.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/video/tracking.hpp>

using namespace std;
using namespace cv;
using namespace cvar;
using namespace cvar::tracking;

kltTrackingOBJ::kltTrackingOBJ(void)
{
	max_corners = 80;
	quality_level = 0.1;
	min_distance = 5;
}


kltTrackingOBJ::~kltTrackingOBJ(void)
{
}


void kltTrackingOBJ::startTracking(const Mat& grayImg, vector<Point2f>& pts)
{
	Mat mask = createMask(grayImg.size(), pts);
	goodFeaturesToTrack(grayImg, corners, max_corners, quality_level, min_distance, mask);
	grayImg.copyTo(prevImg);
	object_position = pts;
	float d[] = {1,0,0,0,1,0,0,0,1};
	homographyMat = Mat(3,3,CV_32FC1,d).clone();
	track_status.clear();
}


bool kltTrackingOBJ::onTracking(const Mat& grayImg)
{
	std::vector<cv::Point2f> next_corners;
	std::vector<float> err;
	calcOpticalFlowPyrLK(prevImg, grayImg, corners, next_corners, track_status, err);
		
	int tr_num = 0;
	vector<unsigned char>::iterator status_itr = track_status.begin();
	while(status_itr != track_status.end()){
		if(*status_itr > 0)
			tr_num++;
		status_itr++;
	}
	if(tr_num < 6){
		return false;
	}
	else{
//		homographyMat = findHomography(transPointVecToMat(corners,track_status), transPointVecToMat(next_corners,track_status),CV_RANSAC,5);
		homographyMat = findHomography(Mat(corners), Mat(next_corners),track_status,CV_RANSAC,5);
//		Mat poseMat = findHomography(transPointVecToMat(corners), transPointVecToMat(next_corners), status, CV_RANSAC);

		if(countNonZero(homographyMat)==0){
			return false;
		}
		else{
			vector<Point2f> next_object_position = calcAffineTransformPoints(object_position, homographyMat);
			if(!checkPtInsideImage(prevImg.size(), next_object_position)){
				return false;
			}
			if(!checkRectShape(next_object_position)){
				return false;
			}
			int ret = checkInsideArea(next_corners, next_object_position, track_status);
			if(ret < 6){
				return false;
			}
			grayImg.copyTo(prevImg);
			corners = next_corners;
			object_position = next_object_position;
		}
	}
	return true;
}
