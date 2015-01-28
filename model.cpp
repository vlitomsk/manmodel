#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdexcept>

using namespace std;

struct FVec {
  float x,y,z;
  FVec() : x(0), y(0), z(0) {}
  FVec(float xc, float yc, float zc) : x(xc), y(yc), z(zc) {}

  const FVec& operator+=(const FVec& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  void rotX(float cosA, float sinA) {
    float y1 = cosA * y + sinA * z,
          z1 = -sinA * y + cosA * z;
    y = y1;
    z = z1;
  }

  void rotY(float cosA, float sinA) {
    float x1 = cosA * x + sinA * z,
          z1 = -sinA * x + cosA * z;
    x = x1;
    z = z1;
  }

  void rotZ(float cosA, float sinA) {
    float x1 = cosA * x + sinA * y,
          y1 = -sinA * x + cosA * y;
    x = x1;
    y = y1;
  }
};

void projectModel(const std::vector<FVec>& points, uint8_t *rawImg, int w, int h, 
                  float transX, float transY, float transZ,
                  float xAngle, float yAngle, float zAngle) 
{
  const float cosRx = cos(xAngle), sinRx = sin(xAngle), cosRy = cos(yAngle), sinRy = sin(yAngle), cosRz = cos(zAngle), sinRz = sin(zAngle);
  const FVec vectT0(-603, -285, 0);
  const float projDist = 1000.0f;

  fill(rawImg, rawImg + w * h, 0);
  for (auto vectR : points) {
    vectR += vectT0;
    vectR.rotX(cosRx, sinRx);
    vectR.rotY(cosRy, sinRy);
    vectR.rotZ(cosRz, sinRz);
    vectR += FVec(transX, transY, transZ);

    float xProj = vectR.x * projDist / vectR.y + 400.0f; 
    float zProj = vectR.z * projDist / vectR.y + 400.0f;
    
    int xc = round(xProj);
    int yc = round(zProj);

    if (xc >= 0 && yc >= 0 && xc < w && yc < h)
      rawImg[yc * w + xc] = 255;
  }
}

int main(int argc, char **argv) {
  /* Loading points ... */
  const float deltaZ = 2.f;
  float x_m, y_m, z = -400.f;
  size_t loaded = 0;
  vector<FVec> points;
  
  ifstream ptStream("points.dat");
  if (!ptStream.is_open())
    throw std::runtime_error("Can't open points.dat");

  while (ptStream >> x_m >> y_m) {
    if (loaded % 10000 == 0)
      cout << "Loaded " << loaded << " points" << endl;
    if (x_m != -1) {
      points.push_back(FVec(x_m, y_m, z));
    } else {
      z += deltaZ;
    }
    loaded++;
  }
  ptStream.close();
  cout << "Done loading points" << endl;
  cout << "Total points: " << points.size() << endl;

  /* Projection & visualization ... */
   
  const int w = 1000, h = 1000;
  const float deltaAngle = 3.1415926f / 36.f; 
  float xAngle = .0f, yAngle = .0f, zAngle = .0f;
  bool running = true;

  unique_ptr<uint8_t[]> rawImg(new uint8_t[w * h]);
  cv::Mat cvImg(w, h, CV_8U, rawImg.get());

  while (running) {
    projectModel(points, rawImg.get(), w, h, 0, 2000, 0, xAngle, yAngle, zAngle);
    cv::imshow("cvImg", cvImg);

    int key;
    while ((key = cv::waitKey(30))) {
      if (key == 27) 
        running = false;
      else if (key == 'a') 
        xAngle += deltaAngle;
      else if (key == 'z') 
        xAngle -= deltaAngle;
      else if (key == 's')
        yAngle += deltaAngle;
      else if (key == 'x')
        yAngle -= deltaAngle;
      else if (key == 'd')
        zAngle += deltaAngle;
      else if (key == 'c')
        zAngle -= deltaAngle;
      else 
        continue;
      break;
    }
  }

  return EXIT_SUCCESS;
}
