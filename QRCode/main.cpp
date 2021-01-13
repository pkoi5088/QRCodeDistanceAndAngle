#include "opencv2/opencv.hpp"
#include <iostream>
#include <string>
#include <cmath>
#define PI 3.141592

using namespace cv;
using namespace std;

const double DEFAULT_SIDE = 48.3; //QR�ڵ� �� ���� ����(cm)
const String INPUT_IMG[5] = { "images/test2.jpg","images/test2.jpg","images/test2.jpg","images/test2.jpg","images/test2.jpg" };//5���� �̹��� ���
const int realAngle[5] = { -20,-10,0,10,20 };//5���� �̹����� ���� ����
double Z;//�����Ÿ�
Mat input_mat[5];//5���� �̹����� ������ mat

//�Լ� �����
void QRCodeDetect(String img, vector<Point>& ret, Mat& rMat);
Point2d getCenterPoint(vector<Point>& points);
double calHeight(vector<Point>& inputs, Point2d& center, Mat& mat);
long double execute(vector<Point>& inputs, Point2d& center, double height);
double getDistance(Point& p1, Point& p2);

int main(void)
{
    //�����Ÿ� �Է�
    cout << "�����Ÿ�(Z)�Է�: ";
    cin >> Z;

    //�� �̹����� ���ؼ�
    for (int k = 0; k < 5; k++) {
        vector<Point> input_points;
        double input_height, input_angle;
        cout << "------------------------------------------------\n";
        cout << "�Է��̹���(INPUT_IMG["<<k<<"]) ��� �Է�: " << INPUT_IMG[k] << endl;

        //QR�ڵ� ����
        QRCodeDetect(INPUT_IMG[k], input_points, input_mat[k]);

        //������ ���� �ʾҴٸ� �ش� �̹����� �н�
        if (input_points.size() == 0) {
            cout << "input_point is empty!" << endl;
            continue;
        }

        //��ǥȭ(������ ���߾��� 0,0)
        for (int i = 0; i < 4; i++) {
            input_points[i].x -= input_mat[k].cols / 2.0;
            input_points[i].y = input_mat[k].rows / 2.0 - input_points[i].y;
        }

        //����� QR�ڵ��� ������ ��ǥ�� ���Ѵ�.
        Point2d inputCenter = getCenterPoint(input_points);

        //���̿� ������ ���Ѵ�.
        input_height = calHeight(input_points, inputCenter, input_mat[k]);
        input_angle = execute(input_points, inputCenter, input_height);

        //��� ���
        cout << "distance: " << round((input_height) * 100) / 100 << " cm" << endl;
        cout << "angle: " << round((input_angle * 180 / PI) * 100) / 100 << "��" << endl;
        string sDistance = to_string(round((input_height) * 100) / 100);
        string sAngle = to_string(round((input_angle * 180 / PI) * 100) / 100);
        putText(input_mat[k], "distance: " + sDistance.substr(0, sDistance.find('.') + 3) + "cm", Point(20, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
        putText(input_mat[k], "angle: " + sAngle.substr(0, sAngle.find('.') + 3) + "degree", Point(20, 60), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));
    }

    //������ ���
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

//QR�ڵ� ����
void QRCodeDetect(String img, vector<Point>& ret, Mat& rMat)
{
    QRCodeDetector detector;

    //�̹��� �Է�, ũ������, threshold
    Mat frame = imread(img, 0);
    Mat R, B;
    resize(frame, R, Size(frame.cols, frame.rows), 0, 0, 1);
    threshold(R, B, 100, 255, THRESH_BINARY);
    if (B.empty()) {
        cerr << "Frame load failed!" << endl;
        return;
    }

    //QR�ڵ� ���� �� ����� �簢�� ����
    vector<Point> points;
    String info = detector.detectAndDecode(B, points);

    //����Ǿ��� ���
    if (!info.empty()) {
        //�簢���� �׸� �� 4���� ���� ���ڸ� ����Ѵ�.
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

//�� �밢���� ������ ��ġ�� ���ϴ� �Լ�
Point2d getCenterPoint(vector<Point>& points) {
    double a1 = (double)(points[0].y - points[2].y) / (points[0].x - points[2].x), b1 = -a1 * points[0].x + points[0].y;
    double a2 = (double)(points[1].y - points[3].y) / (points[1].x - points[3].x), b2 = -a2 * points[1].x + points[1].y;
    double x = (-1 * (b1 - b2)) / (a1 - a2), y = a1 * x + b1;
    Point2d result(x, y);
    return result;
}

//QR�ڵ� �������� �Ÿ��� ���ϴ� �Լ�
double calHeight(vector<Point>& inputs, Point2d& center, Mat& mat) {
    //������ �ִ��ϴ� �� ���� ã�´�.
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

    //������ �������� �Ÿ�(dist1), ������ �Ʒ������� �Ÿ�(dist2)
    double dist1 = abs((pair1.first.y + pair1.second.y) / 2.0 - (double)center.y);
    double dist2 = abs((pair2.first.y + pair2.second.y) / 2.0 - (double)center.y);

    //������ ����(length1), �Ʒ����� ����(length2)
    double length1 = getDistance(pair1.first, pair1.second);
    double length2 = getDistance(pair2.first, pair2.second);

    //�Ÿ� ���� �� ��ȯ
    if (dist1 < dist2) {
        return Z * DEFAULT_SIDE / (length1 + (abs(length2 - length1) * dist1 / (dist1 + dist2)));
    }
    else {
        return Z * DEFAULT_SIDE / (length2 + (abs(length2 - length1) * dist2 / (dist1 + dist2)));
    }
}

//���� ���� �˰���
long double execute(vector<Point>& inputs, Point2d& center, double height) {
    //������ �ִ��ϴ� �� ���� ã�´�.
    pair<Point, Point> pair1 = make_pair(inputs[0], inputs[1]);
    pair<Point, Point> pair2;
    if (abs(inputs[0].y - inputs[1].y) > abs(inputs[0].y - inputs[3].y)) {
        pair1.second = inputs[3];
        pair2 = make_pair(inputs[1], inputs[2]);
    }
    else {
        pair2 = make_pair(inputs[2], inputs[3]);
    }

    //������ �������� �Ÿ�(dist1), ������ �Ʒ������� �Ÿ�(dist2)
    double dist1 = abs((pair1.first.y + pair1.second.y) / 2.0 - (double)center.y);
    double dist2 = abs((pair2.first.y + pair2.second.y) / 2.0 - (double)center.y);

    //������ ����(length1), �Ʒ����� ����(length2)
    double length1 = getDistance(pair1.first, pair1.second);
    double length2 = getDistance(pair2.first, pair2.second);
    
    //alpha ���
    double alpha = atan(dist1 / Z);
    //���� ���� �� ��ȯ
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

//�� �������� �Ÿ�
double getDistance(Point& p1, Point& p2) {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}