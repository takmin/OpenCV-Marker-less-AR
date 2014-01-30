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
#include "controlOR.h"
#include <opencv2/nonfree/nonfree.hpp>
#include <sstream>

using namespace std;
using namespace cv;
using namespace cvar;
using namespace cvar::or;
//#include <iostream>

controlOR::controlOR(void)
{
//	featureDetector = 0;
	detectorType = "SURF";
	descriptorType = "SURF";
	feature_detector = 0;
	descriptor_extractor = 0;
	initializeFeatureDetector();
//	visual_words.setFeatureDimention(feature_dimention);
	voteNum = 1;
	visual_words.setVoteNum(voteNum);
	image_db.setVoteNum(voteNum);
}

controlOR::~controlOR(void)
{
	releaseFeatureDetector();
}

/////// create visual word ///////
int controlOR::addFeaturesForVW(const Mat& src_img)
{
	vector<KeyPoint> kp_vec;
//	vector<float> desc_vec;
	cv::Mat desc_vec;

	extractFeatures(src_img, kp_vec, desc_vec);
	visual_words.addFeatures(desc_vec);

//	kp_vec.clear();
//	desc_vec.clear();

	return 0;
}


int controlOR::createVisualWords(int cluster_number)
{
	visual_words.createVW(cluster_number);

	return 0;
}


int controlOR::registImage(const cv::Mat& src_img, int img_id)
{
	vector<KeyPoint> kp_vec;
//	vector<float> desc_vec;
	cv::Mat desc_vec;

	if(img_id <=0)	return -1;

	try{
		extractFeatures(src_img, kp_vec, desc_vec);

		vector<int> id_list;
		getFeatureIdVec(desc_vec, id_list);
//		vector<featureData> img_data;
//		calcFeatureData(kp_vec,desc_vec,img_id, img_data);

		int ret = image_db.registImageFeatures(img_id, src_img.size(), kp_vec, id_list);

		if(ret < 0)
			return -1;
	}
	catch(cv::Exception e){
		throw e;
	}
	catch(std::exception e2){
		throw e2;
	}

	return 0;
}


int controlOR::removeImage(int img_id)
{
	return image_db.removeImageId(img_id);
}


void controlOR::releaseObjectDB()
{
	image_db.release();
}


vector<resultInfo> controlOR::queryImage(const Mat& src_img, int result_num)
{
	vector<resultInfo>	retInfo;

	vector<KeyPoint> kp_vec;
//	vector<float> desc_vec;
	cv::Mat desc_vec;

	try{
		extractFeatures(src_img, kp_vec, desc_vec);

		vector<int> id_list;
		int ret = getFeatureIdVec(desc_vec, id_list);
		if(ret < 0)
			return retInfo;

		retInfo = image_db.retrieveImageId(kp_vec, id_list, src_img.size(), visual_words.getVisualWordNum(), result_num);

		kp_vec.clear();
		id_list.clear();
//		desc_vec.clear();
	}
	catch(cv::Exception e)
	{
		throw e;
	}
	catch(std::exception e2)
	{
		throw e2;
	}

	return retInfo;
}


bool controlOR::setDetectorType(const std::string& detector_type)
{
	cv::Ptr<cv::FeatureDetector> tmp_detector;
	try{
		tmp_detector = FeatureDetector::create(detector_type);
		if(tmp_detector.empty()){
			return false;
		}
	}
	catch(cv::Exception e){
		return false;
	}
	this->feature_detector = tmp_detector;
	this->detectorType = detector_type;

	return true;
}


bool controlOR::setDescriptorType(const std::string& descriptor_type)
{
	cv::Ptr<cv::DescriptorExtractor> tmp_descriptor;
	try{
		tmp_descriptor = DescriptorExtractor::create(descriptor_type);
		if(tmp_descriptor.empty()){
			return false;
		}
	}
	catch(cv::Exception e){
		return false;
	}
	this->descriptor_extractor = tmp_descriptor;
	this->descriptorType = descriptor_type;

	return true;
}


//int controlOR::getFeatureIdVec(const vector<float>& desc_vec, vector<int>& id_list)
int controlOR::getFeatureIdVec(const cv::Mat& desc_vec, vector<int>& id_list)
{
	if(desc_vec.empty()){
		return -1;
	}
//	vector<float> desc_point(feature_dimention);

	try{
		// convert feature vector to Mat
//		Mat desc_points = visual_words.convertFeatureMat(desc_vec);
	
		// query and obtain feature point indices
		Mat indices = visual_words.querySearchDB(desc_vec);
//		Mat indices = visual_words.querySearchDB(desc_points);

		int size = desc_vec.rows;
//		int size = desc_points.rows;

		int i,j;
		for(i=0; i<size; i++){
			for(j=0; j<voteNum; j++){
				id_list.push_back(indices.at<int>(i,j));
			}
		}
	}
	catch(std::exception e){
		throw e;
	}

	return 0;
}


bool controlOR::saveVisualWords(const string& filename) const
{
	bool ret = visual_words.save(filename);
	return ret;
}


bool controlOR::saveVisualWordsBinary(const string& filename, const string& idxname) const
{
	bool ret = visual_words.saveBinary(filename, idxname);
	return ret;
}


bool controlOR::loadVisualWords(const string& filename)
{
	try{
		bool ret = visual_words.load(filename);
		return ret;
	}
	catch(std::exception e)
	{
//		throw e;
		return false;
	}
}


bool controlOR::loadVisualWordsBinary(const string& filename, const string& idxname)
{
	try{
		bool ret = visual_words.loadBinary(filename, idxname);
		return ret;
	}
	catch(std::exception e)
	{
//		throw e;
		return false;
	}
}


int controlOR::loadObjectDB(const string filename)
{
	try{
//		image_db.load(filename.c_str());
		FileStorage cvfs(filename,FileStorage::READ);
		FileNode cvfn(cvfs.fs, NULL);

		FileNode cvfn1 = cvfn["controlOR"];
		read(cvfn1);

		FileNode cvfn2 = cvfn["imageDB"];
		image_db.read(cvfs, cvfn2);

		visual_words.setVoteNum(voteNum);
		image_db.setVoteNum(voteNum);
	}
	catch(std::exception e2){
		throw e2;
	}

	return 0;
}


void controlOR::read(FileNode& cvfn)
{
	voteNum = cvfn["voteNum"];
	detectorType = cvfn["detectorType"];
	descriptorType = cvfn["descriptorType"];
	feature_detector->create(detectorType);
	descriptor_extractor->create(descriptorType);
}


int controlOR::saveObjectDB(const string filename) const
{
//	image_db.save(filename.c_str());
	FileStorage cvfs(filename,FileStorage::WRITE);
	write(cvfs,"controlOR");
	image_db.write(cvfs, "imageDB");

	return 0;
}


void controlOR::write(FileStorage& fs, string name) const
{
	WriteStructContext ws(fs, name, CV_NODE_MAP);
	cv::write(fs, "voteNum", voteNum);
	cv::write(fs, "detectorType", detectorType);
	cv::write(fs, "descriptorType", descriptorType);
}

/////// Feature Detector ////////

// initialize
int controlOR::initializeFeatureDetector()
{
//	if(featureDetector)
	if(feature_detector || descriptor_extractor)
		releaseFeatureDetector();
//	SURF* surf_pt = new SURF(500,4,2,true);
//	featureDetector = surf_pt;
//	feature_dimention = 128;
	cv::initModule_nonfree();
	feature_detector = FeatureDetector::create(detectorType);	// create feature detector
	descriptor_extractor = DescriptorExtractor::create(descriptorType);	// create descriptor extractor

	return 0;
}

//int controlOR::extractFeatures(const Mat& src_img, vector<KeyPoint>& kpt, vector<float>& descriptor)
int controlOR::extractFeatures(const cv::Mat& src_img, cv::vector<cv::KeyPoint>& kpt, cv::Mat& descriptor) const
{
	// extract freak
	try{
		// keypoints detection from a query image
		feature_detector->detect(src_img, kpt);
		// descriptor extraction
		descriptor_extractor->compute(src_img, kpt, descriptor);

//		(*(SURF*)featureDetector)(src_img, Mat(), kpt, descriptor);
	//	cout << ",kpt:" << kpt.size() << ",";
	}
	catch(cv::Exception e)
	{
		orCvException or_e;
		or_e.setCvExceptionClass(e);
		or_e.setFunctionName("SURF()");

		throw or_e;
	}
	
	return 0;
}

int controlOR::releaseFeatureDetector()
{
//	delete (SURF*)featureDetector;
//	featureDetector = 0;
	feature_detector.release();
	feature_detector = 0;
	descriptor_extractor.release();
	descriptor_extractor = 0;

	return 0;
}


void controlOR::setRecogThreshold(float th)
{
	image_db.setThreshold(th);
}


float controlOR::getRecogThreshold() const
{
	return	image_db.getThreshold();
}
