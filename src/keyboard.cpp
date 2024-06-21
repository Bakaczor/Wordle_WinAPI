#include "framework.h"
#include "keyboard.h"

const std::string game_keyboard::alphabet = "QWERTYUIOPASDFGHJKLZXCVBNM";
LONG game_keyboard::numOfBoards = 1;

game_keyboard::game_keyboard(int n)
{
	numOfBoards = n;
	key K;
	int j = 0;
	for (int i = 0; i < 10; i++)
	{
		char letter = alphabet[j++];
		K.position.left = i * (field_size + margin) + outermargin;
		K.position.top = outermargin;
		K.position.right = K.position.left + field_size;
		K.position.bottom = K.position.top + field_size;
		keyboard.insert(std::make_pair(letter, K));
		keyboard[letter].color = new COLORREF[numOfBoards];
		for (int i = 0; i < numOfBoards; i++) keyboard[letter].color[i] = RGB(251, 252, 255);
	}
	for (int i = 0; i < 9; i++)
	{
		char letter = alphabet[j++];
		K.position.left = i * (field_size + margin) + outermargin + field_size / 2;
		K.position.top = outermargin + field_size + margin;
		K.position.right = K.position.left + field_size;
		K.position.bottom = K.position.top + field_size;
		keyboard.insert(std::make_pair(letter, K));
		keyboard[letter].color = new COLORREF[numOfBoards];
		for (int i = 0; i < numOfBoards; i++) keyboard[letter].color[i] = RGB(251, 252, 255);
	}
	for (int i = 0; i < 7; i++)
	{
		char letter = alphabet[j++];
		K.position.left = (i + 1) * (field_size + margin) + outermargin + field_size / 2;
		K.position.top = outermargin + 2 * (field_size + margin);
		K.position.right = K.position.left + field_size;
		K.position.bottom = K.position.top + field_size;
		keyboard.insert(std::make_pair(letter, K));
		keyboard[letter].color = new COLORREF[numOfBoards];
		for (int i = 0; i < numOfBoards; i++) keyboard[letter].color[i] = RGB(251, 252, 255);
	}
}

game_keyboard::~game_keyboard()
{
	for (std::pair<char, key> K : keyboard)
		delete[] K.second.color;
	
}

void game_keyboard::update_keyboard(char letter, int color, int k)
{
	switch(color)
	{
	case 0: //sukces
		keyboard.at(letter).color[k] = RGB(121, 184, 81);
		break;
	case 1: //drobne b³êdy
		keyboard.at(letter).color[k] = RGB(243, 194, 55);
		break;
	case 2: //niepowodzenie
		keyboard.at(letter).color[k] = RGB(164, 174, 196);
		break;
	default:
		keyboard.at(letter).color[k] = RGB(251, 252, 255);
		break;
	}
}