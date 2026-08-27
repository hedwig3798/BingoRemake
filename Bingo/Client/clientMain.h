#pragma once

#include <iostream>
#include <afxwin.h>

class CLoginFrame 
	: public CFrameWnd 
{
private:
	CEdit m_idInput;

public:
	CLoginFrame();

protected:
	afx_msg int OnCreate();
};