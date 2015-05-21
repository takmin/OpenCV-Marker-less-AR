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
#ifndef __VIEW_MODEL__
#define __VIEW_MODEL__

#include <opencv2/core/core.hpp>
#include "modelObjectFactory.h"
//#include "GLMetaseq.h"	// Model loader

namespace cvar{
namespace overlay{

typedef struct{
	modelObject* model;	// Model class
	double scale;	// It specifies the size of the model
	std::string modelFilename;	// The path to the model file
	cv::Mat initRot;	// 3 Å~ 3 rotation matrix to the initial position (in the XY plane of markers Z = 0, Z is + direction is up, Y is + direction of depth)
	cv::Mat initTrans;	// Movement component to the initial position
	cv::Point2f	markerCenter;	// Marker center position
	cv::Size markerSize;	// Marker Size
}MODEL_INFO;

class viewModel
{
private:
	static viewModel* vmInstance;
	viewModel(void);
	viewModel(viewModel*){}
	~viewModel(void);

public:
//	viewModel(void);
//	~viewModel(void);

	static viewModel* getInstance(){
		if(!vmInstance){
			vmInstance = new viewModel();
		}
		return vmInstance;
	}

	static void deleteInstance(){
		if(vmInstance){
			delete vmInstance;
		}
		vmInstance = 0;
	}

public:
	cv::Mat	resized_frame;

	int two_power_width;	// Resize destination size to match the power of two
	int two_power_height;	// 512 or appropriate per 1024

	int window_width;
	int window_height;

	int capture_width;
	int capture_height;

	GLuint texture[1];
//	MQO_MODEL g_mqoModel;	// MQO model
	double aspect_rate;			// Aspect ratio of capture
	float focal_length;		// Focal length

	cv::Mat cameraMatrix;	// Camera parameter matrix

//	double model_scale;
	std::map<int,MODEL_INFO>	model_map;

	// ë“ÇøéÛÇØ
	int wait_frames;
	MODEL_INFO wait_model;

protected:	
	cv::Mat accHomMat;
	int mat_type;
	modelObjectFactory modelFac;
	MODEL_INFO*	curModel;
	bool mirror_f;
	cv::Mat mirrorMat;

public:
	bool init(cv::Size& frame_size, cv::Mat& cameraMat);
	bool init(cv::Size& frame_size, cv::Mat& cameraMat, int type);
	void resize(int win_w, int win_h);	// Window size change function
	void updateTexture(cv::Mat& frame);	// Update function
	void release();
	void exitFunc();

	bool setTwoPowerSize(int w, int h);
	void setCameraMatrix(cv::Mat& cameraMat);
	void setMirrorMode(bool flag);
	void setFocalLength(float len);
//	void setMarkerSize(cv::Size);

	void drawScene(cv::Mat& img);
	void drawObject(cv::Mat& homographyMat, int seq_id);
	void drawWaitModel(int seq_id);
	template<typename _Tp> void drawObjectType(cv::Mat& homographyMat, int seq_id);
	void initAccHomMat();
//	void initAccHomMat(cv::Mat& homMat);
	bool setRecogId(int id, cv::Mat& homMat);
	bool addModel(int id, cv::Size& markerSize, int model_type, const std::string& model_filename, double scale = 1.0);
	bool addModel(int id, cv::Size& markerSize, int model_type, const std::string& model_filename, double scale, cv::Mat& initRot, cv::Mat& initTrans);
	void releaseModel();

	bool addWaitModel(int wait_frame_num, int model_type, const std::string& model_filename, double scale = 1.0);
	bool addWaitModel(int wait_frame_num, int model_type, const std::string& model_filename, double scale, cv::Mat& initRot, cv::Mat& initTrans);
	void releaseWaitModel();

//	cv::Mat convertMatType(cv::Mat& src_mat);
};

};
};
#endif