#ifndef SHARPCONTRAST_H
#define SHARPCONTRAST_H

#include <opencv/cv.hpp>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstdio>
#include <string.h>
#include <cmath>
#include <math.h>

using namespace std;
using namespace cv;

Mat lutL(1, 256, CV_8U);
Mat lutC(1, 256, CV_8U);

void exponent_correction(Mat& src, float gamma) {
    // to se ne uporablja, ker sem implementiral z LUT
    for (int i = 0; i < src.rows; i++) {
        uchar* clr = src.ptr<uchar>(i);
        for (int j = 0; j < src.cols; j++) {
            float cur = clr[j];
            if (cur > 10 && cur < 150) {
                cur = pow((cur / 255.0f), 1 / gamma) * 255.f;
                clr[j] = (uchar)cur;
            }
        }
    }
}

Mat vibrance(Mat& src, float gamma) {
    Q_UNUSED(gamma);
    Mat dst, HSV;
    src.copyTo(dst);

    vector<Mat> hsv_planes;
    cvtColor(src, HSV, COLOR_BGR2HSV);
    split(HSV, hsv_planes);

    LUT(hsv_planes[1], lutC, hsv_planes[1]);

    merge(hsv_planes, HSV);
    cvtColor(HSV, dst, COLOR_HSV2BGR);
    return dst;
}

void calculate_lutC(float gamma) {
    uchar* p = lutC.ptr();
    for ( int i = 0; i < 256; ++i)
      //p[i] = saturate_cast<uchar>(pow((float)(i / 255.0), dGamma) * 255.0f); // originalna formula
        p[i] = saturate_cast<uchar>(pow((float)(i / 255.0f), 1/gamma) * 255.0f);  //sprememba namesta 1/gamma
}

void calculate_lutDark(float gamma) {
    uchar* p = lutL.ptr();
    for ( int i = 0; i < 256; ++i)
      //p[i] = saturate_cast<uchar>(pow((float)(i / 255.0), dGamma) * 255.0f); // originalna formula
     {p[i] = saturate_cast<uchar>(pow((float)(i / 255.0f), gamma) * 255.0f);}  //Darken image

}

void calculate_lutLight(float gamma) {
    uchar* p = lutL.ptr();
    for ( int i = 0; i < 256; ++i)
      //p[i] = saturate_cast<uchar>(pow((float)(i / 255.0), dGamma) * 255.0f); // originalna formula

      {p[i] = saturate_cast<uchar>(pow((float)(i / 255.0f), 1/gamma) * 255.0f);}  //Lighten image
}

Mat GammaVibrancePreprocessing(Mat& src,double gammal,double gammac,int DarkLight) {

    Mat tmp,dst;

    calculate_lutC(gammac);

    //Glede na tip slike izberi bodisi posvetlitev oz. potemnitev
    if (DarkLight==0) {calculate_lutDark(gammal);}  //   gamma   Bio Inspired Image Darkening
    if (DarkLight==1) {calculate_lutLight(gammal);} //   1/gamma Bio Inspired Image brightening

    LUT(src, lutL, tmp);

    //Vibrance Bio Inspired Color Saturation
    dst = vibrance(tmp, gammac);

    return dst;
}

Mat SharpnessPreprocessing(Mat& src,double sigma,double threshold,double amount,int cliplimit,int Contrast ) {

    Mat dst,tmp;

    Mat lowContrastMask,sharpened,blurred;
    GaussianBlur(src, blurred, Size(), sigma, sigma);
    lowContrastMask = abs(src - blurred) < threshold;
    sharpened = src*(1+amount) + blurred*(-(amount));
    src.copyTo(sharpened, lowContrastMask);                    // CONDITIONAL COPY IMAGE

    cv::Mat lab_image;
    cv::cvtColor(sharpened, lab_image, cv::COLOR_BGR2Lab);

    // Extract the L channel
    std::vector<cv::Mat> lab_planes(3);
    cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(cliplimit);
    clahe->setTilesGridSize(Size(Contrast,Contrast));
    clahe->apply(lab_planes[0], tmp);

    // Merge the the color planes back into an Lab image
    tmp.copyTo(lab_planes[0]);
    cv::merge(lab_planes, lab_image);

    // convert back to RGB
    cv::cvtColor(lab_image, dst, cv::COLOR_Lab2BGR);

    return dst;
}

cv::Mat SharpContrast(cv::Mat frame, int DarkLight=0, int Intensity=5, int Vibrance=5, int Sharpness=1, int Contrast=1) {

    Mat final;

    float gammal = 0;
    float gammac = 0;

    int cliplimit=1;
    int sharpthreshold=1, sharpamount=1;

    Intensity < 1 ? Intensity = 1 : Intensity;
    Vibrance < 1 ? Vibrance = 1 : Vibrance;

    gammal = 1+Intensity / 1000.f;
    gammac = 1+Vibrance / 1000.f;

    if (Sharpness==0) Sharpness=1;
    if (Contrast==0) Contrast=1;

    double sigma = Sharpness+Sharpness/10.f;
    double threshold = sharpthreshold+sharpthreshold/10.f;
    double amount = sharpamount+sharpamount/10.f;

    final = GammaVibrancePreprocessing(frame, gammal, gammac, DarkLight);
    final = SharpnessPreprocessing(final, sigma,threshold,amount,cliplimit,Contrast);

    return final;
}


#endif // SHARPCONTRAST_H
