#include <time.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <opencv2/imgproc/types_c.h>

using namespace std;
using namespace cv;

int valuearray[2000] = { 0 };//记录连通域数值对应关系

class colorobj
{
public:
	int value;
	Scalar mycolor;
};

vector<colorobj> setcolor;//收集需要上色的灰度对象

bool equal255or0(int &value)//判断元素是否等于255或者0
{
	if (value == 255 || value == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void setvalue(int &value, int &minimun)//设置配对值
{
	if (value != 0)
	{
		if (valuearray[value] > minimun && valuearray[value] != 255)
		{
			int savemidvalue = valuearray[value];
			while (true)//将映射表调整
			{
				if (valuearray[savemidvalue] > minimun)
				{
					int mid = valuearray[savemidvalue];
					valuearray[savemidvalue] = minimun;
					savemidvalue = mid;
				}
				else
				{
					break;
				}
			}
			valuearray[value] = minimun;
		}
		value = minimun;
	}
}

void compare(int &value, int &minimun)//比较大小
{
	if (value != 0 && value != 255)
	{
		if (minimun >= value)
		{
			minimun = value;
		}
	}
}

Scalar GetRandomColor()//彩色显示
{
	uchar r = 255 * (rand() / (1.0 + RAND_MAX));
	uchar g = 255 * (rand() / (1.0 + RAND_MAX));
	uchar b = 255 * (rand() / (1.0 + RAND_MAX));
	return cv::Scalar(b, g, r);
}


int rgb2gray(Mat src, Mat dest)
{
	//RGB图像转灰度图
	//用at获取每一个像素点值，对于RGB图有3个通道
	//用将at获得的数据转换成Vec3b类型,分别存放3个通道值，类型为uchar
	//用优化后的心理学灰度图公式写入新的每点像素值
	int r, g, b;
	for (int i = 0; i < src.rows; i++)
	{
		uchar* data = dest.ptr<uchar>(i);
		for (int j = 0; j < src.cols; j++)
		{
			b = src.at<Vec3b>(i, j)[0];
			g = src.at<Vec3b>(i, j)[1];
			r = src.at<Vec3b>(i, j)[2];
			data[j] = (r * 76 + g * 150 + b * 30) >> 8;
		}
	}
	return 1;
}

int gray2bin(Mat src, Mat dest, int b)
{
	//灰度图转二值化图
	//用at获取每点像素值，灰度图只有一通道
	//如果像素阈值小于100(颜色较深的细胞部分)，就将该像素值设为255，否则为0
	uchar pixelvalue;
	for (int i = 0; i < src.rows; i++)
	{
		uchar* data = dest.ptr<uchar>(i);
		for (int j = 0; j < src.cols; j++)
		{
			pixelvalue = src.at<uchar>(i, j);
			if (pixelvalue < b)
				data[j] = 255;
			else
				data[j] = 0;
		}
	}
	return 1;
}

void main()
{
	Mat img = imread("E:\\1364009764\\FileRecv\\数字图像处理\\exam.png");
	imshow("原图", img);
	Mat myrgb2gray(img.rows, img.cols, CV_8UC1);
	rgb2gray(img, myrgb2gray);	//自己写的RGB转灰度图
	imshow("灰度图", myrgb2gray);
	Mat mygray2bin(myrgb2gray.rows, myrgb2gray.cols, CV_8UC1);
	gray2bin(myrgb2gray, mygray2bin, 100);	//自己写的灰度图转二值化图
	imshow("二值化图", mygray2bin);

	int contourmark = 1; //连通域标志
	Mat mybinInt;//将图像的像素变量从uchar转为int，以防止后面标志位大于255程序无法工作的情况
	mygray2bin.convertTo(mybinInt, CV_32SC1);//将图像像素变量转为int

	//遍历图像，搜索连通域
	for (int Y = 1; Y < mybinInt.rows - 1; Y++)//遍历图像,Y为行，X为列
		for (int X = 1; X < mybinInt.cols - 1; X++)
		{
			if (mybinInt.at<int>(Y, X) != 0)//记住这里是先行后列
			{
				//如果不属于任何一个连通域
				if (mybinInt.at<int>(Y, X) == 255 && //本元素
					equal255or0(mybinInt.at<int>(Y - 1, X)) && //上方元素
					equal255or0(mybinInt.at<int>(Y + 1, X)) && //下方元素
					equal255or0(mybinInt.at<int>(Y, X - 1)) && //左方元素
					equal255or0(mybinInt.at<int>(Y, X + 1)))//右方元素
				{
					valuearray[contourmark] = contourmark;
					mybinInt.at<int>(Y, X) = contourmark;
					if (mybinInt.at<int>(Y - 1, X) == 255)
					{
						mybinInt.at<int>(Y - 1, X) = contourmark;
					}
					if (mybinInt.at<int>(Y + 1, X) == 255)
					{
						mybinInt.at<int>(Y + 1, X) = contourmark;
					}
					if (mybinInt.at<int>(Y, X - 1) == 255)
					{
						mybinInt.at<int>(Y, X - 1) = contourmark;
					}
					if (mybinInt.at<int>(Y, X + 1) == 255)
					{
						mybinInt.at<int>(Y, X + 1) = contourmark;
					}
					contourmark++;
					if (contourmark == 255)//防止冲突
					{
						valuearray[contourmark] = 255;
						contourmark = 256;
					}
				}
				else//已经属于某一个连通域
				{
					int getminimum = 200000000;
					//取得上下左右最小的标志位
					compare(mybinInt.at<int>(Y, X), getminimum);
					compare(mybinInt.at<int>(Y - 1, X), getminimum);
					compare(mybinInt.at<int>(Y + 1, X), getminimum);
					compare(mybinInt.at<int>(Y, X - 1), getminimum);
					compare(mybinInt.at<int>(Y, X + 1), getminimum);
					//将最小的标志位赋值给目标
					setvalue(mybinInt.at<int>(Y, X), getminimum);
					setvalue(mybinInt.at<int>(Y - 1, X), getminimum);
					setvalue(mybinInt.at<int>(Y + 1, X), getminimum);
					setvalue(mybinInt.at<int>(Y, X - 1), getminimum);
					setvalue(mybinInt.at<int>(Y, X + 1), getminimum);
				}
			}
		}

	for (size_t i = 1; i <= contourmark; i++)//将同一个连通域的对象映射表调整好，做完这一步映射表制作完成
	{
		valuearray[i] = valuearray[valuearray[i]];
	}

	for (int Y = 1; Y < mybinInt.rows; Y++)//根据映射表对图像像素进行重新赋值
		for (int X = 1; X < mybinInt.cols; X++)
		{
			if (mybinInt.at<int>(Y, X) != 0)
			{
				mybinInt.at<int>(Y, X) = valuearray[mybinInt.at<int>(Y, X)];
			}
		}

	for (int j = 1; j < contourmark; j++)//获得需要上色的对象值
	{
		if (j == 255)//跳过无意义值
		{
			continue;
		}
		bool dopush = true;
		for (int i = 0; i < setcolor.size(); i++)
		{
			if (setcolor[i].value == valuearray[j])
			{
				dopush = false;
				break;
			}
		}
		if (dopush == true)
		{
			colorobj mycolorobj;
			mycolorobj.value = valuearray[j];
			mycolorobj.mycolor = GetRandomColor();
			setcolor.push_back(mycolorobj);
		}
	}

	//彩色显示
	Mat colorLabelImg;
	colorLabelImg.create(mybinInt.rows, mybinInt.cols, CV_8UC3);
	colorLabelImg = Scalar::all(255);//初始化，将待显示图片的背景设置为白色

	for (int Y = 0; Y < mybinInt.rows; Y++)//颜色填充
		for (int X = 0; X < mybinInt.cols; X++)
		{
			if (mybinInt.at<int>(Y, X) != 0)
			{
				for (int i = 0; i < setcolor.size(); i++)
				{
					if (mybinInt.at<int>(Y, X) == setcolor[i].value)
					{
						colorLabelImg.at<Vec3b>(Y, X)[0] = setcolor[i].mycolor[0];
						colorLabelImg.at<Vec3b>(Y, X)[1] = setcolor[i].mycolor[1];
						colorLabelImg.at<Vec3b>(Y, X)[2] = setcolor[i].mycolor[2];
						break;
					}
				}
			}
		}
	imwrite("标记后.bmp", colorLabelImg);
	imshow("标记后的图", colorLabelImg);
	waitKey(0);
}