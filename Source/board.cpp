#include "framework.h"
#include "board.h"

LONG game_board::rows = 6;
LONG game_board::field_count = 30;
LONG game_board::height = 372;
LONG game_board::numOfBoards = 1;

game_board::game_board(int n)
{
	numOfBoards = n;
	switch (numOfBoards)
	{
	case 1: rows = 6; break;
	case 2: rows = 8; break;
	case 4: rows = 10; break;
	default: rows = 6; break;
	}
	field_count = columns * rows;
	height = rows * (field_size + margin) - margin + 2 * outermargin;

	boards = new field **[numOfBoards];
	for (int k = 0; k < numOfBoards; ++k)
	{
		boards[k] = new field*[columns];
		for (int j = 0; j < columns; ++j)
		{
			boards[k][j] = new field[rows];
			for (int i = 0; i < rows; ++i)
			{
				field& f = boards[k][j][i];
				f.position.top = i * (field_size + margin) + outermargin;
				f.position.left = j * (field_size + margin) + outermargin;
				f.position.bottom = f.position.top + field_size;
				f.position.right = f.position.left + field_size;
				f.letter = '\0';
				f.color = RGB(251, 252, 255);
			}
		}
	}
}

void game_board::update_board(int k, int j, int i, char letter, int color)
{
	field& f = boards[k][j][i];
	f.letter = letter;

	switch (color)
	{
	case 0: //sukces
		f.color = RGB(121, 184, 81);
		break;
	case 1: //drobne błędy
		f.color = RGB(243, 194, 55);
		break;
	case 2: //niepowodzenie
		f.color = RGB(164, 174, 196);
		break;
	default:
		f.color = RGB(251, 252, 255);
		break;
	}
}

game_board::~game_board()
{
	for (int k = 0; k < numOfBoards; ++k)
	{
		for (int j = 0; j < columns; ++j)
		{
			delete[] boards[k][j];
		}
		delete[] boards[k];
	}
	delete[] boards;
}