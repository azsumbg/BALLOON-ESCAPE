#include "framework.h"
#include "BALLOON ESCAPE.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "ErrH.h"
#include "FCheck.h"
#include "D2BMPLOADER.h"
#include "gifresizer.h"
#include "balloon.h"
#include <chrono>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "gifresizer.lib")
#pragma comment(lib, "ballon.lib")

constexpr wchar_t bWinClassName[]{ L"Balloons" };
constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int no_record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int record{ 2003 };

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
HICON mainIcon{ nullptr };
HCURSOR mainCur{ nullptr };
HCURSOR outCur{ nullptr };
POINT cur_pos{};
PAINTSTRUCT bPaint{};
HDC PaintDC{ nullptr };
MSG bMsg{};
BOOL bRet{ 0 };
UINT bTimer{ 0 };

D2D1_RECT_F b1Rect(40.0f, 0, scr_width / 3.0f - 50.0f, 50.0f);
D2D1_RECT_F b2Rect(scr_width / 3.0f + 40.0f, 0, scr_width * 2 / 3.0f - 50.0f, 50.0f);
D2D1_RECT_F b3Rect(scr_width * 2 / 3.0f + 40.0f, 0, scr_width / 3.0f - 50.0f, 50.0f);

D2D1_RECT_F b1TxtRect(70.0f, 5.0f, scr_width / 3.0f - 50.0f, 50.0f);
D2D1_RECT_F b2TxtRect(scr_width / 3.0f + 70.0f, 5.0f, scr_width * 2 / 3.0f - 50.0f, 50.0f);
D2D1_RECT_F b3TxtRect(scr_width * 2 / 3.0f + 70.0f, 5.0f, scr_width / 3.0f - 50.0f, 50.0f);

bool pause = false;
bool in_client = true;
bool show_help = false;
bool sound = true;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

float scale_x{ 0 };
float scale_y{ 0 };

int level = 1;
int score = 0;
int distance = 240;

wchar_t current_player[16]{ L"TARLYO" };

dirs nature_dir = dirs::stop;

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

ID2D1SolidColorBrush* statBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmFormat{ nullptr };
IDWriteTextFormat* midFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpBannana{ nullptr };
ID2D1Bitmap* bmpCloud1{ nullptr };
ID2D1Bitmap* bmpCloud2{ nullptr };
ID2D1Bitmap* bmpField1{ nullptr };
ID2D1Bitmap* bmpField2{ nullptr };
ID2D1Bitmap* bmpFirstRecord{ nullptr };
ID2D1Bitmap* bmpNoRecord{ nullptr };
ID2D1Bitmap* bmpRecord{ nullptr };
ID2D1Bitmap* bmpLevelUp{ nullptr };
ID2D1Bitmap* bmpSun{ nullptr };
ID2D1Bitmap* bmpLogo{ nullptr };

ID2D1Bitmap* bmpIntro[78]{ nullptr };
ID2D1Bitmap* bmpBalloon[17]{ nullptr };
ID2D1Bitmap* bmpBird1L[54]{ nullptr };
ID2D1Bitmap* bmpBird1R[54]{ nullptr };
ID2D1Bitmap* bmpBird2L[8]{ nullptr };
ID2D1Bitmap* bmpBird2R[8]{ nullptr };
ID2D1Bitmap* bmpBird3L[9]{ nullptr };
ID2D1Bitmap* bmpBird3R[9]{ nullptr };
ID2D1Bitmap* bmpGorrillaL[7]{ nullptr };
ID2D1Bitmap* bmpGorrillaR[7]{ nullptr };

///////////////////////////////////////////////////////////////

std::vector<dll::FIELDS*> vFields;




/////////////////////////////////////

template<typename T>concept HasRelease = requires(T var)
{
	var.Release();
};
template<HasRelease T>bool FreeMem(T** var)
{
	if ((*var))
	{
		(*var)->Release();
		(*var) = nullptr;
		return true;
	}
	return false;
};

void LogErr(LPCWSTR what)
{
	std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
	err << what << L" Time stamp: " << std::chrono::system_clock::now() << std::endl << std::endl;
	err.close();
}
void ClearResources()
{
	if (!FreeMem(&iFactory))LogErr(L"Error releasing iFactory !");
	if (!FreeMem(&Draw))LogErr(L"Error releasing HwndRenderTarget !");

	if (!FreeMem(&b1BckgBrush))LogErr(L"Error releasing b1BckgBrush !");
	if (!FreeMem(&b2BckgBrush))LogErr(L"Error releasing b2BckgBrush !");
	if (!FreeMem(&b3BckgBrush))LogErr(L"Error releasing b3BckgBrush !");

	if (!FreeMem(&statBrush))LogErr(L"Error releasing statBrush !");
	if (!FreeMem(&txtBrush))LogErr(L"Error releasing txtBrush !");
	if (!FreeMem(&hgltBrush))LogErr(L"Error releasing hgltBrush !");
	if (!FreeMem(&inactBrush))LogErr(L"Error releasing inactBrush !");

	if (!FreeMem(&iWriteFactory))LogErr(L"Error releasing iWriteFactory !");
	if (!FreeMem(&nrmFormat))LogErr(L"Error releasing nrmFormat !");
	if (!FreeMem(&midFormat))LogErr(L"Error releasing midFormat !");
	if (!FreeMem(&bigFormat))LogErr(L"Error releasing bigFormat !");

	if (!FreeMem(&bmpBannana))LogErr(L"Error releasing bmpBannana !");
	if (!FreeMem(&bmpCloud1))LogErr(L"Error releasing bmpCloud1 !");
	if (!FreeMem(&bmpCloud2))LogErr(L"Error releasing bmpCloud2 !");
	if (!FreeMem(&bmpField1))LogErr(L"Error releasing bmpField1 !");
	if (!FreeMem(&bmpField2))LogErr(L"Error releasing bmpField2 !");
	if (!FreeMem(&bmpFirstRecord))LogErr(L"Error releasing bmpFirstRecord !");
	if (!FreeMem(&bmpNoRecord))LogErr(L"Error releasing bmpNoRecord !");
	if (!FreeMem(&bmpRecord))LogErr(L"Error releasing bmpRecord !");
	if (!FreeMem(&bmpLogo))LogErr(L"Error releasing bmpLogo !");
	if (!FreeMem(&bmpLevelUp))LogErr(L"Error releasing bmpLevelUp !");
	if (!FreeMem(&bmpSun))LogErr(L"Error releasing bmpSun !");

	for (int i = 0; i < 78; ++i)if (!FreeMem(&bmpIntro[i]))LogErr(L"Error releasing bmpIntro !");
	for (int i = 0; i < 17; ++i)if (!FreeMem(&bmpBalloon[i]))LogErr(L"Error releasing bmpBalloon !");

	for (int i = 0; i < 54; ++i)if (!FreeMem(&bmpBird1L[i]))LogErr(L"Error releasing bmpBird1L !");
	for (int i = 0; i < 54; ++i)if (!FreeMem(&bmpBird1R[i]))LogErr(L"Error releasing bmpBird1R !");

	for (int i = 0; i < 8; ++i)if (!FreeMem(&bmpBird2L[i]))LogErr(L"Error releasing bmpBird2L !");
	for (int i = 0; i < 8; ++i)if (!FreeMem(&bmpBird2R[i]))LogErr(L"Error releasing bmpBird2R !");

	for (int i = 0; i < 9; ++i)if (!FreeMem(&bmpBird3L[i]))LogErr(L"Error releasing bmpBird3L !");
	for (int i = 0; i < 9; ++i)if (!FreeMem(&bmpBird3R[i]))LogErr(L"Error releasing bmpBird3R !");

	for (int i = 0; i < 7; ++i)if (!FreeMem(&bmpGorrillaL[i]))LogErr(L"Error releasing bmpGorrillaL !");
	for (int i = 0; i < 7; ++i)if (!FreeMem(&bmpGorrillaR[i]))LogErr(L"Error releasing bmpGorrillaR !");
}
void ErrExit(int what)
{
	MessageBeep(MB_ICONERROR);
	MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

	ClearResources();
	std::remove(tmp_file);
	exit(1);
}

void GameOver()
{
	PlaySound(NULL, NULL, NULL);
	KillTimer(bHwnd, bTimer);


	bMsg.message = WM_QUIT;
	bMsg.wParam = 0;
}
void InitGame()
{
	wcscpy_s(current_player, L"TARLYO");
	name_set = false;

	level = 1;
	score = 0;
	distance = 240;

	if (!vFields.empty())
		for (int i = 0; i < vFields.size(); ++i)FreeMem(&vFields[i]);
	vFields.clear();


}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(mainIcon));
		return true;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		case IDOK:
			if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
				wcscpy_s(current_player, L"TARLYO");
				MessageBox(bHwnd, L"Ха, ха, ха ! Забави си името !", L"Забраватор", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
				EndDialog(hwnd, IDCANCEL);
				break;
			}
			EndDialog(hwnd, IDOK);
			break;
		}
		break;
	}

	return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_CREATE:
		if (bIns)
		{
			SetTimer(hwnd, bTimer, 1000, NULL);

			bBar = CreateMenu();
			bMain = CreateMenu();
			bStore = CreateMenu();

			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

			AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
			AppendMenu(bMain, MF_STRING, mLvl, L"Следващо ниво");
			AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bMain, MF_STRING, mExit, L"Изход");

			AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
			AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
			AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");

			SetMenu(hwnd, bBar);

			InitGame();
		}
		break;

	case WM_CLOSE:
		pause = true;
		if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
			L"Изход", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
		{
			pause = false;
			break;
		}
		GameOver();
		break;

	case WM_PAINT:
		PaintDC = BeginPaint(hwnd, &bPaint);
		FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(10, 10, 10)));
		EndPaint(hwnd, &bPaint);
		break;

	case WM_SETCURSOR:
		GetCursorPos(&cur_pos);
		ScreenToClient(hwnd, &cur_pos);
		if (LOWORD(lParam) == HTCLIENT)
		{
			if (!in_client)
			{
				in_client = true;
				pause = false;
			}

			if (cur_pos.x * scale_y <= 50)
			{
				if (cur_pos.x * scale_x >= b1Rect.left && cur_pos.x * scale_x <= b1Rect.right)
				{
					if (!b1Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = true;
						b2Hglt = false;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * scale_x >= b2Rect.left && cur_pos.x * scale_x <= b2Rect.right)
				{
					if (!b2Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = true;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * scale_x >= b3Rect.left && cur_pos.x * scale_x <= b3Rect.right)
				{
					if (!b3Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = false;
						b3Hglt = true;
					}
				}
				else if (b1Hglt || b2Hglt || b3Hglt)
				{
					if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
					b1Hglt = false;
					b2Hglt = false;
					b3Hglt = false;
				}
				SetCursor(outCur);
				return true;
			}
			else
			{
				if (b1Hglt || b2Hglt || b3Hglt)
				{
					if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
					b1Hglt = false;
					b2Hglt = false;
					b3Hglt = false;
				}
				SetCursor(mainCur);
				return true;
			}
		}
		else
		{
			if (in_client)
			{
				in_client = false;
				pause = true;
			}

			if (b1Hglt || b2Hglt || b3Hglt)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}

			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return true;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case mNew:
			pause = true;
			if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Ако рестартираш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
				L"Рестарт", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
			{
				pause = false;
				break;
			}
			InitGame();
			break;


		case mExit:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			break;

		}
		break;



	default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
	}

	return (LRESULT)(FALSE);
}












int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	_wsetlocale(LOCALE_ALL, L"");
	bIns = hInstance;
	if (!bIns)ErrExit(eClass);














	ClearResources();
	std::remove(tmp_file);
    
    return (int) bMsg.wParam;
}