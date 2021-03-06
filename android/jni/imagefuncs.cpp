#include "imagefuncs.h"


#define LOG_TAG "Image Processing"
#ifdef ANDROID
#include <android/log.h>
#  define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#  define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#  define QUOTEME_(x) #x
#  define QUOTEME(x) QUOTEME_(x)
#  define LOGI(...) printf("I/" LOG_TAG " (" __FILE__ ":" QUOTEME(__LINE__) "): " __VA_ARGS__)
#  define LOGE(...) printf("E/" LOG_TAG "(" ")" __VA_ARGS__)
#endif


using namespace cv;

/* Calculate the circularity of a contour */
double calcCircularity(vector<Point> contour) {
	// Check circularity
	double perimeter = arcLength(contour, true);
	double area = contourArea(contour);
	double circularity = 4 * 3.14159265 * area / (perimeter * perimeter);
	return circularity;
}

/* Write image to the SDCARD */
//imwrite("/sdcard/Todos/afilename.png", mbgra);


/* Finds all the rectangles in a region */
vector<vector<Point> > findAllRectangles(Mat& mbgra) {
	LOGI("Finding rectangles...");

	int width = mbgra.size[1];
	int height = mbgra.size[0];

	// Separate the image in 3 places ( B, G and R )
	vector<Mat> rgbPlanes;
	split(mbgra, rgbPlanes);

	/*
	Get the yellow plane since most people wont write in yellow on a white board, 
	then apply an adaptive threshold to look for contrast,
	save the contrast into the thresh matrix
	*/
	Mat yellow = rgbPlanes[1]/2+rgbPlanes[2]/2; 
	Mat thresh;
	adaptiveThreshold(yellow, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 31, 11);
	//Blackboard dont do an inverse, it will already be dark on light : adaptiveThreshold(yellow, thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 31, 10);
	
	
	
	/*
	Take the thresh matrix, and look for contours in it
	*/
	vector<vector<Point> > contours;
	findContours(thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//###
	LOGI("Contours: %d", contours.size());


	/*
	For each contour, find out how rectangle it is (between .5 and 1 is a pretty good rectangle candidate)
	*/
	vector<vector<Point> > checkboxes;
	for (int i = 0; i < contours.size(); i++) {
		Rect rect = boundingRect(contours[i]);

		// Make contour convex QUESTION: why?
		vector<Point> convex;
		convexHull(contours[i], convex);

		// Check rectangularity
		double rectangularity = calcCircularity(convex);

		if (rectangularity < 0.8)
			continue;
		checkboxes.push_back (convex);
	}
	LOGI("Potential Checkboxes: %d", checkboxes.size());
	
	
	checkboxes = filterSquareByArea(checkboxes);
	
	checkboxes = filterSquareByCoordinates(checkboxes,mbgra);
	
    drawContours(mbgra, checkboxes, -1, Scalar(0, 255, 0, 255), 2);
	
	
	mbgra.setTo(Scalar(0, 0, 255, 255), thresh);//thresh is the mask to draw
	
	
	findDivisionBasedOnWhiteSpace(checkboxes, mbgra);
	getCropOfToDoLines(checkboxes, mbgra);
		
	return checkboxes;
}


/*
Find the vertical divide line where most of the rectangles are on one side of the image.
- get a histogram of the image
- find the spike where the checkbox are, followed by a valley where there is white space, 
- consider contours with  x's less than the white space to be checkboxes
http://laconsigna.wordpress.com/2011/04/29/1d-histogram-on-opencv/
*/
vector<vector<Point> > findDivisionBasedOnWhiteSpace(vector<vector<Point> > potentialCheckboxes, Mat& mbgra)
{
	return potentialCheckboxes;
}
/*
Crop the lines
*/
vector<vector<Point> > getCropOfToDoLines(vector<vector<Point> > checkboxes, Mat& mbgra)
{

	//vector<CString> filenames;
	
	for (int i = 0; i < checkboxes.size(); i++) {
		Rect rect = boundingRect(checkboxes[i]);
		int margin = 5;

		int a = rect.x - margin ;
		int b = rect.y - margin -120;
		int c = rect.width + margin * 2 ;
		int d = rect.height + margin * 2 +80;
		
		
		// Draw rectangle
		//rectangle(
		//		mbgra,
		//		Rect(a, 
		//				b,
		//				c,
		//				d), Scalar(200, 200, 0, 255), 2);
		
		
		// Draw cross-hairs
		int width = mbgra.size[0];
		int height = mbgra.size[1];
		line(mbgra
				, Point(rect.x, 0)
				, Point(rect.x, height )
				, Scalar(200, 200, 0, 255)
				, 2);
		/*
		draw a line below check box if you wish
		*/
		line(mbgra
				, Point(rect.x + rect.width, 0)
				, Point(rect.x + rect.width, height)
				, Scalar(200, 0, 0, 255)
				, 2);
		
		
		//cvLine(mbgra, pt1, pt2, Scalar(0, 200,200, 255), 2)
		//Mat imgroi = mbgra(Rect(a, b, c, d));
		
		//CString cs;
		//cs.Format("/sdcard/Todos/temp%d", i);
		//LOGI(cs);
	
		//imwrite(cs, imgroi);
		//imwrite("/sdcard/Todos/afilename.png", imgroi);
		

	}
	return checkboxes;
}
/*
e <= jni_part.cpp
Compile++ thumb  : native_sample <= imagefuncs.cpp
jni/imagefuncs.h: In function 'std::vector<std::vector<cv::Point_<int> > > findAllRectangles(cv::Mat&)':
jni/imagefuncs.h:12: error: too few arguments to function 'std::vector<std::vector<cv::Point_<int> > > getCropOfToDoLines(std::vector<std::vector<cv::Point_<int> > >, cv::Mat&)'
jni/imagefuncs.cpp:95: error: at this point in file
jni/imagefuncs.cpp: In function 'std::vector<std::vector<cv::Point_<int> > > getCropOfToDoLines(std::vector<std::vector<cv::Point_<int> > >, cv::Mat&)':
jni/imagefuncs.cpp:118: error: 'contours' was not declared in this scope
make: *** [obj/local/armeabi/objs/native_sample/imagefuncs.o] Error 1
*/


 //TODO : don't use the biggest area as reference to avoid bug if a big square exist
 //return the squares with the biggest area 
 vector<vector<Point> > filterSquareByArea(vector<vector<Point> > checkboxes)
 {
 	vector<double> checkBoxAreas;
 	
 	//get checkboxes areas
 	for (int i = 0; i < checkboxes.size(); i++) {
 		vector<Point> checkbox;
 	    checkbox = checkboxes[i]; 
 	    double potentialCheckboxArea = contourArea(checkbox);
 	    checkBoxAreas.push_back(potentialCheckboxArea);
 	}
 	
 	sort(checkBoxAreas.begin() , checkBoxAreas.end());
 	double bigestArea = checkBoxAreas.back();
 	
 	const double miniumAreaPercentage = 0.3;
 	double minimumAreaRequired  = bigestArea * miniumAreaPercentage;
 	
 	vector<vector<Point> > selectedCheckboxes;
 	//get bigest checkboxes
 	for (int i = 0; i < checkboxes.size(); i++) {
 		vector<Point> checkbox;
 	    checkbox = checkboxes[i];
 	     
 	    double checkboxArea = contourArea(checkbox);
 	    if(checkboxArea > minimumAreaRequired){
 	    	selectedCheckboxes.push_back(checkbox);
 	    }
 	}
 	return selectedCheckboxes;
 }
 
 
  vector<vector<Point> > filterSquareByCoordinates(vector<vector<Point> > checkboxes,  Mat& mbgra)
 {
 	vector<Rect> checkBoxRect;
 	//get checkboxes boundingRect
 	for (int i = 0; i < checkboxes.size(); i++) {
 		vector<Point> checkbox;
 	    checkbox = checkboxes[i]; 
 	    
 	    Rect rect = boundingRect(checkboxes[i]);
 	    
 	    checkBoxRect.push_back(rect);
 	}
 	
 	sort(checkBoxRect.begin() , checkBoxRect.end(),sortByY);
 	Rect rect = checkBoxRect.back();
 	double leftY = rect.y;
 	LOGI("X : %f  width : %f", leftY, rect.width);
 	double minimumYRequired  = leftY - 20 ;
 
 	vector<vector<Point> > selectedCheckboxes;
 	//get left checkboxes
 	for (int i = 0; i < checkboxes.size(); i++) {
 		vector<Point> checkbox;
 	    checkbox = checkboxes[i];
 	     
 	    Rect checkboxSquare = boundingRect(checkbox);
 	    if(checkboxSquare.y > minimumYRequired){
 	    	selectedCheckboxes.push_back(checkbox);
 	    }
 	}

 	return selectedCheckboxes;
 }
 
 
 bool sortByY(Rect rect1, Rect rect2){
 	return (rect2.y-rect1.y > 0);
 }
 

/* Finds the circle in a region */
vector<Point> findCircle(Mat& mbgra) {
	int width = mbgra.size[1];
	int height = mbgra.size[0];

	// Separate the image in 3 places ( B, G and R )
	vector<Mat> rgbPlanes;
	split(mbgra, rgbPlanes);

	// Get center rectangle
	Mat green = rgbPlanes[1];
	Mat center = green(Range(height * 0.4, height * 0.6),
			Range(width * 0.4, width * 0.6));

	// Get threshold for circle
	double threshval = mean(center)[0] * 0.75;

	// Threshold
	Mat thresh;
	threshold(green, thresh, threshval, 255, THRESH_BINARY);

	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	morphologyEx(thresh, thresh, MORPH_CLOSE, kernel);

	vector<vector<Point> > contours;
	findContours(thresh, contours, CV_RETR_EXTERNAL, CHAIN_APPROX_NONE);

	// Find contour that encompasses cross-hairs and has big area
	for (int i = 0; i < contours.size(); i++) {
		Rect rect = boundingRect(contours[i]);

		if (!rect.contains(Point(width / 2, height / 2)))
			continue;

		if (rect.width < width * 0.25 || rect.height < height * 0.25)
			continue;

		// Check that center is inside
		if (pointPolygonTest(contours[i], Point(width / 2, height / 2), false)
				<= 0)
			continue;

		// Make contour convex
		vector<Point> convex;
		convexHull(contours[i], convex);

		// Check circularity
		double circularity = calcCircularity(convex);

		if (circularity < 0.9)
			continue;

		return convex;
	}
	return vector<Point>(0);
}

/* Highpass of the image 
  image is a 3-channel 8-bit image
  mask3C is a 3-channel mask with 255 for yes, 0 for no.
  blursize is size of box filter applied
  returns float image scaled around 1.0
  */
Mat highpass(Mat& image, Mat& mask3C, int blursize) {
	// Create low-pass filter, only within mask
	Mat blurred;
	Mat blurredCount;
	boxFilter(image & mask3C, blurred, CV_32FC3, Size(blursize, blursize),
			Point(-1, -1), false, BORDER_CONSTANT);

	boxFilter(mask3C, blurredCount, CV_32FC3, Size(blursize, blursize),
			Point(-1, -1), false, BORDER_CONSTANT);

	Mat highpass;
	image.convertTo(highpass, CV_32FC3);

	Mat lowpass = blurred/blurredCount;

	highpass/=lowpass;

	return highpass/255;
}

/* Removes yellow lines from the image by minimizing the edges when 
 calculating green*(1+lambda)-blue*lambda.
 */
void removeyellow(Mat& img) {
	vector<Mat> bgr;
	split(img, bgr);

	Mat g = bgr[1]-1.0;
	Mat b = bgr[0]-1.0;

	double lambda = mean(g.mul(b-g))[0]/mean(g.mul(g)-2*g.mul(b)+b.mul(b))[0];

	bgr[1]=bgr[1]*(1+lambda)-bgr[0]*lambda;

	// Also reset blue
	bgr[0]=bgr[1];

	merge(bgr, img);
}

Mat findColonies(Mat& mbgr, int& colonies) {
	int width = mbgr.size[1];
	int height = mbgr.size[0];

	LOGI("Find colonies on %d, %d", width, height);

	vector<Point> contour = findCircle(mbgr);
	if (contour.size() == 0)
		return Mat(100, 100, CV_8UC3, Scalar(0, 0, 255));

	LOGI("Contour with %d points", contour.size());

	// Extract petri region
	Rect petriRect = boundingRect(contour);
	Mat petri = mbgr(petriRect);

	LOGI("Contour rect (%d,%d,%d,%d)", petriRect.x, petriRect.y,
			petriRect.width, petriRect.height);

	// Create overall mask
	Mat maskBig = Mat(mbgr.size(), CV_8UC1, Scalar(0));
	fillConvexPoly(maskBig, contour, Scalar(255));

	LOGI("fillConvexPoly");

	// Create petri mask
	Mat mask = maskBig(petriRect);

	// Erode mask to remove edge effects
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(21, 21));
	erode(mask, mask, kernel);

	LOGI("resized mask (%d,%d)", mask.size[1], mask.size[0]);

	Mat mask3C = Mat(petri.size(), CV_8UC3, Scalar(0, 0, 0));
	mask3C.setTo(Scalar(255, 255, 255), mask);

	// Create low-pass filter, only within mask
	Mat blurred;
	Mat blurredCount;
	int blursize = (petri.rows / 12) * 2 + 1;
	boxFilter(petri & mask3C, blurred, CV_32FC3, Size(blursize, blursize),
			Point(-1, -1), false, BORDER_CONSTANT);

	LOGI("boxFilter #1");

	boxFilter(mask3C, blurredCount, CV_32FC3, Size(blursize, blursize),
			Point(-1, -1), false, BORDER_CONSTANT);

	LOGI("boxFilter #2");
	blurred = blurred / (blurredCount / 255);

	LOGI("blurred");

	// High-pass image
	Mat highpass;
	petri.convertTo(highpass, CV_32FC3);
	highpass = highpass / blurred * 200;

	LOGI("highpassed");

	// Convert back to 8-bit
	Mat highpass8;
	highpass.convertTo(highpass8, CV_8UC3);

	LOGI("8-bitted");

	// Mask outside of circle to background
	highpass8.setTo(Scalar(200, 200, 200), 255 - mask);

	LOGI("reset outside mask");

	// Remove yellow lines
	removeyellow(highpass8);

	// Remove noise
	blur(highpass8, highpass8, Size(3,3));

	// Split into channels
	vector<Mat> rgbPlanes;
	split(highpass8, rgbPlanes);

	// Find colonies
	Mat colthresh;
	threshold(rgbPlanes[1], colthresh, 150, 200, CV_THRESH_BINARY_INV);

//	// Remove single pixels
//	kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
//	erode(colthresh, colthresh, kernel);

	int mingap = petri.rows / 100;

	// Encompass entire colonies
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(mingap, mingap));
	dilate(colthresh, colthresh, kernel);

	vector<vector<Point> > contours;
	findContours(colthresh, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);//###

	LOGI("Colony contours: %d", contours.size());

	colonies = 0;
	for (int i = 0; i < contours.size(); i++) {
		Rect rect = boundingRect(contours[i]);

		int neighsize = petri.rows / 50;

//		// Ignore if too large
//		if (rect.height > neighsize*2 || rect.width > neighsize*2)
//			continue;

		Point center = Point(rect.x + rect.width / 2, rect.y + rect.height / 2);

		// Draw rectangle
		rectangle(
				highpass8,
				Rect(rect.x - neighsize, rect.y - neighsize,
						rect.width + neighsize * 2,
						rect.height + neighsize * 2), Scalar(0, 0, 255, 255), 2);

		colonies++;
//		// Draw rectangle
//		rectangle(
//				highpass8,
//				Rect(rect.x, rect.y,
//						rect.width,
//						rect.height), Scalar(0, 0, 255, 255), 3);
	}

	return highpass8;
}
