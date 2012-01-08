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
#include "visualWords.h"
#include "commonCvFunctions.h"
#include "orException.h"
#include <opencv2/flann/flann.hpp>
#include <fstream>

using namespace std;
using namespace cv;
using namespace cv::flann;
using namespace cvar;
using namespace cvar::or;

visualWords::visualWords(void)
{
//	searchDB = 0;
	this->matcherType = "FlannBased";
	this->descriptor_matcher = DescriptorMatcher::create(matcherType);
	voteNum = 1;
	radius = 0.2;
}

visualWords::~visualWords(void)
{
	release();
}

void visualWords::release()
{
//	addedFeatures.clear();
//	releaseSearchDB();
//	DBdata.release();
	descriptor_matcher->clear();
}

bool visualWords::isReady()
{
	if(descriptor_matcher.empty() || descriptor_matcher->empty()){
		return false;
	}
	return true;
}

//void visualWords::setFeatureDimention(int size)
//{
//	this->feature_dim = size;
//}

int visualWords::getVisualWordNum() const
{
	int num = 0;
	vector<Mat> mat_vec = descriptor_matcher->getTrainDescriptors();
	vector<Mat>::iterator itr, it_end;
	it_end = mat_vec.end();
	for(itr = mat_vec.begin(); itr != it_end; itr++){
		num += itr->rows;
	}
//	return this->DBdata.rows;
	return num;
}

void visualWords::setVoteNum(int vote_num)
{
	voteNum = vote_num;
}

//int visualWords::addFeatures(vector<float>& feature)
void visualWords::addFeatures(const cv::Mat& feature)
{
	vector<Mat> feat_vec;
	feat_vec.push_back(feature);
	descriptor_matcher->add(feat_vec);
}

void visualWords::createVW(int cluster_num)
{
	try{
		// if cluster_num <= 0 then feature vector index is created without clustering. 
		if(cluster_num > 0){
			///////// kemans clustering to create visual words ///////////////
			vector<Mat> feature_vec = descriptor_matcher->getTrainDescriptors();
			if(!feature_vec.empty() && feature_vec[0].type() == CV_32FC1){
				Mat featureMat;
				convertFeatureMat(feature_vec, featureMat);

				Mat label(featureMat.rows, 1, CV_32SC1);
				Mat centroid(cluster_num, featureMat.cols,featureMat.type());

				double ret = kmeans(featureMat, cluster_num, label, TermCriteria(TermCriteria::MAX_ITER, 10, 0.5),2,KMEANS_PP_CENTERS, centroid);
		
				descriptor_matcher->clear();
				descriptor_matcher->add(centroid);
			}
		}
		descriptor_matcher->train();
	}
	catch(cv::Exception e){
		orCvException ce;
		ce.setFunctionName("visualWords::createVW");
		ce.setCvExceptionClass(e);
		throw ce;
	}
	catch(std::exception e2){
		throw e2;
	}
}


void visualWords::convertFeatureMat(const vector<cv::Mat>& feature, cv::Mat& featureMat)
{
	if(feature.empty()){
		orArgException e("Empty Input Feature");
		e.setFunctionName("visualWords::convertFeatureMat()");
		throw e;
	}

	//int row = feature.size() / feature_dim;
	int row = 0;
	int feature_dim = feature[0].cols;
	vector<cv::Mat>::const_iterator itr;
	for(itr=feature.begin(); itr!=feature.end(); itr++){
		row += itr->rows;
		if(feature_dim != itr->cols){
			throw orArgException("Illegal Size of Mat");
		}
	}
	featureMat.create(row, feature_dim, feature[0].type());

	unsigned char* feature_pt = featureMat.data;
	int num_bytes = featureMat.elemSize();
	int feature_size = feature.size();
	int total;
	for(int i=0; i<feature_size; i++){
		total = feature[i].total();
		memcpy(feature_pt, feature[i].data, total * num_bytes);
		feature_pt += total;
	}
}

///////// load & save /////////////
bool visualWords::saveIndex(const string& filename) const
{
	try{
		cv::FileStorage fs(filename, FileStorage::WRITE);
		writeIndex(fs, "VW_Index");
		return true;
	}
	catch(std::exception e){
//		std::cerr << e.what() << std::endl;
		return false;
	}
}


void visualWords::writeIndex(cv::FileStorage& FS, const std::string& name) const
{
	try{
		cv::WriteStructContext ws(FS, name, CV_NODE_MAP);
		cv::write(FS, "matcherType", matcherType);
		descriptor_matcher->write(FS);
	}
	catch(cv::Exception e)
	{
		throw e;
	}
}

bool visualWords::loadIndex(const string& filename)
{
	try{
		cv::FileStorage fs(filename, FileStorage::READ);
		readIndex(fs["VW_Index"]); 
		return true;
	}
	catch(std::exception e){
//		std::cerr << e.what() << std::endl;
		return false;
	}
}


void visualWords::readIndex(const cv::FileNode& FN)
{
	try{
		cv::read(FN["matcherType"], this->matcherType, "");
		this->descriptor_matcher = DescriptorMatcher::create(matcherType);	
		descriptor_matcher->read(FN);
	}
	catch(cv::Exception e){
		throw e;
	}
}


bool visualWords::save(const string& filename) const
{
	try{
		FileStorage	FS(filename, FileStorage::WRITE);
		this->writeIndex(FS, "index");
		this->write(FS, "visualWords");
		return true;
	}
	catch(std::exception e)
	{
		return false;
	}
}


bool visualWords::saveBinary(const string& filename, const string& idx_filename) const
{
	try{
		// save index data;
		this->saveIndex(idx_filename);
		save_vw_binary(filename);
		return true;
	}
	catch(std::exception e)
	{
		return false;
	}
}


bool visualWords::load(const string& filename)
{
	try{
		FileStorage cvfs(filename, FileStorage::READ);
		this->readIndex(cvfs["index"]);
		this->read(cvfs["visualWords"]);
		return true;
	}
	catch(cv::Exception e){
		orCvException orce;
		orce.setFunctionName("visualWords::load()");
		orce.setCvExceptionClass(e);
		return false;
	}
	catch(std::exception e2){
		return false;
	}
}


bool visualWords::loadBinary(const string& filename, const string& idx_filename)
{
	try{
		bool ret;
		ret = loadIndex(idx_filename);
		if(!ret){
			return ret;
		}

		ret = load_vw_binary(filename);
		return ret;
	}
	catch(cv::Exception e){
		orCvException orce;
		orce.setFunctionName("visualWords::load()");
		orce.setCvExceptionClass(e);
		return false;
	}
	catch(std::exception e2){
		return false;
	}
}


bool visualWords::save_vw_binary(const string& filename) const
{
	ofstream ofs(filename, ios::binary);
	if(!ofs.is_open()){
//		throw new orArgException("failed to open " + filename);
		return false;
	}
//	int type = DBdata.type();
	ofs.write((const char*)"vw", 2);
	ofs.write((const char*)(&version), sizeof(int));
	ofs.write((const char*)(&radius), sizeof(radius));
	vector<Mat> train_desc = descriptor_matcher->getTrainDescriptors();
	int num = train_desc.size();
	ofs.write((const char*)(&num), sizeof(num));
	vector<Mat>::iterator itr;
	for(itr = train_desc.begin(); itr != train_desc.end(); itr++){
		writeMatBinary(ofs, *itr);
	}

	return true;
}


bool visualWords::load_vw_binary(const string& filename)
{
	ifstream ifs(filename, ios::binary);
	if(!ifs.is_open()){
//		throw new orArgException("failed to open " + filename);
		std::cerr << "failed to open " << filename << std::endl;
		return false;
	}
	char header[2];
	ifs.read(header, sizeof(header));
	if(memcmp(header, (const void*)"vw", 2)!=0)
	{
//		throw new orArgException("wrong format file " + filename);
		std::cerr << "wrong format file " << filename << std::endl;
		return false;
	}
	
	int ver;
	ifs.read((char*)(&ver), sizeof(int));
	if(ver != version)
	{
//		throw new orArgException("wrong version file: " + filename);
		std::cerr << "wrong version file: " << filename << std::endl;
		return false;
	}
	
	ifs.read((char*)(&radius), sizeof(radius));

	int num;
	ifs.read((char*)(&num), sizeof(num));
	descriptor_matcher->clear();
	for(int i=0; i<num; i++){
		Mat DBdata;
		readMatBinary(ifs, DBdata);
		vector<Mat> in_mat_vec;
		in_mat_vec.push_back(DBdata);
		descriptor_matcher->add(in_mat_vec);
	}
	
	return true;
}


//int visualWords::write(CvFileStorage* cvfs, const char* name)
void visualWords::write(FileStorage& fs, const string& name) const
{
	WriteStructContext ws(fs, name, CV_NODE_MAP);
//	cv::write(fs, "feature_dim", feature_dim);
	cv::write(fs, "version", version);
	cv::write(fs, "radius", radius);

	vector<Mat> train_desc = descriptor_matcher->getTrainDescriptors();
	WriteStructContext ws2(fs, "TrainDescriptors", CV_NODE_SEQ);
	vector<Mat>::iterator itr;
	for(itr = train_desc.begin(); itr != train_desc.end(); itr++)
	{
		cv::write(fs, std::string(), *itr);
	}
}


//int visualWords::read(CvFileStorage* cvfs, CvFileNode *node)
void visualWords::read(const FileNode& node)
{
//	int	i;

	try{
//		feature_dim = node["feature_dim"];
		int ver = node["version"];
		if(ver != version)
		{
			throw new orArgException("wrong version file");
		}
		radius = node["radius"];

		descriptor_matcher->clear();
		cv::FileNode data_node = node["TrainDescriptors"];
		cv::FileNodeIterator itr;
		cv::FileNodeIterator it_end = data_node.end();
		for(itr=data_node.begin(); itr != it_end; itr++){
			cv::Mat DBdata;
			cv::read(*itr, DBdata);

			vector<Mat> mat_vec;
			mat_vec.push_back(DBdata);
			descriptor_matcher->add(mat_vec);
		}
	}
	catch(cv::Exception e){
		orCvException orce;
		orce.setFunctionName("visualWords::read()");
		orce.setCvExceptionClass(e);
		throw orce;
	}
	catch(std::exception e2){
		throw e2;
	}
}

//////// Implement Nearest Neighbor Function ///////////////
// Here we use FLANN
/*
int visualWords::createSearchDB(const Mat& featureMat)
{

//	DBdata = featureMat;
//	searchDB = new cv::flann::Index(DBdata,cv::flann::KDTreeIndexParams(4));

	// KmeanTree
	struct cv::flann::KMeansIndexParams kmip(16,11,FLANN_CENTERS_KMEANSPP,0.2);
//	kmip.branching = 8;
//	kmip.branching = 16;
//	kmip.branching = 32;
//	kmip.iterations = -1;
//	kmip.iterations = 11;
//	kmip.centers_init = CENTERS_KMEANSPP;
//	kmip.cb_index = (float)0.2;

	searchDB = new cv::flann::Index(featureMat, kmip);

	return 0;
}*/


Mat visualWords::querySearchDB(const Mat& features)
{
	int knn_size = voteNum + 1;
	int size = features.size().height;

	try{
		Mat indices(size, knn_size, CV_32SC1);
//		Mat dists(size, knn_size, CV_32FC1);

		// search nearest descriptor in database
		vector<vector<DMatch> > match_idx;
		descriptor_matcher->knnMatch(features, match_idx, knn_size);
		for(int y=0; y<size; y++){
			for(int x=0; x<voteNum; x++){
				DMatch d_match(match_idx[y][x]);
				if(d_match.distance >= radius){
					indices.at<int>(y,x) = -1;
				}
				else{
					indices.at<int>(y,x) = d_match.trainIdx;
				}
			}
		}

		return indices;
	}
	catch(cv::Exception e){
		orCvException	e2;
		e2.cv_e = e;
		e2.setFunctionName("visualWords::querySearchDB()");
		
		throw e2;
	}
	return Mat();
}

