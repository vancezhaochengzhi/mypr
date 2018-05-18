
// myPRDlg.cpp : implementation file
//

#include "stdafx.h"
#include "myPR.h"
#include "myPRDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEST_WIDTH 10
#define DEST_HEIGHT 20

//�й�����
const char strCharacters[] = {'0','1','2','3','4','5',\
	'6','7','8','9','A','B', 'C', 'D', 'E','F', 'G', 'H', /* û��I */\
	'J', 'K', 'L', 'M', 'N', /* û��O */ 'P', 'Q', 'R', 'S', 'T', \
	'U','V', 'W', 'X', 'Y', 'Z'}; 
const int numCharacter = 34; /* û��I��O,10��������24��Ӣ���ַ�֮�� */

////����
const char* strChinese[] = {"��","��","��","��","��","��","��","��","��","��","³","��","��","��","��","��","ԥ","��","��" };

const int numChinese = 19;

//58�ֶ�����ͻ�����С�ڵ���2����
const int LBP_NUM[]={0,1,2,3,4,6,7,8,12,14,15,16,24,28,30,31,32,48,56,60,62,63,64,96,112,120,124,126,127,128,129,131,135,
143,159,191,192,193,195,199,207,223,224,225,227,231,239,240,241,243,247,248,249,251,252,253,254,255};
//���ⲻ������58������һ�࣬�ܹ�59�࣬Ҳ����˵lbpֱ��ͼ������59ά��
uchar maptable[256] = {0};

//�ж��ļ��Ƿ����
BOOL IsFileExist(const CString& csFile)
{
    DWORD dwAttrib = GetFileAttributes(csFile);
    return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void getPicFiles(CString path, vector<CString>& files)
{
	//�ļ����
	long   hFile   =   0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	string p;
	if((hFile = _findfirst(path+"\\*",&fileinfo)) !=  -1)
	{
		do
		{
			//�����Ŀ¼,����֮
			//�������,�����б�
			if((fileinfo.attrib &  _A_SUBDIR))
			{
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
					getPicFiles(path+"\\"+fileinfo.name, files );
			}
			else
			{

				char *pp;
				pp = strrchr(fileinfo.name,'.');//���������ֵ�λ��
				if (stricmp(pp,".bmp")==0 || stricmp(pp,".jpg")==0 || stricmp(pp,".png")==0 )//����ҵ�����ͼƬ���д���
				{
					files.push_back(path+"\\"+fileinfo.name);
				}
			}
		}while(_findnext(hFile, &fileinfo)  == 0);
		_findclose(hFile);
	}
}


//IplImage ת BMP
LPBITMAPINFO  CreateMapInfo(IplImage* workImg)    //  ����λͼ��Ϣ
{                                           
	BITMAPINFOHEADER BIH={40,1,1,1,8,0,0,0,0,0,0};
	LPBITMAPINFO lpBmi;
	int          wid, hei, bits, colors,i;
	RGBQUAD  ColorTab[256];
	wid =workImg->width;     hei =workImg->height;
	bits=workImg->depth*workImg->nChannels;
	if (bits>8) colors=0;
	else colors=1<<bits;
	lpBmi=(LPBITMAPINFO) malloc(40+4*colors);
	BIH.biWidth   =wid;     
	BIH.biHeight  =hei;
	BIH.biBitCount=(BYTE) bits;
	memcpy(lpBmi,&BIH,40);                   //  ����λͼ��Ϣͷ
	if (bits==8) {                           //  256 ɫλͼ
		for (i=0;i<256;i++)  {                //  ���ûҽ׵�ɫ��
			ColorTab[i].rgbRed=ColorTab[i].rgbGreen=ColorTab[i].rgbBlue=(BYTE) i;
		}
		memcpy(lpBmi->bmiColors, ColorTab, 1024);
	}
	return(lpBmi);
}


//���������ͼ��Ԥ��������������û�кڱߣ���һ��ͳһ��С
Mat preprocimg(Mat in)
{
	Mat grayI;
	if(in.channels() == 3)
	{
		cvtColor(in,grayI,CV_BGR2GRAY );//תΪ�Ҷ�ͼ��
	}
	else
	{
		grayI = in;
	}

	Mat bwI;
	threshold(grayI,bwI,128,255,CV_THRESH_BINARY);
	
	int rows = bwI.rows;//����
	int cols = bwI.cols;//����
	int left,right,top,bottom;
	//��߽�
	for (int i=0;i<cols;i++)//����ÿһ��
	{	
		Mat data=bwI.col(i);//ȡ��һ��
		int whitenum = countNonZero(data);	//ͳ����һ�л�һ���У�����Ԫ�صĸ���
		if(whitenum > 0)//�ҵ��׵�����
		{
			left = i;//��߽�
			break;
		}

	}

	//�ұ߽�
	for (int i=cols-1; i>=0; i--)//����ÿһ��
	{	
		Mat data=bwI.col(i);//ȡ��һ��
		int whitenum = countNonZero(data);	//ͳ����һ�л�һ���У�����Ԫ�صĸ���
		if(whitenum > 0)//�ҵ��׵�����
		{
			right = i;//�ұ߽�
			break;
		}
	}

	//�ϱ߽�
	for (int i=0;i<rows;i++)//����ÿһ��
	{	
		Mat data=bwI.row(i);//ȡ��һ��
		int whitenum = countNonZero(data);	//ͳ����һ�л�һ���У�����Ԫ�صĸ���
		if(whitenum > 0)//�ҵ��׵�����
		{
			top = i;//��߽�
			break;
		}

	}

	//�±߽�
	for (int i=rows-1; i>=0; i--)//����ÿһ��
	{	
		Mat data=bwI.row(i);//ȡ��һ��
		int whitenum = countNonZero(data);	//ͳ����һ�л�һ���У�����Ԫ�صĸ���
		if(whitenum > 0)//�ҵ��׵�����
		{
			bottom = i;//�±߽�
			break;
		}
	}
	//��֯Ҫ��ȡ������
	Rect r;
	r.x = left;
	r.y = top;
	r.height = bottom-top+1;
	r.width = right-left+1;//

	Mat image_roi = bwI(r);//��ȡ����
	Mat result;
	resize(image_roi, result, Size(DEST_WIDTH,DEST_HEIGHT) );
	threshold(result,result,80,1,CV_THRESH_BINARY);//��ֵ����01����
	return result;
}

Mat features(Mat in)
{

	Mat procI = preprocimg(in);//��ȡ�������,��������

	//Low data feature
	Mat lowData = procI;
	int numCols=lowData.rows*lowData.cols;//��������

	Mat out=Mat::zeros(1,numCols,CV_32F);
	
	int j=0;
	//����ÿ��ÿ�е�����ֵ�����赽out��
	for(int x=0; x<lowData.cols; x++)
	{
		for(int y=0; y<lowData.rows; y++){
			out.at<float>(j)=(float)lowData.at<unsigned char>(y,x);
			j++;
		}
	}

	return out;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CmyPRDlg dialog




CmyPRDlg::CmyPRDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmyPRDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmyPRDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CmyPRDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CmyPRDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_LOADIMG, &CmyPRDlg::OnBnClickedButtonLoadimg)
	ON_BN_CLICKED(IDC_BUTTONPLATELOCAL, &CmyPRDlg::OnBnClickedButtonplatelocal)
	ON_BN_CLICKED(IDC_BUTTONSEGMENT, &CmyPRDlg::OnBnClickedButtonsegment)
	ON_BN_CLICKED(IDC_BUTTON_RECOGN, &CmyPRDlg::OnBnClickedButtonRecogn)
END_MESSAGE_MAP()


// CmyPRDlg message handlers

BOOL CmyPRDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_bload = false;

	if( IsFileExist("model\\char.xml") && IsFileExist("model\\chinese.xml")  && IsFileExist("model\\englishchar.xml"))
	{
		
		m_charann.load("model\\char.xml");

		
		m_chineseann.load("model\\chinese.xml");

		
		m_englishann.load("model\\englishchar.xml");

		m_bload = true;
	}

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CmyPRDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CmyPRDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		if (m_carimg.data)
		{
			IplImage imgTmp = m_carimg;
			show_pic(&imgTmp, IDC_STATIC_CAR);
		}

		if (m_plateimg.data)
		{
			IplImage imgTmp2 = m_plateimg;
			show_pic(&imgTmp2, IDC_STATIC_PLATE);
		}


		for (int j = 0; j < 7; j++)
		{
			if (m_charimg[j].data)
			{
				IplImage imgTmp = m_charimg[j];
				show_pic(&imgTmp, IDC_STATIC_CHAR1 + j);//������ʾ7���ָ���ַ�����Ϊ��Щ�ؼ���ID�ǵ����ģ����Կ���дѭ��
			}
		}
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CmyPRDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



//�ڿؼ�����ʾͼƬ
void CmyPRDlg::show_pic(IplImage *t,int nID)
{
	LPBITMAPINFO lpBmi=CreateMapInfo(t);//����λͼ��Ϣ��������ʾ

	//�趨��ͼ�������CDC
	CDC *pDC = GetDlgItem(nID)->GetDC();
	//	pDC = GetDC();
	CRect rect;
	GetDlgItem(nID)->GetWindowRect(&rect);
	ScreenToClient(rect);

	IplImage* imgcopy = cvCreateImage( cvSize(t->width,t->height), 8, t->nChannels );//�Ҷ�ͼ��
	cvCopy(t,imgcopy);
	cvFlip(imgcopy);


	if( imgcopy->width > rect.Width() )
	{
		SetStretchBltMode(
			pDC->m_hDC,           // handle to device context
			HALFTONE );
	}
	else
	{
		SetStretchBltMode(
			pDC->m_hDC,           // handle to device context
			COLORONCOLOR );
	}
	StretchDIBits(pDC->m_hDC,
		0,0,rect.Width(),rect.Height(),
		0,0,t->width,t->height,
		imgcopy->imageDataOrigin,lpBmi,DIB_RGB_COLORS,SRCCOPY);

	//����ڴ�
	cvReleaseImage(&imgcopy);
	free(lpBmi);
	ReleaseDC(pDC);
}


void CmyPRDlg::OnBnClickedButtonTest()
{
	// TODO: Add your control notification handler code here
	if(!m_bload)//�����û����ģ���
	{
		MessageBox("���ȼ����ַ�ģ��!");
		return;
	}
	//�������Щ����
	m_carimg = Mat();
	m_plateimg = Mat();
	m_charimg[0] = Mat();

	OnBnClickedButtonLoadimg();//����ͼ��
	if(m_carimg.data == NULL)//�ж�����ɹ�����
	{
		return;
	}

	OnBnClickedButtonplatelocal();//��λ����
	if(m_plateimg.data == NULL)//�ж϶�λ�ɹ�����
	{
		return;
	}

	OnBnClickedButtonsegment();
	if(m_charimg[0].data == NULL)//�жϷָ�ɹ�����
	{
		return;
	}

	OnBnClickedButtonRecogn();
}

void CmyPRDlg::OnBnClickedButtonLoadimg()
{
	// TODO: Add your control notification handler code here
	char szFilter[] = "Image Files (*.bmp;*.jpg;*.tiff;*.png)|*.bmp;*.jpg;*.tiff;*.png|All Files (*.*)|*.*||";
	CFileDialog dlg(true, "bmp|jpg|jpeg|gif", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	dlg.m_ofn.lpstrTitle = "��ѡ��һ��ͼƬ";
	
	CString filename;
	
	if(dlg.DoModal() == IDOK)	
	{
		m_carimg = Mat();//���Ƶ�m_drawcarimg��

		m_plateimg = Mat();//���

		for (int i = 0; i<7; i++)
		{
			m_charimg[i] = Mat();//���
		}
		GetDlgItem(IDC_EDIT_RESULT)->SetWindowText("");//ʶ������ʾ

		filename = dlg.GetPathName();
		m_carimg = imread(filename.GetBuffer(0));
		if(m_carimg.data == NULL)
		{
			MessageBox("����ͼ��ʧ��!");
			return;
		}
		/*IplImage imgTmp = m_carimg;
		show_pic(&imgTmp,IDC_STATIC_CAR);*/
		SendMessage(WM_PAINT, NULL, NULL);
		UpdateWindow();
	}
}

//���ƶ�λ
void CmyPRDlg::OnBnClickedButtonplatelocal()
{
	// TODO: Add your control notification handler code here
	if(m_carimg.data == NULL)
	{
		MessageBox("��������ͼ��!");
		return;
	}
	CPlateLocate plate;
	plate.setDebug(1);
	plate.setLifemode(true);

    //plate.setGaussianBlurSize(5);
    //plate.setMorphSizeWidth(7);
    //plate.setMorphSizeHeight(3);
    //plate.setVerifyError(0.9);
    //plate.setVerifyAspect(4);
    //plate.setVerifyMin(1);
    //plate.setVerifyMax(30);

	vector<Mat> resultVec;//��ų���ͼ���б�
	vector<RotatedRect> resultRectVec;//���λ�þ����б�
	int result = plate.plateLocate(m_carimg, resultVec,resultRectVec);
	if(result != 0  || resultVec.size() == 0)
	{
		MessageBox("û�ж�λ�����ƣ�");  
		return;
	}

	//�Լ����ͼ�񣬽���һ��ɸѡ
	vector<Mat> resultVec2;//��ų���ͼ���б�
	vector<RotatedRect> resultRectVec2;//���λ�þ����б�
	CPlateJudge ju;
	int resultJu = ju.plateJudge(resultVec, resultRectVec,resultVec2,resultRectVec2);//svmɸѡ����
	if (0 != resultJu || resultVec2.size() == 0)
	{
		MessageBox("û�ж�λ�����ƣ�");
		return;
	}


	//�������ж�����ƣ�����ѡ�������Ǹ�ȥ�ָ�ʶ��
	int maxsize = 0;
	RotatedRect rr;
	for(int i=0;i<resultRectVec2.size();i++)
	{
		int size = resultRectVec2[i].size.height * resultRectVec2[i].size.width;//������Ƶ����
		if(size > maxsize)//���������������
		{
			maxsize = size;
			rr = resultRectVec2[i];
			m_plateimg = resultVec2[i];
		}
	}
	
	//��ԭͼ���Ƴ��������
	Point2f rect_points[4];
	rr.points(rect_points);//�Ӿ�������õ�4������
	for (int j = 0; j < 4; j++)
	{
		//���λ����ĸ��߶�
		line(m_carimg, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 255), 1, 8);
	}
	/*IplImage imgTmp1 = m_carimg;
	show_pic(&imgTmp1,IDC_STATIC_CAR);


	IplImage imgTmp2 = m_plateimg;
	show_pic(&imgTmp2,IDC_STATIC_PLATE);*/
	SendMessage(WM_PAINT, NULL, NULL);
	UpdateWindow();

}

//���Ʒָ�
void CmyPRDlg::OnBnClickedButtonsegment()
{
	// TODO: Add your control notification handler code here
	if(m_plateimg.data == NULL)
	{
		MessageBox("���ȶ�λ����!");
		return;
	}

	CCharsSegment cs;
	cs.setDebug(1);
	vector<Mat> resultVec;
	int result = cs.charsSegment(m_plateimg, resultVec);
	if (result != 0)
	{
		return;
	}

	int num = resultVec.size();
	for (int j = 0; j < num; j++)
	{
		if(j>=7)
		{
			break;
		}
		m_charimg[j] = resultVec[j];//�浽m_charimg������
	}


	//for(int j = 0; j < 7; j++)
	//{
	//	IplImage imgTmp = m_charimg[j];
	//	show_pic(&imgTmp,IDC_STATIC_CHAR1 + j);//������ʾ7���ָ���ַ�����Ϊ��Щ�ؼ���ID�ǵ����ģ����Կ���дѭ��
	//}

	SendMessage(WM_PAINT, NULL, NULL);
	UpdateWindow();
}

CString CmyPRDlg::imagerecogn(Mat image,int index)
{

	Mat procimg = preprocimg(image);//Ԥ����ͼƬ
	Mat f=features(image);//�������ͼ�������

	

	CString str = "";

	if(index == 0)//��һλ����
	{
		Mat output(1, numChinese, CV_32FC1);//�������������
		m_chineseann.predict(f, output);

		int rsultindex = 0;//ʶ�����������
			
		float maxVal = -2;
		for(int j = 0; j < numChinese; j++)
		{
			float val = output.at<float>(j);
			//cout << "j:" << j << "val:"<< val << endl;
			if (val > maxVal)
			{
				maxVal = val;
				rsultindex = j;
			}
		}
		
		str = str + strChinese[rsultindex];
	}
	else if(index == 1)//�ڶ�λ��ĸ
	{
		Mat output(1, 24, CV_32FC1);//�������������
		m_englishann.predict(f, output);

		int rsultindex = 0;//ʶ�����������
			
		float maxVal = -2;
		for(int j = 0; j < 24; j++)
		{
			float val = output.at<float>(j);
			//cout << "j:" << j << "val:"<< val << endl;
			if (val > maxVal)
			{
				maxVal = val;
				rsultindex = j;
			}
		}

		str = str + strCharacters[rsultindex+10];
	}
	else//3-7������ĸ���
	{
		
		Mat output(1, numCharacter, CV_32FC1);//�������������
		m_charann.predict(f, output);

		int rsultindex = 0;//ʶ�����������
			
		float maxVal = -2;
		for(int j = 0; j < numCharacter; j++)
		{
			float val = output.at<float>(j);
			//cout << "j:" << j << "val:"<< val << endl;
			if (val > maxVal)
			{
				maxVal = val;
				rsultindex = j;
			}
		}

		str = str + strCharacters[rsultindex];
	}
	

	return str;
}

void CmyPRDlg::OnBnClickedButtonRecogn()
{
	// TODO: Add your control notification handler code here

	if(!m_bload)//�����û����xml
	{
		if( IsFileExist("model\\char.xml") && IsFileExist("model\\chinese.xml")  && IsFileExist("model\\englishchar.xml"))
		{
				m_charann.load("model\\char.xml");

		
				m_chineseann.load("model\\chinese.xml");

		
				m_englishann.load("model\\englishchar.xml");

			m_bload = true;
		}
	}
	if(!m_bload)//�����Ȼû�ҵ�xml
	{
		MessageBox("δ�����ַ�ѵ���������");
		return;
	}

	if( m_charimg[0].data == NULL)
	{
		MessageBox("���ȷָ���ַ�!");
		return;
	}


	CString strresult = "";//ʶ����
	for (int j = 0; j < 7; j++)//����ʶ��7���ַ�
	{

		if(m_charimg[j].data)
		{
			Mat resultMat = m_charimg[j];//ȡ��һ���ַ�

			strresult = strresult + imagerecogn(resultMat,j);//ʶ�����������ۼӵ��ַ���

			
		}
	}

	GetDlgItem(IDC_EDIT_RESULT)->SetWindowText(strresult);//ʶ������ʾ
	SendMessage(WM_PAINT, NULL, NULL);
	UpdateWindow();
}

//����ʶ��ֱ�ӵõ�ʶ����ַ���
int CmyPRDlg::platerecogn(Mat img,CString &strresult)
{
	CPlateLocate plate;
	plate.setDebug(0);
	plate.setLifemode(true);


	vector<Mat> resultVec;//��ų���ͼ���б�
	vector<RotatedRect> resultRectVec;//���λ�þ����б�
	int result = plate.plateLocate(img, resultVec,resultRectVec);
	if(result != 0  || resultVec.size() == 0)
	{
		return -1;
	}

	//�Լ����ͼ�񣬽���һ��ɸѡ
	vector<Mat> resultVec2;//��ų���ͼ���б�
	vector<RotatedRect> resultRectVec2;//���λ�þ����б�
	CPlateJudge ju;
	int resultJu = ju.plateJudge(resultVec, resultRectVec,resultVec2,resultRectVec2);//svmɸѡ����
	if (0 != resultJu || resultVec2.size() == 0)
	{
		return -1;
	}


	//�������ж�����ƣ�����ѡ�������Ǹ�ȥ�ָ�ʶ��
	int maxsize = 0;
	RotatedRect rr;
	Mat plateimg;//����ͼ��
	for(int i=0;i<resultRectVec2.size();i++)
	{
		int size = resultRectVec2[i].size.height * resultRectVec2[i].size.width;//������Ƶ����
		if(size > maxsize)//���������������
		{
			maxsize = size;
			rr = resultRectVec2[i];
			plateimg = resultVec2[i];
		}
	}

	//��λ������ʼ�ָ�
	CCharsSegment cs;
	cs.setDebug(0);
	vector<Mat> charList;
	result = cs.charsSegment(plateimg, charList);
	if (result != 0)
	{
		return -1;
	}

	//��ʼʶ��
	int num = charList.size();
	num = MIN(num,7);//���ʶ��7λ

	for (int j = 0; j < num; j++)//����ʶ��7���ַ�
	{

		Mat resultMat = charList[j];
		strresult = strresult + imagerecogn(resultMat,j);//ʶ�����������ۼӵ��ַ���
		
	}

	return 0;
}
