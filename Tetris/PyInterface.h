#pragma once
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <queue>
#include "ASCIIRenderer.h"
#include "Tetris.h"
#define UPDATE_RATE 5

class PyInterface {
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

struct XxWeight {
	int x;
	float weight;

};

