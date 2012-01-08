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
#include "viewModel.h"
#include "commonCvFunctions.h"
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;
using namespace cvar;
using namespace cvar::overlay;

viewModel* viewModel::vmInstance = 0;

viewModel::viewModel(void)
{
	two_power_width = 1024;
	two_power_height = 1024;
	focal_length = 1.0;
	mirror_f = false;
	wait_frames = -1;
	mat_type = -1;
}

viewModel::~viewModel(void)
{
	releaseModel();
	releaseWaitModel();
}


void viewModel::exitFunc()
{
	releaseModel();
	releaseWaitModel();
	mqoCleanup();					// GLMetaseqの終了処理
	glDeleteTextures(1, &texture[0]);
	mat_type = -1;
}

bool viewModel::setTwoPowerSize(int w, int h)
{
	// two_power_wは2のN乗
	int tmp1;
	tmp1 = w;
	while(tmp1 > 1){
		if(tmp1 % 2 > 0){
			return false;
		}
		tmp1 /= 2;
	}

	// two_power_hは2のN乗
	tmp1 = h;
	while(tmp1 > 1){
		if(tmp1 % 2 > 0){
			return false;
		}
		tmp1 /= 2;
	}

	two_power_width = w;
	two_power_height = h;

	return true;
}


void viewModel::setCameraMatrix(Mat& cameraMat)
{
	assert(cameraMat.type() == mat_type);
	this->cameraMatrix = cameraMat.clone();
}


void viewModel::setMirrorMode(bool flag)
{
	this->mirror_f = flag;
	if(mat_type==CV_32FC1){
		float td[] = {-1, 0, capture_width-1, 0, 1, 0, 0, 0, 1};
		mirrorMat = Mat(3, 3, mat_type, td).clone();
	}
	else{
		double td[] = {-1, 0, capture_width-1, 0, 1, 0, 0, 0, 1};
		mirrorMat = Mat(3, 3, mat_type, td).clone();
	}
}


void viewModel::setFocalLength(float len)
{
	this->focal_length = len;
}


bool viewModel::addModel(int id, Size& markerSize, int model_type, const string& model_filename, double scale)
{
	int type = cameraMatrix.type();
	Mat iPos, iTrans;
	if(type == CV_32FC1){
		float f[] = {1,0,0,0,1,0,0,0,1};
		iPos = Mat(3,3,CV_32FC1,f).clone();

		float ft[] = {0, 0, 0};
		iTrans = Mat(3, 1, CV_32FC1, ft).clone();
	}
	else if(type == CV_64FC1){
		double d[] = {1,0,0,0,1,0,0,0,1};
		iPos = Mat(3,3,CV_64FC1,d).clone();

		double ft[] = {0, 0, 0};
		iTrans = Mat(3, 1, CV_64FC1, ft).clone();
	}
	return addModel(id, markerSize, model_type, model_filename, scale, iPos, iTrans);
}

bool viewModel::addModel(int id, Size& markerSize, int model_type, const string& model_filename, double scale, cv::Mat& initRot, cv::Mat& initTrans)
{
	MODEL_INFO	mdl_info;
	mdl_info.markerSize = markerSize;
	mdl_info.markerCenter.x = (float)(markerSize.width) / 2.0;
	mdl_info.markerCenter.y = (float)(markerSize.height) / 2.0;

	mdl_info.modelFilename = model_filename;
	mdl_info.model = modelFac.create(model_type);
	mdl_info.model->init();
	mdl_info.model->loadModelFile((char*)model_filename.c_str());		// モデルのロード
	mdl_info.scale = scale;
//	mdl_info.initRot = convertMatType(initRot);
	initRot.convertTo(mdl_info.initRot, mat_type);
	initTrans.convertTo(mdl_info.initTrans, mat_type);

	pair<map<int,MODEL_INFO>::iterator,bool>	ret_insert;
	ret_insert = model_map.insert(pair<int,MODEL_INFO>(id, mdl_info));

	// ToDo: 例外処理
	if(!(bool)(ret_insert.second)){
		return false;
	}
	return true;
}


void viewModel::releaseModel()
{
	map<int,MODEL_INFO>::iterator itr = model_map.begin();
	while(itr != model_map.end())
	{
		itr->second.model->release();
		delete itr->second.model;
		itr++;
	}
	model_map.clear();
}


bool viewModel::addWaitModel(int wait_frame_num, int model_type, const string& model_filename, double scale)
{
	int type = cameraMatrix.type();
	Mat iPos, iTrans;
	if(type == CV_32FC1){
		float f[] = {1,0,0,0,1,0,0,0,1};
		iPos = Mat(3,3,CV_32FC1,f).clone();

		float ft[] = {0, 0, 0};
		iTrans = Mat(3, 1, CV_32FC1, ft).clone();
	}
	else if(type == CV_64FC1){
		double d[] = {1,0,0,0,1,0,0,0,1};
		iPos = Mat(3,3,CV_64FC1,d).clone();

		double ft[] = {0, 0, 0};
		iTrans = Mat(3, 1, CV_64FC1, ft).clone();
	}
	return addWaitModel(wait_frame_num, model_type, model_filename, scale, iPos, iTrans);
}


bool viewModel::addWaitModel(int wait_frame_num, int model_type, const string& model_filename, double scale, cv::Mat& initRot, cv::Mat& initTrans)
{
	if(wait_frame_num >= 0){
		try{
			wait_frames = wait_frame_num;

			wait_model.modelFilename = model_filename;
			wait_model.model = modelFac.create(model_type);
			wait_model.model->init();
			wait_model.model->loadModelFile((char*)model_filename.c_str());		// モデルのロード
			wait_model.scale = scale;
			initRot.convertTo(wait_model.initRot, mat_type);
			initTrans.convertTo(wait_model.initTrans, mat_type);
		}
		catch(std::exception e){
			wait_frames = -1;
			wait_model.model->release();
			return false;
		}
		return true;
	}
	else{
		return false;
	}
}


void viewModel::releaseWaitModel()
{
	if(wait_frames >=0 ){
		wait_model.model->release();
		delete wait_model.model;
		wait_frames = -1;
	}
}


void viewModel::initAccHomMat()
{
	if(mat_type == CV_32FC1){
		float f[] = {1,0,0,0,1,0,0,0,1};
		this->accHomMat = Mat(3,3,CV_32FC1,f).clone();
	}
	else if(mat_type == CV_64FC1){
		double d[] = {1,0,0,0,1,0,0,0,1};
		this->accHomMat = Mat(3,3,CV_64FC1,d).clone();
	}
}


bool viewModel::setRecogId(int id, Mat& homMat)
{
	map<int, MODEL_INFO>::iterator itr = model_map.find(id);
	
	if(itr==model_map.end()){
		return false;
	}
	else{
		curModel = &(itr->second);

		homMat.convertTo(accHomMat, mat_type);
		if(mirror_f){
			Mat markerMirrorMat = mirrorMat.clone();
			if(mat_type==CV_32FC1){
				markerMirrorMat.at<float>(0,2) = curModel->markerSize.width - 1;
			}
			else{
				markerMirrorMat.at<double>(0,2) = curModel->markerSize.width - 1;
			}
			accHomMat = mirrorMat * accHomMat * markerMirrorMat;
		}
		return true;
	}
}

bool viewModel::init(Size& cap_size, Mat& cameraMat)
{
	return init(cap_size, cameraMat, cameraMat.type());
}

bool viewModel::init(Size& cap_size, Mat& cameraMat, int type)
{
//	assert(cameraMat.type() == CV_32FC1 || cameraMat.type() == CV_64FC1);
	assert(type == CV_32FC1 || type == CV_64FC1);

	mat_type = type;

	capture_width = cap_size.width;
	capture_height = cap_size.height;

	window_width = capture_width;
	window_height = capture_height;

//	this->cameraMatrix = convertMatType(cameraMat);
	cameraMat.convertTo(cameraMatrix, mat_type);
//	initAccHomMat(cameraMatrix.type());

	// モデルを表示させる準備
	mqoInit();											// GLMetaseqの初期化

	//OpenGLさん！テクスチャ作ってくださいな＞＜
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	//テクスチャのいろいろなパラメタ設定
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	//取得画像の幅と高さの比を計算する
	//テクスチャへの貼り付けに使う
	aspect_rate = (double)capture_width / (double)capture_height;
	
	//OpenCV処理用の画像を生成する
	//OpenGLが扱うテクスチャの関係で，二の乗数でな！
	resized_frame.create(two_power_width, two_power_height, CV_8UC3);

	// ビューポート作成
	glViewport(0, 0, window_width, window_height);

	return true;
}


void viewModel::drawScene(Mat& img)
{
	//透視変換行列の設定
	glMatrixMode(GL_PROJECTION);

	glPushMatrix();
	glLoadIdentity();

	//正射影変換
	glOrtho(-aspect_rate, aspect_rate, -1.0, 1.0, -1.0, 1.0);
	
	// 背景テクスチャを描画
	glDisable(GL_DEPTH_TEST);

	//テクスチャを貼り付ける
	updateTexture(img);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	{
		glTexCoord2d(0.0, 0.0);		glVertex2d(-aspect_rate, -1);
		glTexCoord2d(0.0, 1.0);		glVertex2d(-aspect_rate,  1);
		glTexCoord2d(1.0, 1.0);		glVertex2d( aspect_rate,  1);
		glTexCoord2d(1.0, 0.0);		glVertex2d( aspect_rate, -1);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glPopMatrix();
}


// 光源の設定を行う関数
void setLight(void)
{
	GLfloat light_diffuse[]  = { 0.9, 0.9, 0.9, 1.0 };	// 拡散反射光
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };	// 鏡面反射光
	GLfloat light_ambient[]  = { 0.3, 0.3, 0.3, 0.1 };	// 環境光
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };	// 位置と種類

	// 光源の設定
	glLightfv( GL_LIGHT0, GL_DIFFUSE,  light_diffuse );	 // 拡散反射光の設定
	glLightfv( GL_LIGHT0, GL_SPECULAR, light_specular ); // 鏡面反射光の設定
	glLightfv( GL_LIGHT0, GL_AMBIENT,  light_ambient );	 // 環境光の設定
	glLightfv( GL_LIGHT0, GL_POSITION, light_position ); // 位置と種類の設定

	glShadeModel( GL_SMOOTH );	// シェーディングの種類の設定
	glEnable( GL_LIGHT0 );		// 光源の有効化
}

void viewModel::drawObject(Mat& homographyMat, int seq_id)
{
	try{
		if(mat_type == CV_32FC1){
			drawObjectType<float>(homographyMat, seq_id);
		}
		else if(mat_type == CV_64FC1){
			drawObjectType<double>(homographyMat, seq_id);
		}
	}
	catch(std::exception& e){
		throw e;
	}
}

// マーカーはX-Y平面でYが+方向が上。Zが+方向が上。
template<typename _Tp> void viewModel::drawObjectType(Mat& homographyMat, int seq_id)
{
	cv::Mat rotation, translation, Rot, xRot, iTrans;
//	Mat homMat = convertMatType(homographyMat);
	Mat homMat;
	homographyMat.convertTo(homMat, mat_type);

	try{
		if(mirror_f){
			homMat = mirrorMat * homMat * mirrorMat;
		}
		accHomMat = homMat * accHomMat;
//		decomposeHomographyType<_Tp>(accHomMat, cameraMatrix, rotation, translation);
		decomposeHomographyType<_Tp>(accHomMat, cameraMatrix, rotation, translation, curModel->markerCenter);
	}
	catch(std::exception& e){
		throw e;
	}

	double model_scale = curModel->scale;
	
	_Tp d[] = {1,0,0,0,-1,0,0,0,-1};
//	_Tp d[] = {1,0,0,0,1,0,0,0,1};
	xRot = Mat(3,3,rotation.type(),d).clone();
	Rot = rotation * curModel->initRot * xRot;
	iTrans = Rot * curModel->initTrans;
//	invRot = rotation.inv();
	GLdouble mtrx[16];
	for(int y=0; y<3; y++){
		for(int x=0; x<3; x++){
			mtrx[x*4+y] = Rot.at<_Tp>(y, x);
		}
	}

	for(int x=0; x<3; x++){
		mtrx[3 + x*4] = 0;
	}

//	mtrx[12] = translation.at<_Tp>(0,0);
//	mtrx[13] = translation.at<_Tp>(1,0);
//	mtrx[14] = translation.at<_Tp>(2,0);
	mtrx[12] = translation.at<_Tp>(0,0) + iTrans.at<_Tp>(0,0);
	mtrx[13] = translation.at<_Tp>(1,0) + iTrans.at<_Tp>(1,0);
	mtrx[14] = translation.at<_Tp>(2,0) + iTrans.at<_Tp>(2,0);
	mtrx[15] = 1;

	double frust_L = cameraMatrix.at<_Tp>(0,2) * focal_length / cameraMatrix.at<_Tp>(0,0); 
	double frust_R = ((double)capture_width - cameraMatrix.at<_Tp>(0,2))  * focal_length / cameraMatrix.at<_Tp>(0,0); 
	double frust_T = cameraMatrix.at<_Tp>(1,2) * focal_length / cameraMatrix.at<_Tp>(1,1); 
	double frust_D = ((double)capture_height - cameraMatrix.at<_Tp>(1,2)) * focal_length / cameraMatrix.at<_Tp>(1,1); 

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glFrustum(-frust_L, frust_R, -frust_D, frust_T, focal_length, 100.0);
//	glFrustum(-frust_L, frust_R, -frust_D, frust_T, 1.0, 100.0);
//	gluPerspective(50, aspect_rate, 10, 10000);

	gluLookAt(0,0,0,0,0,1,0,-1,0);

	glMatrixMode(GL_MODELVIEW);

//	glTranslatef(0.0, 0.0, -7.0);

	setLight();					// 光源の設定
	glEnable(GL_LIGHTING);		// 光源ON
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);		// 隠面処理の適用
	glEnable(GL_CULL_FACE);

	glPushMatrix();
		glLoadMatrixd(mtrx);
		glScaled(model_scale, model_scale, model_scale);
		curModel->model->drawModel(seq_id);			// MQOモデルのコール
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glDisable(GL_CULL_FACE);
	glDisable( GL_DEPTH_TEST );

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}


void viewModel::drawWaitModel(int seq_id)
{
	if(wait_frames < 0 || seq_id < wait_frames){
		return;
	}

	int seq_num = seq_id - wait_frames;

	// 変換パラメータ取得
	double model_scale = wait_model.scale;
	
	GLdouble mtrx[16];
	if(mat_type == CV_32FC1){
		for(int y=0; y<3; y++){
			for(int x=0; x<3; x++){
				mtrx[x*4+y] = wait_model.initRot.at<float>(y, x);
			}
		}
		mtrx[12] = wait_model.initTrans.at<float>(0,0);
		mtrx[13] = wait_model.initTrans.at<float>(1,0);
		mtrx[14] = wait_model.initTrans.at<float>(2,0);
	}
	else{
		for(int y=0; y<3; y++){
			for(int x=0; x<3; x++){
				mtrx[x*4+y] = wait_model.initRot.at<double>(y, x);
			}
		}
		mtrx[12] = wait_model.initTrans.at<double>(0,0);
		mtrx[13] = wait_model.initTrans.at<double>(1,0);
		mtrx[14] = wait_model.initTrans.at<double>(2,0);
	}

	for(int x=0; x<3; x++){
		mtrx[3 + x*4] = 0;
	}

	mtrx[15] = 1;

	//透視変換行列の設定
	glMatrixMode(GL_PROJECTION);

	glPushMatrix();
	glLoadIdentity();

	//正射影変換
	glOrtho(-aspect_rate, aspect_rate, -1.0, 1.0, 0, 2.0);
	gluLookAt(0,0,1.0,0,0,0,0,1,0);
	
	// モデルを描画
	glMatrixMode(GL_MODELVIEW);

//	glTranslatef(0.0, 0.0, -7.0);

	setLight();					// 光源の設定
	glEnable(GL_LIGHTING);		// 光源ON
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);		// 隠面処理の適用
	glEnable(GL_CULL_FACE);

	glPushMatrix();
		glLoadIdentity();
		glLoadMatrixd(mtrx);
		glScaled(model_scale, model_scale, model_scale);
		wait_model.model->drawModel(seq_num);			// モデルのコール
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
	glDisable(GL_CULL_FACE);
	glDisable( GL_DEPTH_TEST );

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}


// ウィンドウサイズ変更関数
void viewModel::resize(int w, int h)
{
//	window_width = w;
//	window_height = h;

	int sx, sy;

//	aspect_rate = (float)w/h;
	if((float)w/h > aspect_rate){
		window_width = aspect_rate * h;
		window_height = h;
		sx = (w - window_width) / 2;
		sy = 0;
	}
	else{
		window_width = w;
		window_height = w / aspect_rate;
		sx = 0;
		sy = (h - window_height) / 2;
	}

	glViewport(sx, sy, window_width, window_height);
}


// 更新関数
void viewModel::updateTexture(Mat& frame)
{
	//テクスチャを貼り付ける
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	Mat img;

	// OpenGL用に画像反転
	if(mirror_f){
		cv::flip(frame, img, -1);
	}
	else{
		cv::flip(frame, img, 0);
	}

	//テクスチャに貼り付けるため、2の累乗にリサイズする
	cv::resize(img, resized_frame, resized_frame.size());

	//↑のやつをテクスチャに貼り付ける
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		resized_frame.cols,resized_frame.rows,
		0, GL_BGR_EXT, GL_UNSIGNED_BYTE, resized_frame.data);
}

