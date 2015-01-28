#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <limits>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

extern "C" {
#include <sys/types.h>
#include <dirent.h>
}

using namespace std;
using namespace cv;

const Rect roi(103, 61, 1066, 572);
const Scalar bgScalar1(58, 0, 0), bgScalar2(125, 255, 255); // (H, S, V) from .. to ..

vector<string> getDirContents(const string& dirPath) {
  DIR *pd = opendir(dirPath.c_str());
  if (!pd)
    throw std::runtime_error("Can't open directory: " + dirPath);

  dirent *ent;
  vector<string> result;
  while ((ent = readdir(pd)))
    if ((ent->d_name)[0] != '.')
      result.push_back(ent->d_name);

  closedir(pd);
  sort(result.begin(), result.end());
  return result;
}

size_t processImage(const string& path, float z, ostream& out) {
  Mat origImg = imread(path, CV_LOAD_IMAGE_COLOR);
  if (!origImg.data) 
    throw std::runtime_error("Can't load " + path);
  origImg = origImg(roi);
  
  Mat hsv, mask, clipped;
  cvtColor(origImg, hsv, CV_BGR2HSV);
  inRange(hsv, bgScalar1, bgScalar2, mask);
  bitwise_not(mask, mask);
  origImg.copyTo(clipped, mask);

  Mat cannyOutput;
  vector<vector<Point> > contours;
  vector<Vec4i> hier;
  Canny(mask, cannyOutput, 0, 0);
  findContours(cannyOutput, contours, hier, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

  Point topleft(numeric_limits<int>::max(), numeric_limits<int>::max());
  Point botright(numeric_limits<int>::min(), numeric_limits<int>::min());

  size_t pointCount = 0;
  for (int i = 0; i < contours.size(); ++i) {
    pointCount += contours[i].size();
    for (Point pt : contours[i]) { 
      topleft.x = min(topleft.x, pt.x);
      topleft.y = min(topleft.y, pt.y);
      botright.x = max(botright.x, pt.x);
      botright.y = max(botright.y, pt.y);
      //out << (float)pt.x << " " << (float)pt.y << " " << z << endl;
      out << (float)pt.x << " " << (float)pt.y << endl;
    }
  }
  out << "-1 -1" << endl << (topleft.x + botright.x) / 2 << " ";
  out << (topleft.y + botright.y) / 2 << endl;
  return pointCount;
}

int main(int argc, char **argv) {
  ofstream ptStream("points.dat");
/*  ptStream << "VERSION .7" << endl;
  ptStream << "FIELDS x y z" << endl;
  ptStream << "SIZE 4 4 4" << endl; // float
  ptStream << "TYPE F F F" << endl;
  ptStream << "COUNT 1 1 1" << endl;
  ptStream << "WIDTH "; // to be set later..
  long pos1 = ptStream.tellp(); ptStream << "              " << endl;
  ptStream << "HEIGHT 1" << endl;
  ptStream << "VIEWPOINT 0 0 0 1 0 0 0" << endl;
  ptStream << "POINTS "; // to be set later..
  long pos2 = ptStream.tellp(); ptStream << "              " << endl;
  ptStream << "DATA ascii" << endl;*/
  vector<string> jpegs = getDirContents("./pic/");
  float z = -400.0f;
  const float deltaZ = 2.0f;
  size_t pointCount = 0;
  for (auto jpgName : jpegs) { 
    cout << "Processing " << jpgName << " .." << endl;
    pointCount += processImage("./pic/" + jpgName, z, ptStream);
    z += deltaZ;
  }
  /*ptStream.seekp(pos1);
  ptStream << pointCount;
  ptStream.seekp(pos2);
  ptStream << pointCount;*/
  ptStream.close();

  return EXIT_SUCCESS;
}

