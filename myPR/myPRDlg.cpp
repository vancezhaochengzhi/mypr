
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

//中国车牌
const char strCharacters[] = {'0','1','2','3','4','5',\
	'6','7','8','9','A','B', 'C', 'D', 'E','F', 'G', 'H', /* 没有I */\
	'J', 'K', 'L', 'M', 'N', /* 没有O */ 'P', 'Q', 'R', 'S', 'T', \
	'U','V', 'W', 'X', 'Y', 'Z'}; 
const int numCharacter = 34; /* 没有I和O,10个数字与24个英文字符之和 */

////汉字
const char* strChinese[] = {"鄂","赣","黑","沪","吉","冀","津","晋","京","辽","鲁","闽","宁","陕","苏","皖","豫","粤","浙" };

const int numChinese = 19;

//58种二进制突变次数小于等于2的数
const int LBP_NUM[]={0,1,2,3,4,6,7,8,12,14,15,16,24,28,30,31,32,48,56,60,62,63,64,96,112,120,124,126,127,128,129,131,135,
143,159,191,192,193,195,199,207,223,224,225,227,231,239,240,241,243,247,248,249,251,252,253,254,255};
//另外不属于这58个算作一类，总共59类，也就是说lbp直方图特征是59维度
uchar maptable[256] = {0};

//判断文件是否存在
BOOL IsFileExist(const CString& csFile)
{
    DWORD dwAttrib = GetFileAttributes(csFile);
    return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void getPicFiles(CString path, vector<CString>& files)
{
	//文件句柄
	long   hFile   =   0;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	if((hFile = _findfirst(path+"\\*",&fileinfo)) !=  -1)
	{
		do
		{
			//如果是目录,迭代之
			//如果不是,加入列表
			if((fileinfo.attrib &  _A_SUBDIR))
			{
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
					getPicFiles(path+"\\"+fileinfo.name, files );
			}
			else
			{

				char *pp;
				pp = strrchr(fileinfo.name,'.');//查找最后出现的位置
				if (stricmp(pp,".bmp")==0 || stricmp(pp,".jpg")==0 || stricmp(pp,".png")==0 )//如果找到的是图片就行处理
				{
					files.push_back(path+"\\"+fileinfo.name);
				}
			}
		}while(_findnext(hFile, &fileinfo)  == 0);
		_findclose(hFile);
	}
}


//IplImage 转 BMP
LPBITMAPINFO  CreateMapInfo(IplImage* workImg)    //  建立位图信息
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
	memcpy(lpBmi,&BIH,40);                   //  复制位图信息头
	if (bits==8) {                           //  256 色位图
		for (i=0;i<256;i++)  {                //  设置灰阶调色板
			ColorTab[i].rgbRed=ColorTab[i].rgbGreen=ColorTab[i].rgbBlue=(BYTE) i;
		}
		memcpy(lpBmi->bmiColors, ColorTab, 1024);
	}
	return(lpBmi);
}


//这个函数将图像预处理到紧紧包含。没有黑边，归一到统一大小
Mat preprocimg(Mat in)
{
	Mat grayI;
	if(in.channels() == 3)
	{
		cvtColor(in,grayI,CV_BGR2GRAY );//转为灰度图像
	}
	else
	{
		grayI = in;
	}

	Mat bwI;
	threshold(grayI,bwI,128,255,CV_THRESH_BINARY);
	
	int rows = bwI.rows;//行数
	int cols = bwI.cols;//列数
	int left,right,top,bottom;
	//左边界
	for (int i=0;i<cols;i++)//遍历每一列
	{	
		Mat data=bwI.col(i);//取得一列
		int whitenum = countNonZero(data);	//统计这一行或一列中，非零元素的个数
		if(whitenum > 0)//找到白点列了
		{
			left = i;//左边界
			break;
		}

	}

	//右边界
	for (int i=cols-1; i>=0; i--)//遍历每一列
	{	
		Mat data=bwI.col(i);//取得一列
		int whitenum = countNonZero(data);	//统计这一行或一列中，非零元素的个数
		if(whitenum > 0)//找到白点列了
		{
			right = i;//右边界
			break;
		}
	}

	//上边界
	for (int i=0;i<rows;i++)//遍历每一行
	{	
		Mat data=bwI.row(i);//取得一行
		int whitenum = countNonZero(data);	//统计这一行或一列中，非零元素的个数
		if(whitenum > 0)//找到白点行了
		{
			top = i;//左边界
			break;
		}

	}

	//下边界
	for (int i=rows-1; i>=0; i--)//遍历每一行
	{	
		Mat data=bwI.row(i);//取得一行
		int whitenum = countNonZero(data);	//统计这一行或一列中，非零元素的个数
		if(whitenum > 0)//找到白点行了
		{
			bottom = i;//下边界
			break;
		}
	}
	//组织要截取的区域
	Rect r;
	r.x = left;
	r.y = top;
	r.height = bottom-top+1;
	r.width = right-left+1;//

	Mat image_roi = bwI(r);//截取区域
	Mat result;
	resize(image_roi, result, Size(DEST_WIDTH,DEST_HEIGHT) );
	threshold(result,result,80,1,CV_THRESH_BINARY);//二值化到01区间
	return result;
}

Mat features(Mat in)
{

	Mat procI = preprocimg(in);//截取区域出来,紧紧包含

	//Low data feature
	Mat lowData = procI;
	int numCols=lowData.rows*lowData.cols;//像素总数

	Mat out=Mat::zeros(1,numCols,CV_32F);
	
	int j=0;
	//遍历每行每列的像素值，赋予到out内
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
				show_pic(&imgTmp, IDC_STATIC_CHAR1 + j);//依次显示7个分割的字符，因为这些控件的ID是递增的，所以可以写循环
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



//在控件中显示图片
void CmyPRDlg::show_pic(IplImage *t,int nID)
{
	LPBITMAPINFO lpBmi=CreateMapInfo(t);//创建位图信息，用于显示

	//设定绘图的区域和CDC
	CDC *pDC = GetDlgItem(nID)->GetDC();
	//	pDC = GetDC();
	CRect rect;
	GetDlgItem(nID)->GetWindowRect(&rect);
	ScreenToClient(rect);

	IplImage* imgcopy = cvCreateImage( cvSize(t->width,t->height), 8, t->nChannels );//灰度图像
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

	//清空内存
	cvReleaseImage(&imgcopy);
	free(lpBmi);
	ReleaseDC(pDC);
}


void CmyPRDlg::OnBnClickedButtonTest()
{
	// TODO: Add your control notification handler code here
	if(!m_bload)//如果还没加载模板库
	{
		MessageBox("请先加载字符模板!");
		return;
	}
	//先清空这些数组
	m_carimg = Mat();
	m_plateimg = Mat();
	m_charimg[0] = Mat();

	OnBnClickedButtonLoadimg();//载入图像
	if(m_carimg.data == NULL)//判断载入成功了吗
	{
		return;
	}

	OnBnClickedButtonplatelocal();//定位车牌
	if(m_plateimg.data == NULL)//判断定位成功了吗
	{
		return;
	}

	OnBnClickedButtonsegment();
	if(m_charimg[0].data == NULL)//判断分割成功了吗
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
	dlg.m_ofn.lpstrTitle = "请选择一张图片";
	
	CString filename;
	
	if(dlg.DoModal() == IDOK)	
	{
		m_carimg = Mat();//复制到m_drawcarimg内

		m_plateimg = Mat();//清空

		for (int i = 0; i<7; i++)
		{
			m_charimg[i] = Mat();//清空
		}
		GetDlgItem(IDC_EDIT_RESULT)->SetWindowText("");//识别结果显示

		filename = dlg.GetPathName();
		m_carimg = imread(filename.GetBuffer(0));
		if(m_carimg.data == NULL)
		{
			MessageBox("载入图像失败!");
			return;
		}
		/*IplImage imgTmp = m_carimg;
		show_pic(&imgTmp,IDC_STATIC_CAR);*/
		SendMessage(WM_PAINT, NULL, NULL);
		UpdateWindow();
	}
}

//车牌定位
void CmyPRDlg::OnBnClickedButtonplatelocal()
{
	// TODO: Add your control notification handler code here
	if(m_carimg.data == NULL)
	{
		MessageBox("请先载入图像!");
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

	vector<Mat> resultVec;//存放车牌图像列表
	vector<RotatedRect> resultRectVec;//存放位置矩形列表
	int result = plate.plateLocate(m_carimg, resultVec,resultRectVec);
	if(result != 0  || resultVec.size() == 0)
	{
		MessageBox("没有定位出车牌！");  
		return;
	}

	//对检测后的图像，进行一次筛选
	vector<Mat> resultVec2;//存放车牌图像列表
	vector<RotatedRect> resultRectVec2;//存放位置矩形列表
	CPlateJudge ju;
	int resultJu = ju.plateJudge(resultVec, resultRectVec,resultVec2,resultRectVec2);//svm筛选矩形
	if (0 != resultJu || resultVec2.size() == 0)
	{
		MessageBox("没有定位出车牌！");
		return;
	}


	//如果最后还有多个车牌，我们选择最大的那个去分割识别
	int maxsize = 0;
	RotatedRect rr;
	for(int i=0;i<resultRectVec2.size();i++)
	{
		int size = resultRectVec2[i].size.height * resultRectVec2[i].size.width;//这个车牌的面积
		if(size > maxsize)//依次挑出面积最大的
		{
			maxsize = size;
			rr = resultRectVec2[i];
			m_plateimg = resultVec2[i];
		}
	}
	
	//在原图绘制出这个区域
	Point2f rect_points[4];
	rr.points(rect_points);//从矩形区域得到4个顶点
	for (int j = 0; j < 4; j++)
	{
		//依次绘制四个线段
		line(m_carimg, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 255), 1, 8);
	}
	/*IplImage imgTmp1 = m_carimg;
	show_pic(&imgTmp1,IDC_STATIC_CAR);


	IplImage imgTmp2 = m_plateimg;
	show_pic(&imgTmp2,IDC_STATIC_PLATE);*/
	SendMessage(WM_PAINT, NULL, NULL);
	UpdateWindow();

}

//车牌分割
void CmyPRDlg::OnBnClickedButtonsegment()
{
	// TODO: Add your control notification handler code here
	if(m_plateimg.data == NULL)
	{
		MessageBox("请先定位车牌!");
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
		m_charimg[j] = resultVec[j];//存到m_charimg数组内
	}


	//for(int j = 0; j < 7; j++)
	//{
	//	IplImage imgTmp = m_charimg[j];
	//	show_pic(&imgTmp,IDC_STATIC_CHAR1 + j);//依次显示7个分割的字符，因为这些控件的ID是递增的，所以可以写循环
	//}

	SendMessage(WM_PAINT, NULL, NULL);
	UpdateWindow();
}

CString CmyPRDlg::imagerecogn(Mat image,int index)
{

	Mat procimg = preprocimg(image);//预处理图片
	Mat f=features(image);//计算这个图像的特征

	

	CString str = "";

	if(index == 0)//第一位汉字
	{
		Mat output(1, numChinese, CV_32FC1);//存放神经网络的输出
		m_chineseann.predict(f, output);

		int rsultindex = 0;//识别的最终类型
			
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
	else if(index == 1)//第二位字母
	{
		Mat output(1, 24, CV_32FC1);//存放神经网络的输出
		m_englishann.predict(f, output);

		int rsultindex = 0;//识别的最终类型
			
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
	else//3-7数字字母混合
	{
		
		Mat output(1, numCharacter, CV_32FC1);//存放神经网络的输出
		m_charann.predict(f, output);

		int rsultindex = 0;//识别的最终类型
			
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

	if(!m_bload)//如果还没加载xml
	{
		if( IsFileExist("model\\char.xml") && IsFileExist("model\\chinese.xml")  && IsFileExist("model\\englishchar.xml"))
		{
				m_charann.load("model\\char.xml");

		
				m_chineseann.load("model\\chinese.xml");

		
				m_englishann.load("model\\englishchar.xml");

			m_bload = true;
		}
	}
	if(!m_bload)//如果依然没找到xml
	{
		MessageBox("未发现字符训练结果集！");
		return;
	}

	if( m_charimg[0].data == NULL)
	{
		MessageBox("请先分割车牌字符!");
		return;
	}


	CString strresult = "";//识别结果
	for (int j = 0; j < 7; j++)//遍历识别7个字符
	{

		if(m_charimg[j].data)
		{
			Mat resultMat = m_charimg[j];//取出一个字符

			strresult = strresult + imagerecogn(resultMat,j);//识别结果，并且累加到字符串

			
		}
	}

	GetDlgItem(IDC_EDIT_RESULT)->SetWindowText(strresult);//识别结果显示
	SendMessage(WM_PAINT, NULL, NULL);
	UpdateWindow();
}

//车牌识别，直接得到识别的字符串
int CmyPRDlg::platerecogn(Mat img,CString &strresult)
{
	CPlateLocate plate;
	plate.setDebug(0);
	plate.setLifemode(true);


	vector<Mat> resultVec;//存放车牌图像列表
	vector<RotatedRect> resultRectVec;//存放位置矩形列表
	int result = plate.plateLocate(img, resultVec,resultRectVec);
	if(result != 0  || resultVec.size() == 0)
	{
		return -1;
	}

	//对检测后的图像，进行一次筛选
	vector<Mat> resultVec2;//存放车牌图像列表
	vector<RotatedRect> resultRectVec2;//存放位置矩形列表
	CPlateJudge ju;
	int resultJu = ju.plateJudge(resultVec, resultRectVec,resultVec2,resultRectVec2);//svm筛选矩形
	if (0 != resultJu || resultVec2.size() == 0)
	{
		return -1;
	}


	//如果最后还有多个车牌，我们选择最大的那个去分割识别
	int maxsize = 0;
	RotatedRect rr;
	Mat plateimg;//车牌图像
	for(int i=0;i<resultRectVec2.size();i++)
	{
		int size = resultRectVec2[i].size.height * resultRectVec2[i].size.width;//这个车牌的面积
		if(size > maxsize)//依次挑出面积最大的
		{
			maxsize = size;
			rr = resultRectVec2[i];
			plateimg = resultVec2[i];
		}
	}

	//定位出来后开始分割
	CCharsSegment cs;
	cs.setDebug(0);
	vector<Mat> charList;
	result = cs.charsSegment(plateimg, charList);
	if (result != 0)
	{
		return -1;
	}

	//开始识别，
	int num = charList.size();
	num = MIN(num,7);//最多识别7位

	for (int j = 0; j < num; j++)//遍历识别7个字符
	{

		Mat resultMat = charList[j];
		strresult = strresult + imagerecogn(resultMat,j);//识别结果，并且累加到字符串
		
	}

	return 0;
}
