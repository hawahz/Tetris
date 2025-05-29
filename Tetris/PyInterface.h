#pragma once
#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <queue>
#include "ASCIIRenderer.h"
#include "Tetris.h"
#define UPDATE_RATE 5

class PyInterface {
public:
	tetris::Tetris* game = nullptr;
	void init();
};

static PyInterface* pyInter = nullptr;
static ASCIIRenderer* renderer = nullptr;
static float average = 0;

void renderMaxScore(AbstractRenderer&);
float weightEvaluate(tetris::Tetris*);

struct XxWeight {
	int x;
	float weight;

};

namespace pyinter {
	PyInterface* getInter();
	void init();
	void run();
	void step();
	std::vector<int> getMap();
	void autoPlay();
	bool isSolid(int x, int y);
}

