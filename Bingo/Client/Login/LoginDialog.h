#pragma once
#include <afxdialogex.h>
#include "resource.h"
#include "../NetLayer/NetLayer.h"

class CLoginDialog 
	: public CDialogEx
{
public:
	CLoginDialog(CWnd* _pParent = nullptr);

	NetLayer m_net;

	// 대화 상자 리소스 ID 매핑
	enum {
		IDD = IDD_LOGIN_DIALOG
	};

public:
	afx_msg void OnBnClickedLoginButton();

protected:
	virtual void DoDataExchange(CDataExchange* _pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};