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

private:
	CEdit* m_idInput;
	CEdit* m_pwInput;
	CButton* m_loginButton;

public:
	afx_msg void OnBnClickedLoginButton();
	afx_msg LRESULT _LTC_ACK_LOGIN(WPARAM _wParam, LPARAM _lParam);

protected:
	virtual void DoDataExchange(CDataExchange* _pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};