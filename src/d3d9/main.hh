#include "base.h"
#include "D3DWnd.h"
#include "chunkchunk.h"

EXTERN_C __declspec(dllexport) DWORD NvOptimusEnablement = 1;
#ifdef UNICODE
#define H__(a,b)  WCHAR a[sizeof(b)];
#include "szlitera.h"
#endif

static HANDLE s_hStdErr;
static HANDLE s_hStdOut;

static HINSTANCE s_hInst;
HINSTANCE const * const x_p_hInst = &s_hInst;
static HICON s_hIcon;
HICON const * const x_p_hIcon = &s_hIcon;
static HANDLE s_hHeap;
static UINT s_wmTaskbarCreated;
UINT const * const x_p_wmTaskbarCreated = &s_wmTaskbarCreated;
static HWND s_hMainWnd;
HWND const * const x_p_hMainWnd = &s_hMainWnd;
static AppCfg s_appCfg;
AppCfg const *const x_p_appCfg = &s_appCfg;
static HMODULE s_h_d3d9;
static FARPROC s_pfn_Direct3DCreate9;

static PCSTR s_AppWinErrProc;
static DWORD s_AppWinErrCode;
static int s_AppWinErrCodeTy;
enum { AppWinErrCodeTy_None = 0 };
enum { AppWinErrCodeTy_W32 = 1 };
enum { AppWinErrCodeTy_HR = 2 };
