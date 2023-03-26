#pragma once

struct field
{
	RECT position;
	COLORREF color;
	char letter;
};

class game_board
{
public:
	static const LONG outermargin = 10;
	static const LONG margin = 6;
	static const LONG columns = 5;
	static const LONG field_size = 55;
	static const LONG width = columns * (field_size + margin) - margin + 2 * outermargin;

	static LONG rows;
	static LONG field_count;
	static LONG height;
	static LONG numOfBoards;

	field*** boards;

	game_board(int n = 1);
	~game_board();
	void update_board(int k, int j, int i, char letter, int color = 3);
};