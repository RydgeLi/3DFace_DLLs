// Identify_dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#define maxPoints 3871
#define shuMaxPoints 121
#define markMaxPoints 6

#include <iostream>
#include <vector>
#include <math.h>
#include <io.h>
#include <fstream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

typedef struct _Point3D
{
	double x, y, z;
	double w = 1;
}Point3D;

void showRGB(string imageName, string title);
void showRGB_temp(string imageName, string title);

void getFiles(string path, vector<string> &files);
void readFiles(const char *filename, vector<Point3D> &P); //too slow !!!
void readPoint(const char *filename, vector<Point3D> &P);
void shuReadPoint(const char *filename, vector<Point3D> &P);
void readMarkPoint(const char *filename, vector<Point3D> &P);

double calculate(vector<Point3D>, vector<Point3D>, vector<Point3D>);
double calculate(vector<Point3D>, vector<Point3D>);
double calculate_depth(vector<Point3D>, vector<Point3D>);

double distance(double, double, double, double, double, double);
double distance_2D(double x1, double y1, double x2, double y2);

void Weight(vector<Point3D> &p, vector<Point3D> &mark);
void Weight(vector<Point3D> &p);

void showRGB(string imageName, string title)
{
	Mat img = imread(imageName);
	if (img.empty())
	{
		fprintf(stderr, "Can't load image %s\n", imageName);
		return;
	}

	imshow(title, img);
	waitKey();
}

void showRGB_temp(string imageName, string title)
{
	Mat img = imread(imageName);
	if (img.empty())
	{
		fprintf(stderr, "Can't load image %s\n", imageName.c_str());
		return;
	}
	imshow(title, img);
	cvWaitKey(10);
	cvDestroyWindow(title.c_str());
}

void getFiles(string path, vector<string>& files)
{
	//文件句柄
	long   hFile = 0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之，如果不是，加入列表
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

//too slow!!!
void readFiles(const char *filename, vector<Point3D> &P)
{
	fstream file;
	file.open(filename, ios::in);
	while (file.peek() != EOF)
	{
		Point3D p;
		file >> p.x;
		file >> p.y;
		file >> p.z;
		if (file.fail())
			break;
		P.push_back(p);
	}
	file.close();
}

void readPoint(const char*filename, vector<Point3D> &P)
{
	FILE *fp;
	fopen_s(&fp, filename, "r");

	if (fp == NULL)
	{
		printf("Open Error!!!");
		exit(1);
	}
	else
	{
		for (int i = 0; i<maxPoints; i++)
		{
			Point3D p;
			fscanf_s(fp, "%lf%lf%lf", &p.x, &p.y, &p.z);
			P.push_back(p);
		}
	}
	fclose(fp);
}

void shuReadPoint(const char*filename, vector<Point3D> &P)
{
	FILE *fp;
	fopen_s(&fp, filename, "r");

	if (fp == NULL)
	{
		printf("Open Error!!!");
		exit(1);
	}
	else
	{
		for (int i = 0; i<shuMaxPoints; i++)
		{
			Point3D p;
			fscanf_s(fp, "%lf%lf%lf", &p.x, &p.y, &p.z);
			P.push_back(p);
		}
	}
	fclose(fp);
}

void readMarkPoint(const char *filename, vector<Point3D> &P)
{
	FILE *fp;
	fopen_s(&fp, filename, "r");

	if (fp == NULL)
	{
		printf("Open Error!!!");
		exit(1);
	}
	else
	{
		for (int i = 0; i<markMaxPoints; i++)
		{
			Point3D p;
			fscanf_s(fp, "%lf%lf%lf", &p.x, &p.y, &p.z);
			P.push_back(p);
		}
	}
	fclose(fp);
}

double calculate(vector<Point3D> source, vector<Point3D> target, vector<Point3D> mark)
{
	Weight(source, mark);
	vector<Point3D>::iterator it1, it2;
	double dist = 0, temp = 0;
	int count = 0;
	for (it1 = source.begin(), it2 = target.begin(); it1 != source.end() && it2 != target.end(); it1++, it2++)
	{
		temp = distance(it1->x, it1->y, it1->z, it2->x, it2->y, it2->z);
		//if (temp < 10)
		{
			temp = temp*it1->w;
			dist += temp;
		}
		//temp = abs(it1->z - it2->z)*it1->w;
		//count++;
		//cout << "# " << count << " temp distance: " << temp << endl;
		//cout << "weight: " << it1->w << endl;
	}
	return dist;
}

double calculate(vector<Point3D> source, vector<Point3D> target)
{
	Weight(source);
	vector<Point3D>::iterator it1, it2;
	double dist = 0, temp = 0;
	int count = 0;
	for (it1 = source.begin(), it2 = target.begin(); it1 != source.end() && it2 != target.end(); it1++, it2++)
	{
		temp = distance(it1->x, it1->y, it1->z, it2->x, it2->y, it2->z);
		//if (temp < 10)
		{
			temp = temp*it1->w;
			dist += temp;
		}
		//temp = abs(it1->z - it2->z)*it1->w;
		//count++;
		//cout << "# " << count << " temp distance: " << temp << endl;
		//cout << "weight: " << it1->w << endl;
	}
	return dist;
}

double calculate_depth(vector<Point3D> source, vector<Point3D> target)
{
	vector<Point3D>::iterator it1, it2;
	double dist = 0, temp = 0;
	for (it1 = source.begin(), it2 = target.begin(); it1 != source.end() && it2 != target.end(); it1++, it2++) {
		temp = abs(it1->z - it2->z);
		//cout << "# " << count << " temp distance: " << temp << endl;
		//cout << "weight: " << it1->w << endl;
		dist += temp;
	}
	cout << "Total Distance: " << dist << endl;
	return dist;
}

double distance(double x1, double y1, double z1, double x2, double y2, double z2)
{
	double dist;
	dist = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2) + (z1 - z2)*(z1 - z2);
	dist = sqrt(dist);
	return dist;
}

double distance_2D(double x1, double y1, double x2, double y2)
{
	double dist;
	dist = (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
	dist = sqrt(dist);
	return dist;
}

void Weight(vector<Point3D> &p, vector<Point3D> &mark)
{
	vector<Point3D>::size_type i = 2;
	double left_eye[3], right_eye[3], nose[3], left_cheek[3], right_cheek[3], jaw[3];
	double eye_radius = 5, nose_radius = 5, check_radius = 4, jaw_radius = 2;
	vector<Point3D>::iterator w1, w2;
	int count = 0;
	for (w1 = mark.begin(); w1 != mark.end(); w1++)
	{
		count++;
		switch (count)
		{
		case 1:
			left_eye[0] = w1->x;
			left_eye[1] = w1->y;
			left_eye[2] = w1->z;
			break;
		case 2:
			right_eye[0] = w1->x;
			right_eye[1] = w1->y;
			right_eye[2] = w1->z;
			break;
		case 3:
			nose[0] = w1->x;
			nose[1] = w1->y;
			nose[2] = w1->z;
			break;
		case 4:
			left_cheek[0] = w1->x;
			left_cheek[1] = w1->y;
			left_cheek[2] = w1->z;
			break;
		case 5:
			right_cheek[0] = w1->x;
			right_cheek[1] = w1->y;
			right_cheek[2] = w1->z;
			break;
		case 6:
			jaw[0] = w1->x;
			jaw[1] = w1->y;
			jaw[2] = w1->z;
			break;
		default:
			break;
		}
	}

	double origin_x;
	double origin_y;

	for (w2 = p.begin(); w2 != p.end(); w2++)
	{
		origin_x = w2->x + mark[i].x - 25;
		origin_y = w2->y + mark[i].y - 30;

		if (distance_2D(origin_x, origin_y, left_eye[0], left_eye[1]) < eye_radius)
			w2->w += 3;
		if (distance_2D(origin_x, origin_y, right_eye[0], right_eye[1]) < eye_radius)
			w2->w += 3;
		if (distance_2D(origin_x, origin_y, nose[0], nose[1] - 10) < nose_radius)
			w2->w += 2;
		if (distance_2D(origin_x, origin_y, left_cheek[0], left_cheek[1]) < check_radius)
			w2->w += 1;
		if (distance_2D(origin_x, origin_y, right_cheek[0], right_cheek[1]) < check_radius)
			w2->w += 1;
		if (distance_2D(origin_x, origin_y, jaw[0], jaw[1]) < jaw_radius)
			w2->w += 0.5;

	}

}

void Weight(vector<Point3D> &p)
{
	double left_eye[3], right_eye[3], nose[3], left_cheek[3], right_cheek[3], jaw[3];

	double eye_radius = 8, nose_radius = 5, check_radius = 3, jaw_radius = 2;

	vector<Point3D>::iterator w1, w2;
	int count = 0;

	for (w1 = p.begin(); w1 != p.end(); w1++)
	{
		count++;
		switch (count)
		{
		case 1:
			left_eye[0] = w1->x;
			left_eye[1] = w1->y;
			left_eye[2] = w1->z;
			break;
		case 2:
			right_eye[0] = w1->x;
			right_eye[1] = w1->y;
			right_eye[2] = w1->z;
			break;
		case 3:
			nose[0] = w1->x;
			nose[1] = w1->y;
			nose[2] = w1->z;
			break;
		case 4:
			left_cheek[0] = w1->x;
			left_cheek[1] = w1->y;
			left_cheek[2] = w1->z;
			break;
		case 5:
			right_cheek[0] = w1->x;
			right_cheek[1] = w1->y;
			right_cheek[2] = w1->z;
			break;
		case 6:
			jaw[0] = w1->x;
			jaw[1] = w1->y;
			jaw[2] = w1->z;
			break;
		default:
			break;
		}
	}

	for (w2 = p.begin(); w2 != p.end(); w2++)
	{
		if (distance_2D(w2->x, w2->y, left_eye[0], left_eye[1]) < eye_radius)
			w2->w += 3;
		if (distance_2D(w2->x, w2->y, right_eye[0], right_eye[1]) < eye_radius)
			w2->w += 3;
		if (distance_2D(w2->x, w2->y, nose[0], nose[1] - 10) < nose_radius)
			w2->w += 2;
		if (distance_2D(w2->x, w2->y, left_cheek[0], left_cheek[1]) < check_radius)
			w2->w += 1;
		if (distance_2D(w2->x, w2->y, right_cheek[0], right_cheek[1]) < check_radius)
			w2->w += 1;
		if (distance_2D(w2->x, w2->y, jaw[0], jaw[1]) < jaw_radius)
			w2->w += 0.5;
	}
}
extern "C" __declspec(dllexport) void identify_shu()
{
	string probe = "002_s1";
	string canonical = "001_s1";
	string rgbPath = "D:\\BS\\shu_face\\rgb";
	string rgbProbe = rgbPath + "\\" + probe + ".bmp";
	string path = "D:\\BS\\shu_face\\aligned\\" + canonical;
	string depthResult;
	string rgbResult;

	vector<string> depthFiles, rgbFiles;
	double distance_tmp, distance_min = 1000000;

	getFiles(path, depthFiles);
	getFiles(rgbPath, rgbFiles);

	int size = depthFiles.size();

	vector<Point3D> target;
	string targetPath = path + "\\" + probe + " - " + canonical + ".txt";
	shuReadPoint(targetPath.c_str(), target);
	showRGB(rgbProbe, "probe Face");
	for (int i = 0; i < size; i++)
	{
		vector<Point3D> source;

		shuReadPoint(depthFiles[i].c_str(), source);
		cout << "源文件:	" << depthFiles[i].c_str() << endl;

		distance_tmp = calculate(source, target);

		if (distance_tmp < distance_min && distance_tmp > 0)
		{
			distance_min = distance_tmp;
			depthResult = depthFiles[i];
			rgbResult = rgbFiles[i];
		}

		showRGB_temp(rgbFiles[i], "Searching...");
	}
	cout << "识别脸深度文件为：" << depthResult.c_str() << endl;
	cout << "距离为：	" << distance_min << endl;
	cout << "识别脸RGB文件为：" << rgbResult.c_str() << endl;
	showRGB(rgbResult, "识别结果");
};

