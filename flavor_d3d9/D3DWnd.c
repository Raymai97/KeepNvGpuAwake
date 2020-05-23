#include "D3DWnd.hh"

BOOL Create_D3DWnd(HWND *phWnd)
{
	BOOL ok = FALSE;
	WNDCLASS wc;
	DWORD errRegisterClass = 0;
	HWND hWnd = NULL;
	MySelf *pSelf = NULL;
	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = D3DWnd_WndProc;
	wc.lpszClassName = D3DWnd_Class;
	wc.cbWndExtra = sizeof(void *);
	if (!RegisterClass(&wc)) {
		errRegisterClass = GetLastError();
	}
	hWnd = CreateWindowEx(0, D3DWnd_Class, D3DWnd_Class, WS_POPUP, 0, 0, 1, 1, NULL, NULL, APP_hInst, NULL);
	if (!hWnd) {
		if (errRegisterClass) {
			AppWinErrSetW32("RegisterClass", errRegisterClass);
		}
		else {
			AppWinErrSetW32("CreateWindowEx", GetLastError());
		}
		PrnNowExoticErr("D3DWnd");
		goto eof;
	}
	ok = D3DWnd_NewSelf(&pSelf, hWnd);
	if (!ok) goto eof;
	ok = TRUE; {
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)pSelf);
		*phWnd = hWnd;
	}
eof:
	if (!ok) {
		SAFEFREE(pSelf, D3DWnd_DelSelf);
		SAFEFREE(hWnd, DestroyWindow);
	}
	return ok;
}

BOOL Start_D3DWnd(HWND hWnd)
{
	MySelf *pSelf = D3DWnd_GetSelf(hWnd);
	D3DWnd_AddNotiIcon(pSelf);
	return D3DWnd_Step(pSelf);
}

static void D3DWnd_DelSelf(MySelf *pSelf)
{
	SAFERELEASE(pSelf->pDevice);
	SAFERELEASE(pSelf->pD3D);
	pSelf->hWnd = NULL;
	MemFree(pSelf);
}

static BOOL D3DWnd_NewSelf(MySelf **ppSelf, HWND hWnd)
{
	BOOL ok = FALSE;
	MySelf *pSelf = NULL;
	pSelf = MemAllocZero(sizeof(*pSelf));
	if (!pSelf) {
		PrnNowExoticErr(NULL);
		goto eof;
	}
	pSelf->hWnd = hWnd;
	{
		NotifyIconDataV1 *p = &pSelf->notifyIconData;
		p->cbSize = sizeof(*p);
		p->hWnd = pSelf->hWnd;
		p->uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		p->hIcon = APP_hIcon;
		lstrcpy(p->szTip, AppTitle);
		p->uCallbackMessage = NotiIconMsg;
	}
	ok = TRUE; {
		*ppSelf = pSelf;
	}
eof:
	if (!ok) {
		SAFEFREE(pSelf, D3DWnd_DelSelf);
	}
	return ok;
}

static MySelf *D3DWnd_GetSelf(HWND hWnd)
{
	return (MySelf *)GetWindowLongPtr(hWnd, 0);
}

static BOOL D3DWnd_AddNotiIcon(MySelf *pSelf)
{
	NotifyIconDataV1 *p = &pSelf->notifyIconData;
	return Shell_NotifyIcon(NIM_ADD, (NOTIFYICONDATA*)p);
}

static void D3DWnd_DelNotiIcon(MySelf *pSelf)
{
	NotifyIconDataV1 *p = &pSelf->notifyIconData; 
	Shell_NotifyIcon(NIM_DELETE, (NOTIFYICONDATA*)p);
}

static BOOL D3DWnd_CreateD3D(MySelf *pSelf)
{
	BOOL ok = FALSE;
	HRESULT hr = 0;
	D3DADAPTER_IDENTIFIER9 id;
	PrnNowOut("Direct3DCreate9...\n");
	pSelf->pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pSelf->pD3D) {
		PrnNowExoticErr("Direct3DCreate9 returned NULL");
		goto eof;
	}
	PrnNowOut("GetAdapterIdentifier...\n");
	hr = pSelf->pD3D->lpVtbl->GetAdapterIdentifier(pSelf->pD3D,
		D3DADAPTER_DEFAULT, 0, &id);
	if (FAILED(hr)) {
		AppWinErrSetHR("IDirect3D9::GetAdapterIdentifier", hr);
		PrnNowExoticErr(NULL);
		goto eof;
	}
	PrnOut("|- Description: %s\n", id.Description);
	if (memcmp(id.Description, "NVIDIA", 6) != 0) {
		PrnOut(" |- Error: I want NVIDIA only!\n");
		goto eof;
	}
	ZeroMemory(&pSelf->d3dpp, sizeof(pSelf->d3dpp));
	pSelf->d3dpp.Windowed = TRUE;
	pSelf->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	PrnNowOut("CreateDevice...\n");
	hr = pSelf->pD3D->lpVtbl->CreateDevice(pSelf->pD3D,
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, pSelf->hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&pSelf->d3dpp, &pSelf->pDevice);
	if (FAILED(hr)) {
		AppWinErrSetHR("IDirect3D9::CreateDevice", hr);
		PrnNowExoticErr(NULL);
		goto eof;
	}
	ok = TRUE;
eof:
	if (!ok) {
		SAFERELEASE(pSelf->pDevice);
		SAFERELEASE(pSelf->pD3D);
	}
	return ok;
}

static BOOL D3DWnd_Step(MySelf *pSelf)
{
	BOOL ok = FALSE;
	HRESULT d3derr = 0;
	AppWinErrReset();
	if (!pSelf->pDevice) {
		ok = D3DWnd_CreateD3D(pSelf) && pSelf->pDevice;
		if (!ok) {
			SetTimer(pSelf->hWnd, TimerID_Step, StepDelay_CreateD3D, NULL);
			goto eof;
		}
		SetTimer(pSelf->hWnd, TimerID_Step, StepDelay_ResetPresent, NULL);
	}
	PrnNowOut("Reset... ");
	d3derr = pSelf->pDevice->lpVtbl->Reset(pSelf->pDevice, &pSelf->d3dpp);
	if (FAILED(d3derr)) {
		PrnOut("\n");
		goto eof;
	}
	PrnOut("Present...\n");
	d3derr = pSelf->pDevice->lpVtbl->Present(pSelf->pDevice, NULL, NULL, NULL, NULL);
	if (FAILED(d3derr)) {
		goto eof;
	}
	ok = TRUE;
eof:
	if (d3derr == D3DERR_DEVICELOST) {
		PrnOut("|- D3DERR_DEVICELOST : Device lost \n");
		SAFERELEASE(pSelf->pDevice);
		SAFERELEASE(pSelf->pD3D);
		SetTimer(pSelf->hWnd, TimerID_Step, StepDelay_CreateD3D, NULL);
	}
	else if (FAILED(d3derr)) {
		PrnOut("|- D3DERR HRESULT 0x%08lX \n", d3derr);
	}
	return ok;
}

static LRESULT CALLBACK D3DWnd_WndProc(HWND hWnd, UINT msg, WPARAM w, LPARAM l)
{
	LRESULT lRes = 0;
	BOOL overrid = FALSE;
	switch (msg) {
	case WM_NCDESTROY:
		On_D3DWnd_NcDestroy(hWnd);
		break;
	case WM_COMMAND:
		On_D3DWnd_Command(hWnd, LOWORD(w));
		break;
	case WM_TIMER:
		On_D3DWnd_Timer(hWnd, (UINT)w);
		break;
	case NotiIconMsg:
		switch ((UINT)l) {
		case WM_LBUTTONUP:
			On_D3DWnd_NotiIcon_LButtonUp(hWnd);
			break;
		case WM_RBUTTONUP:
			On_D3DWnd_NotiIcon_RButtonUp(hWnd);
			break;
		}
		break;
	default:
		if (msg == WM_TaskbarCreated) {
			On_D3DWnd_TaskbarCreated(hWnd);
		}
		break;
	}
	if (!overrid) {
		lRes = DefWindowProc(hWnd, msg, w, l);
	}
	return lRes;
}

static void On_D3DWnd_NcDestroy(HWND hWnd)
{
	MySelf *pSelf = D3DWnd_GetSelf(hWnd);
	D3DWnd_DelNotiIcon(pSelf);
	D3DWnd_DelSelf(pSelf);
}

static void On_D3DWnd_Command(HWND hWnd, UINT cmdId)
{
	if (cmdId == CmdId_Exit) {
		SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
}

static void On_D3DWnd_Timer(HWND hWnd, UINT timerId)
{
	if (timerId == TimerID_Step) {
		D3DWnd_Step(D3DWnd_GetSelf(hWnd));
	}
}

static void On_D3DWnd_TaskbarCreated(HWND hWnd)
{
	MySelf *pSelf = D3DWnd_GetSelf(hWnd);
	D3DWnd_AddNotiIcon(pSelf);
}

static void On_D3DWnd_NotiIcon_LButtonUp(HWND hWnd)
{
	hWnd = APP_MainWnd;
	if (IsWindowVisible(hWnd)) {
		ShowWindow(hWnd, HIDE_WINDOW);
	}
	else {
		ShowWindow(hWnd, SHOW_OPENWINDOW);
		SetForegroundWindow(hWnd);
	}
}

static void On_D3DWnd_NotiIcon_RButtonUp(HWND hWnd)
{
	POINT m;
	if (GetCursorPos(&m)) {
		HMENU hMenu = CreatePopupMenu();
		if (hMenu) {
			ChangeMenu(hMenu, 0, AppTitle, 0, MF_APPEND | MFS_DISABLED);
			ChangeMenu(hMenu, 0, NULL, 0, MF_APPEND | MF_SEPARATOR);
			ChangeMenu(hMenu, 0, CmdSz_Exit, CmdId_Exit, MF_APPEND | MF_BYCOMMAND);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, m.x, m.y, 0, hWnd, NULL);
		}
	}
}
