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

cv::Mat transPointVecToMat(std::vector<cv::Point2f>& pt_vec, std::vector<unsigned char>& mask);	// Point2f構造体をMat型へ変換
cv::Mat transPointVecToMat(std::vector<cv::Point2f>& pt_vec);
cv::Mat transPointVecToMatHom(std::vector<cv::Point2f>& pt_vec);	// Point2f構造体を斉次座標にしてMat型へ変換
cv::Mat transPointVecToMat2D(std::vector<cv::Point2f>& pt_vec, std::vector<unsigned char>& mask);	// Point2f構造体をMat型へ変換
cv::Mat transPointVecToMat2D(std::vector<cv::Point2f>& pt_vec);
std::vector<cv::Point2f> calcAffineTransformRect(cv::Size& regimg_size, cv::Mat& transMat);
std::vector<cv::Point2f> calcAffineTransformPoints(std::vector<cv::Point2f>& pts_vec, cv::Mat& transMat);
//int checkPointsDistance(std::vector<cv::Point2f> &src_pts, std::vector<cv::Point2f> &dest_pts, double dist_threshold, std::vector<unsigned char>& status);	// アフィン変換後の2つの点の距離が閾値以上のものはstatusを0にする
bool checkRectShape(std::vector<cv::Point2f>& rect_pt);	// 射影変換した矩形の形状をチェックする
cv::Mat createMask(cv::Size img_size, std::vector<cv::Point2f>& pts);	// ptsで指定した4点を囲むマスクを作成する
int checkInsideArea(std::vector<cv::Point2f>& points, std::vector<cv::Point2f>& corner_pts, std::vector<unsigned char>& status);
bool checkPtInsideImage(cv::Size img_size, std::vector<cv::Point2f>& pts);	// ptsが画像領域内にあるかの判定
//double erf(double x);	// error function

void resizeMatChannel(cv::Mat& src_mat, cv::Mat& dest_mat, double val = 0);	// src_matのチャネル数をchannelに変換。空いたチャネルにvalを入れる
template<typename _Tp> void resizeMatChannelType(cv::Mat& src_mat, cv::Mat& dest_mat, double val = 0);	// src_matのチャネル数をchannelに変換。空いたチャネルにvalを入れる
void setChannelValue(cv::Mat& dest_mat, int channel, double val = 0);	// 指定チャネルの値をvalに設定
template <typename _Tp> void setChannelValueType(cv::Mat& dest_mat, int channel, double val = 0);	// 指定チャネルの値をvalに設定

std::vector<cv::Point2f> scalePoints(std::vector<cv::Point2f>& point_vec, double scale);
void decomposeHomography(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation);		// ホモグラフィを回転行列と並進行列へ変更
template<typename _Tp> void decomposeHomographyType(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation);
void decomposeHomography(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation, cv::Point2f marker_center);		// ホモグラフィを回転行列と並進行列へ変更
template<typename _Tp> void decomposeHomographyType(cv::Mat& H_mat, cv::Mat& camera_matrix, cv::Mat& rotation, cv::Mat& translation, cv::Point2f marker_center);

};
#endif