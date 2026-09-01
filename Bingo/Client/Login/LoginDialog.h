#pragma once
#include <afxdialogex.h>
#include "resource.h"
#include "../NetLayer/NetLayer.h"

/// <summary>
/// 가장 처음 나오는 다이얼로그
/// 로그인 및 가입 창이 나오는 곳
/// </summary>
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
	virtual void DoDataExchange(CDataExchange* _pDX) override;
	virtual BOOL OnInitDialog() override;
	DECLARE_MESSAGE_MAP()
};