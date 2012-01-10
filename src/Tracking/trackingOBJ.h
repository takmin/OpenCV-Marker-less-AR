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
#ifndef __TRACKING_OBJ__
#define __TRACKING_OBJ__

#include <opencv2/core/core.hpp>

namespace cvar{
namespace tracking{

class trackingOBJ
{
protected:
	trackingOBJ(void);

public:
	virtual ~trackingOBJ(void);

	typedef enum{
		TRACKER_KLT = 1,
		TRACKER_ORB = 2
	}TRACKER_TYPE;

public:
	//! create tracking OBJ
	/*!
	\param[in] type tracker type
	\return pointer of tracker
	*/
	static trackingOBJ* create(TRACKER_TYPE type);

	//! Start Tracking
	/*! 
	\param[in] grayImg first farme in gray scale
	\param[in] pts initial object position: pts[0]:Top Left, pts[1]:Bottom Left, pts[2]:Bottom Right, pts[3]:Top Right
	*/
	virtual void startTracking(const cv::Mat& grayImg, std::vector<cv::Point2f>& pts) = 0;

	//! Continue Tracking
	/*!
	\param[in] grayImg input gray scale image
	\return false if tracking failed
	*/
	virtual bool onTracking(const cv::Mat& grayImg) = 0;

	//! Get current obj position
	/*!
	\return Homography from previous frame
	*/
	virtual cv::Mat& getHomographyMat() = 0;
};

};
};
#endif