#include "LoginDialog.h"
#include "MassageDefine.h"
#include "Password.h"
#include <iostream>
CLoginDialog::CLoginDialog(CWnd* _pParent /*= nullptr*/)
	: CDialogEx(IDD_LOGIN_DIALOG, _pParent)
	, m_net()
	, m_idInput(nullptr)
	, m_pwInput(nullptr)
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

	bool test = AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);
	std::cout.clear();
	m_net.InitNetLayer(this);

	m_idInput = (CEdit*)GetDlgItem(IDC_ID_INPUT);
	m_pwInput = (CEdit*)GetDlgItem(IDC_PW_INPUT);
	m_loginButton = (CButton*)GetDlgItem(IDC_LOGIN_BUTTON);

	return TRUE;
}

void CLoginDialog::OnBnClickedLoginButton()
{

	if (nullptr == m_idInput
		|| nullptr == m_pwInput)
	{
		std::cerr << "cannot find text box\n";
		return;
	}

	// 답 패킷을 기다리고 있으면 무시
	if (true == m_net.IsWaitingPacket(LTC_ACK_LOGIN::PACKET_ID))
	{
		return;
	}
	m_loginButton->EnableWindow(FALSE);
	CString id;
	m_idInput->GetWindowTextA(id);

	CString pw;
	m_pwInput->GetWindowTextA(pw);

	if ("" == id
		|| "" == pw)
	{
		AfxMessageBox(_T("로그인 정보를 입력해주세요"), MB_ICONWARNING | MB_OK);
		return;
	}

	CTL_RES_LOGIN packetData;
	packetData.m_ID = CT2CA(id);
	packetData.m_hashedPW = PW::HashSHA256S(std::string(CT2CA(pw)));


	m_net.Send(packetData);
	m_net.SetWaitPacket(LTC_ACK_LOGIN::PACKET_ID, 60);
}

LRESULT CLoginDialog::_LTC_ACK_LOGIN(WPARAM _wParam, LPARAM _lParam)
{
	m_net.RelaseWaiting(LTC_ACK_LOGIN::PACKET_ID);

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
	ON_MESSAGE(CM_LTC_ACK_LOGIN, &CLoginDialog::_LTC_ACK_LOGIN)
END_MESSAGE_MAP()
