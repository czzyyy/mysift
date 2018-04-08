
// mysiftDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "mysift.h"
#include "mysiftDlg.h"
#include "afxdialogex.h"


#include <string>
#include<iostream>

using namespace cv;
using namespace std;

#define KDTREE_BBF_MAX_NN_CHKS 200
#define NN_SQ_DIST_RATIO_THR 0.5

//SIFT算法头文件
//加extern "C"，告诉编译器按C语言的方式编译和连接
extern "C"
{
#include "imgfeatures.h"
#include "kdtree.h"
#include "minpq.h"
#include "sift.h"
#include "utils.h"
#include "xform.h"
}


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CmysiftDlg 对话框



CmysiftDlg::CmysiftDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmysiftDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmysiftDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH_PIC2, edit_path2);
	DDX_Control(pDX, IDC_PROGRESS1, m_pro);
	DDX_Control(pDX, IDC_EDIT_NUMBER, m_edit_number);
}

BEGIN_MESSAGE_MAP(CmysiftDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PIC2, &CmysiftDlg::OnBnClickedButtonPic2)
	ON_BN_CLICKED(IDC_BUTTON_HORIZONTAL, &CmysiftDlg::OnBnClickedButtonHorizontal)
	ON_BN_CLICKED(IDC_BUTTON_SHOWPOINTS, &CmysiftDlg::OnBnClickedButtonShowpoints)
	ON_BN_CLICKED(IDC_BUTTON_STACK, &CmysiftDlg::OnBnClickedButtonStack)
	ON_BN_CLICKED(IDC_BUTTON_STACK_RANSAC, &CmysiftDlg::OnBnClickedButtonStackRansac)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_FINAL, &CmysiftDlg::OnBnClickedButtonShowFinal)
	ON_BN_CLICKED(IDC_BUTTON_START, &CmysiftDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_NUMBER_OK, &CmysiftDlg::OnBnClickedButtonNumberOk)
END_MESSAGE_MAP()


// CmysiftDlg 消息处理程序

BOOL CmysiftDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	PicPath = "";
	HorizontalPic = NULL;
	ShowpointsPic = NULL;
	StackPic = NULL;
	StackRansacPic = NULL;
	ShowFinalPic = NULL;
	count = 0;
	times = 0;
	ImageNumber = 0;
	ImageArray = NULL;

	m_pro.SetRange(0, 100);
	m_pro.SetStep(10);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CmysiftDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CmysiftDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CmysiftDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CmysiftDlg::ShiftProgress(){
	m_pro.StepIt();
	m_pro.StepIt();
	IplImage *img1, *img2;
	IplImage* xformed_proc12 = NULL;
	times = 0;
	if (ImageNumber > 1){
		m_pro.StepIt();
		m_pro.StepIt();
		while (times < (ImageNumber-1)){
			for (int i = 1; i < ImageNumber; i++){
				img1 = ImageArray[0];
				img2 = ImageArray[i];
				if (img2 != NULL&&img1 != NULL){
					xformed_proc12 = spliceImage(img1, img2);
					if (xformed_proc12 != NULL){
						if (NULL != img1)
						{
							cvReleaseImage(&img1);
						}

						if (NULL != img2)
						{
							cvReleaseImage(&img2);
						}
						ImageArray[0] = xformed_proc12;
						ImageArray[i] = NULL;
						times++;
						break;
					}
				}
			}
		}
		m_pro.StepIt();
		m_pro.StepIt();
	}
	else{
		AfxMessageBox(_T("输入的数目无效!"));
	}
	m_pro.StepIt();
	m_pro.StepIt();
	xformed_proc12 = ImageArray[0];
	if (NULL != xformed_proc12)
	{
		cvSaveImage("result.png", xformed_proc12);
		cvWaitKey(10);
	}
	else if (NULL == xformed_proc12){
		AfxMessageBox(_T("图片拼接失败!"));
	}
	m_pro.StepIt();
	m_pro.StepIt();
	/*if (NULL != xformed_proc12)
	{
		cvReleaseImage(&xformed_proc12);
	}*/
}
void CmysiftDlg::CalcFourCorner(CvMat* &H, CvPoint& leftTop, CvPoint& leftBottom, CvPoint& rightTop, CvPoint& rightBottom, IplImage* img2){
	//计算图2的四个角经矩阵H变换后的坐标
	double v2[] = { 0, 0, 1 };//左上角
	double v1[3];//变换后的坐标值
	CvMat V2 = cvMat(3, 1, CV_64FC1, v2);
	CvMat V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);//矩阵乘法
	leftTop.x = cvRound(v1[0] / v1[2]);
	leftTop.y = cvRound(v1[1] / v1[2]);
	//cvCircle(xformed,leftTop,7,CV_RGB(255,0,0),2);

	//将v2中数据设为左下角坐标
	v2[0] = 0;
	v2[1] = img2->height;
	V2 = cvMat(3, 1, CV_64FC1, v2);
	V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);
	leftBottom.x = cvRound(v1[0] / v1[2]);
	leftBottom.y = cvRound(v1[1] / v1[2]);
	//cvCircle(xformed,leftBottom,7,CV_RGB(255,0,0),2);

	//将v2中数据设为右上角坐标
	v2[0] = img2->width;
	v2[1] = 0;
	V2 = cvMat(3, 1, CV_64FC1, v2);
	V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);
	rightTop.x = cvRound(v1[0] / v1[2]);
	rightTop.y = cvRound(v1[1] / v1[2]);
	//cvCircle(xformed,rightTop,7,CV_RGB(255,0,0),2);

	//将v2中数据设为右下角坐标
	v2[0] = img2->width;
	v2[1] = img2->height;
	V2 = cvMat(3, 1, CV_64FC1, v2);
	V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);
	rightBottom.x = cvRound(v1[0] / v1[2]);
	rightBottom.y = cvRound(v1[1] / v1[2]);
}
IplImage* CmysiftDlg::spliceImage(IplImage* img1, IplImage* img2){
	struct feature *feat1, *feat2;//feat1：图1的特征点数组，feat2：图2的特征点数组
	int n1, n2;//n1:图1中的特征点个数，n2：图2中的特征点个数
	struct feature *feat;//每个特征点
	struct kd_node *kd_root;//k-d树的树根
	struct feature **nbrs;//当前特征点的最近邻点数组
	CvMat * H = NULL;//RANSAC算法求出的变换矩阵
	struct feature **inliers;//精RANSAC筛选后的内点数组
	int n_inliers;//经RANSAC算法筛选后的内点个数,即feat2中具有符合要求的特征点的个数

	IplImage *xformed = NULL, *xformed_proc = NULL;//xformed临时拼接图，即只将图2变换后的图,xformed_proc是最终合成的图

	//图2的四个角经矩阵H变换后的坐标
	CvPoint leftTop, leftBottom, rightTop, rightBottom;
	///////////////////////////////////////////////////////////////////

	//特征点检测
	n1 = sift_features(img1, &feat1);//检测图1中的SIFT特征点,n1是图1的特征点个数
	//提取并显示第2幅图片上的特征点
	n2 = sift_features(img2, &feat2);//检测图2中的SIFT特征点，n2是图2的特征点个数

	//画出特征点图像
	IplImage* img1_feat, *img2_feat;
	img1_feat = cvCloneImage(img1);
	img2_feat = cvCloneImage(img2);
	draw_features(img1_feat, feat1, n1);
	draw_features(img2_feat, feat2, n2);


	IplImage * stacked = stack_imgs_horizontal(img1, img2);
	HorizontalPic = stack_imgs_horizontal(img1, img2);
	ShowpointsPic = stack_imgs_horizontal(img1_feat, img2_feat);
	//cvShowImage("horizontal", stacked);
	//cvWaitKey(0);

	//特征匹配
	//根据图1的特征点集feat1建立k-d树，返回k-d树根给kd_root
	kd_root = kdtree_build(feat1, n1);
	CvPoint pt1, pt2;//连线的两个端点
	double d0, d1;//feat2中每个特征点到最近邻和次近邻的距离
	int matchNum = 0;//经距离比值法筛选后的匹配点对的个数
	//遍历特征点集feat2，针对feat2中每个特征点feat，选取符合距离比值条件的匹配点，放到feat的fwd_match域中
	for (int i = 0; i < n2; i++)
	{
		feat = feat2 + i;//第i个特征点的指针
		//在kd_root中搜索目标点feat的2个最近邻点，存放在nbrs中，返回实际找到的近邻点个数
		int k = kdtree_bbf_knn(kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS);
		if (k == 2)
		{
			d0 = descr_dist_sq(feat, nbrs[0]);//feat与最近邻点的距离的平方
			d1 = descr_dist_sq(feat, nbrs[1]);//feat与次近邻点的距离的平方
			//若d0和d1的比值小于阈值NN_SQ_DIST_RATIO_THR，则接受此匹配，否则剔除
			if (d0 < d1 * NN_SQ_DIST_RATIO_THR)
			{   //将目标点feat和最近邻点作为匹配点对

				pt2 = cvPoint(cvRound(feat->x), cvRound(feat->y));//图2中点的坐标
				pt1 = cvPoint(cvRound(nbrs[0]->x), cvRound(nbrs[0]->y));//图1中点的坐标(feat的最近邻点)
				pt2.x += img1->width;//由于两幅图是左右排列的，pt2的横坐标加上图1的宽度，作为连线的终点
				cvLine(stacked, pt1, pt2, CV_RGB(255, 0, 255), 1, 8, 0);//画出连线
				matchNum++;//统计匹配点对的个数
				feat2[i].fwd_match = nbrs[0];//使点feat的fwd_match域指向其对应的匹配点(feat1中的点)
			}
		}
		free(nbrs);//释放近邻数组
	}
	StackPic = stacked;
	/*cvShowImage("NO RANSAC", stacked);
	cvWaitKey(0);*/

	//利用RANSAC算法筛选匹配点,计算变换矩阵H，
	//无论img1和img2的左右顺序，H永远是将feat2中的特征点变换为其匹配点，即将img2中的点变换为img1中的对应点
	H = ransac_xform(feat2, n2, FEATURE_FWD_MATCH, lsq_homog, 4, 0.01, homog_xfer_err, 3.0, &inliers, &n_inliers);

	//若能成功计算出变换矩阵，即两幅图中有共同区域
	IplImage* stacked_ransac;
	//stacked_ransac = stack_imgs(img1, img2);
	//将两张图按左右顺序排列
	stacked_ransac = stack_imgs_horizontal(img1, img2);

	if (H)
	{
		int invertNum = 0;//统计pt2.x > pt1.x的匹配点对的个数，来判断img1中是否右图  

		//遍历经RANSAC算法筛选后的特征点集合inliers，找到每个特征点的匹配点
		for (int i = 0; i < n_inliers; i++)
		{
			feat = inliers[i];//第i个特征点  
			pt2 = cvPoint(cvRound(feat->x), cvRound(feat->y));//图2中点的坐标  
			pt1 = cvPoint(cvRound(feat->fwd_match->x), cvRound(feat->fwd_match->y));//图1中点的坐标(feat的匹配点)  

			//统计匹配点的左右位置关系，来判断图1和图2的左右位置关系  
			if (pt2.x > pt1.x)
				invertNum++;

			// pt2.y += img1->height;//由于两幅图是左右排列的，pt2的横坐标加上图1的宽度，作为连线的终点 
			pt2.x += img1->width;//由于两幅图是左右排列的，pt2的横坐标加上图1的宽度，作为连线的终点  
			cvLine(stacked_ransac, pt1, pt2, CV_RGB(255, 0, 255), 1, 8, 0);//在匹配图上画出连线  
		}
		StackRansacPic = stacked_ransac;

		if (invertNum > (n_inliers / 2)){
			IplImage * tempImg;
			tempImg = img1;
			img1 = img2;
			img2 = tempImg;
			int tempn;
			tempn = n1;
			n1 = n2;
			n2 = tempn;
			feature * tempFeature;
			tempFeature = feat1;
			feat1 = feat2;
			feat2 = tempFeature;

			kd_root = kdtree_build(feat1, n1);
			for (int i = 0; i < n2; i++)
			{
				feat = feat2 + i;//第i个特征点的指针
				//在kd_root中搜索目标点feat的2个最近邻点，存放在nbrs中，返回实际找到的近邻点个数
				int k = kdtree_bbf_knn(kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS);
				if (k == 2)
				{
					d0 = descr_dist_sq(feat, nbrs[0]);//feat与最近邻点的距离的平方
					d1 = descr_dist_sq(feat, nbrs[1]);//feat与次近邻点的距离的平方
					//若d0和d1的比值小于阈值NN_SQ_DIST_RATIO_THR，则接受此匹配，否则剔除
					if (d0 < d1 * NN_SQ_DIST_RATIO_THR)
					{   //将目标点feat和最近邻点作为匹配点对
						matchNum++;//统计匹配点对的个数
						feat2[i].fwd_match = nbrs[0];//使点feat的fwd_match域指向其对应的匹配点(feat1中的点)
					}
				}
				//free(nbrs);//释放近邻数组
			}

			H = ransac_xform(feat2, n2, FEATURE_FWD_MATCH, lsq_homog, 4, 0.01, homog_xfer_err, 3.0, &inliers, &n_inliers);
			stacked_ransac = stack_imgs_horizontal(img1, img2);
		}
	}

	if (H)
	{
		//全景拼接
		//若能成功计算出变换矩阵，即两幅图中有共同区域，才可以进行全景拼接
		//拼接图像，img1是左图，img2是右图
		CalcFourCorner(H, leftTop, leftBottom, rightTop, rightBottom, img2);//计算图2的四个角经变换后的坐标
		//为拼接结果图xformed分配空间,高度为图1图2高度的较大者，根据变换后的图2右上角和右下角点的位置决定拼接图的宽度
		xformed = cvCreateImage(cvSize(MAX(rightTop.x, rightBottom.x), MIN(img1->height, img2->height)), IPL_DEPTH_8U, 3);

		//用变换矩阵H对右图img2做投影变换(变换后会有坐标平移)，结果放到xformed中
		cvWarpPerspective(img2, xformed, H, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));

		//处理后的拼接图，克隆自xformed
		xformed_proc = cvCloneImage(xformed);
		if (img1->width > xformed->width){
			xformed = cvCreateImage(cvSize(MAX(rightTop.x, rightBottom.x), MAX(img1->height, img2->height)), IPL_DEPTH_8U, 3);
			cvWarpPerspective(img2, xformed, H, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
			xformed_proc = cvCloneImage(img1);
			//重叠区域右边的部分完全取自图1
			/*cout << "state2" << endl;*/
			cvSetImageROI(img1, cvRect(MAX(rightTop.x, rightBottom.x), 0, img1->width - MAX(rightTop.x, rightBottom.x), xformed_proc->height));

			cvSetImageROI(xformed_proc, cvRect(MAX(rightTop.x, rightBottom.x), 0, img1->width - MAX(rightTop.x, rightBottom.x), xformed_proc->height));
			cvAddWeighted(img1, 1, img1, 0, 0, xformed_proc);
			cvResetImageROI(img1);
			cvResetImageROI(xformed_proc);
		}


		//重叠区域左边的部分完全取自图1
		cvSetImageROI(img1, cvRect(0, 0, MIN(leftTop.x, leftBottom.x), xformed_proc->height));

		cvSetImageROI(xformed, cvRect(0, 0, MIN(leftTop.x, leftBottom.x), xformed_proc->height));

		cvSetImageROI(xformed_proc, cvRect(0, 0, MIN(leftTop.x, leftBottom.x), xformed_proc->height));

		cvAddWeighted(img1, 1, xformed, 0, 0, xformed_proc);	

		cvResetImageROI(img1);
		cvResetImageROI(xformed);
		cvResetImageROI(xformed_proc);

		

		////////////////////////////////////////////////////////////
		//图像融合
		//采用加权平均的方法融合重叠区域
		int start = MIN(leftTop.x, leftBottom.x);//开始位置，即重叠区域的左边界
		double processWidth = img1->width - start;//重叠区域的宽度
		double alpha = 1;//img1中像素的权重
		for (int i = 0; i < xformed_proc->height; i++)//遍历行
		{
			const uchar * pixel_img1 = ((uchar *)(img1->imageData + img1->widthStep * i));//img1中第i行数据的指针
			const uchar * pixel_xformed = ((uchar *)(xformed->imageData + xformed->widthStep * i));//xformed中第i行数据的指针
			uchar * pixel_xformed_proc = ((uchar *)(xformed_proc->imageData + xformed_proc->widthStep * i));//xformed_proc中第i行数据的指针
			for (int j = start; j < img1->width; j++)//遍历重叠区域的列
			{
				//如果遇到图像xformed中无像素的黑点，则完全拷贝图1中的数据
				if (pixel_xformed[j * 3] < 50 && pixel_xformed[j * 3 + 1] < 50 && pixel_xformed[j * 3 + 2] < 50)
				{
					alpha = 1;
				}
				else
				{   //img1中像素的权重，与当前处理点距重叠区域左边界的距离成正比
					alpha = (processWidth - (j - start)) / processWidth;
				}
				pixel_xformed_proc[j * 3] = pixel_img1[j * 3] * alpha + pixel_xformed[j * 3] * (1 - alpha);//B通道
				pixel_xformed_proc[j * 3 + 1] = pixel_img1[j * 3 + 1] * alpha + pixel_xformed[j * 3 + 1] * (1 - alpha);//G通道
				pixel_xformed_proc[j * 3 + 2] = pixel_img1[j * 3 + 2] * alpha + pixel_xformed[j * 3 + 2] * (1 - alpha);//R通道
			}
		}

	}
	else //无法计算出变换矩阵，即两幅图中没有重合区域
	{
		return NULL;
	}

	///////////////////////////////////////////////////////////////////////////
	kdtree_release(kd_root);//释放kd树
	//只有在RANSAC算法成功算出变换矩阵时，才需要进一步释放下面的内存空间
	if (H)
	{
		cvReleaseMat(&H);//释放变换矩阵H
		free(inliers);//释放内点数组
	}
	ShowFinalPic = xformed_proc;
	//cvShowImage("final result",xformed_proc);
	//cvWaitKey(0);

	if (NULL != xformed)
	{
		cvReleaseImage(&xformed);
	}
	return xformed_proc;
}




void CmysiftDlg::OnBnClickedButtonPic2()
{
	// TODO: Add your control notification handler code here
	// 打开文件 定义四种格式的文件bmp gif jpg tiff png
	//ImageList
	CString filter;
	filter = "所有文件(*.bmp,*.jpg,*.gif,*tiff,*png)|*.bmp;*.jpg;*.gif;*.tiff;*.png| BMP(*.bmp)|*.bmp| PNG(*.png)|*.png| JPG(*.jpg)|*.jpg| GIF(*.gif)|*.gif| TIFF(*.tiff)|*.tiff||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter, NULL);
	CString tmp;
	//按下确定按钮 dlg.DoModal() 函数显示对话框
	if (ImageNumber == 0){
		AfxMessageBox(_T("输入的数目无效!"));
	}
	if (count < ImageNumber)
	{
		if (dlg.DoModal() == IDOK){
			count++;
			tmp = dlg.GetPathName();
			PicPath = PicPath + tmp;     //获取文件路径名   如D:\pic\abc.bmp
		}
		edit_path2.SetWindowTextW(PicPath);
		string path = (LPCSTR)CStringA(tmp);
		if (path == ""){
			AfxMessageBox(_T("路径不对!"));
			edit_path2.SetWindowTextW(_T(""));
		}
		else{
			ImageArray[count - 1] = (cvLoadImage(path.c_str()));
		}	
	}
	else{
		AfxMessageBox(_T("图片已经达到指定数目!"));
	}
}


void CmysiftDlg::OnBnClickedButtonHorizontal()
{
	// TODO: Add your control notification handler code here
	cvShowImage("horizontal", HorizontalPic);
	HorizontalPic = NULL;
}


void CmysiftDlg::OnBnClickedButtonShowpoints()
{
	// TODO: Add your control notification handler code here
	cvShowImage("showpoints", ShowpointsPic);
	ShowpointsPic = NULL;
}


void CmysiftDlg::OnBnClickedButtonStack()
{
	// TODO: Add your control notification handler code here
	cvShowImage("no ransac" , StackPic);
	StackPic = NULL;
}


void CmysiftDlg::OnBnClickedButtonStackRansac()
{
	// TODO: Add your control notification handler code here
	cvShowImage("ransac" , StackRansacPic);//显示经RANSAC算法筛选后的匹配图 
	StackRansacPic = NULL;
}


void CmysiftDlg::OnBnClickedButtonShowFinal()
{
	// TODO: Add your control notification handler code here
	cvShowImage("final result" , ShowFinalPic);
	ShowFinalPic = NULL;
	edit_path2.SetWindowTextW(_T(""));
	m_pro.SetPos(0);
	cvReleaseImage(&HorizontalPic);
	cvReleaseImage(&ShowpointsPic);
	cvReleaseImage(&StackPic);
	cvReleaseImage(&StackRansacPic);
	cvReleaseImage(&ShowFinalPic);
	for (int i = 0; i < ImageNumber; i++){
		cvReleaseImage(&ImageArray[i]);
	}
	count = 0;
	ImageNumber = 0;
	PicPath = "";
}

void CmysiftDlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here
	this->ShiftProgress();
}


void CmysiftDlg::OnBnClickedButtonNumberOk()
{
	// TODO: Add your control notification handler code here
	CString tmp;
	m_edit_number.GetWindowTextW(tmp);
	ImageNumber = _ttoi(tmp);
	if (ImageNumber <= 1){
		AfxMessageBox(_T("输入的数目无效!"));
	}
	else{
		AfxMessageBox(_T("输入的数目为：") + tmp);
	}
	ImageArray = new IplImage*[ImageNumber];
}
