#include "Tetris.h"
#define KEY_d 100
#define KEY_D 68
#define KEY_a 97
#define KEY_A 65
#define KEY_s 115
#define KEY_S 83
#define KEY_r 114
#define KEY_R 113
#define KEY_SPACE 32

tetris::Box::Box(int shape0, int shape1): posX(0), posY(-4), shape(nullptr){
	shapes[0] = shape0;
	shapes[1] = shape1;
}

tetris::Box::Box(const Box* shape, int posX, int posY) : posX(posX), posY(posY), shape(shape) {
	shapes[0] = 0;
	shapes[1] = 0;
}

tetris::Box::Box() : posX(tetris::game->subWidth / 2 - 2), posY(0), shape(&boxes[std::rand() % 7]){
	shapes[0] = 0;
	shapes[1] = 0;
	state = tetris::Rotate(std::rand() % 4);
}

bool tetris::Box::isPixelValid(int x, int y) {
	return isPixelValidLocal(x + posX, y + posY);
}

bool tetris::Box::isPixelValidLocal(int x, int y) {
	int idx = 31 - (y * 4 + x + state % 2 * 16);
	return ((shape->shapes[state / 2] >> idx) & 1);
}

bool tetris::Box::isValidPos() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (isPixelValidLocal(i, j) && !tetris::game->mapValid(i + this->posX, j + this->posY))
				return false;
		}
	}
	return true;
}

void tetris::Box::render(AbstractRenderer* renderer) {
	renderShadow(renderer);
	for (int i = 0; i < 16; i++) {
		int c = 255 * ((shapes[0] ? shapes[state / 2] : shape->shapes[state / 2]) >> (32 - (i + state % 2 * 16)) & 1);
		int idx = 31 - (i + state % 2 * 16);
		char ch = (((shapes[0] ? shapes[state / 2] : shape->shapes[state / 2]) >> idx) & 1);
		renderer->renderSubPixel(posX + i % 4, posY + i / 4, renderer->blockColor * (((shapes[0] ? shapes[state / 2] : shape->shapes[state/2]) >> idx) & 1));
	}
}

void tetris::Box::renderShadow(AbstractRenderer* renderer) {
	int shadowY = this->posY;
	while (this->isValidPos()) {
		this->posY++;
	}
	this->posY--;
	if (this->posY == shadowY)
		return;
	for (int i = 0; i < 16; i++) {
		int c = 255 * ((shapes[0] ? shapes[state / 2] : shape->shapes[state / 2]) >> (32 - (i + state % 2 * 16)) & 1);
		int idx = 31 - (i + state % 2 * 16);
		char ch = (((shapes[0] ? shapes[state / 2] : shape->shapes[state / 2]) >> idx) & 1);
		renderer->renderSubPixel(posX + i % 4, posY + i / 4, renderer->shadowColor * (((shapes[0] ? shapes[state / 2] : shape->shapes[state / 2]) >> idx) & 1));
	}
	this->posY = shadowY;
}

void tetris::Box::rotate(int dir) {
	this->state = tetris::Rotate((this->state + dir < 0 ? this->state + dir + 4 : this->state + dir) % 4);
	if (this->isValidPos())
		return;
	this->state = tetris::Rotate((this->state - dir < 0 ? this->state - dir + 4 : this->state - dir) % 4);
}

tetris::Tetris::Tetris(AbstractRenderer* renderer) : renderer(renderer), subHeight(0), subWidth(0) {
	renderer->setSubWindow(3, 3, renderer->getHeight() - 6, renderer->getWidth() - 6);
	tetris::game = this;
	map = new unsigned int[subHeight]{0};
	for (int i = 0; i < (int) subHeight; i++) {
		map[i] = 0;
	}
	currentBox = new Box();
}

void tetris::Tetris::logicUpdate() {
	if (!fall()) {
		for (int x = 0; x < 4; x++) {
			for (int y = 0; y < 4; y++) {
				map[y + this->currentBox->posY] |= this->currentBox->isPixelValidLocal(x, y) << (x + this->currentBox->posX);
			}
		}
		delete this->currentBox;
		this->currentBox = new Box();
		this->eliminateTest();
		if (map[1])
			gameover = true;
	}
}

void tetris::Tetris::renderUpdate() {
	if (this->pause) {
		/*
		for (int i = 0; i < 6; i++) {
			this->renderer->renderCharacter(renderer->startX + renderer->subWidth / 2 - 3 + i, this->subHeight / 3, "paused"[i]);
		}
		*/
		this->renderer->renderString(renderer->startX + renderer->subWidth / 2 - 3, this->subHeight / 3, "paused");
		renderer->update();
		return;
	}
	if (this->gameover) {
		/*
		for (int i = 0; i < 8; i++) {
			this->renderer->renderCharacter(renderer->startX + renderer->subWidth / 2 - 4 + i, this->subHeight / 3, "gameover"[i]);
		}
		*/
		this->renderer->renderString(renderer->startX + renderer->subWidth / 2 - 4, this->subHeight / 3, "gameover");
		renderer->update();
		return;
	}
	renderer->clear();
	for (auto func : renderEvents) {
		func(*this->renderer);
	}
	for (int i = 0; i < (int) this->subWidth + 2; i++) {
		renderer->renderPixel(renderer->startX - 1 + i, renderer->startY - 1, renderer->wallColorTop);
	}
	for (int i = 0; i < (int) this->subHeight; i++) {
		renderer->renderPixel(renderer->startX - 1, i + renderer->startY, renderer->wallColorSide);
		renderer->renderPixel(renderer->startX + renderer->subWidth, i + renderer->startY, renderer->wallColorSide);
	}
	for (int i = 0; i < (int) this->subWidth + 2; i++) {
		renderer->renderPixel(renderer->startX - 1 + i, renderer->startY + this->subHeight, renderer->wallColorBottom);
	}
	for (int i = 0; i < (int) this->subHeight; i++) {
		if (this->map[i]) {
			for (int j = 0; j < (int) this->subWidth; j++) {
				renderer->renderPixel(renderer->startX + j, renderer->startY + i, renderer->blockColor * !mapValid(j, i));
			}
		}
	}
	if (this->score < 100000) {
		int sc = this->score;
		/*
		int pos = 0;
		if (sc == 0) {
			this->renderer->renderCharacter(pos + SCORE_X, 10, '0');
			pos--;
		}
		while (sc > 0) {
			this->renderer->renderCharacter(pos + SCORE_X, 10, ('0' + (sc % 10)));
			sc /= 10;
			pos--;
		}
		
		for (int i = 0; i < 6; i++) {
			this->renderer->renderCharacter(pos + SCORE_X - i, 10, "score:"[5 - i]);
		}
		*/
		int sX = this->subWidth + 2 + this->renderer->startX + 3;
		this->renderer->renderString(sX, 10, "score:");
		this->renderer->renderString(sX + 6, 10, std::to_string(sc));
		
	}
	
	this->currentBox->render(renderer);

	

	renderer->update();
}

void tetris::Tetris::eliminateTest() {
	for (int i = this->subHeight - 1; i + 1; i--) {
		if (this->map[i] == full) {
			this->map[i] = 0;
			for (int j = i - 1; j + 1; j--) {
				this->map[j + 1] = this->map[j];
			}
			this->map[0] = 0;
			this->score += 10;
			i++;
		}
	}
}

bool tetris::Tetris::fall() {
	for (int x = 0; x < 4; x++) {
		for (int y = 3; y + 1; y--) {
			if (this->currentBox->isPixelValidLocal(x, y)) {
				if (!mapValid(x + this->currentBox->posX, y + this->currentBox->posY + 1))
					return false;
				else
					break;
			}
		}
	}
	this->currentBox->posY++;
	return true;
}

std::pair<int, int> tetris::Tetris::search(float (*fx)(unsigned int*)) {

	return std::pair<int, int>();
}

float tetris::Tetris::searchExtruder(float(*fx)(Tetris*)) {
	int prevY = this->currentBox->posY;
	while (fall()) {
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			map[y + this->currentBox->posY] |= this->currentBox->isPixelValidLocal(x, y) << (x + this->currentBox->posX);
		}
	}

	float sc = fx(this);

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			map[y + this->currentBox->posY] &= ~(this->currentBox->isPixelValidLocal(x, y) << (x + this->currentBox->posX));
		}
	}

	this->currentBox->posY = prevY;

	return sc;
}

int tetris::Tetris::scoreExtruder() {
	int prevY = this->currentBox->posY;
	while (fall()) {
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			map[y + this->currentBox->posY] |= this->currentBox->isPixelValidLocal(x, y) << (x + this->currentBox->posX);
		}
	}

	int sc = eliminateCount();

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			map[y + this->currentBox->posY] &= ~(this->currentBox->isPixelValidLocal(x, y) << (x + this->currentBox->posX));
		}
	}

	this->currentBox->posY = prevY;

	return sc;
}

int tetris::Tetris::heightExtruder() {
	int prevY = this->currentBox->posY;
	int height = 0;
	while (fall()) {
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			map[y + this->currentBox->posY] |= this->currentBox->isPixelValidLocal(x, y) << (x + this->currentBox->posX);
		}
	}

	for (int i = 0; i < (int) this->subHeight; i++) {
		if (this->map[i] != 0) {
			height = subHeight - i;
			break;
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			map[y + this->currentBox->posY] &= ~(this->currentBox->isPixelValidLocal(x, y) << (x + this->currentBox->posX));
		}
	}

	this->currentBox->posY = prevY;

	return height;
}

int tetris::Tetris::eliminateCount() {
	int score = 0;
	for (int i = this->subHeight - 1; i + 1; i--) {
		if (this->map[i] == full) {
			score += 10;
		}
	}
	return score;
}

inline bool tetris::Tetris::mapValid(int x, int y) {
	return y < (int) subHeight && x < (int) subWidth && y + 1 && x + 1 && !((int) map[y] >> x & 1);
}

void tetris::Tetris::move(int step) {
	this->currentBox->posX += step;
	if (this->currentBox->isValidPos())
		return;
	this->currentBox->posX -= step;
}

void tetris::Tetris::reset() {
	delete this->map;
	this->map = new unsigned int[subHeight] {0};
	this->score = 0;
}

void tetris::Tetris::update(int delta) {
	if (pause) {
		if (_kbhit() && _getch() == 'p')
			pause = false;
		else {
			renderUpdate();
			return;
		}
	}
	if (gameover) {
		if (_kbhit()) {
			gameover = false;
			reset();
			return;
		}
		else {
			renderUpdate();
			return;
		}
	}
	gameTick++;
	if (gameTick >= loopLen / delta * 10) {
		gameTick = 0;
		logicUpdate();
	}
	if (gameTick == loopLen / delta * 10 / 2) {
		renderUpdate();
	}
	if (_kbhit()) {//如果有按键按下，则_kbhit()函数返回真
		int ch = _getch();//使用_getch()函数获取按下的键值
		switch (ch) {
		case 'A':
		case 'a':
			move(-1);
			break;
		case 'D':
		case 'd':
			move(1);
			break;
		case 'R':
		case 'r':
			this->currentBox->rotate();
			break;
		case 'S':
		case 's':
			this->logicUpdate();
			gameTick = 0;
			break;
		case ' ':
			while (fall()) {
			}
			break;
		case 'P':
		case 'p':
			pause = true;
			break;
		default:
			return;
		}
		this->renderUpdate();
	}
}

void tetris::Tetris::setSubWindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
	this->subWidth = width;
	this->subHeight = height;
	this->renderer->setSubWindow(x, y, width, height);
	map = new unsigned int[subHeight] {0};
	this->currentBox->posX = this->subWidth / 2 - 2;
	full = 0;
	for (int i = 0; i < (int) subWidth; i++) {
		full |= 1 << i;
	}
}
