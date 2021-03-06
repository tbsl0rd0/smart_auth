//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CCommandWindow provides a way to emulate external "connect" and "disconnect" 
// events, which are invoked via toggle button on a window. The window is launched
// and managed on a separate thread, which is necessary to ensure it gets pumped.
//

#pragma once

#include <windows.h>
#include "SmartAuthProvider.h"
#include "SerialClass.h"

class CCommandWindow
{
public:
    CCommandWindow();
    ~CCommandWindow();
    HRESULT Initialize(__in SmartAuthProvider *pProvider);
    BOOL GetConnectedStatus();

private:
    HRESULT _MyRegisterClass();
    HRESULT _InitInstance();
    BOOL _ProcessNextMessage();
	BOOL SubProc(Serial* SP);
    
    static DWORD WINAPI _ThreadProc(__in LPVOID lpParameter);
	static DWORD WINAPI _ThreadProc2(__in LPVOID lpParameter);
    static LRESULT CALLBACK    _WndProc(__in HWND hWnd, __in UINT message, __in WPARAM wParam, __in LPARAM lParam);
    
	SmartAuthProvider            *_pProvider;        // Pointer to our owner.
    HWND                        _hWnd;             // Handle to our window.
    HWND                        _hWndButton;       // Handle to our window's button.
    HINSTANCE                   _hInst;            // Current instance
    BOOL                        _fConnected;       // Whether or not we're connected.

	HANDLE hThread;

	BOOL is_donglein_used;
	CHAR donglein_key[256];
	BOOL is_smart_id_card_used;
};
