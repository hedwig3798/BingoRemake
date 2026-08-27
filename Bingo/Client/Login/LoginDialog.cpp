#include "LoginDialog.h"
#include <iostream>
CLoginDialog::CLoginDialog(CWnd* _pParent /*= nullptr*/)
	: CDialogEx(IDD_LOGIN_DIALOG, _pParent)
	, m_net()
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
	m_net.InitNetLayer();

	return TRUE;
}

void CLoginDialog::OnBnClickedLoginButton()
{
	LOGIN_PACKET_SEND testPacket;
	testPacket.m_ID = "testID";
	testPacket.m_hashedPW = "testPW";
	m_net.Send(testPacket);
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
	ON_BN_CLICKED(IDC_LOGIN_BUTTON, &CLoginDialog::OnBnClickedLoginButton)
END_MESSAGE_MAP()
