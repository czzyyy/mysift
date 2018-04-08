
// mysiftDlg.cpp : ʵ���ļ�
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

//SIFT�㷨ͷ�ļ�
//��extern "C"�����߱�������C���Եķ�ʽ���������
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


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CmysiftDlg �Ի���



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


// CmysiftDlg ��Ϣ�������

BOOL CmysiftDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
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
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CmysiftDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
		AfxMessageBox(_T("�������Ŀ��Ч!"));
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
		AfxMessageBox(_T("ͼƬƴ��ʧ��!"));
	}
	m_pro.StepIt();
	m_pro.StepIt();
	/*if (NULL != xformed_proc12)
	{
		cvReleaseImage(&xformed_proc12);
	}*/
}
void CmysiftDlg::CalcFourCorner(CvMat* &H, CvPoint& leftTop, CvPoint& leftBottom, CvPoint& rightTop, CvPoint& rightBottom, IplImage* img2){
	//����ͼ2���ĸ��Ǿ�����H�任�������
	double v2[] = { 0, 0, 1 };//���Ͻ�
	double v1[3];//�任�������ֵ
	CvMat V2 = cvMat(3, 1, CV_64FC1, v2);
	CvMat V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);//����˷�
	leftTop.x = cvRound(v1[0] / v1[2]);
	leftTop.y = cvRound(v1[1] / v1[2]);
	//cvCircle(xformed,leftTop,7,CV_RGB(255,0,0),2);

	//��v2��������Ϊ���½�����
	v2[0] = 0;
	v2[1] = img2->height;
	V2 = cvMat(3, 1, CV_64FC1, v2);
	V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);
	leftBottom.x = cvRound(v1[0] / v1[2]);
	leftBottom.y = cvRound(v1[1] / v1[2]);
	//cvCircle(xformed,leftBottom,7,CV_RGB(255,0,0),2);

	//��v2��������Ϊ���Ͻ�����
	v2[0] = img2->width;
	v2[1] = 0;
	V2 = cvMat(3, 1, CV_64FC1, v2);
	V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);
	rightTop.x = cvRound(v1[0] / v1[2]);
	rightTop.y = cvRound(v1[1] / v1[2]);
	//cvCircle(xformed,rightTop,7,CV_RGB(255,0,0),2);

	//��v2��������Ϊ���½�����
	v2[0] = img2->width;
	v2[1] = img2->height;
	V2 = cvMat(3, 1, CV_64FC1, v2);
	V1 = cvMat(3, 1, CV_64FC1, v1);
	cvGEMM(H, &V2, 1, 0, 1, &V1);
	rightBottom.x = cvRound(v1[0] / v1[2]);
	rightBottom.y = cvRound(v1[1] / v1[2]);
}
IplImage* CmysiftDlg::spliceImage(IplImage* img1, IplImage* img2){
	struct feature *feat1, *feat2;//feat1��ͼ1�����������飬feat2��ͼ2������������
	int n1, n2;//n1:ͼ1�е������������n2��ͼ2�е����������
	struct feature *feat;//ÿ��������
	struct kd_node *kd_root;//k-d��������
	struct feature **nbrs;//��ǰ�����������ڵ�����
	CvMat * H = NULL;//RANSAC�㷨����ı任����
	struct feature **inliers;//��RANSACɸѡ����ڵ�����
	int n_inliers;//��RANSAC�㷨ɸѡ����ڵ����,��feat2�о��з���Ҫ���������ĸ���

	IplImage *xformed = NULL, *xformed_proc = NULL;//xformed��ʱƴ��ͼ����ֻ��ͼ2�任���ͼ,xformed_proc�����պϳɵ�ͼ

	//ͼ2���ĸ��Ǿ�����H�任�������
	CvPoint leftTop, leftBottom, rightTop, rightBottom;
	///////////////////////////////////////////////////////////////////

	//��������
	n1 = sift_features(img1, &feat1);//���ͼ1�е�SIFT������,n1��ͼ1�����������
	//��ȡ����ʾ��2��ͼƬ�ϵ�������
	n2 = sift_features(img2, &feat2);//���ͼ2�е�SIFT�����㣬n2��ͼ2�����������

	//����������ͼ��
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

	//����ƥ��
	//����ͼ1�������㼯feat1����k-d��������k-d������kd_root
	kd_root = kdtree_build(feat1, n1);
	CvPoint pt1, pt2;//���ߵ������˵�
	double d0, d1;//feat2��ÿ�������㵽����ںʹν��ڵľ���
	int matchNum = 0;//�������ֵ��ɸѡ���ƥ���Եĸ���
	//���������㼯feat2�����feat2��ÿ��������feat��ѡȡ���Ͼ����ֵ������ƥ��㣬�ŵ�feat��fwd_match����
	for (int i = 0; i < n2; i++)
	{
		feat = feat2 + i;//��i���������ָ��
		//��kd_root������Ŀ���feat��2������ڵ㣬�����nbrs�У�����ʵ���ҵ��Ľ��ڵ����
		int k = kdtree_bbf_knn(kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS);
		if (k == 2)
		{
			d0 = descr_dist_sq(feat, nbrs[0]);//feat������ڵ�ľ����ƽ��
			d1 = descr_dist_sq(feat, nbrs[1]);//feat��ν��ڵ�ľ����ƽ��
			//��d0��d1�ı�ֵС����ֵNN_SQ_DIST_RATIO_THR������ܴ�ƥ�䣬�����޳�
			if (d0 < d1 * NN_SQ_DIST_RATIO_THR)
			{   //��Ŀ���feat������ڵ���Ϊƥ����

				pt2 = cvPoint(cvRound(feat->x), cvRound(feat->y));//ͼ2�е������
				pt1 = cvPoint(cvRound(nbrs[0]->x), cvRound(nbrs[0]->y));//ͼ1�е������(feat������ڵ�)
				pt2.x += img1->width;//��������ͼ���������еģ�pt2�ĺ��������ͼ1�Ŀ�ȣ���Ϊ���ߵ��յ�
				cvLine(stacked, pt1, pt2, CV_RGB(255, 0, 255), 1, 8, 0);//��������
				matchNum++;//ͳ��ƥ���Եĸ���
				feat2[i].fwd_match = nbrs[0];//ʹ��feat��fwd_match��ָ�����Ӧ��ƥ���(feat1�еĵ�)
			}
		}
		free(nbrs);//�ͷŽ�������
	}
	StackPic = stacked;
	/*cvShowImage("NO RANSAC", stacked);
	cvWaitKey(0);*/

	//����RANSAC�㷨ɸѡƥ���,����任����H��
	//����img1��img2������˳��H��Զ�ǽ�feat2�е�������任Ϊ��ƥ��㣬����img2�еĵ�任Ϊimg1�еĶ�Ӧ��
	H = ransac_xform(feat2, n2, FEATURE_FWD_MATCH, lsq_homog, 4, 0.01, homog_xfer_err, 3.0, &inliers, &n_inliers);

	//���ܳɹ�������任���󣬼�����ͼ���й�ͬ����
	IplImage* stacked_ransac;
	//stacked_ransac = stack_imgs(img1, img2);
	//������ͼ������˳������
	stacked_ransac = stack_imgs_horizontal(img1, img2);

	if (H)
	{
		int invertNum = 0;//ͳ��pt2.x > pt1.x��ƥ���Եĸ��������ж�img1���Ƿ���ͼ  

		//������RANSAC�㷨ɸѡ��������㼯��inliers���ҵ�ÿ���������ƥ���
		for (int i = 0; i < n_inliers; i++)
		{
			feat = inliers[i];//��i��������  
			pt2 = cvPoint(cvRound(feat->x), cvRound(feat->y));//ͼ2�е������  
			pt1 = cvPoint(cvRound(feat->fwd_match->x), cvRound(feat->fwd_match->y));//ͼ1�е������(feat��ƥ���)  

			//ͳ��ƥ��������λ�ù�ϵ�����ж�ͼ1��ͼ2������λ�ù�ϵ  
			if (pt2.x > pt1.x)
				invertNum++;

			// pt2.y += img1->height;//��������ͼ���������еģ�pt2�ĺ��������ͼ1�Ŀ�ȣ���Ϊ���ߵ��յ� 
			pt2.x += img1->width;//��������ͼ���������еģ�pt2�ĺ��������ͼ1�Ŀ�ȣ���Ϊ���ߵ��յ�  
			cvLine(stacked_ransac, pt1, pt2, CV_RGB(255, 0, 255), 1, 8, 0);//��ƥ��ͼ�ϻ�������  
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
				feat = feat2 + i;//��i���������ָ��
				//��kd_root������Ŀ���feat��2������ڵ㣬�����nbrs�У�����ʵ���ҵ��Ľ��ڵ����
				int k = kdtree_bbf_knn(kd_root, feat, 2, &nbrs, KDTREE_BBF_MAX_NN_CHKS);
				if (k == 2)
				{
					d0 = descr_dist_sq(feat, nbrs[0]);//feat������ڵ�ľ����ƽ��
					d1 = descr_dist_sq(feat, nbrs[1]);//feat��ν��ڵ�ľ����ƽ��
					//��d0��d1�ı�ֵС����ֵNN_SQ_DIST_RATIO_THR������ܴ�ƥ�䣬�����޳�
					if (d0 < d1 * NN_SQ_DIST_RATIO_THR)
					{   //��Ŀ���feat������ڵ���Ϊƥ����
						matchNum++;//ͳ��ƥ���Եĸ���
						feat2[i].fwd_match = nbrs[0];//ʹ��feat��fwd_match��ָ�����Ӧ��ƥ���(feat1�еĵ�)
					}
				}
				//free(nbrs);//�ͷŽ�������
			}

			H = ransac_xform(feat2, n2, FEATURE_FWD_MATCH, lsq_homog, 4, 0.01, homog_xfer_err, 3.0, &inliers, &n_inliers);
			stacked_ransac = stack_imgs_horizontal(img1, img2);
		}
	}

	if (H)
	{
		//ȫ��ƴ��
		//���ܳɹ�������任���󣬼�����ͼ���й�ͬ���򣬲ſ��Խ���ȫ��ƴ��
		//ƴ��ͼ��img1����ͼ��img2����ͼ
		CalcFourCorner(H, leftTop, leftBottom, rightTop, rightBottom, img2);//����ͼ2���ĸ��Ǿ��任�������
		//Ϊƴ�ӽ��ͼxformed����ռ�,�߶�Ϊͼ1ͼ2�߶ȵĽϴ��ߣ����ݱ任���ͼ2���ϽǺ����½ǵ��λ�þ���ƴ��ͼ�Ŀ��
		xformed = cvCreateImage(cvSize(MAX(rightTop.x, rightBottom.x), MIN(img1->height, img2->height)), IPL_DEPTH_8U, 3);

		//�ñ任����H����ͼimg2��ͶӰ�任(�任���������ƽ��)������ŵ�xformed��
		cvWarpPerspective(img2, xformed, H, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));

		//������ƴ��ͼ����¡��xformed
		xformed_proc = cvCloneImage(xformed);
		if (img1->width > xformed->width){
			xformed = cvCreateImage(cvSize(MAX(rightTop.x, rightBottom.x), MAX(img1->height, img2->height)), IPL_DEPTH_8U, 3);
			cvWarpPerspective(img2, xformed, H, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
			xformed_proc = cvCloneImage(img1);
			//�ص������ұߵĲ�����ȫȡ��ͼ1
			/*cout << "state2" << endl;*/
			cvSetImageROI(img1, cvRect(MAX(rightTop.x, rightBottom.x), 0, img1->width - MAX(rightTop.x, rightBottom.x), xformed_proc->height));

			cvSetImageROI(xformed_proc, cvRect(MAX(rightTop.x, rightBottom.x), 0, img1->width - MAX(rightTop.x, rightBottom.x), xformed_proc->height));
			cvAddWeighted(img1, 1, img1, 0, 0, xformed_proc);
			cvResetImageROI(img1);
			cvResetImageROI(xformed_proc);
		}


		//�ص�������ߵĲ�����ȫȡ��ͼ1
		cvSetImageROI(img1, cvRect(0, 0, MIN(leftTop.x, leftBottom.x), xformed_proc->height));

		cvSetImageROI(xformed, cvRect(0, 0, MIN(leftTop.x, leftBottom.x), xformed_proc->height));

		cvSetImageROI(xformed_proc, cvRect(0, 0, MIN(leftTop.x, leftBottom.x), xformed_proc->height));

		cvAddWeighted(img1, 1, xformed, 0, 0, xformed_proc);	

		cvResetImageROI(img1);
		cvResetImageROI(xformed);
		cvResetImageROI(xformed_proc);

		

		////////////////////////////////////////////////////////////
		//ͼ���ں�
		//���ü�Ȩƽ���ķ����ں��ص�����
		int start = MIN(leftTop.x, leftBottom.x);//��ʼλ�ã����ص��������߽�
		double processWidth = img1->width - start;//�ص�����Ŀ��
		double alpha = 1;//img1�����ص�Ȩ��
		for (int i = 0; i < xformed_proc->height; i++)//������
		{
			const uchar * pixel_img1 = ((uchar *)(img1->imageData + img1->widthStep * i));//img1�е�i�����ݵ�ָ��
			const uchar * pixel_xformed = ((uchar *)(xformed->imageData + xformed->widthStep * i));//xformed�е�i�����ݵ�ָ��
			uchar * pixel_xformed_proc = ((uchar *)(xformed_proc->imageData + xformed_proc->widthStep * i));//xformed_proc�е�i�����ݵ�ָ��
			for (int j = start; j < img1->width; j++)//�����ص��������
			{
				//�������ͼ��xformed�������صĺڵ㣬����ȫ����ͼ1�е�����
				if (pixel_xformed[j * 3] < 50 && pixel_xformed[j * 3 + 1] < 50 && pixel_xformed[j * 3 + 2] < 50)
				{
					alpha = 1;
				}
				else
				{   //img1�����ص�Ȩ�أ��뵱ǰ�������ص�������߽�ľ��������
					alpha = (processWidth - (j - start)) / processWidth;
				}
				pixel_xformed_proc[j * 3] = pixel_img1[j * 3] * alpha + pixel_xformed[j * 3] * (1 - alpha);//Bͨ��
				pixel_xformed_proc[j * 3 + 1] = pixel_img1[j * 3 + 1] * alpha + pixel_xformed[j * 3 + 1] * (1 - alpha);//Gͨ��
				pixel_xformed_proc[j * 3 + 2] = pixel_img1[j * 3 + 2] * alpha + pixel_xformed[j * 3 + 2] * (1 - alpha);//Rͨ��
			}
		}

	}
	else //�޷�������任���󣬼�����ͼ��û���غ�����
	{
		return NULL;
	}

	///////////////////////////////////////////////////////////////////////////
	kdtree_release(kd_root);//�ͷ�kd��
	//ֻ����RANSAC�㷨�ɹ�����任����ʱ������Ҫ��һ���ͷ�������ڴ�ռ�
	if (H)
	{
		cvReleaseMat(&H);//�ͷű任����H
		free(inliers);//�ͷ��ڵ�����
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
	// ���ļ� �������ָ�ʽ���ļ�bmp gif jpg tiff png
	//ImageList
	CString filter;
	filter = "�����ļ�(*.bmp,*.jpg,*.gif,*tiff,*png)|*.bmp;*.jpg;*.gif;*.tiff;*.png| BMP(*.bmp)|*.bmp| PNG(*.png)|*.png| JPG(*.jpg)|*.jpg| GIF(*.gif)|*.gif| TIFF(*.tiff)|*.tiff||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter, NULL);
	CString tmp;
	//����ȷ����ť dlg.DoModal() ������ʾ�Ի���
	if (ImageNumber == 0){
		AfxMessageBox(_T("�������Ŀ��Ч!"));
	}
	if (count < ImageNumber)
	{
		if (dlg.DoModal() == IDOK){
			count++;
			tmp = dlg.GetPathName();
			PicPath = PicPath + tmp;     //��ȡ�ļ�·����   ��D:\pic\abc.bmp
		}
		edit_path2.SetWindowTextW(PicPath);
		string path = (LPCSTR)CStringA(tmp);
		if (path == ""){
			AfxMessageBox(_T("·������!"));
			edit_path2.SetWindowTextW(_T(""));
		}
		else{
			ImageArray[count - 1] = (cvLoadImage(path.c_str()));
		}	
	}
	else{
		AfxMessageBox(_T("ͼƬ�Ѿ��ﵽָ����Ŀ!"));
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
	cvShowImage("ransac" , StackRansacPic);//��ʾ��RANSAC�㷨ɸѡ���ƥ��ͼ 
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
		AfxMessageBox(_T("�������Ŀ��Ч!"));
	}
	else{
		AfxMessageBox(_T("�������ĿΪ��") + tmp);
	}
	ImageArray = new IplImage*[ImageNumber];
}
