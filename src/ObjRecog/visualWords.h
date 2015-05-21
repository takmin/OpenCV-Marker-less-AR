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
#ifndef __VISUAL_WORDS__
#define __VISUAL_WORDS__

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

namespace cvar{
namespace or{

class visualWords
{
public:
	visualWords(void);
	~visualWords(void);

	//! Release All
	void release();

	//! Is Feature Dictionary ready?
	/*!
	\return if the index of feature dictionary is ready, return true
	*/
	bool isReady();

	void addFeatures(const cv::Mat& feature);	// Add feature amount for the creation of Visual Word
	void createVW(int cluster_num = 0);	// Create a Visual Word

	// Load & Save
	bool save(const std::string& filename) const;
	bool saveBinary(const std::string& filename, const std::string& idx_filename) const;
	void write(cv::FileStorage& FS, const std::string& name) const;
	bool load(const std::string& filename);
	bool loadBinary(const std::string& filename, const std::string& idx_filename);
	void read(const cv::FileNode& node);

	void setVoteNum(int vote_num);

	int getVisualWordNum() const;

	cv::Mat querySearchDB(const cv::Mat& features);

private:
	// search DB functions
//	int releaseSearchDB();
	static void convertFeatureMat(const std::vector<cv::Mat>& src_feature, cv::Mat& dest_feature);	// combine vector to Mat
	bool save_vw_binary(const std::string& filename) const;	// Save visualWord data in binary format
	bool load_vw_binary(const std::string& filename);	// Save visualWord data in binary format
	bool saveIndex(const std::string& filename) const;
	bool loadIndex(const std::string& filename);
	void writeIndex(cv::FileStorage& FS, const std::string& name) const;
	void readIndex(const cv::FileNode& node);

private:
	std::string matcherType;
	cv::Ptr<cv::DescriptorMatcher>	descriptor_matcher;
	int voteNum;
	float radius;

	static const int version = 120;
};

};
};
#endif