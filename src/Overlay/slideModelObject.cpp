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
#include "slideModelObject.h"
#include <fstream>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "orException.h"
#include "commonCvFunctions.h"

using namespace std;
using namespace cv;
using namespace cvar;
using namespace cvar::overlay;

//// misc functions ////
void printMatrix(GLfloat* matrix){
	for(int y=0; y<4; y++){
		for(int x=0; x<4; x++){
			printf("%f,", matrix[4*x + y]);
		}
		printf("\n");
	}
	printf("\n");
}

///////////////////////

slideModelObject::slideModelObject(void)
{
	status = UNINIT;
	size_fix_mode = NON_FIX;
	spread_interval = 10;
	morph_interval = 5;
	fore_dist = 0.1;
}


slideModelObject::~slideModelObject(void)
{
	release();
}


void slideModelObject::init()
{
	if(status == UNINIT){
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//		glGenTextures(1, texture);
		glGenTextures(2, texture);
		
		for(int i=0; i<2; i++){
			glBindTexture(GL_TEXTURE_2D, texture[i]);

			//テクスチャのいろいろなパラメタ設定
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}

//		texture_img.create(TWO_POWER_WIDTH, TWO_POWER_HEIGHT, CV_8UC4);
		texture_img.create(TWO_POWER_WIDTH, TWO_POWER_HEIGHT, CV_8UC3);
		foreground_img.create(TWO_POWER_WIDTH, TWO_POWER_HEIGHT, CV_8UC4);
		status = INIT;
	}
}


void slideModelObject::loadModelFile(string filename)
{
	if(status == UNINIT){
		return;
	}
	else if(status & LOADED){
		release();
	}

	try{
		FileStorage cvfs;
		// Configファイルの読み込み
		cvfs.open(filename, CV_STORAGE_READ);
		
		FileNode fn,fn2;
		fn2 = cvfs["Slides"];
		fn = fn2["DisplaySize"];

		if(!fn.empty()){
			string fix_mode_str;
			fn["type"] >> fix_mode_str;

			if(fix_mode_str == "WIDTH"){
				size_fix_mode = WIDTH_FIX;
			}
			else if(fix_mode_str == "HEIGHT"){
				size_fix_mode = HEIGHT_FIX;
			}
			else if(fix_mode_str == "LONG_SIDE"){
				size_fix_mode = LONGER_FIX;
			}
			else if(fix_mode_str == "SHORT_SIDE"){
				size_fix_mode = SHORTER_FIX;
			}
			else{
				size_fix_mode = NON_FIX;
			}

			if(!fn["value"].empty()){
				fn["value"] >> fixed_size;
			}
		}

		if(!fn2["MorphInterval"].empty()){
			fn2["MorphInterval"] >> morph_interval;
		}

		fn = fn2["Images"];
		if(!fn.empty()){
			FileNodeIterator fn_itr = fn.begin();
			while(fn_itr != fn.end()){
				SLIDE_INFO* sld = new SLIDE_INFO;
				(*fn_itr)["frame_num"] >> sld->frame_num;
				(*fn_itr)["image"] >> sld->image_file;
				slide_vec.push_back(sld);
				cout << sld->image_file << "," << sld->frame_num << endl;
				fn_itr++;
			}
		}

		fn2 = cvfs["SpreadSlides"];
		if(!fn2.empty()){
			if(!fn2["SpreadInterval"].empty()){
				fn2["SpreadInterval"] >> spread_interval;
			}
			fn = fn2["Images"];
			FileNodeIterator fn_itr = fn.begin();
			while(fn_itr != fn.end()){
				SLIDE_INFO* sld = new SLIDE_INFO;
				(*fn_itr)["frame_num"] >> sld->frame_num;
				(*fn_itr)["image"] >> sld->image_file;
				spread_slide_vec.push_back(sld);
				cout << sld->image_file << "," << sld->frame_num << endl;
				fn_itr++;
			}
		}

		slide_itr = slide_vec.begin();
		counter = 0;
		status = status | LOADED;

	}
	catch(std::exception e){
		cout << "Failed to read file " + filename << endl;
		throw e;
	}
}


void slideModelObject::drawModel(int& frame_id)
{
	if(frame_id==0){
		slide_itr = slide_vec.begin();
		slide_status = SLIDE_INIT;
		fore_dist = std::abs(fore_dist);
		counter = 0;
	}

	//// 前景/背景（画像切り替えインターバル）表示切り替え ////
	switch(slide_status){
	case SLIDE_INIT:
	case SLIDE_ALPHA:
	case SLIDE_PRESPREAD:
	case SLIDE_LARGEALPHA:
		if(counter == 0){
			updateForegroundTexture();
		}
		updateAlphaChannel((float)(counter+1) / morph_interval);
		break;

	case SLIDE_NORM:
	case SLIDE_LARGE:
	case SLIDE_SPREADING:
		if(counter == 0){
			switchForeground();
		}
		break;
	}

	///////// 拡大処理用行列計算 ///////////
	switch(slide_status){
	case  SLIDE_SPREADING:
		if(counter == 0){
			GLfloat prj_matrix[16];
			glGetFloatv(GL_PROJECTION_MATRIX, prj_matrix);
			glGetFloatv(GL_MODELVIEW_MATRIX, mv_src_matrix);
		
			calcDestSpreadMatrix(prj_matrix);	// 目標モデルビュー行列セット
		}
		GLfloat mv_mtrx2[16];
		calcSpreadMatrix(mv_mtrx2);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixf(mv_mtrx2);
		break;

	case SLIDE_LARGE:
	case SLIDE_LARGEALPHA:
	case SLIDE_LARGELAST:
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixf(mv_dest_matrix);
		break;
	}


	/////////// 表示 ////////////
	if(slide_status != SLIDE_INIT){
		drawTexture();
	}
	if(slide_status == SLIDE_INIT || slide_status == SLIDE_ALPHA
		|| slide_status == SLIDE_PRESPREAD || slide_status == SLIDE_LARGEALPHA){
		drawForegroundTexture();
	}

	switch(slide_status){
	case SLIDE_LARGE:
	case SLIDE_LARGEALPHA:
	case SLIDE_LARGELAST:
	case SLIDE_SPREADING:
		glPopMatrix();
	}

	///////// ステータスの変更処理 ////////
	int pre_status = slide_status;
	counter++;

	switch(pre_status){
	case SLIDE_INIT:
	case SLIDE_ALPHA:
		if(counter == morph_interval){
			slide_status = SLIDE_NORM;
		}
		break;
	case SLIDE_NORM:
		if(counter == (*slide_itr)->frame_num){
			slide_itr++;
			if(slide_itr != slide_vec.end()){
				slide_status = SLIDE_ALPHA;
			}
			else{
				if(!spread_slide_vec.empty()){
					slide_itr = spread_slide_vec.begin();
					slide_status = SLIDE_PRESPREAD;
				}
				else{
					slide_status = SLIDE_NORMLAST;
				}
			}
		}
		break;
	case SLIDE_PRESPREAD:
		if(counter == morph_interval){
			slide_status = SLIDE_SPREADING;
		}
		break;
	case SLIDE_SPREADING:
		if(counter == spread_interval){
			slide_status = SLIDE_LARGE;
		}
		break;
	case SLIDE_LARGE:
		if(counter == (*slide_itr)->frame_num){
			slide_itr++;
			if(slide_itr != spread_slide_vec.end()){
				slide_status = SLIDE_LARGEALPHA;
			}
			else{
				slide_status = SLIDE_LARGELAST;
			}
		}
		break;
	case SLIDE_LARGEALPHA:
		if(counter == morph_interval){
			slide_status = SLIDE_LARGE;
		}
		break;
	}

	if(pre_status != slide_status){
		counter = 0;
	}
	/////////////////////////
}


void slideModelObject::calcDestSpreadMatrix(GLfloat* prj_mtrx)
{
	float image_scale;
	if(T > L){
		image_scale = T;
	}
	else{
		image_scale = L;
	}

	Mat destMat,prjMat;
	transGLmatrix2CvMat(prj_mtrx, prjMat);
	destMat = prjMat.inv();
	if(std::fabs(destMat.at<float>(0,0)) > std::fabs(destMat.at<float>(1,1))){
		destMat.at<float>(0,0) = std::fabs(destMat.at<float>(1,1)) * ((destMat.at<float>(0,0) > 0) ? 1.0 : -1.0);
	}
	else{
		destMat.at<float>(1,1) = std::fabs(destMat.at<float>(0,0)) * ((destMat.at<float>(1,1) > 0) ? 1.0 : -1.0);
	}
	destMat.at<float>(0,0) /= image_scale;
	destMat.at<float>(1,1) /= image_scale;
	fore_dist = -std::abs(fore_dist);

//	float f[] = {1,0,0,0,0,1,0,0,0,0,1,-fore_dist,0,0,0,1};
//	Mat zmvMat(4,4,CV_32FC1,f);
//	Mat destMat2 = destMat * zmvMat;
//	transCvMat2GLmatrix(destMat2, mv_dest_matrix);
	transCvMat2GLmatrix(destMat, mv_dest_matrix);
}


void slideModelObject::calcSpreadMatrix(GLfloat* mv_mtrx2)
{
	for(int i=0; i<16; i++){
		mv_mtrx2[i] = (mv_src_matrix[i] * (spread_interval - counter) + mv_dest_matrix[i] * counter) / spread_interval;
	}
}
/*
void slideModelObject::calcSpreadMatrix(GLfloat* mv_mtrx2, int cur_num, int end_num)
{
	for(int i=0; i<16; i++){
		mv_mtrx2[i] = (mv_src_matrix[i] * (end_num - cur_num) + mv_dest_matrix[i] * cur_num) / end_num;
	}
}*/


void slideModelObject::transGLmatrix2CvMat(GLfloat* glf, Mat& retMat)
{
	retMat.release();
	retMat.create(4,4,CV_32FC1);

	for(int y=0; y<4; y++){
		for(int x=0; x<4; x++){
			retMat.at<float>(y,x) = glf[4*x+y];
		}
	}
}


void slideModelObject::transCvMat2GLmatrix(Mat& retMat, GLfloat* glf)
{
	for(int y=0; y<4; y++){
		for(int x=0; x<4; x++){
			glf[4*x+y] = retMat.at<float>(y,x);
		}
	}
}


void slideModelObject::release()
{
	releaseSlides();
	texture_img.release();
	foreground_img.release();
	glDeleteTextures(2, texture);
	status = UNINIT;
}


void slideModelObject::releaseSlides()
{
	vector<SLIDE_INFO*>::iterator itr = slide_vec.begin();
	while(itr != slide_vec.end()){
//		(*itr)->image.release();
		(*itr)->image_file.clear();
		delete (*itr);
		itr++;
	}
	slide_vec.clear();

	itr = spread_slide_vec.begin();
	while(itr != spread_slide_vec.end()){
//		(*itr)->image.release();
		(*itr)->image_file.clear();
		delete (*itr);
		itr++;
	}
	spread_slide_vec.clear();

	status = INIT;
}


void slideModelObject::updateTexture()
{
	Mat image = imread((*slide_itr)->image_file);
	if(image.empty()){
		throw orArgException("wrong slide image name");
	}

	int f_mode;

	switch(size_fix_mode){
	case LONGER_FIX:
		if(image.rows > image.cols){
			f_mode = HEIGHT_FIX;
		}
		else{
			f_mode = WIDTH_FIX;
		}
		break;
	case SHORTER_FIX:
		if(image.rows > image.cols){
			f_mode = WIDTH_FIX;
		}
		else{
			f_mode = HEIGHT_FIX;
		}
		break;
	default:
		f_mode = size_fix_mode;
	}

	switch(f_mode){
	case WIDTH_FIX:
		R = (double)fixed_size / 2;
		T = R * image.rows / image.cols;
		break;
	case HEIGHT_FIX:
		T = (double)fixed_size / 2;
		R = T * image.cols / image.rows;
		break;
	default:
		T = (double)(image.rows) / 2;
		R = (double)(image.cols) / 2;
	}
	B = -T;
	L = -R;

	Mat img;

	// OpenGL用に画像反転
	cv::flip(image, img, 0);
//	Mat alphaimg(img.rows,img.cols,CV_8UC4);
//	resizeMatChannel(img, alphaimg, 255);
//	resizeMatChannel(alphaimg, img, 0);

	//テクスチャに貼り付けるため、2の累乗にリサイズする
	cv::resize(img, texture_img, texture_img.size());
//	cvResize(&((IplImage)alphaimg), &((IplImage)texture_img));
}


void slideModelObject::updateForegroundTexture()
{
	Mat image = imread((*slide_itr)->image_file);
	if(image.empty()){
		throw orArgException("wrong slide image name");
	}

	int f_mode;

	switch(size_fix_mode){
	case LONGER_FIX:
		if(image.rows > image.cols){
			f_mode = HEIGHT_FIX;
		}
		else{
			f_mode = WIDTH_FIX;
		}
		break;
	case SHORTER_FIX:
		if(image.rows > image.cols){
			f_mode = WIDTH_FIX;
		}
		else{
			f_mode = HEIGHT_FIX;
		}
		break;
	default:
		f_mode = size_fix_mode;
	}

	switch(f_mode){
	case WIDTH_FIX:
		fR = (double)fixed_size / 2;
		fT = fR * image.rows / image.cols;
		break;
	case HEIGHT_FIX:
		fT = (double)fixed_size / 2;
		fR = fT * image.cols / image.rows;
		break;
	default:
		fT = (double)(image.rows) / 2;
		fR = (double)(image.cols) / 2;
	}
	fB = -fT;
	fL = -fR;

	Mat img;

	// OpenGL用に画像反転
	cv::flip(image, img, 0);
	Mat alphaimg(img.rows,img.cols,CV_8UC4);
	resizeMatChannel(img, alphaimg, 0);
//	resizeMatChannel(alphaimg, img, 0);

	//テクスチャに貼り付けるため、2の累乗にリサイズする
	cv::resize(alphaimg, foreground_img, foreground_img.size());
}


void slideModelObject::switchForeground()
{
	resizeMatChannel(foreground_img, texture_img);
//	foreground_img.copyTo(texture_img);
	T = fT;
	B = fB;
	L = fL;
	R = fR;
}


void slideModelObject::updateAlphaChannel(float ratio)
{
	double val = ratio * 255;
	setChannelValue(foreground_img,3,val);

	//テクスチャを貼り付ける
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	
	//↑のやつをテクスチャに貼り付ける
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		foreground_img.cols, foreground_img.rows,
		0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, foreground_img.data);
}


void slideModelObject::drawTexture()
{
	//テクスチャを貼り付ける
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		texture_img.cols,texture_img.rows,
		0, GL_BGR_EXT, GL_UNSIGNED_BYTE, texture_img.data);

	// 背景テクスチャを描画
//	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
//	glEnable(GL_BLEND);
	glDisable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glNormal3d(0.0,0.0,1.0);

	glBegin(GL_QUADS);
	{
//		glTexCoord2d(0.0, 0.0);		glVertex3d(L, B, 1);
//		glTexCoord2d(0.0, 1.0);		glVertex3d(L, T, 1);
//		glTexCoord2d(1.0, 1.0);		glVertex3d(R, T, 1);
//		glTexCoord2d(1.0, 0.0);		glVertex3d(R, B, 1);
		glTexCoord2d(0.0, 0.0);		glVertex3d(L, B, 0);
		glTexCoord2d(0.0, 1.0);		glVertex3d(L, T, 0);
		glTexCoord2d(1.0, 1.0);		glVertex3d(R, T, 0);
		glTexCoord2d(1.0, 0.0);		glVertex3d(R, B, 0);
	}
	glEnd();
//	glFlush();
	glDisable(GL_TEXTURE_2D);
}



void slideModelObject::drawForegroundTexture()
{
	//テクスチャを貼り付ける
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		foreground_img.cols, foreground_img.rows,
		0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, foreground_img.data);

	// 背景テクスチャを描画
//	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glNormal3d(0.0,0.0,1.0);

	glBegin(GL_QUADS);
	{
		glTexCoord2d(0.0, 0.0);		glVertex3d(fL, fB, fore_dist);
		glTexCoord2d(0.0, 1.0);		glVertex3d(fL, fT, fore_dist);
		glTexCoord2d(1.0, 1.0);		glVertex3d(fR, fT, fore_dist);
		glTexCoord2d(1.0, 0.0);		glVertex3d(fR, fB, fore_dist);
//		glTexCoord2d(0.0, 0.0);		glVertex3d(fL, fB, 0);
//		glTexCoord2d(0.0, 1.0);		glVertex3d(fL, fT, 0);
//		glTexCoord2d(1.0, 1.0);		glVertex3d(fR, fT, 0);
//		glTexCoord2d(1.0, 0.0);		glVertex3d(fR, fB, 0);
	}
	glEnd();
//	glFlush();
	glDisable(GL_TEXTURE_2D);
}
