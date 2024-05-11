#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <chrono>
#include "seam_carving.hpp"

using namespace std;
using namespace cv;
using namespace chrono;

const double INF = 1e9;

int min_index(double x, double y, double z)
{
	if (x < z) {
		if (x < y) {
			return -1;
		}
		else {
			return 0;
		}
	}
	else {
		if (z < y) {
			return 1;
		}
		else {
			return 0;
		}
	}
}

void find_vertical_seam(Mat& channel, Mat& energy, vector<int>& seam) {

	Mat energymap(channel.rows, channel.cols, CV_64FC1);
	Mat pathmap(channel.rows, channel.cols, CV_8SC1);

	for (int i = 0; i < channel.cols; i++) {
		energymap.at<double>(0, i) = energy.at<double>(0, i);
		pathmap.at<char>(0, i) = 0;
	}

	for (int i = 1; i < channel.rows; i++) {
		for (int j = 0; j < channel.cols; j++) {
			int p;
			if (j == 0) {
				p = min_index(INF, energymap.at<double>(i - 1, j), energymap.at<double>(i - 1, j + 1));
			}
			else if (j == channel.cols - 1) {
				p = min_index(energymap.at<double>(i - 1, j - 1), energymap.at<double>(i - 1, j), INF);
			}
			else {
				p = min_index(energymap.at<double>(i - 1, j - 1), energymap.at<double>(i - 1, j), energymap.at<double>(i - 1, j + 1));
			}
			energymap.at<double>(i, j) = energy.at<double>(i, j) + energymap.at<double>(i - 1, j + p);
			pathmap.at<char>(i, j) = p;
		}
	}

	int last_row = channel.rows - 1;
	int min_col = 0;
	double min_val = energymap.at<double>(last_row, 0);
	for (int i = 1; i < channel.cols; i++) {
		if (energymap.at<double>(last_row, i) < min_val) {
			min_val = energymap.at<double>(last_row, i);
			min_col = i;
		}
	}

	seam[last_row] = min_col;
	for (int i = channel.rows - 2; i >= 0; i--) {
		seam[i] = seam[i + 1] + pathmap.at<char>(i + 1, seam[i + 1]);
	}

}

void calculate_energy(Mat channels[], Mat& energy) {
	cv::Mat grad_x[3], grad_y[3];
	cv::Mat abs_grad_x[3], abs_grad_y[3];
	cv::Mat channel_energy[3];

	for (int i = 0; i < 3; ++i) {
		// Sobel算子计算x和y方向的梯度
		cv::Sobel(channels[i], grad_x[i], CV_64FC1, 1, 0, 3, 1, 0, cv::BORDER_REFLECT);
		cv::Sobel(channels[i], grad_y[i], CV_64FC1, 0, 1, 3, 1, 0, cv::BORDER_REFLECT);

		// 计算x和y方向梯度的绝对值
		cv::convertScaleAbs(grad_x[i], abs_grad_x[i]);
		cv::convertScaleAbs(grad_y[i], abs_grad_y[i]);

		// 计算x和y方向梯度的加权和
		cv::addWeighted(abs_grad_x[i], 0.5, abs_grad_y[i], 0.5, 0, channel_energy[i], CV_64FC1);
	}

	energy = channel_energy[0] + channel_energy[1] + channel_energy[2];
}

void remove_seam(Mat& channel, vector<int>& seam) {
	for (int i = 0; i < channel.rows; i++) {
		int j = seam[i];
		for (; j < channel.cols - 1; j++) {
			channel.at<double>(i, j) = channel.at<double>(i, j + 1);
		}
	}
	channel = channel(Rect(0, 0, channel.cols - 1, channel.rows));
}

void seam_carving(const string& path, int new_width, int new_height, const string& output_filename) {

	Mat image;
	try {
		image = imread(path);
	}
	catch (...) {
		cout << "Cannot open file: " << path << endl;
		return;
	}

	if (image.empty()) {
		cout << "Cannot open file: " << path << endl;
		return;
	}

	cout << "正在处理: " << path << endl;
	auto begin = system_clock::now();

	if (new_width > image.cols) {
		cout << "SizeError: original image width: " << image.cols <<
			", " << new_width << "cannot be new width." << endl;
		return;
	}

	if (new_height > image.rows) {
		cout << "SizeError: original image height: " << image.rows <<
			", " << new_height << "cannot be new height." << endl;
		return;
	}

	if (new_width == 0) {
		new_width = image.cols >> 1;
	}
	if (new_height == 0) {
		new_height = image.rows >> 1;
	}
	if (new_width < 0) {
		new_width = image.cols;
	}
	if (new_height < 0) {
		new_height = image.rows;
	}

	// 计算需要删除的行和列数
	int delta_rows = image.rows - new_height;
	int delta_cols = image.cols - new_width;

	image.convertTo(image, CV_64FC1, 1.0 / 255);

	Mat channels[3];
	split(image, channels);

	Mat energy;
	vector<int> seam(max(channels[0].rows, channels[0].cols));

	for (int i = 0; i < delta_cols; ++i) {
		calculate_energy(channels, energy);
		/*if (i == 0) {
			double maxVal = 0;
			double minVal = 0;
			minMaxLoc(energy, &minVal, &maxVal);
			Mat show_energy;
			energy.convertTo(show_energy, CV_8UC1, 255);
			imshow("energy", show_energy);
			waitKey(0);
		}*/
		find_vertical_seam(channels[0], energy, seam);
		for (int c = 0; c < 3; ++c) {
			remove_seam(channels[c], seam);
		}
	}

	Mat transposed_channels[3];
	for (int i = 0; i < 3; ++i) {
		transpose(channels[i], transposed_channels[i]);
	}
	for (int i = 0; i < delta_rows; ++i) {
		calculate_energy(transposed_channels, energy);
		find_vertical_seam(transposed_channels[0], energy, seam);
		for (int c = 0; c < 3; ++c) {
			remove_seam(transposed_channels[c], seam);
		}
	}
	for (int i = 0; i < 3; ++i) {
		transpose(transposed_channels[i], channels[i]);
	}

	Mat ans_image;
	merge(channels, 3, ans_image);
	ans_image.convertTo(ans_image, CV_8UC3, 255);

	auto end = system_clock::now();
	cout << "用时：" << duration_cast<milliseconds>(end - begin).count() << "ms" << endl;

	imshow("Seam Carved", ans_image);
	waitKey(0);

	destroyAllWindows();
	try {
		imwrite(output_filename, ans_image);
	}
	catch (...) {
		cout << "Cannot save to:" << output_filename
			<< endl;
	}

}
