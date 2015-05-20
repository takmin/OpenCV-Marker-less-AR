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
#ifndef __COMMON_CV_FUNCTIONS__
#define __COMMON_CV_FUNCTIONS__

#include <opencv2/core/core.hpp>
#include <fstream>

namespace cvar{

//! Write cv::Mat as binary
/*!
\param[out] ofs output file stream
\param[in] out_mat mat to save
*/
void writeMatBinary(std::ofstream& ofs, const cv::Mat& out_mat);

//! Read cv::Mat from binary
/*!
\param[in] ifs input file stream
\param[out] in_mat mat to load
*/
void readMatBinary(std::ifstream& ifs, cv::Mat& in_mat);

cv::Mat transPointVecToMat(std::vector<cv::Point2f>& pt_vec, std::vector<unsigned char>& mask);	// Convert Point2f structure to Mat type
cv::Mat transPointVecToMat(std::vector<cv::Point2f>& pt_vec);
cv::Mat transPointVecToMatHom(std::vector<cv::Point2f>& pt_vec);	// Convert Point2f structure to Mat type in the homogeneous coordinates
cv::Mat transPointVecToMat2D(std::vector<cv::Point2f>& pt_vec, std::vector<unsigned char>& mask);	// Convert Point2f structure to Mat type
cv::Mat transPointVecToMat2D(std::vector<cv::Point2f>& pt_vec);
std::vector<cv::Point2f> calcAffineTransformRect(cv::Size& regimg_size, cv::Mat& transMat);
std::vector<cv::Point2f> calcAffineTransformPoints(std::vector<cv::Point2f>& pts_vec, cv::Mat& transMat);
//int checkPointsDistance(std::vector<cv::Point2f> &src_pts, std::vector<cv::Point2f> &dest_pts, double dist_threshold, std::vector<unsigned char>& status);	// The distance between two points after the affine transformation is not less than the threshold value a status to 0
bool checkRectShape(std::vector<cv::Point2f>& rect_pt);	// Check a rectangular shape that projective transformation
cv::Mat createMask(cv::Size img_size, std::vector<cv::Point2f>& pts);	// Creating a mask surrounding the four points specified in the pts
int checkInsideArea(std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& corner_pts, std::vector<unsigned char>& status);
bool checkPtInsideImage(cv::Size img_size, std::vector<cv::Point2f>& pts);	// judgment pts is there within the image area
//double erf(double x);	// error function

void resizeMatChannel(cv::Mat& src_mat, cv::Mat& dest_mat, double val = 0);	// Convert the number of channels to channel of src_mat. Add the val in the empty channel
template<typename _Tp> void resizeMatChannelType(cv::Mat& src_mat, cv::Mat& dest_mat, double val = 0);	// Convert the number of channels to channel of src_mat. Add the val in the empty channel
void setChannelValue(cv::Mat& dest_mat, int channel, double val = 0);	// Set the value of the specified channel to val
template <typename _Tp> void setChannelValueType(cv::Mat& dest_mat, int channel, double val = 0);	// Set the value of the specified channel to val

std::vector<cv::Point2f> scalePoints(std::vector<cv::Point2f>& point_vec, double scale);
void decomposeHomography(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation);		// Change homography to the rotation matrix and translation matrix
template<typename _Tp> void decomposeHomographyType(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation);
void decomposeHomography(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation, cv::Point2f marker_center);		// Change homography to the rotation matrix and translation matrix
template<typename _Tp> void decomposeHomographyType(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation, cv::Point2f marker_center);

};
#endif