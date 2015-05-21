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
#ifndef __SLIDE_MODEL_OBJECT__
#define __SLIDE_MODEL_OBJECT__

#include <vector>
#include <GL/glut.h>
#include <opencv2/core/core.hpp>
#include "modelobject.h"

namespace cvar{
namespace overlay{

typedef struct{
//	cv::Mat image;
	std::string image_file;
	int frame_num;
}SLIDE_INFO;

class slideModelObject :
	public modelObject
{
public:
	slideModelObject(void);
	~slideModelObject(void);

public:
	static const int TWO_POWER_WIDTH = 1024;
	static const int TWO_POWER_HEIGHT = 1024;
	GLuint texture[1];
	
	static const int NON_FIX = 0;
	static const int WIDTH_FIX = 1;
	static const int HEIGHT_FIX = 2;
	static const int LONGER_FIX = 3;
	static const int SHORTER_FIX = 4;
	int size_fix_mode;
	int fixed_size;

	int morph_interval;		// Inter-frame switching interval (the number of frames)
	int spread_interval;	// Image enlargement switching interval (the number of frames)

protected:
	int slide_status;	// The current display status
	static const int SLIDE_INIT = 0x00;	// Transparent processing of the first slide
	static const int SLIDE_ALPHA = 0x01;	// Transparent processing of the second and subsequent slide
	static const int SLIDE_NORM = 0x02;	// Normal slide superimposed display
	static const int SLIDE_PRESPREAD = 0x04;	// Slide enlargement process before the transmission processing
	static const int SLIDE_SPREADING = 0x08;	// Enlargement process
	static const int SLIDE_LARGEALPHA = 0x0f;	// Transparent processing of expansion slide
	static const int SLIDE_LARGE = 0x10;	// View larger image
	static const int SLIDE_NORMLAST = 0x20;	// The final slide (superimposed display)
	static const int SLIDE_LARGELAST = 0x40;	// The final slide (view larger)
	std::vector<SLIDE_INFO*>	slide_vec;
	std::vector<SLIDE_INFO*>	spread_slide_vec;
	std::vector<SLIDE_INFO*>::iterator	slide_itr;

	cv::Mat	texture_img;
	cv::Mat foreground_img;
	int counter;
	double T,B,L,R;
	double fT,fB,fL,fR;

//	float image_scale;
	GLfloat mv_src_matrix[16];
	GLfloat mv_dest_matrix[16];

	double fore_dist;

public:
	// Overriding modelObject method
	void init();
	void loadModelFile(std::string filename);
	void drawModel(int& frame_id);
	void release();

protected:
	void releaseSlides();
	void updateForegroundTexture();
	void updateTexture();
	void switchForeground();
	void drawTexture();
	void drawForegroundTexture();

	void updateAlphaChannel(float ratio);

	void calcDestSpreadMatrix(GLfloat* prj_mtrx);
	void calcSpreadMatrix(GLfloat* trans_mtrx);
//	void calcSpreadMatrix(GLfloat* trans_mtrx, int cur_num, int end_num);
	void transGLmatrix2CvMat(GLfloat* glf, cv::Mat& cvmat);
	void transCvMat2GLmatrix(cv::Mat& cvmat, GLfloat* glf);
};

};
};
#endif
