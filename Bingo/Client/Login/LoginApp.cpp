#pragma once
#include "LoginApp.h"
#include "LoginDialog.h"

BOOL CLoginApp::InitInstance()
{
	CWinApp::InitInstance();

	bool test = AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);
	std::cout.clear();

	m_net.InitNetLayer();

	CLoginDialog dlg(&m_net);
	m_pMainWnd = &dlg;

	dlg.DoModal();
	return TRUE;
}