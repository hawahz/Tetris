#pragma once
#include <vector>
#include <queue>
#include <map>
#include "ASCIIRenderer.h"
#include "Tetris.h"
#define UPDATE_RATE 5

const std::map<char, void (*)(tetris::Tetris*)> callableFunctions{
	{'1', [](tetris::Tetris* game) {
		std::cout << "[Callback] ";
		for (int i = 0; i < game->subHeight; i++) {
			std::cout << game->map[i] << ' ';
		}
		std::cout << std::endl;
		}},//get_map()
	{ '2', [](tetris::Tetris* game) {
		std::cout << "[Callback] ";
		std::cout << game->currentBox->shapeId << std::endl;
	}},//get_current_piece
	{ '3', [](tetris::Tetris* game) {
		std::cout << "[Callback] ";
		std::cout << "(" << game->currentBox->posX << ',' << game->currentBox->posY << ")";
	}},//get_piece_pos
	{ '4', [](tetris::Tetris* game) {
		std::cout << "[Callback] " << game->currentBox->state << std::endl;
	}},//get_piece_rotate
};

class GameHandler {
public:
	float average = 0;
	tetris::Tetris* game = nullptr;
	AbstractRenderer* renderer = nullptr;
public:
	void init(int seed = std::time(nullptr));
	void run();
	void step();
	std::vector<int> getMap();
	void rotate();
	void move(int);
	void autoPlay();
	bool isSolid(int x, int y);
	bool gameover();
	void exit();
	void restart();
	void setPos(int x, int r);
	void renderAvgScore(AbstractRenderer&);
	std::vector<int> getInfo();
};


float weightEvaluate(tetris::Tetris*);

bool inputHandler(tetris::Tetris* g, short& time);

struct XxWeight {
	int x;
	float weight;

};

