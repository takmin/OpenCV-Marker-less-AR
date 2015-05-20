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
#ifndef __CAMERA_CALIBRATION__
#define __CAMERA_CALIBRATION__

#include <vector>
#include <opencv2/core/core.hpp>

namespace cvar{

class cameraCalibration
{
public:
	cameraCalibration(void);
	~cameraCalibration(void);

public:
	cv::Mat camera_matrix;
	cv::Mat distortion;
	std::vector<cv::Mat> rotation;
	std::vector<cv::Mat> translation;
	std::vector<cv::Mat>	checker_image_list;

protected:
	int max_img_num;	/* The maximum number of images */
	int pat_row;		/* Number of rows in the pattern */
	int pat_col;		/* Number of columns in the pattern */
	float chess_size;	/* Size of each side pattern 1 Mass [mm] */

public:
	void setMaxImageNum(int num);
	void setBoardColsAndRows(int r, int c);
	void setChessSize(float size);
	bool addCheckerImage(cv::Mat& img);
	void releaseCheckerImage();
	bool doCalibration();
	void saveCameraMatrix(const std::string& filename);
	void writeCameraMatrix(cv::FileStorage& cvfs, const std::string& name);
};

};
#endif