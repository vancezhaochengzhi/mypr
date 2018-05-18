
// myPRDlg.h : header file
//

#pragma once

#include "src/include/prep.h"
#include "src/include/plate_detect.h"
#include "src/include/chars_segment.h"
#include "src/include/util.h"
#include "src/include/features.h"
#include "src/include/CParser.h"

using namespace easypr; 

// CmyPRDlg dialog
class CmyPRDlg : public CDialogEx
{
// Construction
public:
	CmyPRDlg(CWnd* pParent = NULL);	// standard constructor
	
	void show_pic(IplImage *t,int nID);


	int platerecogn(Mat img,CString &strresult);//车牌识别，直接得到识别的字符串


	bool m_bload;//是否加载svm

	Mat m_carimg;//整车大图
	Mat m_plateimg;//车牌图
	Mat m_charimg[7];//字符图

	CvANN_MLP m_chineseann;//汉字ann模型
	CvANN_MLP m_englishann;//英文ann模型
	CvANN_MLP m_charann;//英文ann模型

	Mat m_chineselowvec;//每个维度的最小值
	Mat m_chineseupvec;//每个维度的最大值 ，用于归一化数据

	Mat m_englishlowvec;//每个维度的最小值
	Mat m_englishupvec;//每个维度的最大值 ，用于归一化数据

	Mat m_charlowvec;//每个维度的最小值
	Mat m_charupvec;//每个维度的最大值 ，用于归一化数据


	CString imagerecogn(Mat image,int index);//单个字符识别
// Dialog Data
	enum { IDD = IDD_MYPR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonLoadimg();
	afx_msg void OnBnClickedButtonplatelocal();
	afx_msg void OnBnClickedButtonsegment();
	afx_msg void OnBnClickedButtonRecogn();  
};
