#include "main.hh"

void *MemAllocZero(SIZE_T cb)
{
	void *p = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, cb);
	if (!p) {
		AppWinErrSetW32("HeapAlloc", GetLastError());
	}
	return p;
}

void MemFree(void *p)
{
	HeapFree(s_hHeap, 0, p);
}

void AppWinErrSetW32(PCSTR pszProc, DWORD w32err)
{
	s_AppWinErrProc = pszProc;
	s_AppWinErrCode = w32err;
	s_AppWinErrCodeTy = AppWinErrCodeTy_W32;
}

void AppWinErrSetHR(PCSTR pszProc, HRESULT hr)
{
	s_AppWinErrProc = pszProc;
	s_AppWinErrCode = hr;
	s_AppWinErrCodeTy = AppWinErrCodeTy_HR;
}

void AppWinErrReset(void)
{
	s_AppWinErrProc = NULL;
	s_AppWinErrCode = 0;
	s_AppWinErrCodeTy = AppWinErrCodeTy_None;
}

int App_fWrite(HANDLE hFile, void const *pBuf, int cbBuf)
{
	DWORD cbWri;
	if (!WriteFile(hFile, pBuf, cbBuf, &cbWri, NULL)) {
		AppWinErrSetW32("WriteFile", GetLastError());
		return -1;
	}
	return (int)cbWri;
}

int App_vfPrnf(HANDLE hFile, char const *pszFmt, va_list ap)
{
	char szBuf[1024];
	int cbBuf = wvsprintfA(szBuf, pszFmt, ap);
	return App_fWrite(hFile, szBuf, cbBuf);
}

int App_fPrnf(HANDLE hFile, char const *pszFmt, ...)
{
	int ret = 0;
	va_list ap;
	va_start(ap, pszFmt);
	ret = App_vfPrnf(hFile, pszFmt, ap);
	va_end(ap);
	return ret;
}

BOOL App_tcs_to_UINT(LPCTSTR lpsz, UINT* pVal)
{
	BOOL ok = FALSE;
	HWND hDlg = 0;
	HWND hCtl = 0;
	hDlg = CreateWindow(TEXT("edit"), 0, WS_POPUP, 0, 0, 0, 0,
		0, 0, 0, 0);
	if (!hDlg) goto eof;
	hCtl = CreateWindow(TEXT("edit"), 0, WS_CHILD, 0, 0, 0, 0,
		hDlg, (HMENU)(WPARAM)(1234), 0, 0);
	if (!hCtl) goto eof;
	SetDlgItemText(hDlg, 1234, lpsz);
	*pVal = GetDlgItemInt(hDlg, 1234, &ok, FALSE);
eof:
	// hCtl will be destroyed when hDlg is destroyed
	if (hDlg) DestroyWindow(hDlg);
	return ok;
}

void PrnExoticErr_(HANDLE hFile, PCSTR pszCtx)
{
	if (s_AppWinErrProc) {
		if (s_AppWinErrCodeTy == AppWinErrCodeTy_W32) {
			App_fPrnf(hFile, "Win32 error %lu on %s\n", s_AppWinErrCode, s_AppWinErrProc);
		}
		if (s_AppWinErrCodeTy == AppWinErrCodeTy_HR) {
			App_fPrnf(hFile, "Error 0x%08lX on %s\n", s_AppWinErrCode, s_AppWinErrProc);
		}
		if (pszCtx) {
			App_fPrnf(hFile, "Context: %s\n", pszCtx);
		}
	}
	else if (pszCtx) {
		App_fPrnf(hFile, "%s\n", pszCtx);
	}
}

void PrnNow_(HANDLE hFile)
{
	SYSTEMTIME s;
	GetLocalTime(&s);
	App_fPrnf(hFile, "%u/%02u/%02u %02u:%02u:%02u.%03u",
		s.wYear, s.wMonth, s.wDay, s.wHour,
		s.wMinute, s.wSecond, s.wMilliseconds);
}

void PrnNowExoticErr(PCSTR pszCtx)
{
	PrnNow_(s_hStdErr);
	PrnErr(": ");
	PrnExoticErr_(s_hStdErr, pszCtx);
}

void PrnNowErr(PCSTR pszFmt, ...)
{
	va_list ap;
	va_start(ap, pszFmt);
	PrnNow_(s_hStdErr);
	PrnErr(": ");
	App_vfPrnf(s_hStdErr, pszFmt, ap);
	va_end(ap);
}

void PrnNowOut(PCSTR pszFmt, ...)
{
	va_list ap;
	va_start(ap, pszFmt);
	PrnNow_(s_hStdOut);
	PrnOut(": ");
	App_vfPrnf(s_hStdOut, pszFmt, ap);
	va_end(ap);
}

void PrnErr(char const *pszFmt, ...)
{
	va_list ap;
	va_start(ap, pszFmt);
	App_vfPrnf(s_hStdErr, pszFmt, ap);
	va_end(ap);
}

void PrnOut(char const *pszFmt, ...)
{
	va_list ap;
	va_start(ap, pszFmt);
	App_vfPrnf(s_hStdOut, pszFmt, ap);
	va_end(ap);
}

IDirect3D9 *Dywa_Direct3DCreate9(UINT sdkVer)
{
	typedef IDirect3D9 *(WINAPI *fn_t)(UINT);
	return ((fn_t)s_pfn_Direct3DCreate9)(sdkVer);
}

static BOOL My_StartWith(ChunkChunk *pCC, char const *pszKey, size_t *pcchMatch)
{
	BOOL ok = FALSE;
	size_t n = 0;
	for (;; ++n)
	{
		if (!pszKey[n]) { ok = TRUE; break; }
		if (pCC->szChunk[n] != pszKey[n]) break;
	}
	*pcchMatch = n;
	return ok;
}

static void My_ParseCommandLine(void)
{
	ChunkChunk cc;
	int iChunk;
	size_t cchMatch = 0;
	ChunkChunkInit(&cc, GetCommandLine());
	for (iChunk = 0; ChunkChunkNext(&cc); ++iChunk)
	{
		if (iChunk == 0) continue;
		if (My_StartWith(&cc, "--wake-interval=", &cchMatch))
		{
			if (!App_tcs_to_UINT(&cc.szChunk[cchMatch], &s_appCfg.wake_interval))
			{
				PrnNowExoticErr("Bad command line param for 'wake-interval'.");
			}
		}
	}
}

static int AppMain(void)
{
	HWND hD3DWnd = NULL;
#ifdef UNICODE
	{
		WCHAR *p; char const *q;
#define H__(a,b)  p = a; q = b; for (; *p = *q, *q; ++p, ++q);
#include "szlitera.h"
	}
#endif
	s_hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	s_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	PrnOut("%s [build 2022.01.20] by raymai97\n\n", APP_TITLE);

	s_hHeap = GetProcessHeap();
	if (!s_hHeap) goto eof;
	s_hInst = GetModuleHandle(NULL);
	if (!s_hInst) goto eof;

	s_appCfg.wake_interval = APPCFGDEF_WAKE_INTERVAL;
	My_ParseCommandLine();
	s_hIcon = LoadIcon(APP_hInst, (void*)IDI_ICON1);
	if (!s_hIcon) {
		AppWinErrSetW32("LoadIcon", GetLastError());
		PrnNowExoticErr("ICON1");
		goto eof;
	}
	s_wmTaskbarCreated = RegisterWindowMessage(TaskbarCreated);
	if (!s_wmTaskbarCreated) {
		AppWinErrSetW32("RegisterWindowMessage", GetLastError());
		PrnNowExoticErr("TaskbarCreated");
		goto eof;
	}

	s_hMainWnd = GetConsoleWindow();
	if (s_hMainWnd) {
		SetWindowText(s_hMainWnd, AppTitle);
		SendMessage(s_hMainWnd, WM_SETICON, 0, (LPARAM)s_hIcon);
		SendMessage(s_hMainWnd, WM_SETICON, 1, (LPARAM)s_hIcon);
	}

	s_h_d3d9 = LoadLibrary(d3d9);
	if (!s_h_d3d9) {
		AppWinErrSetW32("LoadLibrary", GetLastError());
		PrnNowExoticErr("d3d9");
		goto eof;
	}
	s_pfn_Direct3DCreate9 = GetProcAddress(s_h_d3d9, "Direct3DCreate9");
	if (!s_pfn_Direct3DCreate9) {
		AppWinErrSetW32("GetProcAddress", GetLastError());
		PrnNowExoticErr("Direct3DCreate9");
		goto eof;
	}
	if (!Create_D3DWnd(&hD3DWnd)) {
		goto eof;
	}
	Start_D3DWnd(hD3DWnd);
	while (IsWindow(hD3DWnd)) {
		MSG msg;
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
eof:
	return 0;
}

void RawMain(void)
{
	ExitProcess(AppMain());
}

int main(void)
{
	return AppMain();
}
