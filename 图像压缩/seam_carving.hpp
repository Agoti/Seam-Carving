#ifndef SEAM_CARVING_HPP
#define SEAM_CARVING_HPP

#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

void seam_carving(const string& path, int new_width, int new_height, const string& output_filename);

#endif // !SEAM_CARVING_HPP
