#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;

// This Section is for importing images

Mat imgOriginal, imgGray, imgCanny, imgThre, imgBlur, imgDilate, imgWarp;
vector<Point> initialPoints, docPoints;

float w = 420, h = 596;

Mat preProcessing(Mat img) {
	cvtColor(img, imgGray, COLOR_BGR2GRAY);

	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);

	//Seems to keep it a ration of 1 to 3
	Canny(imgBlur, imgCanny, 25, 75);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	//Image Dilation and errosion
	dilate(imgCanny, imgDilate, kernel);
	return imgDilate;
}

vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	Point myPoint(0, 0);
	//In open cv this draws the contours
	//drawContours(img, contours, -1, Scalar(255, 0, 255), 2);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());
	
	//We find the area of these contours and filter them out.
	vector<Point> biggest;
	int maxArea = 0;
	for (int i = 0; i < contours.size(); i++) {
		int area = contourArea(contours[i]);

		if (area > 1000) {
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			
			if (area > maxArea && conPoly[i].size() == 4) {
				//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 5);
				biggest = { conPoly[i][0],conPoly[i][1] ,conPoly[i][2], conPoly[i][3] };
				maxArea = area;
			}
			
			//drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 2);
			//rectangle(imgOriginal, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0), 5);

		}
	}
	return biggest;
}

void drawPoints(vector<Point> points, Scalar color) {
	for (int i = 0; i < points.size(); i++) {
		circle(imgOriginal, points[i], 10, color, FILLED);
		putText(imgOriginal, to_string(i), points[i], FONT_HERSHEY_PLAIN, 4, color,2);
	}

}

vector<Point> reorder(vector<Point> points){
	vector<Point> newPoints;
	vector<int> sumPoints, subPoints;
	
	for (int i = 0; i < 4; i++) {
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}
	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]);
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]);
	
	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h) {
	Point2f src[4] = { points[0], points[1], points[2], points[3] };
	Point2f dst[4] = { {0.0f, 0.0f}, {w, 0.0f}, {0.0f,h}, {w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));
	return imgWarp;
}

// This code shows how to import and show an image
void main() {
	string path = "Resources/paper.jpg";
	imgOriginal= imread(path);
	//Scaling image down
	//resize(imgOriginal, imgOriginal, Size(), 0.5, 0.5);
	
	// Preprocessing
	imgThre = preProcessing(imgOriginal);
	// get Contours - Find Biggest Rectangle
	initialPoints = getContours(imgThre);
	//drawPoints(initialPoints, Scalar(0, 0, 255));
	docPoints = reorder(initialPoints);
	//drawPoints(docPoints, Scalar(255, 0, 0));
	
	//Warp the Image
	imgWarp = getWarp(imgOriginal, docPoints, w, h);


	imshow("Image", imgOriginal);
	imshow("Dialation", imgThre);
	imshow("Image Warp", imgWarp);
	waitKey(0);


}
