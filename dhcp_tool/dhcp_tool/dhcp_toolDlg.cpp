// dhcp_toolDlg.cpp : implementation file
//

#include "stdafx.h"
#include <sys/types.h>
#include <sys/timeb.h>
#include <cassert>
#include <iostream>
#include <algorithm>

#include "dhcp_tool.h"
#include "dhcp_toolDlg.h"
#include "my_network_utils.h"
#include "my_string_utils.h"
#include "my_trace.h"
#include "dhcp_packet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

#pragma warning( disable : 4996 )

/****************************************** Global Variable **************************************************/
static const B_U32 DHCP_IP_STR_MAX_LEN   = 15;
static const B_U32 DHCP_MAC_STR_MAX_LEN  = 17;
static const B_U32 DHCP_TIME_STR_MAX_LEN = 32;

static const B_U16 DHCP_SERVER_PORT      = 67;
static const B_U16 DHCP_CLIENT_PORT		 = 68;
static const B_U32 DHCP_RECV_WAIT_TIME   = 1; // 1ms

static const B_U32 IDT_TIMER1            = 10000;

typedef enum {
	GENERATE_RANDOM_MAC_E,
	GENERATE_SORTED_MAC_E,
	GENERATE_FIXED_MAC_E
} GENERATE_MAC_WAY_E;


static ConfigFile g_configFile("dhcp_tool.config");
static Cdhcp_toolDlg* gpDhcpDlg;
static LogFile glogFile("dhcp_tool_err.log");

/*************************************************************************************************************/
static bool ConvertStrToIP(CString str, B_U32& ip);
static bool ReleaseThreadResource(HANDLE& hd, B_U32& exitCode);
static SOCKET OpenSock(B_U32 localip, B_U16 port);
static void WaitThreadExit(HANDLE hd, volatile bool& exitFlag);
static bool IsValidMacStr(const char* macStr);

static VOID CALLBACK TestTimerProc(HWND , UINT , UINT_PTR , DWORD );
static DWORD WINAPI RefreshResultThread(PVOID pParam);
static DWORD WINAPI RecvThread(PVOID pParam);
static DWORD WINAPI SendThread(PVOID pParam);


/*************************************************************************************************************/


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	EnableActiveAccessibility();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// Cdhcp_toolDlg dialog




Cdhcp_toolDlg::Cdhcp_toolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cdhcp_toolDlg::IDD, pParent)
	, csLocalIp(_T(""))
	, szServerIp(_T(""))
	, csInterval(_T(""))
	, csTotalTime(_T(""))
	, csDiscover(_T(""))
	, csOffer(_T(""))
	, csRequest(_T(""))
	, csAck(_T(""))
	, csPerformance(_T(""))
	, csMac(_T(""))
	, fixedSubnet(FALSE)
	, csSubnet(_T(""))
	, bFixedCnt(FALSE)
	, csCnt(_T(""))
	, csRepeatCnt(_T(""))
	, bReleaseAfterUsed(FALSE)
    , ulRepeatCnt(0)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cdhcp_toolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LOCALIP, csLocalIp);
	DDX_Text(pDX, IDC_EDIT_SERVERIP, szServerIp);
	DDX_Text(pDX, IDC_EDIT_INTERVAL, csInterval);
	DDX_Text(pDX, IDC_EDIT_TOTALTIME, csTotalTime);
	DDX_Text(pDX, IDC_EDIT_DISCOVER, csDiscover);
	DDV_MaxChars(pDX, csLocalIp, 15);
	DDV_MaxChars(pDX, csInterval, 32);
	DDV_MaxChars(pDX, csTotalTime, 32);
	DDV_MaxChars(pDX, szServerIp, 15);
	DDX_Text(pDX, IDC_EDIT_OFFER, csOffer);
	DDX_Text(pDX, IDC_EDIT_REQUEST, csRequest);
	DDX_Text(pDX, IDC_EDIT_ACK, csAck);
	DDX_Text(pDX, IDC_EDIT_PERFORMANCE, csPerformance);
	DDX_Text(pDX, IDC_EDIT_MAC, csMac);
	DDV_MaxChars(pDX, csMac, 17);
	DDX_Check(pDX, IDC_CHECK_FIXSUBNET, fixedSubnet);
	DDX_Text(pDX, IDC_EDIT_SUBNET, csSubnet);
	DDV_MaxChars(pDX, csSubnet, 15);
	DDX_Check(pDX, IDC_CHECK_FIXCNT, bFixedCnt);
	DDX_Check(pDX, IDC_CHECK_LOOP, bRepeatMac);
	DDX_Text(pDX, IDC_EDIT_CNT, csCnt);
	DDX_Text(pDX, IDC_EDIT_LOOP, csRepeatCnt);
	DDX_Check(pDX, IDC_CHECK_RELEASE, bReleaseAfterUsed);
	DDX_Radio( pDX, IDC_RADIO_MAC1, macWay);
//	DDX_Radio( pDX, IDC_RADIO_MAC2, bCheckMac2);
//	DDX_Radio( pDX, IDC_RADIO_MAC3, bCheckMac3);
}

BEGIN_MESSAGE_MAP(Cdhcp_toolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &Cdhcp_toolDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDEXIT, &Cdhcp_toolDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDSTOP, &Cdhcp_toolDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_CHECK_FIXSUBNET, &Cdhcp_toolDlg::OnBnClickedCheckFixsubnet)
	ON_BN_CLICKED(IDC_CHECK_FIXCNT, &Cdhcp_toolDlg::OnBnClickedCheckFixcnt)
	ON_BN_CLICKED(IDC_RADIO_MAC1, &Cdhcp_toolDlg::OnBnClickedRadioMac1)
	ON_BN_CLICKED(IDC_RADIO_MAC2, &Cdhcp_toolDlg::OnBnClickedRadioMac2)
	ON_BN_CLICKED(IDC_RADIO_MAC3, &Cdhcp_toolDlg::OnBnClickedRadioMac3)
    ON_BN_CLICKED(IDC_CHECK_LOOP, &Cdhcp_toolDlg::OnBnClickedCheckLoop)
    ON_MESSAGE(MY_WM_UPDATE, &Cdhcp_toolDlg::UpdateShowResult)
END_MESSAGE_MAP()


// Cdhcp_toolDlg message handlers

BOOL Cdhcp_toolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	TRACE("OnInitDialog start\n");

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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
	macWay = GENERATE_RANDOM_MAC_E;
	InitMember();
	GetDlgItem(IDC_EDIT_MAC)->EnableWindow(GENERATE_FIXED_MAC_E == macWay);
	TRACE("OnInitDialog exit\n");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cdhcp_toolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{	
		if (SC_CLOSE == nID)
		{
			SaveMember();
			ReleaseRes();
			WSACleanup();
		}
	
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cdhcp_toolDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cdhcp_toolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void Cdhcp_toolDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
	UpdateData(TRUE);
	ResetValues();

	if (!CheckInput()) {
		//error
		return;
	}

	InitServerSockAddr();
	
	// create recv thread
	handleRecvThread = CreateThread(NULL, 0, RecvThread, this, 0, &dwRecvThreadID);
	if (NULL == handleRecvThread) {
		glogFile<< "OnBnClickedOk, CreateThread failed, error: "<<GetLastError()<<endl;
		MessageBox("Create recv thread error",NULL,MB_OK);
		ReleaseRes();
		return;
	}
	// create send thread
	handleSendThread = CreateThread(NULL, 0, SendThread, this, 0, &dwSendThreadID);
	if (NULL == handleSendThread) {
		glogFile<<"OnBnClickedOk, CreateThread failed, error: "<<GetLastError()<<endl;
		MessageBox("Create send thread error",NULL,MB_OK);
		ReleaseRes();
		return;
	}
#if 0	
	// create refresh thread
	handleRefreshThread = CreateThread(NULL, 0, RefreshResultThread, this, 0, &dwRefreshThreadID);
	if (NULL == handleRefreshThread) {
		glogFile<<"OnBnClickedOk, CreateThread failed, error: "<<GetLastError()<<endl;
		MessageBox("Create refresh thread error",NULL,MB_OK);
		ReleaseRes();
		return;				
	}
#endif
	ChangeControlStatus(false);

	// set timer
	timer = ::SetTimer(NULL, IDT_TIMER1, atoi(csTotalTime)*1000, (TIMERPROC)TestTimerProc);
	if (0 == timer) {
		glogFile<<"OnBnClickedOk, ::SetTimer failed, error: "<<GetLastError()<<endl;
		MessageBox("Create timer error",NULL,MB_OK);
		timeout = true;
		ChangeControlStatus(TRUE);
		ReleaseRes();
		return;
	}
	_ftime_s(&timeStart);
}

void Cdhcp_toolDlg::OnBnClickedStop()
{
	// TODO: Add your control notification handler code here	
	gpDhcpDlg->SetTimeout(true);
	gpDhcpDlg->ReleaseRes();
	gpDhcpDlg->UpdateResult();
	gpDhcpDlg->ChangeControlStatus(true);
}


void Cdhcp_toolDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here	
	SaveMember();
	ReleaseRes();
	WSACleanup();
	OnCancel();
}

void Cdhcp_toolDlg::OnBnClickedCheckFixsubnet()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_SUBNET)->EnableWindow(fixedSubnet);
}

void Cdhcp_toolDlg::OnBnClickedCheckFixcnt()
{
	// TODO: Add your control notification handler code here	
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_CNT)->EnableWindow(bFixedCnt);
}

void Cdhcp_toolDlg::OnBnClickedCheckLoop()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_LOOP)->EnableWindow(bRepeatMac);
}


void Cdhcp_toolDlg::OnBnClickedRadioMac1()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_MAC)->EnableWindow(FALSE);
}

void Cdhcp_toolDlg::OnBnClickedRadioMac2()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_MAC)->EnableWindow(FALSE);
}

void Cdhcp_toolDlg::OnBnClickedRadioMac3()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT_MAC)->EnableWindow(TRUE);
}

LRESULT Cdhcp_toolDlg::UpdateShowResult(WPARAM,LPARAM)
{
	UpdateResult();
    return 0;
}


/******************************** My Codes *******************************************/
void Cdhcp_toolDlg::InitMember()
{
	csLocalIp = inet_ntoa_e(g_configFile.GetLocalIp());
	szServerIp = inet_ntoa_e(g_configFile.GetServerIp());
	csInterval = ConvertNumToString(g_configFile.GetInterval()).c_str() ;
	csTotalTime = ConvertNumToString(g_configFile.GetTotaltime()).c_str();
	bReleaseAfterUsed = g_configFile.GetReleaseFlag();
	
	macWay = g_configFile.GetMacSettingFlag();
	if (macWay > GENERATE_FIXED_MAC_E) {
		macWay = GENERATE_RANDOM_MAC_E;
	}

	ResetValues();

	gpDhcpDlg = this;
	
	UpdateData(FALSE);
}

void Cdhcp_toolDlg::ResetValues()
{	
	csDiscover = "0";
	csOffer = "0";
	csRequest = "0";
	csAck = "0";
	csPerformance = "0.00";

	timeout = false;
	handleSendThread = NULL;
	handleRecvThread = NULL;
	handleRefreshThread = NULL;
	dwSendThreadID = 0;
	dwRecvThreadID = 0;
	dwRefreshThreadID = 0;
	timer = 0;

	discoverCnt = 0;
	offerCnt = 0;
	requestCnt = 0;
	ackCnt = 0;

	UpdateData(FALSE);
}

bool Cdhcp_toolDlg::CheckInput()
{
	if (!ConvertStrToIP(csLocalIp, localIp)) {
		MessageBox("Local IP error", NULL, MB_OK);
		return false;
	}
	if (!ConvertStrToIP(szServerIp, serverIp)) {
		MessageBox("Server IP error", NULL, MB_OK);
		return false;
	}
	if (fixedSubnet && !ConvertStrToIP(csSubnet, subnet)) {		
		MessageBox("Fixed subnet error", NULL, MB_OK);
		return false;
	}

	if (bFixedCnt) {		
		fixedCnt = atoi(csCnt);
		if (0 == fixedCnt) {		
			MessageBox("Fixed Count couldn't be 0", NULL, MB_OK);
			return false;				
		}
	}

	if (bRepeatMac) {
		rptCnt = atoi(csRepeatCnt);
		if (0 == rptCnt) {
			MessageBox("Repeat Count couldn't be 0", NULL, MB_OK);
			return false;							
		}
	}
	
	if (GENERATE_FIXED_MAC_E == macWay && !IsValidMacStr(csMac)) {
		MessageBox("Fixed MAC error", NULL, MB_OK);
		return false;				
	}

	if (!g_configFile.IsValidLocalIp(localIp)) {
		return (MessageBox("Can't confirm the local IP is true. Continue?", NULL, MB_YESNO) == IDYES);
	}
	
	if (0 == atoi(csTotalTime)) {
		MessageBox("Please enter total time", NULL, MB_OK);
		return false;
	}
		
	return TRUE;
}

void Cdhcp_toolDlg::SaveMember()
{
	UpdateData(TRUE);

	//Save local ip
	g_configFile.SetLocalIp(inet_addr(csLocalIp));
	
	//Save server ip
	g_configFile.SetServerIP(inet_addr(szServerIp));

	//Save interval	
	g_configFile.SetInterval(atoi(csInterval));

	//save total time
	g_configFile.SetTotaltime(atoi(csTotalTime));

	//Save the release flag
	g_configFile.SetReleaseFlag(bReleaseAfterUsed);

	//Save the mac-setting flag
	g_configFile.SetMacSettingFlag(macWay);
}

void Cdhcp_toolDlg::InitServerSockAddr()
{
	memset(&sockaddrServer, 0, sizeof(sockaddrServer));
    sockaddrServer.sin_family = AF_INET;

    sockaddrServer.sin_port = htons(DHCP_SERVER_PORT);
    sockaddrServer.sin_addr.s_addr = serverIp;
}

void Cdhcp_toolDlg::ReleaseRes()
{
	//Release receive thread resource
	if (NULL != handleRecvThread) {
		DWORD exitCode = 0;

		WaitThreadExit(handleRecvThread, recvExit);

		ReleaseThreadResource(handleRecvThread, exitCode);
		if (exitCode) {
			MessageBox("Recv Thread exit abnormally", NULL, MB_OK);
		}
	}

	if (NULL != handleSendThread) {
		DWORD exitCode = 0;

		WaitThreadExit(handleSendThread, sendExit);
		ReleaseThreadResource(handleSendThread, exitCode);
		if (exitCode) {
			MessageBox("Send Thread exit abnormally", NULL, MB_OK);
		}
	}

	if (NULL != handleRefreshThread) {		
		DWORD exitCode = 0;

		WaitThreadExit(handleRefreshThread, refreshExit);
		ReleaseThreadResource(handleRefreshThread, exitCode);
		if (exitCode) {
			MessageBox("Refresh Thread exit abnormally", NULL, MB_OK);
		}
	}

	if (0 != timer) {
		if (!::KillTimer(NULL, timer)) {
			glogFile<<"ReleaseRes, KillTimer failed, error: "<<GetLastError()<<endl;
			MessageBox("Failed to free timer resource", NULL, MB_OK);
		}
		timer = 0;
	}
}

void Cdhcp_toolDlg::UpdateResult()
{
	_ftime_s(&timeEnd);

	//Now update the result
	char szNumStr[64];
	_snprintf(szNumStr, sizeof(szNumStr), "%lu", discoverCnt);
	csDiscover = szNumStr;
	_snprintf(szNumStr, sizeof(szNumStr), "%lu", offerCnt);
	csOffer = szNumStr;
	_snprintf(szNumStr, sizeof(szNumStr), "%lu", requestCnt);
	csRequest = szNumStr;
	_snprintf(szNumStr, sizeof(szNumStr), "%lu", ackCnt);
	csAck = szNumStr;

	//performance
	B_U64 run_time = timeEnd.time -timeStart.time;
	if (run_time > 0) {
   		_snprintf(szNumStr, sizeof(szNumStr), "%.2f", ackCnt == 0? 0.00: static_cast<float>(ackCnt)/(run_time));
	}
	csPerformance = szNumStr;
	
	UpdateData(FALSE);
}

void Cdhcp_toolDlg::ChangeControlStatus(bool defaultValue)
{
	//input control
	//Basic Setting
	GetDlgItem(IDC_EDIT_LOCALIP)->EnableWindow(defaultValue);
	GetDlgItem(IDC_EDIT_SERVERIP)->EnableWindow(defaultValue);
	GetDlgItem(IDC_EDIT_INTERVAL)->EnableWindow(defaultValue);
	GetDlgItem(IDC_EDIT_TOTALTIME)->EnableWindow(defaultValue);
	GetDlgItem(IDC_CHECK_RELEASE)->EnableWindow(defaultValue);
	
	//Advanced setting
	GetDlgItem(IDC_EDIT_MAC)->EnableWindow(defaultValue&&(GENERATE_FIXED_MAC_E == macWay));
	GetDlgItem(IDC_CHECK_FIXSUBNET)->EnableWindow(defaultValue);
	GetDlgItem(IDC_EDIT_SUBNET)->EnableWindow(defaultValue&&fixedSubnet);
	GetDlgItem(IDC_CHECK_FIXCNT)->EnableWindow(defaultValue);
	GetDlgItem(IDC_EDIT_CNT)->EnableWindow(defaultValue&&bFixedCnt);	
	GetDlgItem(IDC_EDIT_LOOP)->EnableWindow(defaultValue&&bRepeatMac);
	GetDlgItem(IDC_RADIO_MAC1)->EnableWindow(defaultValue);
	GetDlgItem(IDC_RADIO_MAC2)->EnableWindow(defaultValue);
	GetDlgItem(IDC_RADIO_MAC3)->EnableWindow(defaultValue);

	//Action control
	GetDlgItem(IDOK)->EnableWindow(defaultValue);
	GetDlgItem(IDEXIT)->EnableWindow(defaultValue);
	GetDlgItem(IDSTOP)->EnableWindow(!defaultValue);

	UpdateData(FALSE);
}

void Cdhcp_toolDlg::WinSockStartUp()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 2, 2 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		return;
	}
 
	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */
 
	if ( LOBYTE( wsaData.wVersion ) != 2 ||
			HIBYTE( wsaData.wVersion ) != 2 ) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		WSACleanup( );
		return; 
	}
}


/********************************************************************************************/
static bool IsValidMacStr(const char* macStr)
{
	if (17 != strlen(macStr)) {
		return false;
	}

	bool isCheckSeperator = false;
	int  numCnt = 0;
	while (*macStr) {
		if (isCheckSeperator) {
			if (':' != *macStr) {
				return false;
			}
			isCheckSeperator = false;
		}
		else {
			if (isxdigit((unsigned char)*macStr)) {
				++numCnt;
			}
			else {
				return false;
			}

			if (0 == numCnt%2) {
				isCheckSeperator = true;
				numCnt = 0;
			}
		}
		++macStr;
	}

	return true;
}

static bool ConvertStrToIP(CString str, B_U32& ip)
{
	ip = inet_addr(str);
	return (INADDR_NONE != ip);
}

static bool ReleaseThreadResource(HANDLE& hd, B_U32& exitCode)
{
	bool ret = true;

	if (!GetExitCodeThread(hd, &exitCode)) {
		glogFile<<"ReleaseThreadResource, GetExitCodeThread failed, error: "<<GetLastError()<<endl;
		MessageBox(NULL, "Failed to free thread resource", NULL, MB_OK);
		ret = false;
	}
	CloseHandle(hd);
	hd = NULL;		

	return ret;
}

static SOCKET OpenSock(B_U32 localip, B_U16 port)
{
	struct sockaddr_in sockaddr;
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == sock)
    {
		glogFile<<"OpenSock, socket failed, error: "<<WSAGetLastError()<<endl;
        goto error;
    }

    /*
    Set socket option
    */
    int optval = 1;
    int optlen = sizeof(optval);
    if (SOCKET_ERROR == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, optlen)) {    	
    	glogFile<<"OpenSock, Set SO_REUSEADDR failed, error: "<<WSAGetLastError()<<endl;
        goto error;
    }
	
	memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    sockaddr.sin_addr.s_addr = localip;
    if (-1 == bind(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr)))
    {
    	glogFile<<"OpenSock, bind failed, error: "<<WSAGetLastError()<<endl;
        goto error;
    }

    return sock;
error:
    if (INVALID_SOCKET != sock)
    {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    return sock;
}

static void WaitThreadExit(HANDLE hd, volatile bool& exitFlag)
{
	int cnt = 0;

	//wait 5 seconds
	while (!exitFlag || cnt > 100) {
		Sleep(50);
		++cnt;
	}

	if (!exitFlag) {
		TerminateThread(hd, DWORD(-1));
	}
}

/* timer callback */
static VOID CALLBACK TestTimerProc(HWND , UINT , UINT_PTR , DWORD)
{
	TRACE("TestTimerProc start\n");
	gpDhcpDlg->SetTimeout(true);
	gpDhcpDlg->ReleaseRes();
	gpDhcpDlg->UpdateResult();
	gpDhcpDlg->ChangeControlStatus(true);
	TRACE("TestTimerProc end\n");
}

/* Refresh Result thread */
static DWORD WINAPI RefreshResultThread(PVOID pParam)
{
	Cdhcp_toolDlg* pDlg = (Cdhcp_toolDlg*)pParam;
	ExitFlag exitFlag(pDlg->refreshExit);

	while (!pDlg->GetTimeout()) {		
		Sleep(100);
		gpDhcpDlg->UpdateResult();
	}

	return 0;
}

/* recv thread */
static DWORD WINAPI RecvThread(PVOID pParam) 
{
	TRACE("RecvThread start\n");
	Cdhcp_toolDlg* pDlg = (Cdhcp_toolDlg*)pParam;
	ExitFlag exitFlag(pDlg->recvExit);
	
	SOCKET sock = OpenSock(pDlg->GetLocalIp(), DHCP_SERVER_PORT);
	if (INVALID_SOCKET == sock) {
		TRACE("RecvThread exit 1\n");
		MessageBox(NULL, "RecvThread, failed to open recv socket", NULL, MB_OK);
		return 1;
	}

	DhcpPacket dhcpPacket;
	struct timeval	timeOut;
	bzero(&timeOut, sizeof(timeOut));
	fd_set	rSet;

static const B_U32 DHCP_PACKET_RECV_BUF_LEN = 2048;
	char *packetBuf = new char [DHCP_PACKET_RECV_BUF_LEN];
	struct sockaddr_in addr;
	int fromLen = sizeof(addr);
	
	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(DHCP_SERVER_PORT);
	serverAddr.sin_addr.s_addr = pDlg->GetServerIp();
	
	while (!pDlg->GetTimeout()) {				
		bzero(&addr, sizeof(addr));
	
		// 1 ms;
		timeOut.tv_usec = DHCP_RECV_WAIT_TIME;
		FD_ZERO(&rSet);
		FD_SET(sock, &rSet);

		int ret = select(static_cast<int>(sock)+1, &rSet, NULL, NULL, &timeOut);
		if (SOCKET_ERROR == ret) {
			glogFile<<"RecvThread, select failed, error: "<<WSAGetLastError()<<endl;
			closesocket(sock);
			sock = INVALID_SOCKET;
			TRACE("RecvThread exit 2\n");
			delete [] packetBuf;
			return ret;						
		}
		
		if (FD_ISSET(sock, &rSet)) {
			ret = recvfrom(sock, packetBuf, DHCP_PACKET_RECV_BUF_LEN, 0, 
						   (struct sockaddr*)(&addr), &fromLen);
			if (SOCKET_ERROR == ret) {				
				glogFile<<"RecvThread, recvfrom failed, error: "<<WSAGetLastError()<<endl;
				closesocket(sock);
				sock = INVALID_SOCKET;
				TRACE("RecvThread exit 3\n");
				delete [] packetBuf;
				return ret; 					
			}

			if (!dhcpPacket.CopyDhcpPacket(packetBuf, ret)) {
				glogFile<<"RecvThread, CopyDhcpPacket failed"<<endl;
				TRACE("RecvThread, CopyDhcpPacket failed\n");
				continue;
			}

			if (dhcpPacket.IsOfferMsg()) {
				++pDlg->offerCnt;
				dhcpPacket.SetMsgType(DHCP_MSG_TYPE_REQUEST_E);
//				dhcpPacket.SetClientIp(dhcpPacket.GetYourIp());

				/* add option */
				DHCP_OPTION_SET_S opt;
				B_U32 request_ip = dhcpPacket.GetYourIp();
		
				bzero(&opt, sizeof(opt));
				opt.eType = DHCP_OPTION_REQUEST_IP_E;
				opt.len = sizeof(request_ip);
				memcpy(opt.data, &request_ip, sizeof(request_ip));

				dhcpPacket.AddOption(opt);
				
				dhcpPacket.SetYourIp(0); 
				dhcpPacket.SetEndOption();
				dhcpPacket.SendPacket(sock, serverAddr);
				++pDlg->requestCnt;
			}
			else if (dhcpPacket.IsAckMsg()) {
				++pDlg->ackCnt;
				if (pDlg->IsReleaseAfterUsed()) {					
					dhcpPacket.SetMsgType(DHCP_MSG_TYPE_RELEASE_E);
					dhcpPacket.SetClientIp(dhcpPacket.GetYourIp());
					dhcpPacket.SetYourIp(0);
					dhcpPacket.SetEndOption();
					dhcpPacket.SendPacket(sock, serverAddr);
				}			
			}
		}
	}

	closesocket(sock);
	sock = INVALID_SOCKET;
	delete [] packetBuf;
	TRACE("RecvThread exit\n");
	return 0;
}

#include "windows.h"
/* send thread */
static DWORD WINAPI SendThread(PVOID pParam)
{	
	TRACE("SendThread start\n");
	Cdhcp_toolDlg* pDlg = (Cdhcp_toolDlg*)pParam;
	ExitFlag exitFlag(pDlg->sendExit);
	
	SOCKET sock = OpenSock(pDlg->GetLocalIp(), DHCP_CLIENT_PORT);
	if (INVALID_SOCKET == sock) {
		TRACE("SendThread exit 1\n");
		MessageBox(NULL, "SendThread, failed to open send socket", NULL, MB_OK);
		return 1;
	}

	struct sockaddr_in serverAddr;
	int len = sizeof(serverAddr);
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(DHCP_SERVER_PORT);
	serverAddr.sin_addr.s_addr = pDlg->GetServerIp();

	if (connect(sock, (const sockaddr *)&serverAddr, len)) {
		TRACE("SendTread, connect failed exit 2\n");
		MessageBox(NULL, "SendThread, failed to connect to server", NULL, MB_OK);
		return 2;
	}
	

	DhcpPacket dhcpPacket;

	if (pDlg->IsFixedSubnet()) {
		B_U32 subnet = pDlg->GetFixedSubnet();
		DHCP_OPTION_SET_S opt;
		
		bzero(&opt, sizeof(opt));
		opt.eType = DHCP_OPTION_SUBNET_SELECTION_E;
		opt.len = sizeof(subnet);
		memcpy(opt.data, &subnet, sizeof(subnet));

		dhcpPacket.AddOption(opt);
	}

	dhcpPacket.SetPacketType(DHCP_PACKET_TYPE_DISCOVER_E);
	dhcpPacket.SetRouteAddr(pDlg->GetLocalIp());
	dhcpPacket.SetEndOption();

	struct _timeb curTime;
	MY_MAC_ADDR mc;
	vector<MY_MAC_ADDR> vecMc;
	
	_ftime_s(&curTime);

	B_U32 total_cnt = 0;
	if (pDlg->IsFixedCnt()) {
		total_cnt = pDlg->GetFixedCnt();
	}
	else {
		total_cnt = (B_U32)-1;
	}

	long long macValue = 1;
	int macWay = pDlg->GetMacWay();
	B_U32 xid = (B_U32)curTime.time;
	
	if (GENERATE_FIXED_MAC_E == macWay) {
		const char* pszMacStr = pDlg->GetMacString();
		mc = ConvertMacStr2Value(pszMacStr);
		dhcpPacket.SetClientMac(mc);
	}


	if (GENERATE_RANDOM_MAC_E == macWay) {
		/* Set random seed */
		srand((B_U32)curTime.time);
	}
	
	while (!pDlg->GetTimeout() && pDlg->discoverCnt < total_cnt) {

		if (pDlg->IsRepeatMac() && pDlg->discoverCnt >= pDlg->GetRepeatCnt()) {
			int i = pDlg->discoverCnt%pDlg->GetRepeatCnt();
			mc = vecMc[i];
			dhcpPacket.SetClientMac(mc);
		}
		else {			
		    if (GENERATE_RANDOM_MAC_E == macWay) {
				int lowBits = (int)curTime.time;
				int highBits = (int)(curTime.time>>32);
			
				lowBits = htonl(lowBits);
				highBits = htonl(highBits);
			
				lowBits += rand();
				highBits += rand();
			
				mc.len = 6;
				memcpy(mc.addr, &highBits, 2);
				memcpy(mc.addr+2, &lowBits, 4);
				dhcpPacket.SetClientMac(mc);
			}
			else if (GENERATE_SORTED_MAC_E == macWay) {
				int lowBits = (int)macValue;
				int highBits = (int)(macValue>>32);
				
				lowBits = htonl(lowBits);
				highBits = htonl(highBits);
				mc.len = 6;
				memcpy(mc.addr, &highBits, 2);
				memcpy(mc.addr+2, &lowBits, 4);
				dhcpPacket.SetClientMac(mc);
				
				++macValue;
			}
			
			if (pDlg->IsRepeatMac()) {
				vecMc.push_back(mc);
			}
		}
		
		
		dhcpPacket.SetXid(xid);
		++xid;
	
		dhcpPacket.SendPacket(sock, serverAddr);
		++pDlg->discoverCnt;
		if (!PostMessage(pDlg->m_hWnd, MY_WM_UPDATE, 0, 0)) {
			TRACE("SendThread, post msg error\n");
		}
        //pDlg->UpdateResult();
		Sleep(pDlg->GetInterval());
	}

	closesocket(sock);
	sock = INVALID_SOCKET;
	TRACE("SendThread exit\n");
	return 0;
}
