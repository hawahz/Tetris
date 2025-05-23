#pragma once
#include <vector>
#include <pybind11/pybind11.h>
#include <queue>
#include "ASCIIRenderer.h"
#include "Tetris.h"
#define UPDATE_RATE 5

typedef struct {
	int x;
	float weight;
} XxWeight;



class PyInterface {
public:
	tetris::Tetris* game = nullptr;
	void init();
};

static PyInterface* pyInter = nullptr;
static ASCIIRenderer* renderer = nullptr;
static int maxScore = 0;

void renderMaxScore(AbstractRenderer&);
float weightEvaluate();

namespace pyinter {
	void init();
	void run();
	void step();
	std::vector<int> getMap();
	void autoPlay();
}

