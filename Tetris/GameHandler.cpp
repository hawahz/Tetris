#include "GameHandler.h"

void GameHandler::init(int seed) {
    std::srand(seed);
    if (this->game) {
        delete this->game;
        this->game = nullptr;
    }
    if (!this->renderer) {
        renderer = new ASCIIRenderer(40, 34);
    }
    average = 0;
    this->game = new tetris::Tetris(renderer);
    
    this->game->setSubWindow(3, 3, 10, 20);
}

void GameHandler::run() {
    if (!this->game) {
        std::cerr << "[MAIN] No Game Handle";
        return;
    }
    tetris::Tetris* g = game;
    std::cout << "[MAIN] Waiting for game start..." << std::endl;
    Sleep(1000);
    std::cout << "[MAIN] Game start successful" << std::endl;

    std::cout << "[MAIN] Attaching logic thread..." << std::endl;
    std::thread logicThread([g]() {
        short time = 0;
        while (true) {
            if (g->killed) {
                return;
            }
                
            time++;
            if (!(time %= 20)) {
                g->logicUpdate();
            }
            Sleep(5);
            if (!g->cmd)
                continue;
            inputHandler(g, time);
        }
    });
    logicThread.detach();

    std::cout << "[MAIN] Attaching render thread..." << std::endl;
    std::thread renderThread([g]() {
        while (true) {
            if (g->killed)
                return;
            g->renderUpdate();
            Sleep(5);
        }
        });
    renderThread.detach();

    std::cout << "[MAIN] Attaching keyboard handler thread..." << std::endl;
    std::thread keyboardThread([g]() {
        while (true) {
            if (g->killed)
                return;
            g->keyboardCapture();
        }
        });

    std::cout << "[MAIN] Game Ready!" << std::endl;
    keyboardThread.join();
}

bool inputHandler(tetris::Tetris* g, short& time) {
    bool flag = true;
    switch (g->cmd) {
    case 'A':
    case 'a':
        g->move(-1);
        break;
    case 'D':
    case 'd':
        g->move(1);
        break;
    case 'R':
    case 'r':
        g->currentBox->rotate();
        break;
    case 'S':
    case 's':
        g->logicUpdate();
        time = 0;
        break;
    case ' ':
        while (g->fall()) {
        }
        break;
    case 'P':
    case 'p':
        g->pause = !g->pause;
        break;
    case 'e':
    case 'E':
        g->killed = true;
        break;
    case VK_ESCAPE:
        g->killed = true;
        break;
    default:
        flag = false;
        auto ptr = callableFunctions.find((char)g->cmd);
        if (ptr != callableFunctions.end()) {
            callableFunctions.at(g->cmd)(g);
        }
    }
    if (g->cmd != 'p' && g->cmd != 'P') {
        g->pause = false;
    }

    g->cmd = 0;
    return flag;
}

void GameHandler::step() {
    if (!this->game)
        return;
    tetris::Tetris* g = game;
    Sleep(1000);
    std::thread keyboardThread([g]() {
        while (true) {
            if (g->killed)
                return;
            g->keyboardCapture();
        }
        });
    keyboardThread.detach();
    this->game->renderUpdate();
    short time = 0;
    while (true) {
        if (this->game->cmd == 0)
            continue;
        if (!inputHandler(this->game, time))
            continue;
        ;
        this->game->logicUpdate();
        this->game->renderUpdate();
    }
}

std::vector<int> GameHandler::getMap() {
    if (!this->game)
        return std::vector<int>();
    std::vector<int> ret;
    for (int i = 0; i < this->game->subHeight; i++) {
        ret.push_back(this->game->map[i]);
    }

    return ret;
}

void GameHandler::rotate() {
    this->game->currentBox->rotate();
}

void GameHandler::move(int dir) {
    this->setPos(this->game->currentBox->posX + dir, this->game->currentBox->state);
}

void GameHandler::autoPlay() {
    if (!this->game)
        return;
    this->game->renderEvents.push_back([](AbstractRenderer& r) -> void {
            r.renderString(r.subWidth + 2 + r.startX + 3, 11, "avg:");
            r.renderString(r.subWidth + 2 + r.startX + 3 + 4, 11, std::to_string(0));
        });
    int times = 0;
    while (1) {
        if (this->game->killed) {
            delete this->game;
            this->game = nullptr;
            return;
        }
        if (this->game->gameover) {
            average = (this->game->score + average * times)/float(times+1);
            this->game->gameover = false;
            this->game->reset();
            times++;
        }
        int x = 0, score = -1000000;
        tetris::Rotate rm = this->game->currentBox->state, rHm = this->game->currentBox->state;
        this->game->currentBox->posX = 0;
        for (int j = 0; j < 4; j++) {
            this->game->currentBox->state = tetris::Rotate(j);
            this->game->currentBox->posX = -2;
            for (int i = 0; i < this->game->subWidth + 4; i++, this->game->currentBox->posX++) {
                if (!this->game->currentBox->isValidPos())
                    continue;
                //this->game->renderUpdate();
                int sc = this->game->searchExtruder(weightEvaluate);
                //Sleep(10);
                if (score < sc) {
                    x = this->game->currentBox->posX;
                    score = sc;
                    rm = tetris::Rotate(this->game->currentBox->state);
                }
                
            }
        }
        this->game->currentBox->posX = x;
        this->game->currentBox->state = rm;







        while (this->game->fall()) {
        }
        //Sleep(50);
    }
}

bool GameHandler::isSolid(int x, int y) {
    if (!this->game)
        return false;
    return this->game->mapValid(x, y);
}

bool GameHandler::gameover() {
    return this->game->gameover;
}

void GameHandler::exit() {
    delete this->game;
    this->game = nullptr;
}

void GameHandler::restart() {
    this->game->reset();
}

void GameHandler::setPos(int x, int r) {
    this->game->setPos(x, r);
}

void GameHandler::renderAvgScore(AbstractRenderer& r) {
    r.renderString(r.subWidth + 2 + r.startX + 3, 11, "avg:");
    r.renderString(r.subWidth + 2 + r.startX + 3 + 4, 11, std::to_string(average));
}

std::vector<int> GameHandler::getInfo() {
    if (this->game) {
        return std::vector<int>(this->game->getInfo());
    }
    return std::vector<int>({0, 0, 0, 0, 0});
}

float weightEvaluate(tetris::Tetris* game) {

    std::vector<int> info = game->getInfo();
    float weight = -info[1];
    //TODO ”≈ªØ
    int height = info[0];

    int bumpiness = info[2];

    return weight * 2 - height * height * height - bumpiness;
}
