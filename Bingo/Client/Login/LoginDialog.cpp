#include "LoginDialog.h"
#include "MassageDefine.h"
#include "Password.h"
#include "SignInDialog.h"
#include <iostream>
CLoginDialog::CLoginDialog(NetLayer* _net, CWnd* _pParent /*= nullptr*/)
	: CDialogEx(IDD_LOGIN_DIALOG, _pParent)
	, m_net(_net)
	, m_idInput(nullptr)
	, m_pwInput(nullptr)
	, m_loginButton(nullptr)
{
}

void CLoginDialog::DoDataExchange(CDataExchange* _pDX)
{
	CDialogEx::DoDataExchange(_pDX);
}

BOOL CLoginDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CWnd* pEdit = GetDlgItem(IDC_ID_INPUT);
	if (pEdit != nullptr)
	{
		pEdit->SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)L"ID");
	}

	m_idInput = (CEdit*)GetDlgItem(IDC_ID_INPUT);
	m_pwInput = (CEdit*)GetDlgItem(IDC_PW_INPUT);
	m_loginButton = (CButton*)GetDlgItem(IDC_LOGIN_BUTTON);

	if (nullptr != m_net)
	{
		m_net->SetDialog(this);
	}

	return TRUE;
}

void CLoginDialog::OnBnClickedLoginButton()
{
	m_loginButton->EnableWindow(FALSE);

	if (nullptr == m_idInput
		|| nullptr == m_pwInput
		|| nullptr == m_net)
	{
		std::cerr << "cannot find text box\n";
		m_loginButton->EnableWindow(TRUE);
		return;
	}

	// 답 패킷을 기다리고 있으면 무시
	if (true == m_net->IsWaitingPacket(LTC_ACK_LOGIN::PACKET_ID))
	{
		m_loginButton->EnableWindow(TRUE);
		return;
	}
	CString id;
	m_idInput->GetWindowTextA(id);

	CString pw;
	m_pwInput->GetWindowTextA(pw);

	if ("" == id
		|| "" == pw)
	{
		AfxMessageBox(_T("로그인 정보를 입력해주세요"), MB_ICONWARNING | MB_OK);
		m_loginButton->EnableWindow(TRUE);
		return;
	}

	CTL_RES_LOGIN packetData;
	packetData.m_ID = CT2CA(id);
	packetData.m_hashedPW = PW::HashSHA256S(std::string(CT2CA(pw)));


	m_net->Send(packetData);
	m_net->SetWaitPacket(LTC_ACK_LOGIN::PACKET_ID, 60);
}

void CLoginDialog::OnBnClickedCreateAccountButton()
{
	CSignInDialog dlg(m_net);
	dlg.DoModal();

	if (nullptr != m_net)
	{
		m_net->SetDialog(this);
	}
}

LRESULT CLoginDialog::_LTC_ACK_LOGIN(WPARAM _wParam, LPARAM _lParam)
{
	m_net->RelaseWaiting(LTC_ACK_LOGIN::PACKET_ID);

	LTC_ACK_LOGIN* data = (LTC_ACK_LOGIN*)_wParam;
	if (nullptr == data)
	{
		std::cerr << "data nullptr at _LTC_ACK_LOGIN\n";
		return 0;
	}

	switch (data->m_netError)
	{
	case NET_ERROR::NET_OK:
	{
		AfxMessageBox(_T("로그인 성공"), MB_ICONWARNING | MB_OK);
		break;
	}
	case NET_ERROR::NO_ID_EXITS:
	case NET_ERROR::PW_NOT_MATCH:
	{
		AfxMessageBox(_T("아이디 혹은 비밀번호가 맞지 않습니다."), MB_ICONWARNING | MB_OK);
		break;
	}
	default:
	{
		AfxMessageBox(_T("로그인에 실패했습니다."), MB_ICONWARNING | MB_OK);
		break;
	}
	}

	delete data;
	m_loginButton->EnableWindow(TRUE);

	return 0;
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
	ON_BN_CLICKED(IDC_LOGIN_BUTTON, &CLoginDialog::OnBnClickedLoginButton)
	ON_BN_CLICKED(IDC_SIGNIN_BUTTON, &CLoginDialog::OnBnClickedCreateAccountButton)
	ON_MESSAGE(CM_LTC_ACK_LOGIN, &CLoginDialog::_LTC_ACK_LOGIN)
END_MESSAGE_MAP()
