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
#ifndef __IMAGE_DB__
#define __IMAGE_DB__

#include <opencv2/features2d/features2d.hpp>

namespace cvar{
namespace or{

typedef struct{
	int in_feat_i;
	int keypoint_id;
}featureVote;

typedef struct{
	int keypoint_id;
	int img_id;
}featureInfo;

typedef struct{
	int feature_num;
	cv::Size img_size;
}imageInfo;

typedef struct{
	int	img_id;
	float	probability;
	int	matched_num;
	cv::Mat	pose_mat;
	cv::Size img_size;
	std::vector<cv::Point2f> object_position;
}resultInfo;

class imageDB
{
public:
	imageDB(void);
	~imageDB(void);

	// Operational Functions
	int registImageFeatures(int img_id, cv::Size img_size, std::vector<cv::KeyPoint> kp_vec, std::vector<int> id_list);	// Add image feature id to DB
	std::vector<resultInfo> retrieveImageId(const std::vector<cv::KeyPoint>& kp_vec, const std::vector<int>& id_list, cv::Size img_size, const int visual_word_num, int result_num = 1);	// Add image feature id to DB
	int removeImageId(int img_id);	// remove image in DB
	void setThreshold(float th);
	float getThreshold() const;
	imageInfo getImageInfo(int img_id);
//	void setVisualWordNum(int vw);
//	int getVisualWordNum();
	void setVoteNum(int vote_num);
	void release();

	// Load & Save
	int load(const std::string& filename);
	int save(const std::string& filename) const;
	int read(const cv::FileStorage& cvfs, const cv::FileNode& node);
	int write(cv::FileStorage& cvfs, const std::string& name) const;

private:
	int readFeatureKptMap(const cv::FileStorage& cvfs, const cv::FileNode& node);
	int writeFeatureKptMap(cv::FileStorage& cvfs, const std::string& name) const;
	int readKeyMap(const cv::FileStorage& cvfs, const cv::FileNode& node);
	int writeKeyMap(cv::FileStorage& cvfs, const std::string& name) const;
	int readImgInfoMap(const cv::FileStorage& cvfs, const cv::FileNode& node);
	int writeImgInfoMap(cv::FileStorage& cvfs, const std::string& name) const;

	// Internal Functions
	int getVacantKptId();
	void voteInputFeatures(const std::vector<cv::KeyPoint>& kp_vec, const std::vector<int>& id_list);
	std::vector<resultInfo> calcMatchCountResult(const std::vector<cv::KeyPoint>& kp_vec, int visual_word_num);
	std::vector<resultInfo> calcGeometryConsistentResult(const std::vector<cv::KeyPoint>& kp_vec, const std::vector<resultInfo>& tmp_result_vec, cv::Size img_size, int result_num);
	void calcPointPair(const std::vector<cv::KeyPoint>& kp_vec, std::vector<featureVote>& vote_vec, std::vector<cv::Point2f>& query_pts, std::vector<cv::Point2f>& reg_pts);
	float calcIntegBinDistribution(int in_feats_num, int match_num, float Pp);
	int countAffineInlier(std::vector<cv::Point2f>& src_pts, std::vector<cv::Point2f>& dest_pts, cv::Mat& affMat, double dist_threthold);
//	cv::Mat transKptPointToMat(std::vector<cv::KeyPoint>& kpt_vec);
//	cv::Mat transKptScaleAngleToMat(cv::vector<cv::KeyPoint>& kpt_vec);
//	double calcDistThreshold(cv::Mat& affMat, int img_id);
//	void calcKeypointPair(const std::vector<cv::KeyPoint>& kp_vec, std::vector<featureVote>& vote_table, std::vector<cv::KeyPoint>& query_kpt, std::vector<cv::KeyPoint>& reg_kpt);

	void clearVoteTables();
	void releaseImgVoteMap();

private:
	std::multimap<int,featureInfo>	feature_KPT_map;	// Search keypoint_id and img_id the feature_id to key
	std::map<int,cv::KeyPoint>	keypoint_map;	// Search KeyPoint to key keypoint_id
	std::map<int,imageInfo>	imgInfo_map;	// Search feature_num to key img_id
	std::map<int,std::vector<featureVote>*>	imgVote_map;	// Search voteTable to key img_id

//	int visual_word_num;	// The number of visual word
	int imageNum;	// Registration number of images
	int featureNum;	// Registered feature points
	int voteNum;
	float threshold;	// Feature points match threshold (0 - 1)
	float geo_threshold;
	double dist_diff_threshold;	// The tolerance on the position of the corresponding point (the ratio of the image size: 0 - 1)
//	double angle_diff_threshold;	// Tolerance 180 degrees ~ about the angle of the corresponding point (0 - 1)
//	double scale_diff_threshold;	// Tolerance on the scale of the corresponding point (the ratio of the size:> 1)
};

};
};
#endif