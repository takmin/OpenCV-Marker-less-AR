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
#ifndef __UTIL_FUNCTIONS__
#define __UTIL_FUNCTIONS__

#include <opencv2/core/core.hpp>

namespace cvar{

// Debugging functions
CvMat* loadCsvFileAsMatrix(char* filename, int cv_type);
void createMatchingImage(cv::Mat& src_img, cv::Mat& dest_img, std::vector<cv::Point>& src_pts, std::vector<cv::Point>& dest_pts);
void createMatchingImage(cv::Mat& src_img, cv::Mat& dest_img, std::vector<cv::Point2f>& src_pts, std::vector<cv::Point2f>& dest_pts);
void truncatePoint(cv::Size& size, cv::Point2f& pt);	// To approximate the point that protrude a specified size to the size in the vicinity of point
void drawLineContour(cv::Mat& src_img, std::vector<cv::Point2f>& points, cv::Scalar& color, int thickness=1, int lineType=8, int shift=0);	// Draw a straight line connecting the four points
void drawPoints(cv::Mat& src_img, std::vector<cv::Point2f>& points, std::vector<unsigned char>& mask_vec, cv::Scalar& color, int thickness=1, int lineType=8, int shift=0);	// Draw a straight line connecting the four points

};

#endif