#pragma once
#include "AbstractRenderer.h"
#include <random>
#include <conio.h>
#include <queue>
#include <iostream>
#include <Windows.h>
#include <thread>
#define SCORE_X 30

namespace tetris {
	class Tetris;
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
		const Box* shape;
		const int shapeId;
		tetris::Tetris* const parent;
	public:
		int posX, posY;
		Box(int shape0, int shape1);
		Box(int shapeId, tetris::Tetris*, int posX = 0, int posY = 0);
		Box(tetris::Tetris*);
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

		std::atomic<char> cmd = 0;

		/*
	
		void keyboardCapture();
		�����������
		
		*/
		void keyboardCapture();

		/*
		*
		* std::vector<void (*)(AbstractRenderer&)> renderEvents
		* �ڴ��������е����к�����������Ļ���������������ִ�л��Ʋ���
		*
		*/
		std::vector<void (*)(AbstractRenderer&)> renderEvents;
		bool fall();
		bool pause = false;
		bool isPiped;
		bool gameover = false;
		bool killed = false;
		std::queue<int> actions;
		
	public:
		Tetris(AbstractRenderer* renderer);
		unsigned int subWidth, subHeight;
		static int width, height;
		unsigned int* map;
		int score = 0;
		int full = 0;

		/*
		*
		* std::pair<int, int> searchExtruder(float (*)(unsigned int* map))
		* ��������ʽ���������ݺ�����ֵ�ҵ����λ��
		* ����ֵ��һλ��λ�ã��ڶ�λ����ת״̬
		*
		*/
		std::pair<int, int> search(float (*)(unsigned int* map));


		float searchExtruder(float (*fx)(Tetris*));

		/*
		* 
		* int scoreExtruder()
		* ���㵱ǰλ��������㣬���ݴ˽������������еĵ÷�
		* ��ȡ���ݵĲ��Զ�������Ԥ��
		* ����ֵ�Ǹ�������ջ�õķ���
		* 
		*/
		int scoreExtruder();

		/*
		*
		* int heightExtruder()
		* ���㵱ǰλ��������㣬���ݴ˽�����������ĸ߶�
		* �� scoreExtruder() ��ȡ��ͬ�Ĳ���
		* ����ֵ�Ǹ��������ȥ���п����к�ĸ߶�
		*
		*/
		int heightExtruder();

		/*
		*
		* int eliminateCount()
		* ���㵱ǰ��Ϸ״̬�ĵ÷�
		* ���ص÷�
		*
		*/
		int eliminateCount();

		/*
		*
		* bool mapValid(int x, int y)
		* ������(x, y)���괦�Ƿ��пռ�
		*
		*/
		inline bool mapValid(int x, int y);

		/*
		*
		* void update(int delta)
		* ����Ϸ״̬���и���, deltaֵ���������ٶ�
		*
		*/
		//void update(int delta, bool autoLoop = true);

		/*
		*
		* void move(int step)
		* ����Ϸ״̬���и���, deltaֵ���������ٶ�
		*
		*/
		void move(int step);

		/*
		*
		* void reset()
		* ������Ϸ״̬
		*
		*/
		void reset();

		void setPos(int x, int rotate);

		/*
		*
		* std::vector<int> getInfo()
		* ��ȡ��Ϸ״̬
		* height, holes, bumpiness, score/10
		*
		*/
		std::vector<int> getInfo();
		void setSubWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	};
}




