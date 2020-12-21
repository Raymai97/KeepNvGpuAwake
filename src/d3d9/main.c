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
	if (!s_hStdErr) goto eof;
	s_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (!s_hStdOut) goto eof;
	PrnOut("%s [build 2020.12.21] by raymai97\n\n", APP_TITLE);

	s_hHeap = GetProcessHeap();
	if (!s_hHeap) goto eof;
	s_hInst = GetModuleHandle(NULL);
	if (!s_hInst) goto eof;

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
	if (!s_hMainWnd) {
		AppWinErrSetW32("GetConsoleWindow", GetLastError());
		PrnNowExoticErr(NULL);
		goto eof;
	}
	SetWindowText(s_hMainWnd, AppTitle);
	SendMessage(s_hMainWnd, WM_SETICON, 0, (LPARAM)s_hIcon);
	SendMessage(s_hMainWnd, WM_SETICON, 1, (LPARAM)s_hIcon);

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
