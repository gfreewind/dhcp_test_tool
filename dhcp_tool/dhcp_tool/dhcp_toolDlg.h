// dhcp_toolDlg.h : header file
//

#pragma once

#include <sys/timeb.h>
#include "config_file.h"
#include "afxwin.h"

enum {
    MY_WM_UPDATE = WM_USER+1,
};

// Cdhcp_toolDlg dialog
class Cdhcp_toolDlg : public CDialog
{
// Construction
public:
	Cdhcp_toolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DHCP_TOOL_DIALOG };

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
    UINT_PTR GetTimerHandle() const { return timer;}
    bool GetTimeout() {return timeout;}
    void SetTimeout(bool value) {timeout = value;}
    void SetTimerHandle(UINT_PTR hd) {timer = hd;}

    // Get basic setting
    B_U32 GetLocalIp() const {return localIp;}
    B_U32 GetServerIp() const {return serverIp;}
    B_U32 GetInterval() const {return atoi(csInterval);}
    BOOL  IsReleaseAfterUsed() const {return bReleaseAfterUsed;};
    int   GetMacWay() const { return macWay; };

    // Get Advanced setting
    const char* GetMacString() const {return csMac;};
    BOOL IsFixedSubnet() const {return fixedSubnet;};
    B_U32 GetFixedSubnet() const {return subnet;};
    BOOL IsFixedCnt() const {return bFixedCnt;};
    BOOL IsRepeatMac() const {return bRepeatMac;};
    B_U32 GetFixedCnt() const {return fixedCnt;};
    B_U32 GetRepeatCnt() const {return rptCnt;};
    
    
    void ReleaseRes();	
    void ChangeControlStatus(bool defaultValue);
    void UpdateResult();
    
	afx_msg void OnBnClickedOk();
	
    volatile bool    recvExit;
    volatile bool    sendExit;
    volatile bool    refreshExit;
    
	B_U32   discoverCnt;
	B_U32   offerCnt;
	B_U32   requestCnt;
	B_U32   ackCnt;
	
private:
    void InitMember();
	void ResetValues();

    bool CheckInput();
    void InitServerSockAddr();
    void SaveMember();

    void WinSockStartUp();

    //Basic Setting
	CString csLocalIp;
	CString szServerIp;
	CString csInterval;
	CString csTotalTime;
	B_U32   localIp; //network order
	B_U32   serverIp;//network order
	BOOL    bReleaseAfterUsed;
	int     macWay;

    //Advanced setting
	CString csMac;
	BOOL    fixedSubnet;
	CString csSubnet;
	B_U32   subnet;//network order	
	BOOL    bFixedCnt;
	CString csCnt;
	B_U32   fixedCnt;
	CString csRepeatCnt;
	B_U32   rptCnt;

    //Result	
	CString csDiscover;	
	CString csOffer;
	CString csRequest;
	CString csAck;

    volatile bool    timeout;    
	
	struct sockaddr_in sockaddrServer;	
	HANDLE	handleSendThread;
	HANDLE	handleRecvThread;
	HANDLE  handleRefreshThread;
	DWORD	dwSendThreadID;
	DWORD	dwRecvThreadID;
	DWORD   dwRefreshThreadID;

	UINT_PTR timer;

	struct _timeb timeStart;
	struct _timeb timeEnd;
	
	afx_msg void OnBnClickedCancel();
private:
	CString csPerformance;
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedCheckFixsubnet();
    afx_msg void OnBnClickedCheckFixcnt();
	afx_msg void OnBnClickedRadioMac1();
	afx_msg void OnBnClickedRadioMac2();
	afx_msg void OnBnClickedRadioMac3();
	afx_msg LRESULT UpdateShowResult(WPARAM,LPARAM);
public:
    BOOL bRepeatMac;
    afx_msg void OnBnClickedCheckLoop();
    unsigned long ulRepeatCnt;
};
