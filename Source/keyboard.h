#pragma once

struct key
{
	RECT position;
	COLORREF* color;
};

class game_keyboard
{
public:
	static const LONG outermargin = 10;
	static const LONG margin = 6;
	static const LONG field_size = 55;
	static const LONG width = 10 * (field_size + margin) - margin + 2 * outermargin;
	static const LONG height = 3 * (field_size + margin) - margin + 2 * outermargin;
	static const std::string alphabet;

	static LONG numOfBoards;
	
	std::map<char, key> keyboard;
	
	game_keyboard(int n = 1);
	~game_keyboard();
	void update_keyboard(char, int, int);
};