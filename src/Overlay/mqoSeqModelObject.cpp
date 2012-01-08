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
#include "mqoSeqModelObject.h"
#include "orException.h"

using namespace std;
using namespace cv;
using namespace cvar;
using namespace cvar::overlay;

mqoSeqModelObject::mqoSeqModelObject(void)
{
	status = UNINIT;
}


mqoSeqModelObject::~mqoSeqModelObject(void)
{
	release();
}


void mqoSeqModelObject::init()
{
	status = INIT;
}


void mqoSeqModelObject::loadModelFile(string filename)
{
	if(status & INIT){
		if(status & LOADED){
			mqoDeleteSequence(model_seq);
		}

		try{
			FileStorage fs(filename, FileStorage::READ);

			string fh;
			int num;
			fs["fileheader"] >> fh;
			fs["number"] >> num;

			string::size_type fname_top = filename.find_last_of("\\");
			char fullfilename[256];

			if(fname_top==string::npos){
				fname_top = filename.find_last_of("/");
			}
			
			if(fname_top!=string::npos){
				string dirname = filename.substr(0, fname_top);
				sprintf(fullfilename, "%s/%s%%d.mqo", dirname.c_str(), fh.c_str());
			}
			else{
				sprintf(fullfilename, "%s\%%d.mqo", fh.c_str());
			}

			model_seq = mqoCreateSequence(fullfilename,num,1.0);
//			model_seq = mqoCreateSequence("ardemo/miku/miku%d.mqo",num,1.0);
//			model_seq = mqoCreateSequence((char*)filename.c_str(),1.0);

			status = status | LOADED;
		}
		catch(std::exception e){
			throw e;
		}
	}
}

void mqoSeqModelObject::drawModel(int& frame_id)
{
	int seq_id = frame_id % model_seq.n_frame;
	mqoCallSequence(model_seq, seq_id);
}

void mqoSeqModelObject::release()
{
	if(status & LOADED){
		mqoDeleteSequence(model_seq);
//		status = status ^ LOADED;
		status = status - (status & LOADED);
	}
}