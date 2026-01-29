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
#pragma comment(lib, "balloon.lib")

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
D2D1_RECT_F b3Rect(scr_width * 2 / 3.0f + 40.0f, 0, scr_width - 50.0f, 50.0f);

D2D1_RECT_F b1TxtRect(70.0f, 5.0f, scr_width / 3.0f - 50.0f, 50.0f);
D2D1_RECT_F b2TxtRect(scr_width / 3.0f + 70.0f, 5.0f, scr_width * 2 / 3.0f - 50.0f, 50.0f);
D2D1_RECT_F b3TxtRect(scr_width * 2 / 3.0f + 70.0f, 5.0f, scr_width  - 50.0f, 50.0f);

bool pause = false;
bool in_client = true;
bool show_help = false;
bool sound = true;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;
bool level_skipped = false;

bool need_right_field = false;
bool need_left_field = false;

float scale_x{ 0 };
float scale_y{ 0 };

int level = 1;
int score = 0;
float distance = 240;

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
ID2D1Bitmap* bmpCrashed{ nullptr };

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

dll::RANDIT RandIt{};

std::vector<dll::FIELDS*> vFields;
std::vector<dll::FIELDS*> vSkies;

dll::BALLOON* Balloon{ nullptr };
bool killed = false;
dll::PROTON* FallingBalloon{nullptr};

std::vector<dll::EVILS*> vBirds;
std::vector<dll::EVILS*> vGorillas;

std::vector<dll::BANNANA*> vBannanas;

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

int GetIntroFrame()
{
	static int frame{ 0 };
	static int frame_delay{ 2 };

	--frame_delay;
	if (frame_delay <= 0)
	{
		frame_delay = 2;
		++frame;
		if (frame > 77)frame = 0;
	}
	return frame;
}

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
	if (!FreeMem(&bmpCrashed))LogErr(L"Error releasing bmpCrashed !");

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
BOOL CheckRecord()
{
	if (score < 1)return no_record;
	int result{ 0 };

	CheckFile(record_file, &result);
	if (result == FILE_NOT_EXIST)
	{
		std::wofstream rec(record_file);
		rec << score << std::endl;
		for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
		rec.close();
		return first_record;
	}
	else
	{
		std::wifstream check(record_file);
		check >> result;
		check.close();
	}

	if (result < score)
	{
		std::wofstream rec(record_file);
		rec << score << std::endl;
		for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
		rec.close();
		return record;
	}

	return no_record;
}
void GameOver()
{
	PlaySound(NULL, NULL, NULL);
	KillTimer(bHwnd, bTimer);

	switch (CheckRecord())
	{
	case no_record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpNoRecord, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_SYNC);
		else Sleep(2500);
		break;

	case first_record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpFirstRecord, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
		else Sleep(2500);
		break;

	case record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpRecord, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
		else Sleep(2500);
		break;
	}


	bMsg.message = WM_QUIT;
	bMsg.wParam = 0;
}
void ShowRecord()
{
	int result{ 0 };
	CheckFile(record_file, &result);
	if (result == FILE_NOT_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Все още няма рекорд на играта !\n\nПостарай се повече !", L"Липсва файл",
			MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
		return;
	}

	wchar_t rec_txt[100]{ L"НАЙ-ДОБЪР КАПИТАН: " };
	wchar_t saved_player[16]{ L"\0" };
	wchar_t add[5]{ L"\0" };

	std::wifstream rec{ record_file };

	rec >> result;
	swprintf_s(add, 5, L"%d", result);
	
	result = 0;

	for (int i = 0; i < 16; ++i)
	{
		int letter = 0;
		rec >> letter;

		saved_player[i] = static_cast<wchar_t>(letter);
	}
	rec.close();

	wcscat_s(rec_txt, saved_player);
	wcscat_s(rec_txt, L"\n\nСВЕТОВЕН РЕКОРД: ");
	wcscat_s(rec_txt, add);

	for (int i = 0; i < 100; ++i)
	{
		if (rec_txt[i] != '\0')++result;
		else break;
	}

	if (sound)mciSendString(L"play .\\res\\snd\\showrec.wav", NULL, NULL, NULL);

	if (bigFormat && hgltBrush)
	{
		Draw->BeginDraw();
		Draw->Clear(D2D1::ColorF(D2D1::ColorF::AliceBlue));
		Draw->DrawTextW(rec_txt, result, bigFormat, D2D1::RectF(10.0f, 80.0f, scr_width, scr_height), hgltBrush);
		Draw->EndDraw();
	}
	Sleep(4000);
}
void InitGame()
{
	wcscpy_s(current_player, L"TARLYO");
	name_set = false;

	killed = false;
	FreeMem(&FallingBalloon);

	level = 1;
	score = 0;
	distance = 80.0f;
	level_skipped = false;

	need_right_field = false;
	need_left_field = false;

	if (!vFields.empty())
		for (int i = 0; i < vFields.size(); ++i)FreeMem(&vFields[i]);
	vFields.clear();
	for (float x = -scr_width; x < 2 * scr_width; x += scr_width)
		vFields.push_back(dll::FIELDS::create(static_cast<nature>(RandIt(3, 4)), x, 50.0f));

	if (!vSkies.empty())
		for (int i = 0; i < vSkies.size(); ++i)FreeMem(&vSkies[i]);
	vSkies.clear();
	vSkies.push_back(dll::FIELDS::create(nature::sun, scr_width - 100.0f, 60.0f));

	if (!vBirds.empty())
		for (int i = 0; i < vBirds.size(); ++i)FreeMem(&vBirds[i]);
	vBirds.clear();

	if (!vGorillas.empty())
		for (int i = 0; i < vGorillas.size(); ++i)FreeMem(&vGorillas[i]);
	vGorillas.clear();

	if (!vBannanas.empty())
		for (int i = 0; i < vBannanas.size(); ++i)FreeMem(&vBannanas[i]);
	vBannanas.clear();

	FreeMem(&Balloon);
	Balloon = dll::BALLOON::create(10.0f, scr_height / 2.0f);

}
void LevelUp()
{
	if (!level_skipped)score += 100 * level;
	level_skipped = false;
	
	Draw->BeginDraw();
	Draw->DrawBitmap(bmpLevelUp, D2D1::RectF(0, 0, scr_width, scr_height));
	Draw->EndDraw();

	if (sound)
	{
		PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
		Sleep(2000);
	}
	else Sleep(3500);

	++level;

	distance = 80.0f + 10.0f * (float)(level);

	need_right_field = false;
	need_left_field = false;

	if (!vFields.empty())
		for (int i = 0; i < vFields.size(); ++i)FreeMem(&vFields[i]);
	vFields.clear();
	for (float x = -scr_width; x < 2 * scr_width; x += scr_width)
		vFields.push_back(dll::FIELDS::create(static_cast<nature>(RandIt(3, 4)), x, 50.0f));

	if (!vSkies.empty())
		for (int i = 0; i < vSkies.size(); ++i)FreeMem(&vSkies[i]);
	vSkies.clear();
	vSkies.push_back(dll::FIELDS::create(nature::sun, scr_width - 100.0f, 60.0f));

	if (!vBirds.empty())
		for (int i = 0; i < vBirds.size(); ++i)FreeMem(&vBirds[i]);
	vBirds.clear();

	if (!vGorillas.empty())
		for (int i = 0; i < vGorillas.size(); ++i)FreeMem(&vGorillas[i]);
	vGorillas.clear();

	if (!vBannanas.empty())
		for (int i = 0; i < vBannanas.size(); ++i)FreeMem(&vBannanas[i]);
	vBannanas.clear();

	FreeMem(&Balloon);
	Balloon = dll::BALLOON::create(10.0f, scr_height / 2.0f);
}
void SaveGame()
{
	int result{ 0 };
	CheckFile(save_file, &result);
	if (result == FILE_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(bHwnd, L"Има предишна записана игра, която ще загубиш !\n\nНаистина ли я презаписваш ?",
			L"Презапис", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
	}

	result = 0;

	std::wofstream save(save_file);

	save << killed << std::endl;
	save << level << std::endl;
	save << score << std::endl;
	save << distance << std::endl;
	save << level_skipped << std::endl;

	for (int i = 0; i < 16; ++i)save << static_cast<int>(current_player[i]) << std::endl;
	save << name_set << std::endl;

	save << Balloon->start.x << std::endl;
	save << Balloon->start.y << std::endl;

	save << vGorillas.size() << std::endl;
	if (vGorillas.size() > 0)
	{
		for (int i = 0; i < vGorillas.size(); ++i)
		{
			save << vGorillas[i]->start.x << std::endl;
			save << vGorillas[i]->start.y << std::endl;
		}
	}

	save << vBirds.size() << std::endl;
	if (vBirds.size() > 0)
	{
		for (int i = 0; i < vBirds.size(); ++i)
		{
			save << static_cast<int>(vBirds[i]->get_type()) << std::endl;
			save << vBirds[i]->start.x << std::endl;
			save << vBirds[i]->start.y << std::endl;
		}
	}

	save.close();

	if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);

	MessageBox(bHwnd, L"Играта е запазена !", L"Запис !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
	int result{ 0 };
	CheckFile(save_file, &result);

	if (result == FILE_NOT_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Все още няма записана игра !\n\nПостарай се повече !", L"Липсва файл",
			MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
		return;
	}
	else
	{
		if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(bHwnd, L"Ако продължиш, ще загубиш тази игра !\n\nНаистина ли зареждаш записаната игра ?",
			L"Зареждане", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
	}

	if (!vFields.empty())
		for (int i = 0; i < vFields.size(); ++i)FreeMem(&vFields[i]);
	vFields.clear();
	for (float x = -scr_width; x < 2 * scr_width; x += scr_width)
		vFields.push_back(dll::FIELDS::create(static_cast<nature>(RandIt(3, 4)), x, 50.0f));

	if (!vSkies.empty())
		for (int i = 0; i < vSkies.size(); ++i)FreeMem(&vSkies[i]);
	vSkies.clear();
	vSkies.push_back(dll::FIELDS::create(nature::sun, scr_width - 100.0f, 60.0f));

	if (!vBirds.empty())
		for (int i = 0; i < vBirds.size(); ++i)FreeMem(&vBirds[i]);
	vBirds.clear();

	if (!vGorillas.empty())
		for (int i = 0; i < vGorillas.size(); ++i)FreeMem(&vGorillas[i]);
	vGorillas.clear();

	if (!vBannanas.empty())
		for (int i = 0; i < vBannanas.size(); ++i)FreeMem(&vBannanas[i]);
	vBannanas.clear();

	FreeMem(&Balloon);
	Balloon = dll::BALLOON::create(10.0f, scr_height / 2.0f);

	std::wifstream save(save_file);

	save >> killed;
	if (killed)GameOver();

	save >> level;
	save >> score;
	save >> distance;
	save >> level_skipped;

	for (int i = 0; i < 16; ++i)
	{
		int letter = 0;

		save >> letter;
		current_player[i] = static_cast<wchar_t>(letter);
	}
	save >> name_set;

	save >> Balloon->start.x;
	save >> Balloon->start.y;

	save >> result;
	if (result > 0)
	{
		for (int i = 0; i < result; ++i)
		{
			float sx{ 0 };
			float sy{ 0 };

			save >> sx;
			save >> sy;

			vGorillas.push_back(dll::GORRILLA::create(evils::gorilla, sx, sy));
		}
	}

	save >> result;
	if (result > 0)
	{
		for (int i = 0; i < result; ++i)
		{
			float sx{ 0 };
			float sy{ 0 };
			int ttype{ -1 };

			save >> ttype;
			save >> sx;
			save >> sy;

			switch (ttype)
			{
			case 0:
				vBirds.push_back(dll::BIRD1::create(evils::bird1, sx, sy));
				break;

			case 1:
				vBirds.push_back(dll::BIRD2::create(evils::bird1, sx, sy));
				break;

			case 2:
				vBirds.push_back(dll::BIRD3::create(evils::bird1, sx, sy));
				break;
			}
		}
	}

	save.close();

	if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);

	MessageBox(bHwnd, L"Играта е заредена !", L"Зареждане !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
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

			if (cur_pos.y * scale_y <= 50)
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

		case mLvl:
			pause = true;
			if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Ако прескочиш нивото, ще загубиш бонуса !\n\nНаистина ли прескачаш нивото?",
				L"Следващо ниво", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
			{
				pause = false;
				break;
			}
			level_skipped = true;
			LevelUp();
			break;

		case mExit:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			break;

		case mSave:
			pause = true;
			SaveGame();
			pause = false;
			break;

		case mLoad:
			pause = true;
			LoadGame();
			pause = false;
			break;

		case mHoF:
			pause = true;
			ShowRecord();
			pause = false;
			break;
		}
		break;

	case WM_KEYDOWN:
		if (pause || !Balloon)break;
		switch (LOWORD(wParam))
		{
		case VK_LEFT:
			Balloon->dir = dirs::left;
			nature_dir = dirs::right;
			break;

		case VK_RIGHT:
			Balloon->dir = dirs::right;
			nature_dir = dirs::left;
			break;

		case VK_UP:
			Balloon->dir = dirs::up;
			break;

		case VK_DOWN:
			Balloon->dir = dirs::down;
			break;

		case VK_SPACE:
			Balloon->dir = dirs::stop;
			nature_dir = dirs::stop;
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		if (HIWORD(lParam) * scale_y <= 50)
		{
			if (LOWORD(lParam) * scale_x >= b1Rect.left && LOWORD(lParam) * scale_x <= b1Rect.right)
			{
				if (name_set)
				{
					if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
					break;
				}
				else if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
				if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
				break;
			}
			if (LOWORD(lParam) * scale_x >= b2Rect.left && LOWORD(lParam) * scale_x <= b2Rect.right)
			{
				mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
				if (sound)
				{
					sound = false;
					PlaySound(NULL, NULL, NULL);
					break;
				}
				else
				{
					sound = true;
					PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
					break;
				}
			}

		}
		break;

	case WM_TIMER:
		if (pause)break;
		if (Balloon)
		{
			if (Balloon->dir == dirs::left && Balloon->start.x <= scr_width / 2.0f)
			{
				distance += 0.1f;
				score -= 10;
				if (score < 0)score = 0;
			}
			else if (Balloon->dir == dirs::right || Balloon->dir == dirs::up || Balloon->dir == dirs::down)
			{
				if (Balloon->start.x >= scr_width / 2.0f)
				{
					distance -= 0.2f;
					score += 2;
				}
				else
				{
					score += 1;
					distance -= 0.1f;
				}
			}
			if (Balloon->lifes + 5 <= 100)Balloon->lifes += 5;
		}
		if (distance <= 0)LevelUp();
		break;

	default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
	}

	return (LRESULT)(FALSE);
}

void CreateResources()
{
	int result{ 0 };
	CheckFile(Ltmp_file, &result);
	if (result == FILE_EXIST)ErrExit(eStarted);
	else
	{
		std::wofstream start(Ltmp_file);
		start << L"Game started at: " << std::chrono::system_clock::now();
		start.close();
	}

	int winx = (int)(GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2.0f));

	if (winx + (int)(scr_width) > GetSystemMetrics(SM_CXSCREEN) || 10 + (int)(scr_height) > GetSystemMetrics(SM_CYSCREEN))
		ErrExit(eScreen);

	mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
	if (!mainIcon)ErrExit(eIcon);
	mainCur = LoadCursorFromFileW(L".\\res\\main.ani");
	outCur = LoadCursorFromFileW(L".\\res\\out.ani");
	if (!mainCur || !outCur)ErrExit(eCursor);

	bWinClass.lpszClassName = bWinClassName;
	bWinClass.hInstance = bIns;
	bWinClass.lpfnWndProc = &WinProc;
	bWinClass.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
	bWinClass.hIcon = mainIcon;
	bWinClass.hCursor = mainCur;
	bWinClass.style = CS_DROPSHADOW;

	if (!RegisterClass(&bWinClass))ErrExit(eClass);

	bHwnd = CreateWindow(bWinClassName, L"BALLOON ESCAPE", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, winx, 10,
		(int)(scr_width), (int)(scr_height), NULL, NULL, bIns, NULL);

	if (!bHwnd)ErrExit(eWindow);
	else
	{
		ShowWindow(bHwnd, SW_SHOWDEFAULT);

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
		if (hr != S_OK)
		{
			LogErr(L"Error creating D2D1 Factory !");
			ErrExit(eD2D);
		}

		if (iFactory)
		{
			hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
				D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 HwndRenderTarget !");
				ErrExit(eD2D);
			}

			if (Draw)
			{
				RECT ClR{};
				GetClientRect(bHwnd, &ClR);

				D2D1_SIZE_F hwndR{ Draw->GetSize() };

				scale_x = hwndR.width / (ClR.right - ClR.left);
				scale_y = hwndR.height / (ClR.bottom - ClR.top);

				D2D1_GRADIENT_STOP gStops[2]{};
				ID2D1GradientStopCollection* gColl{ nullptr };

				gStops[0].position = 0;
				gStops[0].color = D2D1::ColorF(D2D1::ColorF::Chocolate);
				gStops[1].position = 1.0f;
				gStops[1].color = D2D1::ColorF(D2D1::ColorF::Maroon);

				hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
				if (hr != S_OK)
				{
					LogErr(L"Error creating D2D1 GradientStopCollection !");
					ErrExit(eD2D);
				}
				if (gColl)
				{
					hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left +
						(b1Rect.right - b1Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0), 
						(b1Rect.right - b1Rect.left) / 2.0f, 25.0f), gColl, &b1BckgBrush);
					hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
						(b2Rect.right - b2Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0),
						(b2Rect.right - b2Rect.left) / 2.0f, 25.0f), gColl, &b2BckgBrush);
					hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
						(b3Rect.right - b3Rect.left) / 2.0f, 25.0f), D2D1::Point2F(0, 0),
						(b3Rect.right - b3Rect.left) / 2.0f, 25.0f), gColl, &b3BckgBrush);

					if (hr != S_OK)
					{
						LogErr(L"Error creating D2D1 Buttons Bckg Brushes !");
						ErrExit(eD2D);
					}
					FreeMem(&gColl);
				}

				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MediumSpringGreen), &statBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lime), &txtBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gold), &hgltBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue), &inactBrush);

				if (hr != S_OK)
				{
					LogErr(L"Error creating D2D1 SolidColorBrushes !");
					ErrExit(eD2D);
				}

				bmpBannana = Load(L".\\res\\img\\banana.png", Draw);
				if (!bmpBannana)
				{
					LogErr(L"Error loading bmpBannana");
					ErrExit(eD2D);
				}
				bmpCloud1 = Load(L".\\res\\img\\Cloud1.png", Draw);
				if (!bmpCloud1)
				{
					LogErr(L"Error loading bmpCloud1");
					ErrExit(eD2D);
				}
				bmpCloud2 = Load(L".\\res\\img\\Cloud2.png", Draw);
				if (!bmpCloud2)
				{
					LogErr(L"Error loading bmpCloud2");
					ErrExit(eD2D);
				}
				bmpField1 = Load(L".\\res\\img\\Field1.png", Draw);
				if (!bmpField1)
				{
					LogErr(L"Error loading bmpField1");
					ErrExit(eD2D);
				}
				bmpField2 = Load(L".\\res\\img\\Field2.png", Draw);
				if (!bmpField2)
				{
					LogErr(L"Error loading bmpField2");
					ErrExit(eD2D);
				}
				bmpSun = Load(L".\\res\\img\\Sun.png", Draw);
				if (!bmpSun)
				{
					LogErr(L"Error loading bmpSun");
					ErrExit(eD2D);
				}
				bmpCrashed = Load(L".\\res\\img\\crash.png", Draw);
				if (!bmpCrashed)
				{
					LogErr(L"Error loading bmpCrashed");
					ErrExit(eD2D);
				}
				bmpLogo = Load(L".\\res\\img\\Logo.png", Draw);
				if (!bmpLogo)
				{
					LogErr(L"Error loading bmpLogo");
					ErrExit(eD2D);
				}
				bmpLevelUp = Load(L".\\res\\img\\LevelUp.png", Draw);
				if (!bmpLevelUp)
				{
					LogErr(L"Error loading bmpLevelUp");
					ErrExit(eD2D);
				}
				bmpFirstRecord = Load(L".\\res\\img\\FirstRecord.png", Draw);
				if (!bmpFirstRecord)
				{
					LogErr(L"Error loading bmpFirstRecord");
					ErrExit(eD2D);
				}
				bmpNoRecord = Load(L".\\res\\img\\NoRecord.png", Draw);
				if (!bmpNoRecord)
				{
					LogErr(L"Error loading bmpNoRecord");
					ErrExit(eD2D);
				}
				bmpRecord = Load(L".\\res\\img\\Record.png", Draw);
				if (!bmpRecord)
				{
					LogErr(L"Error loading bmpRecord");
					ErrExit(eD2D);
				}

				for (int i = 0; i < 78; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\intro\\0" };
					wchar_t add[5]{ L"\0" };

					if (i < 10)wcscat_s(name, L"0");
					
					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpIntro[i] = Load(name, Draw);

					if (!bmpIntro[i])
					{
						LogErr(L"Error loading bmpIntro");
						ErrExit(eD2D);
					}
				}
				for (int i = 0; i < 17; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\balloon\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpBalloon[i] = Load(name, Draw);

					if (!bmpBalloon[i])
					{
						LogErr(L"Error loading bmpBalloon");
						ErrExit(eD2D);
					}
				}

				for (int i = 0; i < 54; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\bird1\\l\\0" };
					wchar_t add[5]{ L"\0" };

					if (i < 10)wcscat_s(name, L"0");

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpBird1L[i] = Load(name, Draw);

					if (!bmpBird1L[i])
					{
						LogErr(L"Error loading bmpBird1L");
						ErrExit(eD2D);
					}
				}
				for (int i = 0; i < 54; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\bird1\\r\\0" };
					wchar_t add[5]{ L"\0" };

					if (i < 10)wcscat_s(name, L"0");

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpBird1R[i] = Load(name, Draw);

					if (!bmpBird1R[i])
					{
						LogErr(L"Error loading bmpBird1R");
						ErrExit(eD2D);
					}
				}

				for (int i = 0; i < 8; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\bird2\\l\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpBird2L[i] = Load(name, Draw);

					if (!bmpBird2L[i])
					{
						LogErr(L"Error loading bmpBird2L");
						ErrExit(eD2D);
					}
				}
				for (int i = 0; i < 8; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\bird2\\r\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpBird2R[i] = Load(name, Draw);

					if (!bmpBird2R[i])
					{
						LogErr(L"Error loading bmpBird2R");
						ErrExit(eD2D);
					}
				}

				for (int i = 0; i < 9; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\bird3\\l\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpBird3L[i] = Load(name, Draw);

					if (!bmpBird3L[i])
					{
						LogErr(L"Error loading bmpBird3L");
						ErrExit(eD2D);
					}
				}
				for (int i = 0; i < 9; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\bird3\\r\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpBird3R[i] = Load(name, Draw);

					if (!bmpBird3R[i])
					{
						LogErr(L"Error loading bmpBird3R");
						ErrExit(eD2D);
					}
				}

				for (int i = 0; i < 7; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\gorilla\\l\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpGorrillaL[i] = Load(name, Draw);

					if (!bmpGorrillaL[i])
					{
						LogErr(L"Error loading bmpGorrillaL");
						ErrExit(eD2D);
					}
				}
				for (int i = 0; i < 7; ++i)
				{
					wchar_t name[100]{ L".\\res\\img\\gorilla\\r\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpGorrillaR[i] = Load(name, Draw);

					if (!bmpGorrillaR[i])
					{
						LogErr(L"Error loading bmpGorrillaR");
						ErrExit(eD2D);
					}
				}
			}
		}

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&iWriteFactory));
		if (hr != S_OK)
		{
			LogErr(L"Error creating D2D1 WriteFactory !");
			ErrExit(eD2D);
		}

		if (iWriteFactory)
		{
			hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
				DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"", &nrmFormat);
			hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
				DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"", &midFormat);
			hr = iWriteFactory->CreateTextFormat(L"GABRIOLA", NULL, DWRITE_FONT_WEIGHT_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
				DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"", &bigFormat);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 WriteFactory text formats !");
				ErrExit(eD2D);
			}
		}


		PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_ASYNC);

		for (int i = 0; i < 500; ++i)
		{
			Draw->BeginDraw();
			Draw->DrawBitmap(bmpIntro[GetIntroFrame()], D2D1::RectF(0, 0, scr_width, scr_height));
			Draw->DrawBitmap(bmpLogo, D2D1::RectF(0, 0, scr_width, scr_height));
			Draw->EndDraw();
		}
	}

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	_wsetlocale(LOCALE_ALL, L"");
	bIns = hInstance;
	if (!bIns)ErrExit(eClass);

	CreateResources();

	PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);

	while (bMsg.message != WM_QUIT)
	{
		if ((bRet = PeekMessage(&bMsg, nullptr, NULL, NULL, PM_REMOVE)) != 0)
		{
			if (bRet == -1)ErrExit(eMsg);
			TranslateMessage(&bMsg);
			DispatchMessage(&bMsg);
		}

		if (pause)
		{
			if (show_help)continue;

			if (hgltBrush && bigFormat)
			{
				Draw->BeginDraw();
				Draw->DrawBitmap(bmpIntro[GetIntroFrame()], D2D1::RectF(0, 0, scr_width, scr_height));
				Draw->DrawTextW(L"ПАУЗА", 6, bigFormat, D2D1::RectF(scr_width / 2.0f - 100.0f, scr_height / 2.0f - 50.0f,
					scr_width, scr_height), hgltBrush);
				Draw->EndDraw();
			}

			continue;
		}

		/////////////////////////////////////////////////////////////////

		if (Balloon)Balloon->move((float)(level));

		if (!vFields.empty())
		{
			for (int i = 0; i < vFields.size(); ++i)
			{
				vFields[i]->dir = nature_dir;
				if (!vFields[i]->move((float)(level)))
				{
					if (vFields[i]->end.x <= 0 - scr_width)
					{
						FreeMem(&vFields[i]);
						vFields.erase(vFields.begin() + i);
						need_right_field = true;
						break;
					}

					if (vFields[i]->start.x >= 2.0f * scr_width)
					{
						FreeMem(&vFields[i]);
						vFields.erase(vFields.begin() + i);
						need_left_field = true;
						break;
					}
				}
			}
		}

		if (need_right_field)
		{
			vFields.push_back(dll::FIELDS::create(static_cast<nature>(RandIt(3, 4)),
				vFields.back()->end.x, 50.0f));
			need_right_field = false;
		}
		if (need_left_field)
		{
			vFields.insert(vFields.begin(), dll::FIELDS::create(static_cast<nature>(RandIt(3, 4)),
				vFields.front()->start.x - scr_width, 50.0f));
			need_left_field = false;
		}

		if (vSkies.size() < 5 && RandIt(0, 200) == 66)
		{
			vSkies.push_back(dll::FIELDS::create(static_cast<nature>(RandIt(0, 1)), scr_width,
				50.0f + (float)(RandIt(0, 30))));
		}
		if (!vSkies.empty())
		{
			for (int i = 0; i < vSkies.size(); ++i)
			{
				vSkies[i]->dir = nature_dir;
				if (!vSkies[i]->move((float)(level)))
				{
					FreeMem(&vSkies[i]);
					vSkies.erase(vSkies.begin() + i);
					break;
				}
			}
		}

		if (vGorillas.size() < 2 + level && RandIt(0, 350) == 66)
		{
			if (sound)mciSendString(L"play .\\res\\snd\\gorilla.wav", NULL, NULL, NULL);
			vGorillas.push_back(dll::GORRILLA::create(evils::gorilla, scr_width, ground - 200.0f));
		}
		if (!vGorillas.empty() && Balloon)
		{
			for (int i = 0; i < vGorillas.size(); ++i)
			{
				if (Balloon->dir != dirs::stop)vGorillas[i]->set_move_dir(dirs::left);
				else
				{
					if (Balloon)
					{
						if (Balloon->start.x <= vGorillas[i]->start.x)vGorillas[i]->set_move_dir(dirs::left);
						else vGorillas[i]->set_move_dir(dirs::right);
					}
				}
				if (!vGorillas[i]->move((float)(level)))
				{
					FreeMem(&vGorillas[i]);
					vGorillas.erase(vGorillas.begin() + i);
					break;
				}
			}
		}
		if (!vGorillas.empty() && Balloon)
		{
			for (int i = 0; i < vGorillas.size(); ++i)
			{
				if (vGorillas[i]->attack())vBannanas.push_back(dll::BANNANA::create(vGorillas[i]->center.x,
					vGorillas[i]->start.y, Balloon->center.x, Balloon->center.y));
			}
		}

		if (!vBannanas.empty())
		{
			for (std::vector<dll::BANNANA*>::iterator ban = vBannanas.begin(); ban < vBannanas.end(); ++ban)
			{
				if (!(*ban)->move((float)(level)))
				{
					(*ban)->Release();
					vBannanas.erase(ban);
					break;
				}
			}
		}

		if (!vBannanas.empty() && Balloon)
		{
			for (std::vector<dll::BANNANA*>::iterator ban = vBannanas.begin(); ban < vBannanas.end(); ++ban)
			{
				if (dll::Intersect(Balloon->center, (*ban)->center, Balloon->x_rad, (*ban)->x_rad,
					Balloon->y_rad, (*ban)->y_rad))
				{
					if (sound)mciSendString(L"play .\\res\\snd\\splash.wav", NULL, NULL, NULL);
					Balloon->lifes -= 10;
					(*ban)->Release();
					vBannanas.erase(ban);
					if (Balloon->lifes <= 0)
					{
						killed = true;
						FallingBalloon = dll::PROTON::create(Balloon->center.x, Balloon->center.y, 100.0f, 100.0f);
						FreeMem(&Balloon);
					}

					break;
				}
			}
		}
		
		if (vBirds.size() < 3 + level && RandIt(0, 300) == 88)
		{
			if (sound)mciSendString(L"play .\\res\\snd\\bird.wav", NULL, NULL, NULL);
			switch (RandIt(0, 2))
			{
			case 0:
				vBirds.push_back(dll::BIRD1::create(evils::bird1, scr_width, RandIt(sky, ground - 150.0f)));
				break;

			case 1:
				vBirds.push_back(dll::BIRD2::create(evils::bird2, scr_width, RandIt(sky, ground - 150.0f)));
				break;

			case 2:
				vBirds.push_back(dll::BIRD3::create(evils::bird3, scr_width, RandIt(sky, ground - 150.0f)));
				break;
			}
		}
		if (!vBirds.empty())
		{
			for (std::vector<dll::EVILS*>::iterator bird = vBirds.begin(); bird < vBirds.end(); ++bird)
			{
				if (!(*bird)->move((float)(level)))
				{
					(*bird)->Release();
					vBirds.erase(bird);
					break;
				}
			}
		}

		if (!vBirds.empty() && Balloon)
		{
			for (std::vector<dll::EVILS*>::iterator ban = vBirds.begin(); ban < vBirds.end(); ++ban)
			{
				if (dll::Intersect(Balloon->center, (*ban)->center, Balloon->x_rad, (*ban)->x_rad,
					Balloon->y_rad, (*ban)->y_rad))
				{
					if (sound)mciSendString(L"play .\\res\\snd\\smash.wav", NULL, NULL, NULL);
					Balloon->lifes -= 15;
					(*ban)->Release();
					vBirds.erase(ban);
					if (Balloon->lifes <= 0)
					{
						killed = true;
						FallingBalloon = dll::PROTON::create(Balloon->center.x, Balloon->center.y, 100.0f, 100.0f);
						FreeMem(&Balloon);
					}

					break;
				}
			}
		}

		if (!vGorillas.empty() && Balloon)
		{
			for (std::vector<dll::EVILS*>::iterator ban = vGorillas.begin(); ban < vGorillas.end(); ++ban)
			{
				if (dll::Intersect(Balloon->center, (*ban)->center, Balloon->x_rad, (*ban)->x_rad,
					Balloon->y_rad, (*ban)->y_rad))
				{
					if (sound)mciSendString(L"play .\\res\\snd\\smash.wav", NULL, NULL, NULL);
					Balloon->lifes -= 15;
					(*ban)->Release();
					vGorillas.erase(ban);
					if (Balloon->lifes <= 0)
					{
						killed = true;
						FallingBalloon = dll::PROTON::create(Balloon->center.x, Balloon->center.y, 100.0f, 100.0f);
						FreeMem(&Balloon);
					}

					break;
				}
			}
		}

		// DRAW THINGS **************************************************

		Draw->BeginDraw();

		if (nrmFormat && txtBrush && hgltBrush && txtBrush && statBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush)
		{
			Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 10.0f, 20.0f), b1BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 10.0f, 20.0f), b2BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 10.0f, 20.0f), b3BckgBrush);

			if (name_set)Draw->DrawTextW(L"ИМЕ НА ПИЛОТ", 13, nrmFormat, b1TxtRect, inactBrush);
			else
			{
				if (!b1Hglt)Draw->DrawTextW(L"ИМЕ НА ПИЛОТ", 13, nrmFormat, b1TxtRect, txtBrush);
				else Draw->DrawTextW(L"ИМЕ НА ПИЛОТ", 13, nrmFormat, b1TxtRect, hgltBrush);
			}

			if (!b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, txtBrush);
			else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, hgltBrush);

			if (!b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, txtBrush);
			else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, hgltBrush);
		}

		if (!vFields.empty())
		{
			for (int i = 0; i < vFields.size(); ++i)
			{
				if (vFields[i]->get_type() == nature::field1)
					Draw->DrawBitmap(bmpField1, D2D1::RectF(vFields[i]->start.x, vFields[i]->start.y,
						vFields[i]->end.x, vFields[i]->end.y));
				else
					Draw->DrawBitmap(bmpField2, D2D1::RectF(vFields[i]->start.x, vFields[i]->start.y,
						vFields[i]->end.x, vFields[i]->end.y));
			}
		}

		/////////////////////////////////////

		if (Balloon)
		{
			int aframe = Balloon->get_frame();
			Draw->DrawBitmap(bmpBalloon[aframe], Resizer(bmpBalloon[aframe], Balloon->start.x, Balloon->start.y));
			Draw->DrawLine(D2D1::Point2F(Balloon->start.x, Balloon->end.y + 3.0f),
				D2D1::Point2F(Balloon->start.x + (float)(Balloon)->lifes, Balloon->end.y + 3.0f), statBrush, 5.0f);
		}

		if (!vSkies.empty())
		{
			for (int i = 0; i < vSkies.size(); ++i)
			{
				switch (vSkies[i]->get_type())
				{
				case nature::sun:
					Draw->DrawBitmap(bmpSun, D2D1::RectF(vSkies[i]->start.x, vSkies[i]->start.y,
						vSkies[i]->end.x, vSkies[i]->end.y));
					break;

				case nature::cloud1:
					Draw->DrawBitmap(bmpCloud1, D2D1::RectF(vSkies[i]->start.x, vSkies[i]->start.y,
						vSkies[i]->end.x, vSkies[i]->end.y));
					break;

				case nature::cloud2:
					Draw->DrawBitmap(bmpCloud1, D2D1::RectF(vSkies[i]->start.x, vSkies[i]->start.y,
						vSkies[i]->end.x, vSkies[i]->end.y));
					break;
				}
			}
		}

		if (!vGorillas.empty())
		{
			for (int i = 0; i < vGorillas.size(); ++i)
			{
				int aframe = vGorillas[i]->get_frame();
				if (vGorillas[i]->get_move_dir() == dirs::left)
					Draw->DrawBitmap(bmpGorrillaL[aframe], Resizer(bmpGorrillaL[aframe], vGorillas[i]->start.x,
						vGorillas[i]->start.y));
				else Draw->DrawBitmap(bmpGorrillaR[aframe], Resizer(bmpGorrillaR[aframe], vGorillas[i]->start.x,
					vGorillas[i]->start.y));
			}
		}

		if (!vBannanas.empty())
		{
			for (int i = 0; i < vBannanas.size(); ++i)
				Draw->DrawBitmap(bmpBannana, D2D1::RectF(vBannanas[i]->start.x, vBannanas[i]->start.y,
					vBannanas[i]->end.x, vBannanas[i]->end.y));
		}

		if (!vBirds.empty() && Balloon)
		{
			for (int i = 0; i < vBirds.size(); ++i)
			{
				int aframe = vBirds[i]->get_frame();
				dirs cur_dir = vBirds[i]->get_move_dir();

				switch (vBirds[i]->get_type())
				{
				case evils::bird1:
					if (cur_dir == dirs::up_left || cur_dir == dirs::left || cur_dir == dirs::down_left)
						Draw->DrawBitmap(bmpBird1L[aframe], Resizer(bmpBird1L[aframe], vBirds[i]->start.x, vBirds[i]->start.y));
					else if (cur_dir == dirs::up_right || cur_dir == dirs::right || cur_dir == dirs::down_right)
						Draw->DrawBitmap(bmpBird1R[aframe], Resizer(bmpBird1R[aframe], vBirds[i]->start.x, vBirds[i]->start.y));
					else
					{
						if (Balloon->center.x <= vBirds[i]->center.x)
							Draw->DrawBitmap(bmpBird1L[aframe], Resizer(bmpBird1L[aframe], vBirds[i]->start.x, 
								vBirds[i]->start.y));
						else Draw->DrawBitmap(bmpBird1R[aframe], Resizer(bmpBird1R[aframe], vBirds[i]->start.x, 
							vBirds[i]->start.y));
					}
					break;

				case evils::bird2:
					if (cur_dir == dirs::up_left || cur_dir == dirs::left || cur_dir == dirs::down_left)
						Draw->DrawBitmap(bmpBird2L[aframe], Resizer(bmpBird2L[aframe], vBirds[i]->start.x, vBirds[i]->start.y));
					else if (cur_dir == dirs::up_right || cur_dir == dirs::right || cur_dir == dirs::down_right)
						Draw->DrawBitmap(bmpBird2R[aframe], Resizer(bmpBird2R[aframe], vBirds[i]->start.x, vBirds[i]->start.y));
					else
					{
						if (Balloon->center.x <= vBirds[i]->center.x)
							Draw->DrawBitmap(bmpBird2L[aframe], Resizer(bmpBird2L[aframe], vBirds[i]->start.x,
								vBirds[i]->start.y));
						else Draw->DrawBitmap(bmpBird2R[aframe], Resizer(bmpBird2R[aframe], vBirds[i]->start.x,
							vBirds[i]->start.y));
					}
					break;

				case evils::bird3:
					if (cur_dir == dirs::up_left || cur_dir == dirs::left || cur_dir == dirs::down_left)
						Draw->DrawBitmap(bmpBird3L[aframe], Resizer(bmpBird3L[aframe], vBirds[i]->start.x, vBirds[i]->start.y));
					else if (cur_dir == dirs::up_right || cur_dir == dirs::right || cur_dir == dirs::down_right)
						Draw->DrawBitmap(bmpBird3R[aframe], Resizer(bmpBird3R[aframe], vBirds[i]->start.x, vBirds[i]->start.y));
					else
					{
						if (Balloon->center.x <= vBirds[i]->center.x)
							Draw->DrawBitmap(bmpBird3L[aframe], Resizer(bmpBird3L[aframe], vBirds[i]->start.x,
								vBirds[i]->start.y));
						else Draw->DrawBitmap(bmpBird3R[aframe], Resizer(bmpBird3R[aframe], vBirds[i]->start.x,
							vBirds[i]->start.y));
					}
					break;

				}
			}
		}

		if (killed)
		{
			Draw->DrawBitmap(bmpCrashed, D2D1::RectF(FallingBalloon->start.x, FallingBalloon->start.y,
				FallingBalloon->end.x, FallingBalloon->end.y));
		}

		if (midFormat && statBrush)
		{
			wchar_t stat_txt[150]{ L"пилот: " };
			wchar_t add[10]{ L"\0" };
			int stat_size{ 0 };

			wcscat_s(stat_txt, current_player);
			
			wcscat_s(stat_txt, L", ниво: ");
			wsprintf(add, L"%d", level);
			wcscat_s(stat_txt, add);

			wcscat_s(stat_txt, L", точки: ");
			wsprintf(add, L"%d", score);
			wcscat_s(stat_txt, add);

			for (int i = 0; i < 150; ++i)
			{
				if (stat_txt[i] != '\0')++stat_size;
				else break;
			}

			Draw->DrawTextW(stat_txt, stat_size, midFormat, D2D1::RectF(10.0f, ground + 3.0f, scr_width, scr_height), statBrush);

			stat_size = 0;

			wcscpy_s(stat_txt, L"остават: ");
			swprintf_s(add, 10, L"%.2f", distance / 10.0f);
			wcscat_s(stat_txt, add);
			wcscat_s(stat_txt, L" км.");

			for (int i = 0; i < 150; ++i)
			{
				if (stat_txt[i] != '\0')++stat_size;
				else break;
			}

			Draw->DrawTextW(stat_txt, stat_size, midFormat, D2D1::RectF(scr_width - 300.0f, sky + 10.0f,
				scr_width, scr_height), statBrush);

		}
		
		// END DRAW *****************************************************

		Draw->EndDraw();

		if (killed)
		{
			FallingBalloon->start.y++;
			FallingBalloon->set_edges();
			if (FallingBalloon->end.y >= ground)GameOver();
		}
	}

	ClearResources();
	std::remove(tmp_file);
    
    return (int) bMsg.wParam;
}