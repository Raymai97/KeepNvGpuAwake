#pragma once
#include <stdarg.h>
#include <Windows.h>
#include <d3d9.h>
#include <shellapi.h>
#include "resource.h"

#define APP_ID  "KeepNvGpuAwake_d3d9"
#define APP_TITLE  APP_ID
#define CAT__(a,b)  a ## b
#define CAT(a,b)  CAT__(a,b)
#define SAFEFREE(p,fn)  if (p) { (fn)(p); (p) = 0; }
#define SAFERELEASE(p)  if (p) { p->lpVtbl->Release(p); (p) = 0; }

#ifdef UNICODE
#define H__(a,b)  extern WCHAR a[sizeof(b)];
#else
#define H__(a,b)  static PCSTR const a = b;
#endif
#include "szlitera.h"

extern HINSTANCE const * const x_p_hInst;
#define APP_hInst  (*x_p_hInst)
extern HICON const * const x_p_hIcon;
#define APP_hIcon  (*x_p_hIcon)
extern UINT const * const x_p_wmTaskbarCreated;
#define WM_TaskbarCreated  (*x_p_wmTaskbarCreated)
extern HWND const * const x_p_hMainWnd;
#define APP_MainWnd  (*x_p_hMainWnd)

typedef struct NotifyIconDataV1 {
	DWORD cbSize;
	HWND hWnd;
	UINT uID;
	UINT uFlags;
	UINT uCallbackMessage;
	HICON hIcon;
	TCHAR szTip[64];
} NotifyIconDataV1;

void *MemAllocZero(SIZE_T cb);
void MemFree(void *p);

void AppWinErrSetW32(PCSTR pszProc, DWORD w32err);
void AppWinErrSetHR(PCSTR pszProc, HRESULT hr);
void AppWinErrReset(void);

int App_fWrite(HANDLE hFile, void const *pBuf, int cbBuf);
int App_vfPrnf(HANDLE hFile, char const *pszFmt, va_list ap);
int App_fPrnf(HANDLE hFile, char const *pszFmt, ...);

// " 123" -> TRUE, 123
// "123" -> TRUE, 123
// "12,3" -> FALSE
// "-123" -> FALSE
// "123a" -> FALSE
BOOL App_tcs_to_UINT(LPCTSTR lpsz, UINT* pVal);

// To retrieve "lpCmdLine" of WinMain, from GetCommandLine()
LPTSTR App_lpCmdLine(void);

void PrnExoticErr_(HANDLE hFile, PCSTR pszCtx);
void PrnNow_(HANDLE hFile);
void PrnNowExoticErr(PCSTR pszCtx);
void PrnNowErr(PCSTR pszFmt, ...);
void PrnNowOut(PCSTR pszFmt, ...);
void PrnErr(char const *pszFmt, ...);
void PrnOut(char const *pszFmt, ...);

IDirect3D9 *Dywa_Direct3DCreate9(UINT sdkVer);
