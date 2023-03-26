#pragma once
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

struct points
{
	POINT screen_centre;
	POINT screen_halves[2];
	POINT screen_quaters[4];
};

struct check
{
	char word[5];
	int correctness[5];
};

struct animation
{
	bool isanimate;
	int which_row;
};

class my_app
{
private:
	static const std::wstring class_name;
	static const std::wstring overlay_green;
	static const std::wstring overlay_red;
	static const points screen_points;
	static ch::high_resolution_clock::time_point last_time;

	ATOM register_class(COLORREF, std::wstring); 

	static LRESULT CALLBACK window_proc_static(
		HWND window,
		UINT message,
		WPARAM wparam,
		LPARAM lparam);

	LRESULT CALLBACK window_proc(
		HWND window,
		UINT message,
		WPARAM wparam,
		LPARAM lparam);

	HWND create_main_window(DWORD style, DWORD ex_style);
	HWND create_game_window(DWORD style, HWND parent, int num);
	HWND create_overlay_window(int num);

	void draw_keyboard(HDC);
	void draw_board(HDC, int);
	void write_overlay(HDC, int);
	void animate_field(HDC&, const field&);
	bool is_current_empty();
	bool update_point_enter();
	bool update_point_forward();
	bool update_point_backward();
	void change_difficulty(int new_difficulty);
	void load_words();
	void draw_new_words();
	int check_word(check&, int);
	void on_window_move(HWND, int, LPWINDOWPOS);

	HINSTANCE hAppInstance; 
	HWND hMain;
	HWND* hGame;
	game_board* BRD;
	game_keyboard* KBRD;
	std::wstring ini_path;
	std::string* words;
	std::unordered_set<std::string> word_set;

	bool* isfinished;
	HWND* hOverlay;

	bool ismoving;
	POINT mouse_last_pos;

	static int curr_col;
	static int curr_row;
	static animation toAnimate;

	int difficulty; //=numOfBoards

public:
	RECT calculateCorrectRECT(RECT, int);
	std::wstring Narrow2Wide(std::string);

	my_app(HINSTANCE);
	~my_app();
	int run(int nCmdShow);

	friend INT_PTR CALLBACK InfoBox(HWND, UINT, WPARAM, LPARAM);
};