
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


	int platerecogn(Mat img,CString &strresult);//����ʶ��ֱ�ӵõ�ʶ����ַ���


	bool m_bload;//�Ƿ����svm

	Mat m_carimg;//������ͼ
	Mat m_plateimg;//����ͼ
	Mat m_charimg[7];//�ַ�ͼ

	CvANN_MLP m_chineseann;//����annģ��
	CvANN_MLP m_englishann;//Ӣ��annģ��
	CvANN_MLP m_charann;//Ӣ��annģ��

	Mat m_chineselowvec;//ÿ��ά�ȵ���Сֵ
	Mat m_chineseupvec;//ÿ��ά�ȵ����ֵ �����ڹ�һ������

	Mat m_englishlowvec;//ÿ��ά�ȵ���Сֵ
	Mat m_englishupvec;//ÿ��ά�ȵ����ֵ �����ڹ�һ������

	Mat m_charlowvec;//ÿ��ά�ȵ���Сֵ
	Mat m_charupvec;//ÿ��ά�ȵ����ֵ �����ڹ�һ������


	CString imagerecogn(Mat image,int index);//�����ַ�ʶ��
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
