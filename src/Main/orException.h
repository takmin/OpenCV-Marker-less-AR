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
#ifndef __OR_EXCEPTION__
#define __OR_EXCEPTION__

#include<string>
#include <opencv2/core/core.hpp>

namespace cvar{

// General Exception
class orException : public std::exception
{
public:
	orException(void);
	orException(std::string msg);
	virtual ~orException(void) throw(){};

	void setMessage(std::string msg);

public:
	std::string	message;
};


// Exception related to OpenCV Functions
class orCvException : public orException
{
public:
	orCvException(void);
	orCvException(std::string msg);
	virtual ~orCvException(void) throw(){};

	void setFunctionName(std::string name);
	void setCvExceptionClass(cv::Exception e);

public:
	std::string cvfunction;
	cv::Exception cv_e;
};

// Illegal Argument
class orArgException : public orException
{
public:
	orArgException(void);
	orArgException(std::string msg);
	virtual ~orArgException(void) throw(){};

	void setFunctionName(std::string name);

public:
	std::string function;
};


// State Error
class orStateException : public orException
{
public:
	orStateException(void);
	orStateException(std::string msg);
	virtual ~orStateException(void) throw(){};
};

};

#endif