/************************************************************************
* File:	RunTracker.cpp
* Brief: C++ demo for Kaihua Zhang's paper:"Real-Time Compressive Tracking"
* Version: 1.0
* Author: Yang Xian
* Email: yang_xian521@163.com
* Date:	2012/08/03
* History:
************************************************************************/
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include "CompressiveTracker.h"
#include <stdlib.h>  
#include "opencv2/imgproc/imgproc.hpp"   
#include <stdio.h> 


using namespace cv;
using namespace std;

Rect box; // tracking object
bool drawing_box = false;
bool gotBB = false;	// got tracking box or not
bool fromfile = false;
string video;

//*************************************************************8

Mat src; Mat src_gray;
int thresh = 155;//185; //smaller - larger rect
int max_thresh = 255;
RNG rng(12345);

void bounded(Mat src)
{
  /// Convert image to gray and blur it
  cvtColor( src, src_gray, COLOR_BGR2GRAY );
  blur( src_gray, src_gray, Size(3,3) );

  /// Create Window
  //const char* source_window = "Source";
  //namedWindow( source_window, WINDOW_NORMAL );
  //imshow( source_window, src );

  Mat threshold_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detect edges using Threshold
  threshold( src_gray, threshold_output, thresh, 255, THRESH_BINARY );
  /// Find contours
  findContours( threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Approximate contours to polygons + get bounding rects and circles
  vector<vector<Point> > contours_poly( contours.size() );
  vector<Rect> boundRect( contours.size() );
  vector<Point2f>center( contours.size() );
  vector<float>radius( contours.size() );

  for( size_t i = 0; i < contours.size(); i++ )
     { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
       boundRect[i] = boundingRect( Mat(contours_poly[i]) );
       minEnclosingCircle( contours_poly[i], center[i], radius[i] );
     }


  /// Draw polygonal contour + bonding rects + circles
  //Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
  size_t max_index = 0;
  size_t max_area = boundRect[0].area();
  for( size_t i = 0; i< contours.size(); i++ )
     {
       //Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       //drawContours( drawing, contours_poly, (int)i, color, 1, 8, vector<Vec4i>(), 0, Point() );
       //rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
       //circle( drawing, center[i], (int)radius[i], color, 2, 8, 0 );
	if(boundRect[i].area() > max_area){
		max_index = i;
		max_area = boundRect[i].area();
	}
	
     }
	Scalar color = Scalar( 0, 0, 255);
	rectangle( src, boundRect[max_index].tl(), boundRect[max_index].br(), color, 2, 8, 0 );
       printf("Initial Tracking Box = x:%d y:%d h:%d w:%d\n", boundRect[max_index].x, boundRect[max_index].y, boundRect[max_index].width, boundRect[max_index].height);

	box.x = boundRect[max_index].x;
	box.y = boundRect[max_index].y;
	box.width = boundRect[max_index].width;
	box.height = boundRect[max_index].height;

  namedWindow( "initial rect found", CV_WINDOW_AUTOSIZE);
  imshow( "initial rect found", src );

  //waitKey(0);
  cout<<"press q to continue"<<endl;
  while (cvWaitKey(33) != 'q');
  destroyWindow("initial rect found");
}


//*************************************************************8

void readBB(char* file)	// get tracking box from file
{
	ifstream tb_file (file);
	string line;
	getline(tb_file, line);
	istringstream linestream(line);
	string x1, y1, w1, h1;
	getline(linestream, x1, ',');
	getline(linestream, y1, ',');
	getline(linestream, w1, ',');
	getline(linestream, h1, ',');
	int x = atoi(x1.c_str());
	int y = atoi(y1.c_str());
	int w = atoi(w1.c_str());
	int h = atoi(h1.c_str());
	box = Rect(x, y, w, h);
}

// tracking box mouse callback
void mouseHandler(int event, int x, int y, int flags, void *param)
{
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		if (drawing_box)
		{
			box.width = x - box.x;
			box.height = y - box.y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		drawing_box = true;
		box = Rect(x, y, 0, 0);
		break;
	case CV_EVENT_LBUTTONUP:
		drawing_box = false;
		if (box.width < 0)
		{
			box.x += box.width;
			box.width *= -1;
		}
		if( box.height < 0 )
		{
			box.y += box.height;
			box.height *= -1;
		}
		gotBB = true;
		break;
	default:
		break;
	}
}

void print_help(void)
{
	printf("use:\n     welcome to use CompressiveTracking\n");
	printf("Kaihua Zhang's paper:Real-Time Compressive Tracking\n");
	printf("C++ implemented by yang xian\nVersion: 1.0\nEmail: yang_xian521@163.com\nDate:	2012/08/03\n\n");
	printf("-v    source video\n-b        tracking box file\n");
}

void read_options(int argc, char** argv, VideoCapture& capture)
{
	for (int i=0; i<argc; i++)
	{
		if (strcmp(argv[i], "-b") == 0)	// read tracking box from file
		{
			if (argc>i)
			{
				readBB(argv[i+1]);
				gotBB = true;
			}
			else
			{
				print_help();
			}
		}
		if (strcmp(argv[i], "-v") == 0)	// read video from file
		{
			if (argc > i)
			{
				video = string(argv[i+1]);
				capture.open(video);
				fromfile = true;
			}
			else
			{
				print_help();
			}
		}
	}
}

int main(int argc, char * argv[])
{
	VideoCapture capture;
	//capture.open(0); //uncomment this if real time video
	// Read options
	read_options(argc, argv, capture);
	// Init camera
	/*if (!capture.isOpened())  // uncomment this box if real time video
	{
		cout << "capture device failed to open!" << endl;
		return 1;
	}*/
	// Register mouse callback to draw the tracking box
	namedWindow("CT", CV_WINDOW_AUTOSIZE);
	//setMouseCallback("CT", mouseHandler, NULL);
	// CT framework
	CompressiveTracker ct;

	Mat frame;
	Mat last_gray;
	Mat first;
	if (fromfile)
	{
		capture >> frame;
		cvtColor(frame, last_gray, CV_RGB2GRAY);
		frame.copyTo(first);
	}
	else
	{
		capture.set(CV_CAP_PROP_FRAME_WIDTH, 340);
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	}

	// Initialization
	/*while(!gotBB)
	{
		if (!fromfile)
		{
			capture >> frame;
		}
		else
		{
			first.copyTo(frame);
		}
		cvtColor(frame, last_gray, CV_RGB2GRAY);
		rectangle(frame, box, Scalar(0,0,255));
		imshow("CT", frame);
		if (cvWaitKey(33) == 'q') {	return 0; }
	}*/
	

	bounded(first);
	
	// Remove callback
	//setMouseCallback("CT", NULL, NULL);
	//printf("Initial Tracking Box = x:%d y:%d h:%d w:%d\n", box.x, box.y, box.width, box.height);

	// CT initialization
	ct.init(last_gray, box);

	// Run-time
	Mat current_gray;
	
	cout<<"press q to exit"<<endl;

	while(capture.read(frame))
	{
		// get frame
		cvtColor(frame, current_gray, CV_RGB2GRAY);
		// Process Frame
		ct.processFrame(current_gray, box);
		// Draw Points
		rectangle(frame, box, Scalar(0,0,255));
		// Display
		imshow("CT", frame);
		//printf("Current Tracking Box = x:%d y:%d h:%d w:%d\n", box.x, box.y, box.width, box.height);
		
		if (cvWaitKey(33) == 'q') {	break; }
	}
	return 0;
}
