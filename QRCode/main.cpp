#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include <cmath>
#define PI 3.141592

using namespace cv;
using namespace std;

const double DEFAULT_SIDE = 48.3; //QR코드 한 변의 길이(cm)
const String INPUT_IMG[5] = { "images/test2.jpg","images/test2.jpg","images/test2.jpg","images/test2.jpg","images/test2.jpg" };//5개의 이미지 경로
const int realAngle[5] = { -20,-10,0,10,20 };//5개의 이미지에 대한 각도
double Z;//초점거리
Mat input_mat[5];//5개의 이미지를 저장할 mat

//함수 선언부
void QRCodeDetect(String img, vector<Point>& ret, Mat& rMat);
Point2d getCenterPoint(vector<Point>& points);
double calHeight(vector<Point>& inputs, Point2d& center, Mat& mat);
long double execute(vector<Point>& inputs, Point2d& center, double height);
double getDistance(Point& p1, Point& p2);

int main(void)
{
    //초점거리 입력
    cout << "초점거리(Z)입력: ";
    cin >> Z;

    //각 이미지에 대해서
    for (int k = 0; k < 5; k++) {
        vector<Point> input_points;
        double input_height, input_angle;
        cout << "------------------------------------------------\n";
        cout << "입력이미지(INPUT_IMG["<<k<<"]) 경로 입력: " << INPUT_IMG[k] << endl;

        //QR코드 검출
        QRCodeDetect(INPUT_IMG[k], input_points, input_mat[k]);

        //검출이 되지 않았다면 해당 이미지는 패스
        if (input_points.size() == 0) {
            cout << "input_point is empty!" << endl;
            continue;
        }

        //좌표화(프레임 정중앙은 0,0)
        for (int i = 0; i < 4; i++) {
            input_points[i].x -= input_mat[k].cols / 2.0;
            input_points[i].y = input_mat[k].rows / 2.0 - input_points[i].y;
        }

        //검출된 QR코드의 중점의 좌표를 구한다.
        Point2d inputCenter = getCenterPoint(input_points);

        //높이와 각도를 구한다.
        input_height = calHeight(input_points, inputCenter, input_mat[k]);
        input_angle = execute(input_points, inputCenter, input_height);

        //결과 출력
        cout << "distance: " << round((input_height) * 100) / 100 << " cm" << endl;
        cout << "angle: " << round((input_angle * 180 / PI) * 100) / 100 << "º" << endl;
        string sDistance = to_string(round((input_height) * 100) / 100);
        string sAngle = to_string(round((input_angle * 180 / PI) * 100) / 100);
        putText(input_mat[k], "distance: " + sDistance.substr(0, sDistance.find('.') + 3) + "cm", Point(20, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
        putText(input_mat[k], "angle: " + sAngle.substr(0, sAngle.find('.') + 3) + "degree", Point(20, 60), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
    }

    //프레임 출력
    while (true) {
        imshow("INPUT_IMG[0] = " + to_string(realAngle[0]), input_mat[0]);
        imshow("INPUT_IMG[1] = " + to_string(realAngle[1]), input_mat[1]);
        imshow("INPUT_IMG[2] = " + to_string(realAngle[2]), input_mat[2]);
        imshow("INPUT_IMG[3] = " + to_string(realAngle[3]), input_mat[3]);
        imshow("INPUT_IMG[4] = " + to_string(realAngle[4]), input_mat[4]);
        if (waitKey(1) == 27)
            break;
    }

    return 0;
}

//QR코드 검출
void QRCodeDetect(String img, vector<Point>& ret, Mat& rMat)
{
    QRCodeDetector detector;

    //이미지 입력, 크기조절, threshold
    Mat frame = imread(img, 0);
    Mat R, B;
    resize(frame, R, Size(frame.cols, frame.rows), 0, 0, 1);
    threshold(R, B, 100, 255, THRESH_BINARY);
    if (B.empty()) {
        cerr << "Frame load failed!" << endl;
        return;
    }

    //QR코드 검출 후 검출된 사각형 저장
    vector<Point> points;
    String info = detector.detectAndDecode(B, points);

    //검출되었을 경우
    if (!info.empty()) {
        //사각형을 그린 후 4개의 점에 숫자를 출력한다.
        polylines(R, points, true, Scalar(0, 0, 255), 2);
        line(R, points[0], points[2], Scalar(0, 0, 255), 2);
        line(R, points[1], points[3], Scalar(0, 0, 255), 2);
        ret = points;
        putText(R, "0", points[0], FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
        putText(R, "1", points[1], FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
        putText(R, "2", points[2], FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
        putText(R, "3", points[3], FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
    }
    rMat = R;
}

//두 대각선의 교점의 위치를 구하는 함수
Point2d getCenterPoint(vector<Point>& points) {
    double a1 = (double)(points[0].y - points[2].y) / (points[0].x - points[2].x), b1 = -a1 * points[0].x + points[0].y;
    double a2 = (double)(points[1].y - points[3].y) / (points[1].x - points[3].x), b2 = -a2 * points[1].x + points[1].y;
    double x = (-1 * (b1 - b2)) / (a1 - a2), y = a1 * x + b1;
    Point2d result(x, y);
    return result;
}

//QR코드 중점과의 거리를 구하는 함수
double calHeight(vector<Point>& inputs, Point2d& center, Mat& mat) {
    //윗변에 애당하는 두 점을 찾는다.
    pair<Point, Point> pair1 = make_pair(inputs[0], inputs[1]);
    pair<Point, Point> pair2;
    if (abs(inputs[0].y - inputs[1].y) > abs(inputs[0].y - inputs[3].y)) {
        pair1.second = inputs[3];
        pair2 = make_pair(inputs[1], inputs[2]);
    }
    else {
        pair2 = make_pair(inputs[2], inputs[3]);
    }
    line(mat, Point2d((pair1.first.x + pair1.second.x + mat.cols) / 2.0, (mat.rows - pair1.first.y - pair1.second.y) / 2.0), Point2d(center.x + mat.cols / 2.0, mat.rows / 2.0 - center.y), Scalar(0, 0, 255), 2);
    line(mat, Point2d((pair2.first.x + pair2.second.x + mat.cols) / 2.0, (mat.rows - pair2.first.y - pair2.second.y) / 2.0), Point2d(center.x + mat.cols / 2.0, mat.rows / 2.0 - center.y), Scalar(0, 0, 255), 2);

    //중점과 윗변과의 거리(dist1), 중점과 아랫변과의 거리(dist2)
    double dist1 = abs((pair1.first.y + pair1.second.y) / 2.0 - (double)center.y);
    double dist2 = abs((pair2.first.y + pair2.second.y) / 2.0 - (double)center.y);

    //윗변의 길이(length1), 아랫변의 길이(length2)
    double length1 = getDistance(pair1.first, pair1.second);
    double length2 = getDistance(pair2.first, pair2.second);

    //거리 연산 후 반환
    if (dist1 < dist2) {
        return Z * DEFAULT_SIDE / (length1 + (abs(length2 - length1) * dist1 / (dist1 + dist2)));
    }
    else {
        return Z * DEFAULT_SIDE / (length2 + (abs(length2 - length1) * dist2 / (dist1 + dist2)));
    }
}

//각도 측정 알고리즘
long double execute(vector<Point>& inputs, Point2d& center, double height) {
    //윗변에 애당하는 두 점을 찾는다.
    pair<Point, Point> pair1 = make_pair(inputs[0], inputs[1]);
    pair<Point, Point> pair2;
    if (abs(inputs[0].y - inputs[1].y) > abs(inputs[0].y - inputs[3].y)) {
        pair1.second = inputs[3];
        pair2 = make_pair(inputs[1], inputs[2]);
    }
    else {
        pair2 = make_pair(inputs[2], inputs[3]);
    }

    //중점과 윗변과의 거리(dist1), 중점과 아랫변과의 거리(dist2)
    double dist1 = abs((pair1.first.y + pair1.second.y) / 2.0 - (double)center.y);
    double dist2 = abs((pair2.first.y + pair2.second.y) / 2.0 - (double)center.y);

    //윗변의 길이(length1), 아랫변의 길이(length2)
    double length1 = getDistance(pair1.first, pair1.second);
    double length2 = getDistance(pair2.first, pair2.second);
    
    //alpha 계산
    double alpha = atan(dist1 / Z);
    //각도 연산 후 반환
    if (dist1 > dist2) {
        alpha = atan(dist2 / Z);
        long double a = dist2 * height / Z;
        long double b = sqrt((DEFAULT_SIDE/2) * (DEFAULT_SIDE/2) + height * height * cos(alpha) * cos(alpha) - height * height) + height * cos(alpha) - height / cos(alpha);
        long double c = DEFAULT_SIDE/2;
        return -1*(acos((a * a + c * c - b * b) / (2 * a * c)));
    }
    else {
        alpha = atan(dist1 / Z);
        long double a = dist1 * height / Z;
        long double b = sqrt((DEFAULT_SIDE / 2) * (DEFAULT_SIDE / 2) + height * height * cos(alpha) * cos(alpha) - height * height) + height * cos(alpha) - height / cos(alpha);
        long double c = (DEFAULT_SIDE / 2);
        return acos((a * a + c * c - b * b) / (2 * a * c));
    }
}

//두 점사이의 거리
double getDistance(Point& p1, Point& p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}