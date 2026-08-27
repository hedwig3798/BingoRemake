#pragma once
#include "LoginApp.h"
#include "LoginDialog.h"

BOOL CLoginApp::InitInstance()
{
	CWinApp::InitInstance();

	CLoginDialog dlg;
	m_pMainWnd = &dlg;

	dlg.DoModal();
	return TRUE;
}