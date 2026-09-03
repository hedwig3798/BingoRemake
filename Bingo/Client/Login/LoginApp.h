#pragma once

#include <iostream>
#include <afxwin.h>
#include "NetLayer.h"

class CLoginApp
	: public CWinApp
{
private:
	NetLayer m_net;

public:
	virtual BOOL InitInstance() override;
};