#pragma once
#include "AbstractRenderer.h"
#include <random>
#include <conio.h>
#include <queue>
#include <Python.h>
#define SCORE_X 30

namespace tetris {
	
	enum Rotate {
		UP,
		LEFT,
		DOWN,
		RIGHT
	};

	class Box {
	public:
		int shapes[2];
		Rotate state = UP;
		const Box* const shape;
	public:
		int posX, posY;
		Box(int shape0, int shape1);
		Box(const Box* shape, int posX = 0, int posY = 0);
		Box();
		bool isPixelValid(int x, int y);
		bool isPixelValidLocal(int x, int y);
		bool isValidPos();
		void render(AbstractRenderer* renderer);
		void renderShadow(AbstractRenderer* renderer);
		void rotate(int dir = 1);
		
	};
	const Box boxes[] = {
		Box(
			0b00000000111100000010001000100010,
			0b00000000111100000010001000100010
		),
		Box(
			0b00001110001000000100010011000000,
			0b10001110000000000110010001000000
		),
		Box(
			0b00001110100000001100010001000000,
			0b00101110000000000100010001100000
		),
		Box(
			0b00000110011000000000011001100000,
			0b00000110011000000000011001100000
		),
		Box(
			0b00000110110000000100011000100000,
			0b00000110110000000100011000100000
		),
		Box(
			0b00001110010000000100110001000000,
			0b01001110000000000100011001000000
		),
		Box(
			0b00001100011000000010011001000000,
			0b00001100011000000010011001000000
		),
	};

	class Tetris {
	public:
		unsigned int gameTick = 0;
		unsigned int loopLen = 10;
		
		AbstractRenderer* renderer;
		Box* currentBox;

		void logicUpdate();
		void renderUpdate();
		void eliminateTest();
		std::vector<void (*)(AbstractRenderer&)> renderEvents;
		bool fall();
		bool pause = false;
		bool gameover = false;
		std::queue<int> actions;
		
	public:
		Tetris(AbstractRenderer* renderer);
		unsigned int subWidth, subHeight;
		unsigned int* map;
		int score = 0;
		int full = 0;

		/*
		* 
		* int scoreExtruder()
		* 计算当前位置最终落点，并据此结果计算该落点可行的得分
		* 采取回溯的策略对落点进行预测
		* 返回值是该落点最终获得的分数
		* 
		*/
		int scoreExtruder();

		/*
		*
		* int heightExtruder()
		* 计算当前位置最终落点，并据此结果计算该落点后的高度
		* 和 scoreExtruder() 采取相同的策略
		* 返回值是该落点在消去所有可消行后的高度
		*
		*/
		int heightExtruder();

		/*
		*
		* int eliminateCount()
		* 计算当前游戏状态的得分
		* 返回得分
		*
		*/
		int eliminateCount();

		/*
		*
		* bool mapValid(int x, int y)
		* 计算在(x, y)坐标处是否有空间
		*
		*/
		inline bool mapValid(int x, int y);

		/*
		*
		* void update(int delta)
		* 对游戏状态进行更新, delta值决定更新速度
		*
		*/
		void update(int delta);

		/*
		*
		* void move(int step)
		* 对游戏状态进行更新, delta值决定更新速度
		*
		*/
		void move(int step);

		/*
		*
		* void reset()
		* 重置游戏状态
		*
		*/
		void reset();
		void setSubWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	};

	static Tetris* game;
}




