#ifndef NEONEDGE_H
#define NEONEDGE_H

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat lut(1, 256, CV_8UC3);

Scalar get_rgb_from_hsv(int hue, int sat, int val, bool white_black = false) {
    Mat color_mat(1, 1, CV_8UC3, Scalar(hue, sat, val));
    Mat temp;
    cvtColor(color_mat, temp, COLOR_HSV2RGB);
    Vec3b color = temp.at<Vec3b>(0, 0);

    if (white_black) {
        if (hue == 0) {
            return Scalar(0, 0, 0);
        }
        else if (hue == 179) {
            return Scalar(255, 255, 255);
        }
        return Scalar(color.val[0], color.val[1], color.val[2]);
    }
    else {
        return Scalar(color.val[0], color.val[1], color.val[2]);
    }
}

void calculate_lut(int intensity, Scalar color) {
    Q_UNUSED(intensity);
    for ( int i = 0; i < 256; ++i) {
        float f = (i / 255.0f);
        lut.at<Vec3b>(i)[0] = saturate_cast<uchar>(color[0] * f);
        lut.at<Vec3b>(i)[1] = saturate_cast<uchar>(color[1] * f);
        lut.at<Vec3b>(i)[2] = saturate_cast<uchar>(color[2] * f);
    }
}

void calculate_sobel(Mat& gray, Mat& sobel, int scale, double weight,int bold) {
    Mat sobel_x, sobel_y;
       // -----------
    // odvod po x
    Sobel( gray, sobel_x, CV_16S, 1, 0, bold, scale, 11);
    convertScaleAbs(sobel_x, sobel_x);
    // -----------
    // odvod po y
    Sobel( gray, sobel_y, CV_16S, 0, 1, bold, scale, 11);
    convertScaleAbs(sobel_y, sobel_y);
    addWeighted( sobel_x, weight, sobel_y, weight, 0, sobel );
}

Mat EdgeAugumentation(Mat& src,int kernel,int scale, double weight_d, int bold, int cut, int intensity) {

    // Matrix Initialisation
    Mat  gray, sobel, edges, color_edges,dst;

    cvtColor(src, gray, COLOR_BGR2GRAY );
    GaussianBlur(gray, gray, Size(kernel, kernel), 0, 0, BORDER_DEFAULT);
    calculate_sobel(gray, sobel, scale, weight_d,bold);
    cv::threshold(sobel, sobel, cut, 255, THRESH_TOZERO);
    //Sobel Type 2 bolj izraziti robovi
    cvtColor(sobel, edges, COLOR_GRAY2BGR );
    LUT(edges, lut, color_edges);
    src.copyTo(edges, sobel);
    src.setTo(Scalar(0, 0, 0), sobel);
    addWeighted( edges, (intensity * 0.01), color_edges, (1 - (intensity * 0.01)), 0, edges );
    add(src, edges, dst);

    return dst;
}

cv::Mat NeonEdge(cv::Mat frame, int intensity=46, int kernel=9, int weight=32, int scale=4, int cut=100, int hue=42) {

    double weight_d;
    int bold = 1;

    int sat = 255;
    int val = 255;

    Scalar color;

    (kernel > 3 && kernel % 2 == 0) ? kernel++ : kernel < 3 ? kernel = 3 : kernel;
    (bold > 3 && bold % 2 == 0) ? bold++ : bold < 3 ? bold = 1 : bold;
    weight_d = weight * 0.05;
    color = get_rgb_from_hsv(hue, sat, val,true);

//    if (intensity_prev != intensity || hue_prev != hue) {
//        calculate_lut(intensity, color);
//    }

    return EdgeAugumentation(frame,  kernel, scale,  weight_d,  bold,  cut,  intensity);
}



#endif // NEONEDGE_H
