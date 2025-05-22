#pragma once
#pragma comment(lib, "user32.lib")
#include "AbstractRenderer.h"
#include <string>
#include <iostream>
#include <Windows.h>

/*
* ASCII Renderer
* Draw all elements by using ASCII in terminal
*/

typedef struct {
	unsigned int x;
	unsigned int y;
} coord;
const std::string charList = " -+o*#&@|O_";
//0    1 -  2 +  3 o  4 *  5 #  6 &  7 @ 8 |
//" -+>*#&@";
//" .,-'`:!1+*abcdefghijklmnopqrstuvwxyz<>()\\/{}[]?234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ%&@#$"
const int charSize = charList.size();

void DisableQuickEditMode();
void setConsoleWindowSize(SHORT width, SHORT height);

class BiStorage {
	
public:
	unsigned int* biIntChain;
	const int width;
	const int height;
	const int chainLen;
	BiStorage(int width, int height);
	void turnOn(int x, int y);
	void turnOff(int x, int y);
	bool operator[](coord pos);
	class BiOp {
		const int x, y;
		BiStorage& par;
	public:
		BiOp(int x, int y, BiStorage& par);
		BiOp operator[](coord pos);
		void operator=(bool flag);
	};
	BiOp bi();
};

class ASCIIRenderer : public AbstractRenderer {
	unsigned int scale = 1;
	std::string displayText;
	BiStorage biStorage;
public:
	ASCIIRenderer(int width, int height);
	void renderBox(int x, int y, abr::RenderType type = abr::RenderType::MAIN);
	void clear() override;
	void update() override;
	void renderPixel(int x, int y, short color) override;
	void renderPixel(int x, int y, char type);
	void renderSubPixel(int x, int y, short color) override;
	void renderCharacter(int x, int y, char c) override;
	void renderString(int x, int y, std::string message) override;
};

