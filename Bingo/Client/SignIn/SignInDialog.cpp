#include "SignInDialog.h"
#include "MassageDefine.h"
#include "Password.h"
#include <iostream>
CSignInDialog::CSignInDialog(NetLayer* _net, CWnd* _pParent /*= nullptr*/)
	: CDialogEx(IDD_SIGN_IN, _pParent)
	, m_net(_net)
	, m_idInput(nullptr)
	, m_pwInput(nullptr)
	, m_checkButton(nullptr)
	, m_signInButton(nullptr)
	, m_isIdChecked(false)
{
}

void CSignInDialog::OnCheckButtonClick()
{
	m_checkButton->EnableWindow(FALSE);

	if (nullptr == m_net)
	{
		AfxMessageBox(_T("네트워크 오류. 재실행이 필요합니다."), MB_ICONWARNING | MB_OK);
		m_checkButton->EnableWindow(TRUE);
		return;
	}

	if (true == m_net->IsWaitingPacket(LTC_ACK_ID_AVAILABLITY::PACKET_ID))
	{
		m_checkButton->EnableWindow(TRUE);
		return;
	}

	CString inputID;
	m_idInput->GetWindowTextA(inputID);
	if ("" == inputID)
	{
		AfxMessageBox(_T("아이디를 입력해주세요."), MB_ICONWARNING | MB_OK);
		m_checkButton->EnableWindow(TRUE);
		return;
	}

	CTL_RES_ID_AVAILABLITY data;
	data.m_ID = inputID;

	m_net->Send(data);
	m_net->SetWaitPacket(LTC_ACK_ID_AVAILABLITY::PACKET_ID, 60);
}

void CSignInDialog::OnSignInClick()
{
	if (false == m_isIdChecked)
	{
		AfxMessageBox(_T("ID 중복체크가 필요합니다."), MB_ICONWARNING | MB_OK);
	}
}

void CSignInDialog::OnEnchangeIdInput()
{
	m_isIdChecked = false;
}

LRESULT CSignInDialog::_LTC_ACK_ID_AVAILABLITY(WPARAM _wParam, LPARAM _lParam)
{
	m_net->RelaseWaiting(LTC_ACK_ID_AVAILABLITY::PACKET_ID);
	m_checkButton->EnableWindow(TRUE);

	LTC_ACK_ID_AVAILABLITY* data = (LTC_ACK_ID_AVAILABLITY*)_wParam;
	if (nullptr == data)
	{
		std::cerr << "data nullptr at _LTC_ACK_ID_AVAILABLITY\n";
		return 0;
	}

	switch (data->m_netError)
	{
	case NET_ERROR::NET_OK:
	{
		if (false == data->m_isExist)
		{
			m_isIdChecked = true;
			AfxMessageBox(_T("사용 가능한 아이디 입니다."), MB_ICONWARNING | MB_OK);
		}
		else
		{
			m_isIdChecked = false;
			AfxMessageBox(_T("중복된 아이디 입니다."), MB_ICONWARNING | MB_OK);
		}
		break;
	}

	default:
	{
		std::cerr << "net error at _LTC_ACK_ID_AVAILABLITY\n";
		break;
	}
	}
	return 0;
}

void CSignInDialog::DoDataExchange(CDataExchange* _pDX)
{
	CDialogEx::DoDataExchange(_pDX);
}

BOOL CSignInDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_idInput = (CEdit*)GetDlgItem(IDC_CREATE_ID);
	m_pwInput = (CEdit*)GetDlgItem(IDC_CREATE_PW);
	m_checkButton = (CButton*)GetDlgItem(IDC_ID_CHECK);
	m_signInButton = (CButton*)GetDlgItem(IDC_CREATE_ACCOUNT_BUTTON);

	m_net->SetDialog(this);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CSignInDialog, CDialogEx)
	ON_BN_CLICKED(IDC_ID_CHECK, &CSignInDialog::OnCheckButtonClick)
	ON_BN_CLICKED(IDC_CREATE_ACCOUNT_BUTTON, &CSignInDialog::OnSignInClick)
	ON_EN_CHANGE(IDC_CREATE_ID, &CSignInDialog::OnEnchangeIdInput)
	ON_MESSAGE(CM_LTC_ACK_ID_AVAILABLITY, &CSignInDialog::_LTC_ACK_ID_AVAILABLITY)
END_MESSAGE_MAP()
