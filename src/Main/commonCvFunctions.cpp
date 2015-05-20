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
#include "commonCvFunctions.h"
#include "orException.h"
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

namespace cvar{

//! Write cv::Mat to binary file
void writeMatBinary(std::ofstream& ofs, const cv::Mat& out_mat)
{
	if(!ofs.is_open()){
		throw new orArgException("not opened");
	}
	int type = out_mat.type();
	ofs.write((const char*)(&out_mat.rows), sizeof(int));
	ofs.write((const char*)(&out_mat.cols), sizeof(int));
	ofs.write((const char*)(&type), sizeof(int));
	ofs.write((const char*)(out_mat.data), out_mat.elemSize() * out_mat.total());
}

//! Read cv::Mat from binary
void readMatBinary(std::ifstream& ifs, cv::Mat& in_mat)
{
	if(!ifs.is_open()){
		throw new orArgException("Not opened");
	}
	
	int rows, cols, type;
	ifs.read((char*)(&rows), sizeof(int));
	ifs.read((char*)(&cols), sizeof(int));
	ifs.read((char*)(&type), sizeof(int));

	in_mat.release();
	in_mat.create(rows, cols, type);
	ifs.read((char*)(in_mat.data), in_mat.elemSize() * in_mat.total());
}


// Convert Point2f structure vector to Mat type
Mat transPointVecToMat(vector<Point2f>& pt_vec)
{
	vector<unsigned char> mask;
	return transPointVecToMat(pt_vec, mask);
}


Mat transPointVecToMat(vector<Point2f>& pt_vec, vector<unsigned char>& mask)
{
	CV_Assert(mask.empty() || mask.size() == pt_vec.size());

	vector<unsigned char>::iterator s_itr;
	vector<Point2f>::iterator pt_itr;

	int i;
	int size;
	if(mask.empty()){
		size = pt_vec.size();
		for(i=0; i< size; i++){
			mask.push_back(1);
		}
	}
	else{
		size = 0;
		s_itr = mask.begin();
		while(s_itr != mask.end()){
			if(*s_itr > 0)
				size++;
			s_itr++;
		}
	}

	Mat retMat(size, 2, CV_64FC1);

	s_itr = mask.begin();
	pt_itr = pt_vec.begin();
	int col = 0;
	while(pt_itr != pt_vec.end()){
		if(*s_itr > 0){
			retMat.at<double>(col,0) = (double)((*pt_itr).x);
			retMat.at<double>(col,1) = (double)((*pt_itr).y);
			col++;
		}
		pt_itr++;
		s_itr++;
	}

	return retMat;
}


// Convert Point2f structure vector to Mat type
Mat transPointVecToMat2D(vector<Point2f>& pt_vec)
{
	vector<unsigned char> mask;
	return transPointVecToMat2D(pt_vec, mask);
}

Mat transPointVecToMat2D(vector<Point2f>& pt_vec, vector<unsigned char>& mask)
{
	CV_Assert(mask.empty() || mask.size() == pt_vec.size());

	vector<unsigned char>::iterator s_itr;
	vector<Point2f>::iterator pt_itr;

	int i;
	int size;
	if(mask.empty()){
		size = pt_vec.size();
		for(i=0; i< size; i++){
			mask.push_back(1);
		}
	}
	else{
		size = 0;
		s_itr = mask.begin();
		while(s_itr != mask.end()){
			if(*s_itr > 0)
				size++;
			s_itr++;
		}
	}

	Mat retMat(size, 1, CV_32FC2);

	s_itr = mask.begin();
	pt_itr = pt_vec.begin();
	int col = 0;
	while(pt_itr != pt_vec.end()){
		if(*s_itr > 0){
			retMat.at<Vec2f>(col,0)[0] = (*pt_itr).x;
			retMat.at<Vec2f>(col,0)[1] = (*pt_itr).y;
//			retMat.at<float>(col,0) = (float)((*pt_itr).x);
//			retMat.at<float>(col,0) = (double)((*pt_itr).y);
			col++;
		}
		pt_itr++;
		s_itr++;
	}

	return retMat;
}

// Convert Point2f structure vector to Mat type in the homogeneous coordinates
Mat transPointVecToMatHom(vector<Point2f>& pt_vec)
{
	int size = pt_vec.size();
	Mat retMat(3, size, CV_64FC1);

	for(int i=0; i<size; i++){
		retMat.at<double>(0,i) = (double)(pt_vec[i].x);
		retMat.at<double>(1,i) = (double)(pt_vec[i].y);
		retMat.at<double>(2,i) = 1.0;
	}

	return retMat;
}


// Order of output is: Top Left, Bottom Left, Bottom Right, Top Right
vector<Point2f> calcAffineTransformRect(cv::Size& img_size, cv::Mat& transMat)
{
	float width = (float)(img_size.width) - 1;
	float height = (float)(img_size.height) - 1;

	Mat src_mat = (Mat_<double>(3,4) << 0,0,width,width,0,height,height,0,1,1,1,1);
//	Mat dest_mat(3,4,CV_64FC1);

	Mat dest_mat = transMat * src_mat;

	vector<Point2f>	ret_vec;

	Point2f pt;
	for(int i=0; i<4; i++){
		pt.x = (float)(dest_mat.at<double>(0,i) / dest_mat.at<double>(2,i));
		pt.y = (float)(dest_mat.at<double>(1,i) / dest_mat.at<double>(2,i));
		ret_vec.push_back(pt);
	}

	return ret_vec;
}


vector<Point2f> calcAffineTransformPoints(vector<Point2f>& pts_vec, cv::Mat& transMat)
{
	vector<Point2f>	ret_vec;
	if(pts_vec.empty())	return ret_vec;

	Mat src_mat = transPointVecToMatHom(pts_vec);

	Mat dest_mat = transMat * src_mat;

	Point2f pt;
	int cols = dest_mat.cols;
	for(int i=0; i<cols; i++){
		pt.x = (float)(dest_mat.at<double>(0,i) / dest_mat.at<double>(2,i));
		pt.y = (float)(dest_mat.at<double>(1,i) / dest_mat.at<double>(2,i));
		ret_vec.push_back(pt);
	}

	return ret_vec;
}


// Check the validity of transformed rectangle shape
// the sign of outer products of each edge vector must be same
bool checkRectShape(vector<Point2f>& rect_pts)
{
	CV_Assert(rect_pts.size()==4);

	bool result_f = true;
	float vec[4][2];
	int i;

	vec[0][0] = rect_pts[1].x - rect_pts[0].x;
	vec[0][1] = rect_pts[1].y - rect_pts[0].y;
	vec[1][0] = rect_pts[2].x - rect_pts[1].x;
	vec[1][1] = rect_pts[2].y - rect_pts[1].y;
	vec[2][0] = rect_pts[3].x - rect_pts[2].x;
	vec[2][1] = rect_pts[3].y - rect_pts[2].y;
	vec[3][0] = rect_pts[0].x - rect_pts[3].x;
	vec[3][1] = rect_pts[0].y - rect_pts[3].y;

	int s;
	float val = vec[3][0] * vec[0][1] - vec[3][1] * vec[0][0];
	if(val > 0) 
		s = 1;
	else
		s = -1;

//	if(vec[3][0] * vec[0][0] + vec[3][1] * vec[0][1] >= 0)
//		result_f = false;

	for(i=0; i<3; i++){
//		if(vec[i][0] * vec[i+1][0] + vec[i][1] * vec[i+1][1] >= 0){
//			result_f = false;
//			break;
//		}
		val = vec[i][0] * vec[i+1][1] - vec[i][1] * vec[i+1][0];
		if( val * s <= 0){
			result_f = false;
			break;
		}
	}

	return result_f;
}


// pts[0]:Top Left, pts[1]:Bottom Left, pts[2]:Bottom Right, pts[3]:Top Right
Mat createMask(Size img_size, vector<Point2f>& pts)
{
	Mat mask(img_size,CV_8UC1);
	mask = 0;

	// ax+by+c=0
	float a[4];
	float b[4];
	float c[4];

	a[0] = pts[3].y - pts[0].y;
	a[1] = pts[2].y - pts[1].y;
	a[2] = pts[1].y - pts[0].y;
	a[3] = pts[2].y - pts[3].y;

	b[0] = pts[0].x - pts[3].x;
	b[1] = pts[1].x - pts[2].x;
	b[2] = pts[0].x - pts[1].x;
	b[3] = pts[3].x - pts[2].x;

	c[0] = pts[0].y * pts[3].x - pts[3].y * pts[0].x;
	c[1] = pts[1].y * pts[2].x - pts[2].y * pts[1].x;
	c[2] = pts[0].y * pts[1].x - pts[1].y * pts[0].x;
	c[3] = pts[3].y * pts[2].x - pts[2].y * pts[3].x;

	float max_x, min_x, max_y, min_y;
	max_x = 0;
	min_x = img_size.width;
	max_y = 0;
	min_y = img_size.height;

	int i;
	for(i=0;i<4;i++){
		if(pts[i].x > max_x)
			max_x = pts[i].x;
		if(pts[i].x < min_x)
			min_x = pts[i].x;
		if(pts[i].y > max_y)
			max_y = pts[i].y;
		if(pts[i].y < min_y)
			min_y = pts[i].y;
	}
	if(max_x >= img_size.width)
		max_x = img_size.width - 1;
	if(max_y >= img_size.height)
		max_y = img_size.height - 1;
	if(min_x < 0)
		min_x = 0;
	if(min_y < 0)
		min_y = 0;
	
	unsigned char *ptr = mask.data;
	int x,y;
	int offset;
	float val[4];
	for(y=min_y; y<=max_y; y++){
		offset = y * img_size.width;
		for(x=min_x; x<=max_x; x++){
			for(i=0; i<4; i++){
				val[i] = a[i]*x + b[i]*y + c[i];
			}
			if(val[0]*val[1] <= 0 && val[2]*val[3] <= 0)
				*(ptr + offset + x)=255;
		}
	}

	return mask;
}


// position
int checkPointsDistance(vector<Point2f> &src_pts, vector<Point2f> &dest_pts, double dist_threshold, vector<unsigned char>& status)
{
	CV_Assert(src_pts.size() == dest_pts.size());
	CV_Assert(src_pts.size() == status.size());

	try{
		double sub_x, sub_y, dist;
		int count = 0;

		int size = src_pts.size();
		for(int i=0; i<size; i++){
			if(status[i] > 0){
				sub_x = dest_pts[i].x - src_pts[i].x;
				sub_y = dest_pts[i].y - src_pts[i].y;
				dist = sqrt(sub_x * sub_x + sub_y * sub_y);
				if(dist < dist_threshold){
					count++;
				}
				else{
					status[i] = 0;
				}
			}
		}
		return count;
	}
	catch(cv::Exception e){
		orCvException orce;
		orce.setFunctionName("imageDB::countAffineInlier()");
		orce.setCvExceptionClass(e);
		throw orce;
	}
	catch(std::exception e2){
		throw e2;
	}
}


// corner_pts[0]:Top Left, corner_pts[1]:Bottom Left, corner_pts[2]:Bottom Right, corner_pts[3]:Top Right
int checkInsideArea(vector<Point2f>& points, vector<Point2f>& corner_pts, vector<unsigned char>& status)
{
	CV_Assert(corner_pts.size() == 4);
	CV_Assert(points.size() == status.size());

	// ax+by+c=0
	float a[4];
	float b[4];
	float c[4];

	a[0] = corner_pts[3].y - corner_pts[0].y;
	a[1] = corner_pts[2].y - corner_pts[1].y;
	a[2] = corner_pts[1].y - corner_pts[0].y;
	a[3] = corner_pts[2].y - corner_pts[3].y;

	b[0] = corner_pts[0].x - corner_pts[3].x;
	b[1] = corner_pts[1].x - corner_pts[2].x;
	b[2] = corner_pts[0].x - corner_pts[1].x;
	b[3] = corner_pts[3].x - corner_pts[2].x;

	c[0] = corner_pts[0].y * corner_pts[3].x - corner_pts[3].y * corner_pts[0].x;
	c[1] = corner_pts[1].y * corner_pts[2].x - corner_pts[2].y * corner_pts[1].x;
	c[2] = corner_pts[0].y * corner_pts[1].x - corner_pts[1].y * corner_pts[0].x;
	c[3] = corner_pts[3].y * corner_pts[2].x - corner_pts[2].y * corner_pts[3].x;

	float max_x, min_x, max_y, min_y;
	max_x = corner_pts[0].x;
	min_x = corner_pts[0].x;
	max_y = corner_pts[0].y;
	min_y = corner_pts[0].y;

	int i;
	for(i=1;i<4;i++){
		if(corner_pts[i].x > max_x)
			max_x = corner_pts[i].x;
		if(corner_pts[i].x < min_x)
			min_x = corner_pts[i].x;
		if(corner_pts[i].y > max_y)
			max_y = corner_pts[i].y;
		if(corner_pts[i].y < min_y)
			min_y = corner_pts[i].y;
	}
	
	float val[4];
	int size = points.size();
	int count = 0;
	for(int j=0;j<size;j++){
		if(status[j] > 0){
			for(i=0; i<4; i++){
				val[i] = a[i] * points[j].x + b[i] * points[j].y + c[i];
			}
			if(val[0]*val[1] <= 0 && val[2]*val[3] <= 0){
				count++;
			}else{
				status[j] = 0;
			}
		}
	}

	return count;
}


// judgment pts is whether all there within the image area
bool checkPtInsideImage(Size img_size, vector<Point2f>& pts)
{
	vector<Point2f>::iterator itr = pts.begin();
	while(itr != pts.end()){
		if(itr->x < 0 || itr->x >= img_size.width || itr->y < 0 || itr->y >= img_size.height){
			return false;
		}
		else{
			itr++;
		}
	}
	return true;
}

/*
//! error function
double erf(double x)
{
	// calculate error function using taylor expansion
	double sum = x;
	double tmp = x;
	double x2 = x * x;
	for(int n=1; n<1000; n++){
		tmp *= (- x2 / n);
		sum += (tmp / (2.0*n+1.0));
	}
	double ret = 2.0 * sum / sqrt(CV_PI);
	return ret;
}
*/

// Convert the number of channels of src_mat to the number of channels of dest_mat. Add the val in the empty channel
void resizeMatChannel(Mat& src_mat, Mat& dest_mat, double val)
{
	CV_Assert(src_mat.depth() == dest_mat.depth());

	int type = src_mat.depth();
	switch(type){
	case CV_8U:
		resizeMatChannelType<unsigned char>(src_mat, dest_mat, val);
		break;
	case CV_8S:
		resizeMatChannelType<char>(src_mat, dest_mat, val);
		break;
	case CV_16S:
		resizeMatChannelType<short>(src_mat, dest_mat, val);
		break;
	case CV_16U:
		resizeMatChannelType<unsigned short>(src_mat, dest_mat, val);
		break;
	case CV_32S:
		resizeMatChannelType<long>(src_mat, dest_mat, val);
		break;
	case CV_32F:
		resizeMatChannelType<float>(src_mat, dest_mat, val);
		break;
	case CV_64F:
		resizeMatChannelType<double>(src_mat, dest_mat, val);
		break;
	}
}


template<typename _Tp> void resizeMatChannelType(Mat& src_mat, Mat& dest_mat, double val)
{
	CV_Assert(!src_mat.empty() && !dest_mat.empty());
	CV_Assert(src_mat.cols == dest_mat.cols && src_mat.rows == dest_mat.rows);

	int src_channel = src_mat.channels();
	int dest_channel = dest_mat.channels();
	
	int diff_channel = dest_channel - src_channel;
	int byte_size = src_mat.elemSize1();
	int src_block = src_mat.elemSize();
	int dest_block = dest_mat.elemSize();
	int mat_size = src_mat.rows * src_mat.cols;

	int copy_channel = (diff_channel < 0) ? dest_channel : src_channel;
	int copy_byte = copy_channel * byte_size;

	int type = src_mat.depth();

	unsigned char* src_data = src_mat.data;
	unsigned char* dest_data = dest_mat.data;
	unsigned char* dest_ptr;

	int i,j;
	for(i=0; i<mat_size; i++){
		memcpy(dest_data+i*dest_block, src_data+i*src_block, copy_byte);
		for(j=0;j<diff_channel;j++){
			dest_ptr = dest_data + i*dest_block + copy_byte + j*byte_size;
			*((_Tp*)dest_ptr) = (_Tp)val;
		}
	}
}

// Set the value of the specified channel to val
void setChannelValue(Mat& dest_mat, int channel, double val)
{
	int type = dest_mat.depth();
	switch(type){
	case CV_8U:
		setChannelValueType<unsigned char>(dest_mat, channel, val);
		break;
	case CV_8S:
		setChannelValueType<char>(dest_mat, channel, val);
		break;
	case CV_16S:
		setChannelValueType<short>(dest_mat, channel, val);
		break;
	case CV_16U:
		setChannelValueType<unsigned short>(dest_mat, channel, val);
		break;
	case CV_32S:
		setChannelValueType<long>(dest_mat, channel, val);
		break;
	case CV_32F:
		setChannelValueType<float>(dest_mat, channel, val);
		break;
	case CV_64F:
		setChannelValueType<double>(dest_mat, channel, val);
		break;
	}
}


template<typename _Tp> void setChannelValueType(Mat& dest_mat, int channel, double val)
{
	CV_Assert(!dest_mat.empty());
	CV_Assert(channel < dest_mat.channels());

	int dest_channel = dest_mat.channels();
	int mat_size = dest_mat.rows * dest_mat.cols;

	_Tp* dest_data = (_Tp*)(dest_mat.data);

	int i;
	for(i=0; i<mat_size; i++){
		*(dest_data + i*dest_channel + channel) = (_Tp)val;
	}
}


vector<Point2f> scalePoints(vector<Point2f>& point_vec, double scale)
{
	vector<Point2f> ret_vec;

	vector<Point2f>::iterator itr = point_vec.begin();
	while(itr != point_vec.end()){
		ret_vec.push_back(*itr * scale);
		itr++;
	}
	return ret_vec;
}


template<typename _Tp> void decomposeHomographyType(Mat& H_mat, Mat& camera_matrix, Mat& rotation, Mat& translation, Point2f marker_center)
{
	try{
		CV_Assert(H_mat.type() == camera_matrix.type());
		CV_Assert(H_mat.cols == 3 && H_mat.rows == 3 && camera_matrix.cols == 3 && camera_matrix.rows == 3);

		int i,j;
		Mat Amarker = camera_matrix.clone();	// Matrix to convert the world coordinates to the scale of the image coordinates
		Amarker.at<_Tp>(0,2) = marker_center.x;
		Amarker.at<_Tp>(1,2) = marker_center.y;

		Mat invA = camera_matrix.inv();
		Mat tmpM = invA * H_mat * Amarker;
	//	Mat tmpM = invA * H_mat;

		Mat Rcol[2];
		Mat rMat[3];

		double lambda[2];
		for(i=0; i<2; i++){
//			Rcol[i].create(3,1,CV_32FC1);
			Rcol[i] = tmpM.col(i);
			lambda[i] = 1.0 / cv::norm(Rcol[i], NORM_L2);
			rMat[i] = Rcol[i] * lambda[i];
//			lambda[i] /= camera_matrix.at<_Tp>(i, i);
//			printf("lambda %d: %f\n", i, lambda[i]);
		}

		rMat[2] = rMat[0].cross(rMat[1]);
//		printf("rotation & translation:\n");
		rotation.create(3,3,tmpM.type());
		translation.create(3,1,tmpM.type());
		for(j=0; j<3; j++){
			for(i=0; i<3; i++){
				rotation.at<_Tp>(i,j) = rMat[j].at<_Tp>(i,0);
//				printf("%f,",rotation.at<_Tp>(i,j));
			}
			translation.at<_Tp>(j,0) = tmpM.at<_Tp>(j,2) * lambda[0];
//			printf("\t%f\n", translation.at<_Tp>(j,0));
		}
	}
	catch(std::exception& e){
		throw e;
	}
}


// Change homography to the rotation matrix and translation matrix
// H_mat: homography matrix
void decomposeHomography(Mat& H_mat, Mat& camera_matrix, Mat& rotation, Mat& translation, Point2f marker_center)
{
	try{
//		CV_Assert(rotation.cols == 3 && rotation.rows == 3 && translation.cols == 1 && translation.rows == 3);
		CV_Assert(H_mat.type() == CV_32FC1 || H_mat.type() == CV_64FC1);

		if(H_mat.type() == CV_32FC1){
			decomposeHomographyType<float>(H_mat, camera_matrix, rotation, translation, marker_center);
		}
		else if(H_mat.type() == CV_64FC1){
			decomposeHomographyType<double>(H_mat, camera_matrix, rotation, translation, marker_center);
		}
	}
	catch(std::exception& e){
		throw e;
	}
}


template<typename _Tp> void decomposeHomographyType(Mat& H_mat, Mat& camera_matrix, Mat& rotation, Mat& translation)
{
	Point2f marker_center(camera_matrix.at<_Tp>(0,2), camera_matrix.at<_Tp>(1,2));
	try{
		decomposeHomographyType<_Tp>(H_mat, camera_matrix, rotation, translation, marker_center);
	}
	catch(std::exception& e){
		throw e;
	}
}


// Calculate Rotation and Translation Matrix from Homography
// H_mat: Homography Matrix
void decomposeHomography(Mat& H_mat, Mat& camera_matrix, Mat& rotation, Mat& translation)
{
	try{
		CV_Assert(H_mat.type() == CV_32FC1 || H_mat.type() == CV_64FC1);

		if(H_mat.type() == CV_32FC1){
			decomposeHomographyType<float>(H_mat, camera_matrix, rotation, translation);
		}
		else if(H_mat.type() == CV_64FC1){
			decomposeHomographyType<double>(H_mat, camera_matrix, rotation, translation);
		}
	}
	catch(std::exception& e){
		throw e;
	}
}
};