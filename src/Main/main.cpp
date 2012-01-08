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
//#define CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

#include <iostream>
#include <string>
#include <fstream>
#include <time.h>

#include "commonCvFunctions.h"
#include <opencv2/highgui/highgui.hpp>
#include "cameraCalibration.h"
#include "guiAR.h"
#ifdef _DEBUG
#include <gtest/gtest.h>
#endif

using namespace std;
using namespace cv;
using namespace cvar;
using namespace cvar::or;

int main(int argc, char * argv[])
{
#ifdef _DEBUG
	::testing::InitGoogleTest(&argc, argv);
#endif

//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	controlOR	ctrlOR;

	setControlOR(ctrlOR);

	if(argc > 1){
		string arg(argv[1]);
		setARConfigFile(arg);
		startGUI(argc, argv);
	}
	else{
	bool exitflag1 = false;
	string opt;
	clock_t start_time, end_time;

	while(!exitflag1){
		cout << "command: ";
		cin >> opt;
		// help
		if(opt=="h" || opt=="?" || opt=="help"){
			cout << "set_detector" << endl;
			cout << "set_descriptor" << endl;
			cout << "get_detector" << endl;
			cout << "get_descriptor" << endl;
			cout << "create_vw" << endl;
			cout << "load_vw" << endl;
			cout << "save_vw" << endl;
			cout << "load_vw_bin" << endl;
			cout << "save_vw_bin" << endl;
			cout << "registf" << endl;
			cout << "regist" << endl;
			cout << "clear" << endl;
			cout << "remove" << endl;
			cout << "threshold" << endl;
			cout << "query" << endl;
			cout << "queryf" << endl;
			cout << "load_objectDB" << endl;
			cout << "save_objectDB" << endl;
			cout << "get_camera_images" << endl;	
			cout << "camera_calibration" << endl;
			cout << "ar_start" << endl;
			cout << "exit" << endl;
		}
#ifdef _DEBUG
		else if(opt=="run_test"){
			RUN_ALL_TESTS();
		}
#endif
		else if(opt=="set_detector"){
			cout << "detector name: ";
			string detector_type;
			cin >> detector_type;
			bool ret = ctrlOR.setDetectorType(detector_type);
			if(ret){
				cout << "detector " << detector_type << " successfully loaded." << endl;
			}
			else{
				cout << "detector " << detector_type << " load fail." << endl;
			}
		}
		else if(opt=="set_descriptor"){
			cout << "descriptor name: ";
			string descriptor_type;
			cin >> descriptor_type;
			bool ret = ctrlOR.setDescriptorType(descriptor_type);
			if(ret){
				cout << "descriptor " << descriptor_type << " successfully loaded." << endl;
			}
			else{
				cout << "descriptor " << descriptor_type << " load fail." << endl;
			}
		}
		else if(opt=="get_detector"){
			std::cout << "Detector: " << ctrlOR.getDetectorType() << std::endl;
		}
		else if(opt=="get_descriptor"){
			std::cout << "Descriptor: " << ctrlOR.getDescriptorType() << std::endl;
		}
		// Create Visual Word
		else if(opt=="create_vw"){
			string opt2;
			int cls_num = 0;

			cout << "img file list: ";
			cin >> opt2;

			ifstream ifs(opt2.c_str());
			if(ifs!=NULL){
				bool exitflag2 = false;
				int count = 0;
				string buf;
				while(ifs && getline(ifs, buf)){
					cout << buf << "...";
//					studyimg = cvLoadImage(buf.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
					Mat studyimg = imread(buf, 0);
					start_time = clock();
					ctrlOR.addFeaturesForVW(studyimg);
					end_time = clock();
					cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
				}
//				cout << "Start Clustering....";
				start_time = clock();
				ctrlOR.createVisualWords(cls_num);
				end_time = clock();
				cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
				ifs.close();
			}
			else{
				cerr << "ERROR: Could not open " << opt2 << endl;
			}
		}
		else if(opt=="save_vw"){
			string filename, idxname;
			cout << "feature dictionary file: ";
			cin >> filename;
			cout << "save..";
			start_time = clock();
			ctrlOR.saveVisualWords(filename);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
		}
		else if(opt=="load_vw"){
			string filename, idxname;
			cout << "feature dictionary file: ";
			cin >> filename;
			cout << "load..";
			start_time = clock();
			ctrlOR.loadVisualWords(filename);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
		}
		else if(opt=="save_vw_bin"){
			string filename, idxname;
			cout << "feature dictionary file: ";
			cin >> filename;
			cout << "index file name: ";
			cin >> idxname;
			cout << "save..";
			start_time = clock();
			ctrlOR.saveVisualWordsBinary(filename,idxname);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
		}
		else if(opt=="load_vw_bin"){
			string filename, idxname;
			cout << "feature dictionary file: ";
			cin >> filename;
			cout << "index file name: ";
			cin >> idxname;
			cout << "load..";
			start_time = clock();
			ctrlOR.loadVisualWordsBinary(filename,idxname);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
		}
		else if(opt=="registf"){
			string regist_list;
			cout << "regist list file: ";
			cin >> regist_list;

//			char buf[256];
			string buf;
			ifstream ifs(regist_list.c_str());
			if(ifs!=NULL){
//			if(ifs.is_open()){
				bool exitflag2 = false;
				int count = 0;
				int id = 0;
				string::size_type	index;
				string imgfile;
				string id_str;
//				while(!ifs.eof()){
				while(ifs && getline(ifs, buf)){
//					ifs.getline(buf,sizeof(buf));
					index = buf.find(",");
					if(index==string::npos){
						id++;
						imgfile = buf;
					}
					else{
						id_str = buf.substr(0,index);
						id = atoi(id_str.c_str());
						imgfile = buf.substr(index+1);
					}
					cout << id << ":" << imgfile << "...";
//					IplImage* img = cvLoadImage(imgfile, CV_LOAD_IMAGE_GRAYSCALE);
//					Mat studyimg(img);
					start_time = clock();
					Mat studyimg = imread(imgfile, 0);
					ctrlOR.registImage(studyimg, id);
					end_time = clock();
					cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
				}
				ifs.close();
			}
		}
		else if(opt=="regist"){
			string filename;
			int id;
			cout << "image file: ";
			cin >> filename;
			cout << "id: ";
			cin >> id;

			start_time = clock();
			Mat studyimg = imread(filename, 0);
			int ret = ctrlOR.registImage(studyimg, id);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
			if(ret < 0)
				cout << "regist fail" << endl;
		}
		else if(opt=="clear"){
			ctrlOR.releaseObjectDB();
			cout << "DB cleared!" << endl;
		}
		else if(opt=="remove"){
			string filename;
			int id;
			cout << "image id: ";
			cin >> id;

			start_time = clock();
			int ret = ctrlOR.removeImage(id);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
			if(ret < 0)
				cout << "delete fail" << endl;
		}
		else if(opt=="threshold"){
			float th;
			cout << "threshold(0-1): ";
			cin >> th;
			ctrlOR.setRecogThreshold(th);
		}
		else if(opt=="query"){
			string filename;
			cout << "query image file: ";
			cin >> filename;

			try{
				Mat queryimg = imread(filename, 0);
				start_time = clock();
				vector<resultInfo>	result_vec = ctrlOR.queryImage(queryimg);
				end_time = clock();
				cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;

				if(!result_vec.empty()){
					vector<resultInfo>::iterator itr = result_vec.begin();
					while(itr != result_vec.end()){
						cout << itr->img_id << ":" << itr->probability << "," << itr->matched_num << endl;
						Mat* pose_mat = &(itr->pose_mat);
						for(int i=0; i< pose_mat->rows; i++){
							for(int j=0; j<pose_mat->cols; j++){
								cout << pose_mat->at<double>(i,j) << ",";
							}
							cout << endl;
						}
						itr++;
					}

					vector<Point2f> result_pt = result_vec[0].object_position;
//					vector<Point2f> result_pt = ctrlOR.getObjectPosition(result_vec[0]);

					Scalar val(255);
					
					line(queryimg,result_pt[3],result_pt[0],val);
					for(int i=0; i<3; i++){
						line(queryimg,result_pt[i],result_pt[i+1],val);
					}

					namedWindow("result",CV_WINDOW_AUTOSIZE);
					imshow("result", queryimg);
					waitKey(0);
				}
/*				Mat posMat = result_vec[0].pose_mat;
				vector<Point> src_vec, dest_vec;
				int size = posMat.rows;
				Point pt1, pt2;
				for(int i=0; i<size; i++){
					pt1.x = posMat.at<float>(i,0);
					pt1.y = posMat.at<float>(i,1);
					pt2.x = posMat.at<float>(i,2);
					pt2.y = posMat.at<float>(i,3);
					src_vec.push_back(pt1);
					dest_vec.push_back(pt2);
				}

				Mat regimg = imread("img\\t4.jpg", 0);
				createMatchingImage(regimg, queryimg, src_vec, dest_vec);*/
			}
			catch(cv::Exception e){
				cerr << e.err;
			}
			catch(std::exception e2){
				cerr << e2.what() << endl;
			}
		}
		else if(opt=="queryf"){
			string img_list, result_file;
			cout << "img list file: ";
			cin >> img_list;
			cout << "result file: ";
			cin >> result_file;

			vector<resultInfo>	result_vec;

//			char buf[256];
			string buf;
			ifstream ifs(img_list.c_str());
			ofstream ofs(result_file.c_str());
			if(ifs!=NULL){
//			if(ifs.is_open()){
				int id = 1;
//				while(!ifs.eof()){
				while(ifs && getline(ifs, buf)){
//					ifs.getline(buf,sizeof(buf));
					cout << id << ":" << buf << "...";
//					IplImage* img = cvLoadImage(buf, CV_LOAD_IMAGE_GRAYSCALE);
//					Mat studyimg(img);
					try{
						start_time = clock();
						Mat queryimg = imread(buf, 0);
						result_vec = ctrlOR.queryImage(queryimg);
						end_time = clock();
						ofs << buf << ",";
						if(result_vec.empty()){
							ofs << "-1,";
							cout << "-1,";
						}
						else{
							ofs << result_vec[0].img_id << ",";
							cout << result_vec[0].img_id << ",";
						}
						ofs << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
						cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
						result_vec.clear();
					}
					catch(std::exception e){
						ofs << "-1," << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
						cout << "-1," << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
					}

					id++;
				}
				ifs.close();
			}
			ofs.close();
		}
		else if(opt=="save_objectDB"){
			string filename;
			cout << "save file name: ";
			cin >> filename;

			start_time = clock();
			ctrlOR.saveObjectDB(filename);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
		}
		else if(opt=="load_objectDB"){
			string filename;
			cout << "load file name: ";
			cin >> filename;

			start_time = clock();
			ctrlOR.loadObjectDB(filename);
			end_time = clock();
			cout << ((double)end_time - start_time)/CLOCKS_PER_SEC << endl;
		}
		else if(opt=="get_camera_images"){
			string opt2;
			cout << "number of images: ";
			cin >> opt2;
			int img_num = atoi(opt2.c_str());
			cout << "frame interval: ";
			cin >> opt2;
			int interv = atoi(opt2.c_str());

			string dir_name;
			cout << "save directory: ";
			cin >> dir_name;

			string file_name;
			cout << "file header: ";
			cin >> file_name;

			cv::VideoCapture	capture( 0 );

			if( !capture.isOpened() ) {
				cout << "Failed to Open Camera" << std::endl;
			}
			else{
				// create window
				const string		windowNameCam = "Camera Image";
				namedWindow( windowNameCam, CV_WINDOW_AUTOSIZE );


				// for capture image
				Mat	frame;

				// main loop
				cout << "Capture start with \"s\" button." << endl;
				do {
					// image capture from camera
					capture >> frame;
					// show caputred image
					imshow( windowNameCam, frame );
				} while( cv::waitKey( 1 ) != 's' );

				cout << "Capture started!" << endl;
				cout << "Capture stop automatically or with \"q\" button." << endl;
				int counter = 0;
				int num_frames = 0;
				ofstream list_f(dir_name + "\\imglist.txt"); 
				do {
					// カメラから画像をキャプチャ
					capture >> frame;
					// キャプチャした画像を表示
					imshow( windowNameCam, frame );
					if(counter >= interv){
						counter = 0;
						// 保存ファイル名
						std::ostringstream ost;
						ost << dir_name << "\\" << file_name << "_" << num_frames << ".png";
						list_f << ost.str() << endl;
						imwrite(ost.str(), frame);
						cout << ost.str() << endl;
						num_frames++;
					}
					else{
						counter++;
					}
				} while( waitKey( 1 ) != 'q' && num_frames <= img_num);

				cout << " --- Finished." << endl;
				cvDestroyWindow(windowNameCam.c_str());
			}
		}
		else if(opt=="camera_calibration"){
			string img_list;
			cout << "checkboard image list: ";
			cin >> img_list;
			string opt2;
			cout << "checker board row number: ";
			cin >> opt2;
			int row = atoi(opt2.c_str());
			cout << "checker board col number: ";
			cin >> opt2;
			int col = atoi(opt2.c_str());
			cout << "checker size(mm): ";
			cin >> opt2;
			float chess_size = atof(opt2.c_str());

			cameraCalibration	camera_calib;
			camera_calib.setMaxImageNum(50);
			camera_calib.setBoardColsAndRows(row, col);
			camera_calib.setChessSize(chess_size);

			ifstream ifs(img_list.c_str());
			string buf;
			if(ifs!=NULL){
//			if(ifs.is_open()){
				int id = 1;
//				while(!ifs.eof()){
				while(ifs && getline(ifs, buf)){
//					ifs.getline(buf,sizeof(buf));
					cout << id << ":" << buf << "..." << endl;

					// 画像の格納先を定義
					Mat	frame = imread(buf);

					camera_calib.addCheckerImage(frame);
				};

				cout << " --- Finished." << endl;
			}

			cout << "start calibration" << endl;
			bool ret = camera_calib.doCalibration();
			if(ret){
				cout << "camera parameter file name: ";
				cin >> opt2;
				camera_calib.saveCameraMatrix(opt2);
			}
			else{
				cout << "Error: fail to calibrate." << endl;
			}
		}
		else if(opt=="ar_start"){
//			controlAR	ctrlAR(ctrlOR);
//			ctrlAR.startGUI();
//			setControlOR(ctrlOR);
			cout << "AR config file: ";
			string arconfig;
			cin >> arconfig;
			setARConfigFile(arconfig);
			startGUI(argc, argv);
		}
		else if(opt=="exit"){
			exitflag1 = true;
		}
		else{
			cout << "Error: Wrong Command\n" << endl;
		}
	}
	}
//_CrtDumpMemoryLeaks(); 
	return 0;
}
