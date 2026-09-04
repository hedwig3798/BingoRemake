#pragma once
#include <afxdialogex.h>
#include "resource.h"
#include "../NetLayer/NetLayer.h"

/// <summary>
/// 가장 처음 나오는 다이얼로그
/// 로그인 및 가입 창이 나오는 곳
/// </summary>
class CSignInDialog 
	: public CDialogEx
{
public:
	CSignInDialog(NetLayer* _net, CWnd* _pParent = nullptr);

	NetLayer* m_net;

private:
	CEdit* m_idInput;
	CEdit* m_pwInput;
	CButton* m_checkButton;
	CButton* m_signInButton;

	bool m_isIdChecked;

public:

private:
	afx_msg void OnCheckButtonClick();
	afx_msg void OnSignInClick();
	afx_msg void OnEnchangeIdInput();
	afx_msg LRESULT _LTC_ACK_ID_AVAILABLITY(WPARAM _wParam, LPARAM _lParam);
	afx_msg LRESULT _LTC_ACK_SING_UP(WPARAM _wParam, LPARAM _lParam);

protected:
	virtual void DoDataExchange(CDataExchange* _pDX) override;
	virtual BOOL OnInitDialog() override;
	DECLARE_MESSAGE_MAP()
};