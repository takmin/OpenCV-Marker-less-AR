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
#ifndef __KLT_TRACKING_OBJ__
#define __KLT_TRACKING_OBJ__

#include <opencv2/core/core.hpp>
#include "trackingOBJ.h"

namespace cvar{
namespace tracking{

class kltTrackingOBJ : public trackingOBJ
{
public:
	kltTrackingOBJ(void);
	virtual ~kltTrackingOBJ(void);

private:
	cv::Mat prevImg;
	std::vector<cv::Point2f> corners;	// Tracking point
	std::vector<cv::Point2f> object_position;	// Point of the four corners
	std::vector<unsigned char> track_status;	// Mask of the point that you are able to track
	int max_corners;
	double quality_level;
	double min_distance;
	cv::Mat homographyMat;

public:
	//! Start Tracking
	/*! 
	\param[in] grayImg first farme in gray scale
	\param[in] pts initial object position: pts[0]:Top Left, pts[1]:Bottom Left, pts[2]:Bottom Right, pts[3]:Top Right
	*/
	void startTracking(const cv::Mat& grayImg, std::vector<cv::Point2f>& pts);

	//! Continue Tracking
	/*!
	\param[in] grayImg input gray scale image
	\return false if tracking failed
	*/
	bool onTracking(const cv::Mat& grayImg);

	//! Get current obj position
	/*!
	\return Homography from previous frame
	*/
	cv::Mat& getHomographyMat(){return homographyMat;};
};

};
};
#endif
