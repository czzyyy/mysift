
// mysiftDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include <opencv\cv.h>
#include <opencv\cxcore.h>
#include <opencv\highgui.h>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <list>
#include "afxcmn.h"
using namespace std;
// CmysiftDlg �Ի���
class CmysiftDlg : public CDialogEx
{
// ����
public:
	CmysiftDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MYSIFT_DIALOG };
	CString PicPath;
	int ImageNumber;
	int count;
	int times;
	IplImage* HorizontalPic;
	IplImage* ShowpointsPic;
	IplImage* StackPic;
	IplImage* StackRansacPic;
	IplImage* ShowFinalPic;
	IplImage** ImageArray;


	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void ShiftProgress();
	void CalcFourCorner(CvMat* &H, CvPoint& leftTop, CvPoint& leftBottom, CvPoint& rightTop, CvPoint& rightBottom, IplImage* img2);
	IplImage* spliceImage(IplImage* img1, IplImage* img2);
	afx_msg void OnBnClickedButtonPic2();
	CEdit edit_path2;
	afx_msg void OnBnClickedButtonHorizontal();
	afx_msg void OnBnClickedButtonShowpoints();
	afx_msg void OnBnClickedButtonStack();
	afx_msg void OnBnClickedButtonStackRansac();
	afx_msg void OnBnClickedButtonShowFinal();
	afx_msg void OnBnClickedButtonStart();
	CProgressCtrl m_pro;
	CEdit m_edit_number;
	afx_msg void OnBnClickedButtonNumberOk();
};
