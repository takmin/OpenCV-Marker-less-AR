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
#ifndef __CONTROL_OR__
#define __CONTROL_OR__

#include <opencv2/features2d/features2d.hpp>

#include "visualWords.h"
#include "imageDB.h"
#include "orException.h"

namespace cvar{

namespace or{

class controlOR
{
public:
	controlOR(void);
	~controlOR(void);

	// Visual Words + Feature Matching functions
	int addFeaturesForVW(const cv::Mat& src_img);
	int releaseFeaturesForVW();
	int createVisualWords(int cluster_number = 0);
	bool loadVisualWords(const std::string& filename);
	bool loadVisualWordsBinary(const std::string& filename, const std::string& idxname);
	bool saveVisualWords(const std::string& filename) const;
	bool saveVisualWordsBinary(const std::string& filename, const std::string& idxname) const;
	int releaseVisualWords();
	void setRecogThreshold(float th);
	float getRecogThreshold() const;
	std::string getDetectorType() const{return detectorType;};
	bool setDetectorType(const std::string& detector_type);
	std::string getDescriptorType() const{return descriptorType;};
	bool setDescriptorType(const std::string& descriptor_type);

	// ObjectDB functions
	int loadObjectDB(const std::string filename);
	void read(cv::FileNode& fn);
	int saveObjectDB(const std::string filename) const;
	void write(cv::FileStorage& fs, std::string name) const;
	void releaseObjectDB();

	// operateFunctions
	int registImage(const cv::Mat& src_img, int img_id);
	int removeImage(int image_id);
	std::vector<resultInfo> queryImage(const cv::Mat& src_img, int result_num=1);

	imageDB	image_db;

private:
	// internal function
//	int getFeatureIdVec(const std::vector<float>& desc_vec, std::vector<int>& id_list);
	int getFeatureIdVec(const cv::Mat& desc_vec, std::vector<int>& id_list);
//	int calcFeatureData(std::vector<cv::KeyPoint> kp_vec, std::vector<float> desc_vec, int img_id, std::vector<featureData>& img_data);

	// feature detection
	int initializeFeatureDetector();	// initialize feature detector, and obtain number of descriptor dimention
	int extractFeatures(const cv::Mat& src_img, std::vector<cv::KeyPoint>& kpt, cv::Mat& descriptor) const;
//	int extractFeatures(const cv::Mat& src_img, std::vector<cv::KeyPoint>& kpt, std::vector<float>& descriptor);
	int releaseFeatureDetector();

//	void* featureDetector;
	std::string detectorType;	// feature detector name
	std::string descriptorType;	// feature descriptor name
//	std::string matcherType;	// feature matcher name
	cv::Ptr<cv::FeatureDetector>	feature_detector;	// feature detector
	cv::Ptr<cv::DescriptorExtractor> descriptor_extractor;	// descriptor extractor
//	cv::Ptr<cv::DescriptorMatcher>	descriptor_matcher;	// descriptor matcher
//	int feature_dimention;	// dimention of descriptor
	int voteNum;	// number of knn & vote (to keypoint)
	visualWords	visual_words;
};
};
};
#endif