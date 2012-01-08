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
#include "orException.h"

using namespace std;
using namespace cvar;

orException::orException(void)
{
}

orException::orException(string msg)
{
	message = msg;
}

void orException::setMessage(string msg)
{
	message = msg;
}


// orCvException
orCvException::orCvException(void)
{
}

orCvException::orCvException(string msg) : orException(msg)
{
}

void orCvException::setFunctionName(string name)
{
	cvfunction = name;
}

void orCvException::setCvExceptionClass(cv::Exception e)
{
	cv_e = e;
}


// orArgException
orArgException::orArgException(void)
{
}

orArgException::orArgException(string msg) : orException(msg)
{
}

void orArgException::setFunctionName(string name)
{
	function = name;
}


// orStateException
orStateException::orStateException(void)
{
}

orStateException::orStateException(string msg) : orException(msg)
{
}

