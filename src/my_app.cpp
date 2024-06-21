#include "resource.h"
#include "framework.h"
#include "my_app.h"
#include "board.h"
#include "keyboard.h"

/*

	This Wordle application was created as part of the course 
	"Programming in a graphical environment" at the Warsaw University
	of Techonology. The project is purely didactic - I do not claim
	any rights to the Wordle brand owned by the New York Times Company.

	Copyright (c) 2023 Bartosz Kaczorowski

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

*/

ushort constexpr STRING_BUFF = 256;
ushort constexpr ID1_TIMER = 1;
ushort constexpr MAX_RAND = 16825;

const std::wstring my_app::class_name{ L"MY_APP CLASS" };
const std::wstring my_app::overlay_green{ L"GREEN OVERLAY" };
const std::wstring my_app::overlay_red{ L"RED OVERLAY" };
const points my_app::screen_points =
{ 
	.screen_centre = { GetSystemMetrics(SM_CXSCREEN) / 2, GetSystemMetrics(SM_CYSCREEN) / 2 },
	.screen_halves = { { GetSystemMetrics(SM_CXSCREEN) / 4, GetSystemMetrics(SM_CYSCREEN) / 2}, { 3 * GetSystemMetrics(SM_CXSCREEN) / 4, GetSystemMetrics(SM_CYSCREEN) / 2} },
	.screen_quaters =
	{ 
		{ GetSystemMetrics(SM_CXSCREEN) / 4, GetSystemMetrics(SM_CYSCREEN) / 4 + 100}, { 3 * GetSystemMetrics(SM_CXSCREEN) / 4, GetSystemMetrics(SM_CYSCREEN) / 4 + 100},
		{ GetSystemMetrics(SM_CXSCREEN) / 4, 3 * GetSystemMetrics(SM_CYSCREEN) / 4 - 100}, { 3 * GetSystemMetrics(SM_CXSCREEN) / 4, 3 * GetSystemMetrics(SM_CYSCREEN) / 4 - 100}
	}
};
ch::high_resolution_clock::time_point my_app::last_time = ch::high_resolution_clock::now();
int my_app::curr_col = 0;
int my_app::curr_row = 0;
animation my_app::toAnimate = { false, 0 };

INT_PTR CALLBACK InfoBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return static_cast <INT_PTR>(TRUE);

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return static_cast <INT_PTR>(TRUE);
		}
		break;
	}
	return static_cast <INT_PTR>(FALSE);
}

ATOM my_app::register_class(COLORREF color, std::wstring name)
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(hAppInstance, name.c_str(), &desc) != 0) return true;

	desc =
	{
	.cbSize = sizeof(WNDCLASSEXW),
	.lpfnWndProc = window_proc_static,
	.hInstance = hAppInstance,
	.hIcon = LoadIcon(hAppInstance, MAKEINTRESOURCE(IDI_ICON1)),
	.hCursor = LoadCursorW(nullptr, L"IDC_ARROW"),
	.hbrBackground = CreateSolidBrush(color),
	.lpszClassName = name.c_str()
	};

	return RegisterClassExW(&desc) != 0;
}

HWND my_app::create_main_window(DWORD style, DWORD ex_style)
{

	RECT size = { 0, 0, KBRD->width, KBRD->height };
	AdjustWindowRectEx(&size, style, true, ex_style);

	return CreateWindowExW(ex_style, class_name.c_str(), L"WORDLE - KEYBOARD", style,
		screen_points.screen_centre.x - KBRD->width / 2, screen_points.screen_centre.y + KBRD->height + 40, size.right - size.left, size.bottom - size.top, nullptr, nullptr, hAppInstance, this);
}

HWND my_app::create_game_window(DWORD style, HWND parent, int num)
{
	RECT size = { 0, 0, BRD->width, BRD->height };
	AdjustWindowRectEx(&size, style, false, 0);

	int x, y;
	switch (difficulty)
	{
	case 1:
		x = screen_points.screen_centre.x - BRD->width / 2;
		y = screen_points.screen_centre.y - BRD->height / 2;
		break;
	case 2:
		x = screen_points.screen_halves[num].x - BRD->width / 2;
		y = screen_points.screen_halves[num].y - BRD->height / 2;
		break;
	case 4:
		x = screen_points.screen_quaters[num].x - BRD->width / 2;
		y = screen_points.screen_quaters[num].y - BRD->height / 2;
		break;
	}
	std::string name = "WORDLE - PUZZLE - " + std::to_string(num);
	return CreateWindowExW(0, class_name.c_str(), Narrow2Wide(name).c_str(), style, x, y, size.right - size.left, size.bottom - size.top, parent, nullptr, hAppInstance, this);
}

HWND my_app::create_overlay_window(int num)
{
	DWORD style = WS_POPUP | WS_VISIBLE; 
	DWORD ex_style = WS_EX_LAYERED | WS_EX_TOOLWINDOW;
	RECT rect = { 0, 0, BRD->width, BRD->height };
	AdjustWindowRectEx(&rect, style, false, ex_style);

	RECT pRect;
	GetWindowRect(hGame[num], &pRect);

	std::wstring name = isfinished[num] ? overlay_green : overlay_red;
	HWND window = CreateWindowExW(ex_style, name.c_str(), L"", style, pRect.left + 8, pRect.top + 31, rect.right - rect.left, rect.bottom - rect.top, hGame[num], nullptr, hAppInstance, this);
	SetLayeredWindowAttributes(window, 0, 180, LWA_ALPHA);

	return window;
}

LRESULT CALLBACK my_app::window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	my_app* app = nullptr;
	if (message == WM_NCCREATE)
	{
		app = static_cast<my_app*>(reinterpret_cast<LPCREATESTRUCTW>(lparam)->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));

		SetTimer(window, ID1_TIMER, 100, nullptr);
	}
	else app = reinterpret_cast<my_app*>(GetWindowLongPtrW(window, GWLP_USERDATA));

	LRESULT res = app ? app->window_proc(window, message, wparam, lparam) : DefWindowProcW(window, message, wparam, lparam);
	if (message == WM_NCDESTROY) SetWindowLongPtrW(window, GWLP_USERDATA, 0);
	return res;
}

LRESULT CALLBACK my_app::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CLOSE:
		for (int i = 0; i < difficulty; i++) DestroyWindow(hOverlay[i]);
		for (int i = 0; i < difficulty; i++) DestroyWindow(hGame[i]);
		DestroyWindow(hMain);
		return 0;
	case WM_TIMER:
	{
		if (wparam == 1 && window == hMain)
		{
			auto now = ch::high_resolution_clock::now();
			double elapsed = static_cast<double>(ch::duration_cast<ch::milliseconds>(now - last_time).count());

			wchar_t s[STRING_BUFF];
			swprintf_s(s, STRING_BUFF, L"WORDLE - KEYBOARD: %.3f", elapsed / 1000);
			SetWindowText(hMain, s);
		}
	}
	break;
	case WM_COMMAND:
	{
		bool flag = true;
		switch (LOWORD(wparam))
		{
		case ID_DIFFICULTY_EASY:
			WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", L"1", ini_path.c_str());
			change_difficulty(1);
			for (int i = 0; i < difficulty; i++)
				ShowWindow(hGame[i], SW_SHOWNA);
			InvalidateRect(hMain, nullptr, true);
			UpdateWindow(hMain);
		break;
		case ID_DIFFICULTY_MEDIUM:
			WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", L"2", ini_path.c_str());
			change_difficulty(2);
			for (int i = 0; i < difficulty; i++)
				ShowWindow(hGame[i], SW_SHOWNA);
			InvalidateRect(hMain, nullptr, true);
			UpdateWindow(hMain);
		break;
		case ID_DIFFICULTY_HARD:
			WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", L"4", ini_path.c_str());
			change_difficulty(4);
			for (int i = 0; i < difficulty; i++)
				ShowWindow(hGame[i], SW_SHOWNA);
			InvalidateRect(hMain, nullptr, true);
			UpdateWindow(hMain);
		break;
		case ID_HELP_INSTRUCTION:
			flag = false;
			DialogBox(hAppInstance, MAKEINTRESOURCE(IDD_ABOUT), window, InfoBox);
		break;
		}
		if (flag)
		{
			curr_col = 0;
			curr_row = 0;
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;

		if (window == hMain)
		{
			HDC hdc = BeginPaint(hMain, &ps);
			draw_keyboard(hdc);
			EndPaint(hMain, &ps);
		}
			
		for (int k = 0; k < difficulty; k++)
		{
			if (window != hGame[k] || (window == hGame[k] && isfinished[k])) continue;
			HDC hdc = BeginPaint(hGame[k], &ps);
			draw_board(hdc, k);
			EndPaint(hGame[k], &ps);
		}

		for (int k = 0; k < difficulty; k++)
		{
			if (window != hOverlay[k]) continue;
			HDC hdc = BeginPaint(hOverlay[k], &ps);
			write_overlay(hdc, k);
			EndPaint(hOverlay[k], &ps);
		}
	}
	break;
	case WM_CHAR:
	{
		if (window == hMain && 65 <= wparam && wparam <= 122 && isalpha((char)wparam))
		{
			char letter = toupper((char)wparam);
			for (int k = 0; k < difficulty; k++)
			{
				if (isfinished[k]) continue;
				BRD->update_board(k, curr_col, curr_row, letter);
				InvalidateRect(hGame[k], nullptr, false);
				UpdateWindow(hGame[k]);
			}
			update_point_forward();
		}
	}
	break;
	case WM_KEYDOWN:
	{
		if (window == hMain)
		{
			switch (wparam)
			{
			case VK_RETURN:
				if (curr_col == 4 && !is_current_empty())
				{
					bool isrevert = false;
					for (int k = 0; k < difficulty; k++)
					{
						if (isfinished[k]) continue;

						check word_to_check;
						for (int j = 0; j < game_board::columns; j++)
						{
							word_to_check.word[j] = BRD->boards[k][j][curr_row].letter;
							word_to_check.correctness[j] = 2;
						}
						int iscorrect = check_word(word_to_check, k);
						switch (iscorrect)
						{
						case 0: // wygrana
							toAnimate = { true, curr_row };
							for (int j = 0; j < game_board::columns; j++)
							{
								BRD->update_board(k, j, curr_row, word_to_check.word[j], 0);
								KBRD->update_keyboard(word_to_check.word[j], 0, k);
							}
							InvalidateRect(hGame[k], nullptr, false);
							UpdateWindow(hGame[k]);

							isfinished[k] = true;
							hOverlay[k] = create_overlay_window(k);
							ShowWindow(hOverlay[k], SW_SHOWNA);
							UpdateWindow(hOverlay[k]);
						break;
						case 1: // poprawne słowo, ale nie wygrywające
							toAnimate = { true, curr_row };
							for (int j = 0; j < game_board::columns; j++)
							{
								BRD->update_board(k, j, curr_row, word_to_check.word[j], word_to_check.correctness[j]);
								KBRD->update_keyboard(word_to_check.word[j], word_to_check.correctness[j], k);
							}
							if (curr_row == BRD->rows - 1 && !isfinished[k])
							{
								InvalidateRect(hGame[k], nullptr, false);
								UpdateWindow(hGame[k]);

								hOverlay[k] = create_overlay_window(k);
								ShowWindow(hOverlay[k], SW_SHOWNA);
								UpdateWindow(hOverlay[k]);
								isfinished[k] = true;
							}
						break;
						case 2: // niepoprawne słowo
							isrevert = true;
							for (int j = 0; j < game_board::columns; j++) BRD->update_board(k, j, curr_row, '\0');
						break;
						}
					}
					if (isrevert) curr_col = 0;
					for (int k = 0; k < difficulty; k++)
					{
						if (isfinished[k]) continue;
						InvalidateRect(hGame[k], nullptr, false);
						UpdateWindow(hGame[k]);
					}
					InvalidateRect(hMain, nullptr, false);
					UpdateWindow(hMain);
				}
				toAnimate = { false, 0 };
				update_point_enter();
			break;
			case VK_BACK:
				if (is_current_empty()) update_point_backward();
				for (int k = 0; k < difficulty; k++)
				{
					if (isfinished[k]) continue;
					BRD->update_board(k, curr_col, curr_row, '\0');
					InvalidateRect(hGame[k], nullptr, false);
					UpdateWindow(hGame[k]);
				}
			break;
			}
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		POINT mouse_pos;
		GetCursorPos(&mouse_pos);
		ismoving = true;
		mouse_last_pos = mouse_pos;
		SetCapture(window);
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (ismoving)
		{
			POINT mouse_pos;
			GetCursorPos(&mouse_pos);
	
			int dx = mouse_pos.x - mouse_last_pos.x;
			int dy = mouse_pos.y - mouse_last_pos.y;

			RECT window_rect;
			GetWindowRect(window, &window_rect);
			SetWindowPos(window, nullptr, window_rect.left + dx, window_rect.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
			mouse_last_pos = mouse_pos;
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		ismoving = false;
		ReleaseCapture();
	}
	break;
	case WM_WINDOWPOSCHANGED:
		for (int k = 0; k < difficulty; k++)
		{
			if (window != hGame[k] && window != hOverlay[k]) continue;
			on_window_move(window, k, reinterpret_cast<LPWINDOWPOS>(lparam));
		}
	break;
	case WM_DESTROY:
		if (window == hMain) PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

my_app::my_app(HINSTANCE hInstance) : hAppInstance{ hInstance }, KBRD(), ini_path{ fs::current_path().append("Wordle.ini").wstring() }
{
	register_class(RGB(255,255,255), class_name);
	register_class(RGB(0, 255, 0), overlay_green);
	register_class(RGB(255, 0, 0), overlay_red);

	DWORD main_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	hMain = create_main_window(main_style, WS_EX_LAYERED);
	SetLayeredWindowAttributes(hMain, 0, 180, LWA_ALPHA);

	if (GetFileAttributes(ini_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		WritePrivateProfileString(L"WORDLE", L"DIFFICULTY", L"1", ini_path.c_str());

	wchar_t buffer[2];
	GetPrivateProfileString(L"WORDLE", L"DIFFICULTY", L"1", buffer, 2, ini_path.c_str());
	load_words();
	change_difficulty(std::stoi(buffer));

	HMENU hMenu = LoadMenu(hAppInstance, MAKEINTRESOURCEW(IDR_MENU1));
	SetMenu(hMain, hMenu);
}

my_app::~my_app()
{
	delete[] hOverlay;
	delete[] hGame;
	
	delete BRD;
	delete KBRD;
	delete[] words;
	delete[] isfinished;
}

int my_app::run(int nCmdShow)
{
	ShowWindow(hMain, nCmdShow);
	for(int i = 0; i < difficulty; i++)
		ShowWindow(hGame[i], SW_SHOWNA);

	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == BOOL(-1)) return EXIT_FAILURE;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return EXIT_SUCCESS;
}

void my_app::change_difficulty(int new_difficulty)
{
	difficulty = new_difficulty;

	HMENU hMenu = GetMenu(hMain);
	CheckMenuItem(hMenu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_DIFFICULTY_HARD, MF_UNCHECKED);

	switch (difficulty)
	{
	case 1: CheckMenuItem(hMenu, ID_DIFFICULTY_EASY, MF_CHECKED); break;
	case 2: CheckMenuItem(hMenu, ID_DIFFICULTY_MEDIUM, MF_CHECKED); break;
	case 4: CheckMenuItem(hMenu, ID_DIFFICULTY_HARD, MF_CHECKED); break;
	}

	if (hGame)
	{
		for (int i = 0; i < BRD->numOfBoards; i++)
			DestroyWindow(hGame[i]);
		delete[] hGame;
	}

	if (hOverlay)
	{
		for (int i = 0; i < BRD->numOfBoards; i++)
			DestroyWindow(hOverlay[i]);
		delete[] hOverlay;
	}

	if (words) delete[] words;
	draw_new_words();

	if (BRD) delete BRD;
	BRD = new game_board(difficulty);

	if (KBRD) delete KBRD;
	KBRD = new game_keyboard(difficulty);

	if (isfinished) delete[] isfinished;
	isfinished = new bool[difficulty];
	for (int i = 0; i < difficulty; i++) isfinished[i] = false;

	hOverlay = new HWND[difficulty];
	for (int i = 0; i < difficulty; i++) hOverlay[i] = nullptr;

	hGame = new HWND[difficulty];
	for (int i = 0; i < difficulty; i++)
	{
		hGame[i] = create_game_window(WS_CAPTION, hMain, i);
	}
	last_time = ch::high_resolution_clock::now();
}

void my_app::draw_keyboard(HDC hdc)
{
	HFONT font = CreateFontW(
		28,// Height
		0, // Width
		0, // Escapement
		0, // Orientation
		FW_BOLD, // Weight
		FALSE, // Italic
		FALSE, // Underline
		FALSE, // StrikeOut
		EASTEUROPE_CHARSET, // CharSet
		OUT_DEFAULT_PRECIS, // OutPrecision
		CLIP_DEFAULT_PRECIS, // ClipPrecision
		DEFAULT_QUALITY, // Quality
		DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
		TEXT("Arial")); // Facename

	HFONT oldFont = (HFONT)SelectObject(hdc, font);

	for (const std::pair<char, key>& K : KBRD->keyboard)
	{
		for (int k = 0; k < difficulty; k++)
		{
			RECT rect = K.second.position;
			HPEN key_pen, oldPen;
			HBRUSH key_brush, oldBrush;

			if (K.second.color[k] == RGB(251, 252, 255))
			{
				key_pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
				oldPen = (HPEN)SelectObject(hdc, key_pen);
				key_brush = CreateSolidBrush(RGB(251, 252, 255));
				oldBrush = (HBRUSH)SelectObject(hdc, key_brush);
				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
				SelectObject(hdc, oldPen);
				DeleteObject(key_pen);
				SelectObject(hdc, oldBrush);
				DeleteObject(key_brush);
			}
			else
			{
				rect = calculateCorrectRECT(K.second.position, k);
				key_brush = CreateSolidBrush(K.second.color[k]);
				oldBrush = (HBRUSH)SelectObject(hdc, key_brush);
				FillRect(hdc, &rect, key_brush);
				SelectObject(hdc, oldBrush);
				DeleteObject(key_brush);
			}
		}
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(50, 50, 50));

		SIZE letterSize;
		GetTextExtentPoint32(hdc, std::wstring(1, K.first).c_str(), 1, &letterSize);

		int x = K.second.position.left + game_keyboard::field_size / 2 - letterSize.cx / 2;
		int y = K.second.position.top + game_keyboard::field_size / 2 - letterSize.cy / 2;
		TextOut(hdc, x, y, std::wstring(1, K.first).c_str(), 1);
	}

	SelectObject(hdc, oldFont);
	DeleteObject(font);
}

void my_app::draw_board(HDC hdc, int num)
{
	HFONT font = CreateFontW(
		28,// Height
		0, // Width
		0, // Escapement
		0, // Orientation
		FW_BOLD, // Weight
		FALSE, // Italic
		FALSE, // Underline
		FALSE, // StrikeOut
		EASTEUROPE_CHARSET, // CharSet
		OUT_DEFAULT_PRECIS, // OutPrecision
		CLIP_DEFAULT_PRECIS, // ClipPrecision
		DEFAULT_QUALITY, // Quality
		DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
		TEXT("Arial")); // Facename

	HFONT oldFont = (HFONT)SelectObject(hdc, font);

	for (int i = 0; i < BRD->rows; ++i)
	{
		for (int j = 0; j < BRD->columns; ++j)
		{
			const field& f = BRD->boards[num][j][i];

			HPEN key_pen = f.color == RGB(251, 252, 255) ? CreatePen(PS_SOLID, 2, RGB(164, 174, 196)) : CreatePen(PS_SOLID, 2, f.color);
			HPEN oldPen = (HPEN)SelectObject(hdc, key_pen);
			HBRUSH key_brush = CreateSolidBrush(f.color);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, key_brush);

			if (toAnimate.isanimate && toAnimate.which_row == i) animate_field(hdc, f);
			else Rectangle(hdc, f.position.left, f.position.top, f.position.right, f.position.bottom);

			SelectObject(hdc, oldPen);
			DeleteObject(key_pen);
			SelectObject(hdc, oldBrush);
			DeleteObject(key_brush);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(50, 50, 50));

			SIZE letterSize;
			GetTextExtentPoint32(hdc, std::wstring(1, f.letter).c_str(), 1, &letterSize);

			int x = f.position.left + game_board::field_size / 2 - letterSize.cx / 2;
			int y = f.position.top + game_board::field_size / 2 - letterSize.cy / 2;
			TextOut(hdc, x, y, std::wstring(1, f.letter).c_str(), 1);
		}
	}

	SelectObject(hdc, oldFont);
	DeleteObject(font);
}

void my_app::animate_field(HDC& hdc, const field& f)
{
	int top = f.position.top;
	int bottom = f.position.bottom;

	for (int i = 24; i >= 0; i--)
	{
		Rectangle(hdc, f.position.left, top + i, f.position.right, bottom - i);
		//std::this_thread::sleep_for(std::chrono::microseconds(1)); //DEBUG
		std::this_thread::sleep_for(std::chrono::nanoseconds(100)); //RELEASE
	}
}

void my_app::write_overlay(HDC hdc, int num)
{
	HFONT font = CreateFontW(
		40,// Height
		0, // Width
		0, // Escapement
		0, // Orientation
		FW_BOLD, // Weight
		FALSE, // Italic
		FALSE, // Underline
		FALSE, // StrikeOut
		EASTEUROPE_CHARSET, // CharSet
		OUT_DEFAULT_PRECIS, // OutPrecision
		CLIP_DEFAULT_PRECIS, // ClipPrecision
		DEFAULT_QUALITY, // Quality
		DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
		TEXT("Arial")); // Facename

	HFONT oldFont = (HFONT)SelectObject(hdc, font);

	SetBkMode(hdc, RGB(0, 0, 0));
	SetTextColor(hdc, RGB(255, 255, 255));

	RECT rect;
	GetClientRect(hOverlay[num], &rect);
	std::wstring text = Narrow2Wide(words[num]);

	SIZE letterSize;
	GetTextExtentPoint32(hdc, text.c_str(), 5, &letterSize);

	int x = (rect.right - rect.left) / 2 - letterSize.cx / 2;
	int y = (rect.bottom - rect.top) / 2 - letterSize.cy / 2;
	TextOut(hdc, x, y, text.c_str(), 5);

	SelectObject(hdc, oldFont);
	DeleteObject(font);
}

bool my_app::update_point_forward()
{
	if (curr_col == 4) return false;
	curr_col += 1;
	return true;
}
bool my_app::update_point_backward()
{
	if (curr_col == 0) return false;
	curr_col -= 1;
	return true;
}
bool my_app::update_point_enter()
{
	if (curr_col != 4 || is_current_empty() || curr_row == BRD->rows - 1) return false;
	curr_col = 0;
	curr_row += 1;
	return true;
}
bool my_app::is_current_empty()
{
	for (int k = 0; k < difficulty; k++)
		if (!isfinished[k] && BRD->boards[k][curr_col][curr_row].letter == '\0') return true;
	return false;
}

void my_app::draw_new_words()
{
	std::srand((uint)std::time(nullptr));
	words = new std::string[difficulty];
	for (int i = 0; i < difficulty; i++)
	{
		int random = 1 + std::rand() % MAX_RAND;
		auto it = word_set.begin();
		advance(it, random);
		words[i] = *it;
	}
}

void my_app::load_words()
{
	std::ifstream file;
	file.open(fs::current_path().append("Wordle.txt").string(), std::ios::in);
	std::string line;
	while (getline(file, line))
	{
		line.erase(std::remove(line.begin(), line.end(), '\n'), line.cend());
		word_set.insert(line);
	}
	file.close();
}

int my_app::check_word(check& word, int k)
{
	std::string word_str = "";
	for (int i = 0; i < 5; i++) word_str += tolower(word.word[i]);
	if (word_str == words[k]) //sukces
	{
		for (int i = 0; i < 5; i++) word.correctness[i] = 0;
		return 0;
	}

	if (word_set.contains(word_str)) //znaleziono
	{
		for (int i = 0; i < 5; i++)
		{
			if (words[k].at(i) == word_str[i]) word.correctness[i] = 0;
			else
			{
				if (words[k].find(word_str[i]) != std::string::npos) word.correctness[i] = 1;
				else word.correctness[i] = 2;
			}
		}
		return 1;
	}
	return 2; //niepowodzenie
}

RECT my_app::calculateCorrectRECT(RECT key_rect, int which_part)
{
	RECT output = key_rect;
	switch (difficulty)
	{
	case 1: break;
	case 2:
		if (which_part == 0) output.right = (key_rect.right + key_rect.left) / 2;
		else output.left = (key_rect.right + key_rect.left) / 2;
		break;
	case 4:
		switch (which_part)
		{
		case 0:
			output.right = (key_rect.right + key_rect.left) / 2;
			output.bottom = (key_rect.bottom + key_rect.top) / 2;
		break;
		case 1:
			output.left = (key_rect.right + key_rect.left) / 2;
			output.bottom = (key_rect.bottom + key_rect.top) / 2;
		break;
		case 2:
			output.right = (key_rect.right + key_rect.left) / 2;
			output.top = (key_rect.bottom + key_rect.top) / 2;
		break;
		case 3:
			output.left = (key_rect.right + key_rect.left) / 2;
			output.top = (key_rect.bottom + key_rect.top) / 2;
		break;
		}
		break;
	}
	return output;
}

std::wstring my_app::Narrow2Wide(std::string narrow)
{
	int str_len = (int)narrow.size();
	int buff_len = MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), str_len, nullptr, 0);
	wchar_t* text = new wchar_t[buff_len];
	MultiByteToWideChar(CP_UTF8, 0, narrow.c_str(), str_len, text, buff_len);
	for (int i = 0; i < buff_len; i++) text[i] = toupper(text[i]);
	std::wstring wide(text, buff_len);
	delete[] text;
	return wide;
}

void my_app::on_window_move(HWND window, int num, LPWINDOWPOS params)
{
	HWND other;
	int xoff, yoff;
	if (window == hGame[num])
	{
		other = hOverlay[num];
		xoff = 8;
		yoff = 31;
	}
	else
	{
		other = hGame[num];
		xoff = -8;
		yoff = -31;
	}
	
	RECT other_rc;
	GetWindowRect(other, &other_rc);
	if (params->x == other_rc.left && params->y == other_rc.top) return; 
	SetWindowPos(other, nullptr, params->x + xoff, params->y + yoff, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW);
}