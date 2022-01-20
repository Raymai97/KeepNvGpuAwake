#include "D3DWnd.h"

#define Direct3DCreate9  Dywa_Direct3DCreate9
#define MySelf  D3DWnd_Self

typedef struct MySelf {
	HWND hWnd;
	IDirect3D9 *pD3D;
	IDirect3DDevice9 *pDevice;
	D3DPRESENT_PARAMETERS d3dpp;
	NotifyIconDataV1 notifyIconData;
} MySelf;

enum { CmdId_Exit = 111 };

enum { NotiIconMsg = 101 };

enum { TimerID_Step = 100 };
enum { StepDelay_CreateD3D = 1000 };
#define StepDelay_ResetPresent  (APP_Cfg.wake_interval)

static void D3DWnd_DelSelf(MySelf *pSelf);
static BOOL D3DWnd_NewSelf(MySelf **ppSelf, HWND hWnd);
static MySelf *D3DWnd_GetSelf(HWND hWnd);

static BOOL D3DWnd_AddNotiIcon(MySelf *pSelf);
static void D3DWnd_DelNotiIcon(MySelf *pSelf);

static BOOL D3DWnd_CreateD3D(MySelf *pSelf);
static BOOL D3DWnd_Step(MySelf *pSelf);

static LRESULT CALLBACK D3DWnd_WndProc(HWND hWnd, UINT msg, WPARAM w, LPARAM l);
static void On_D3DWnd_NcDestroy(HWND hWnd);
static void On_D3DWnd_Command(HWND hWnd, UINT cmdId);
static void On_D3DWnd_Timer(HWND hWnd, UINT timerId);
static void On_D3DWnd_TaskbarCreated(HWND hWnd);
static void On_D3DWnd_NotiIcon_LButtonUp(HWND hWnd);
static void On_D3DWnd_NotiIcon_RButtonUp(HWND hWnd);
