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
#include "utilFunctions.h"
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

namespace cvar{

// You want to create a matrix from a CSV file
CvMat* loadCsvFileAsMatrix(char* filename, int cv_type)
{
	FILE* fp;

	assert(cv_type == CV_32SC1 || cv_type == CV_8UC1 || cv_type == CV_32FC1 || cv_type == CV_64FC1);

	if((fp = fopen(filename, "r"))!=NULL){
		char line[512];
		int rows, cols;
		int max = 0;
		int i;
		int step;
		CvMat* retMat = NULL;
		rows = 0;
		while(fscanf(fp, "%s", line)!=EOF){
			cols = 0;
			for(i=0;line[i]!='\0';i++){
				if(line[i]==',')	cols++;
			}
			if(max < cols)	max = cols;
			rows++;
		}
		retMat = cvCreateMat(rows,cols+1,cv_type);

		if(cv_type == CV_32SC1){
			step = retMat->step / sizeof(int);
		}else if(cv_type == CV_8UC1){
			step = retMat->step / sizeof(char);
		}else if(cv_type == CV_32FC1){
			step = retMat->step / sizeof(float);
		}
		else{
			step = retMat->step / sizeof(double);
		}

		rewind(fp);
		rows = 0;
		cols = 0;
		i = 0;

		int c;
		while((c = fgetc(fp))!=EOF){
			if(c==','){
				line[i] = '\0';
				if(cv_type == CV_32SC1)
					retMat->data.i[rows*step + cols] = atoi(line);
				else if(cv_type == CV_8UC1)
					retMat->data.ptr[rows*step + cols] = atoi(line);
				else if(cv_type == CV_32FC1)
					retMat->data.fl[rows*step + cols] = (float)atof(line);
				else
					retMat->data.db[rows*step + cols] = atof(line);
				i = 0;
				cols++;
			}else if(c=='\n'){
				line[i] = '\0';
				if(cv_type == CV_32SC1)
					retMat->data.i[rows*step + cols] = atoi(line);
				else if(cv_type == CV_8UC1)
					retMat->data.ptr[rows*step + cols] = atoi(line);
				else if(cv_type == CV_32FC1)
					retMat->data.fl[rows*step + cols] = (float)atof(line);
				else
					retMat->data.db[rows*step + cols] = atof(line);
				i = 0;
				cols = 0;
				rows++;
			}else if(isdigit(c) || c=='.' || c=='-'){
				line[i] = c;
				i++;
			}
		}

		fclose(fp);
		return retMat;
	}else{
		return NULL;
	}
}


void createMatchingImage(Mat& src_img, Mat& dest_img, vector<Point2f>& src_pts, vector<Point2f>& dest_pts)
{
	assert(src_pts.size() == dest_pts.size());
	vector<Point> src_pts_i, dest_pts_i;

	int size = src_pts.size();
	for(int i=0; i<size; i++){
		src_pts_i.push_back((Point)src_pts[i]);
		dest_pts_i.push_back((Point)dest_pts[i]);
	}

	createMatchingImage(src_img, dest_img, src_pts_i, dest_pts_i);
}


void createMatchingImage(Mat& src_img, Mat& dest_img, vector<Point>& src_pts, vector<Point>& dest_pts)
{
	assert(src_pts.size() == dest_pts.size());
	int width, height;
	if(src_img.cols > dest_img.cols)
		width = src_img.cols;
	else
		width = dest_img.cols;
	height = src_img.rows + dest_img.rows;
	Mat resultimg(height, width, CV_8UC1);
	resultimg = Scalar(0);
	Mat tmpMat(resultimg,Rect(0,0, src_img.cols, src_img.rows));
	src_img.copyTo(tmpMat);
	Mat tmpMat2(resultimg,Rect(0, src_img.rows, dest_img.cols, dest_img.rows));
	dest_img.copyTo(tmpMat2);

	int size = src_pts.size();
	Point pt;
	pt.x = 0;
	pt.y = src_img.rows;
	for(int i=0;i<size;i++){
		line(resultimg, src_pts[i], dest_pts[i]+pt,Scalar(255));
	}

	namedWindow("matching",CV_WINDOW_AUTOSIZE);
	imshow("matching", resultimg);
	waitKey(0);
}


// To approximate the point that protrude a specified size to the size in the vicinity of point
void truncatePoint(cv::Size& size, cv::Point2f& pt)
{
	if(pt.x < 0)
		pt.x = 0;
	else if(pt.x >= size.width)
		pt.x = size.width - 1;
	if(pt.y < 0)
		pt.y = 0;
	else if(pt.y >= size.height)
		pt.y = size.height - 1;
}


// Draw a straight line connecting the four points
void drawLineContour(Mat& src_img, vector<Point2f>& points, Scalar& color, int thickness, int lineType, int shift)
{
	int pt_num = points.size();
	assert(pt_num > 1);

	Point2f b_pt, e_pt;
	Size img_size = src_img.size();

	b_pt = points[pt_num-1];
	e_pt = points[0];
	truncatePoint(img_size, b_pt);
	truncatePoint(img_size, e_pt);
	line(src_img, b_pt, e_pt, color, thickness, lineType, shift);

	for(int i=1; i<pt_num; i++){	
		b_pt = points[i-1];
		e_pt = points[i];
		truncatePoint(img_size, b_pt);
		truncatePoint(img_size, e_pt);
		line(src_img,b_pt, e_pt, color, thickness, lineType, shift);
	}

}


void drawPoints(Mat& frame, vector<Point2f>& corners, vector<unsigned char>& mask_vec, Scalar& color, int thickness, int lineType, int shift)
{
	assert(mask_vec.empty() || corners.size() == mask_vec.size());

	int size = corners.size();
	for(int i=0; i<size; i++){
		if(mask_vec.empty() || mask_vec[i] > 0){
			circle(frame, corners[i], thickness, color, lineType, shift);
		}
	}
}
};