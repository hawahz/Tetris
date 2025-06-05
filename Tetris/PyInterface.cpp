#include "PyInterface.h"

PYBIND11_MODULE(Tetris, m) {
    m.doc() = "The general function";
    pybind11::class_<PyInterface>(m, "TetrisGame")
        .def(pybind11::init<>())
        .def("init", &PyInterface::init, pybind11::arg("seed"))
        .def("run", &PyInterface::run)
        .def("get_map", &PyInterface::getMap)
        .def("auto_play", &PyInterface::autoPlay)
        .def("exit", &PyInterface::exit)
        .def("is_solid", &PyInterface::isSolid)
        .def("get_info", &PyInterface::getInfo)
        .def("step", &PyInterface::step)
        .def("set_pos", &PyInterface::setPos)
        .def("gameover", &PyInterface::gameover)
        .def("restart", &PyInterface::restart)
        .def("rotate", &PyInterface::rotate)
        .def("move", &PyInterface::move)

        .def("__repr__", [] (const PyInterface& p) {
            return "<tetris.TetrisGame>";
        });
    //m.def("set_sub_window", &setSubWindow);
}

void PyInterface::init(int seed) {
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

void PyInterface::run() {
    if (!this->game) {
        std::cerr << "No Game Handle";
        return;
    }
    for (int p = 0; ; p++) {
        if (this->game->killed) {
            delete this->game;
            this->game = nullptr;
            std::cerr << "Game Killed";
            return;
        }
        this->game->update(UPDATE_RATE);
        Sleep(UPDATE_RATE);
    }
}

void PyInterface::step() {
    if (!this->game)
        return;
    if (this->game->killed) {
        delete this->game;
        this->game = nullptr;
        return;
    }
    this->game->update(50);
}

std::vector<int> PyInterface::getMap() {
    if (!this->game)
        return std::vector<int>();
    std::vector<int> ret;
    for (int i = 0; i < this->game->subHeight; i++) {
        ret.push_back(this->game->map[i]);
    }

    return ret;
}

void PyInterface::rotate() {
    this->game->currentBox->rotate();
}

void PyInterface::move(int dir) {
    this->setPos(this->game->currentBox->posX + dir, this->game->currentBox->state);
}

void PyInterface::autoPlay() {
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
        /*
        int x = 0, score = 0, minHx = 0, minH = this->game->subHeight;
        tetris::Rotate rm = this->game->currentBox->state, rHm = this->game->currentBox->state;
        this->game->currentBox->posX = 0;
        for (int j = 0; j < 4; j++) {
            this->game->currentBox->state = tetris::Rotate(j);
            for (int i = 0; i < this->game->subWidth; i++, this->game->currentBox->posX++) {
                if (!this->game->currentBox->isValidPos())
                    continue;
                int scEx = this->game->scoreExtruder(), hEx = this->game->heightExtruder();
                if (score < scEx) {
                    x = this->game->currentBox->posX;
                    score = scEx;
                    rm = tetris::Rotate(this->game->currentBox->state);
                }

                if (minH > hEx) {
                    minHx = this->game->currentBox->posX;
                    minH = hEx;
                    rHm = tetris::Rotate(this->game->currentBox->state);
                }
            }
        }
        if (score) {
            this->game->currentBox->posX = x;
            this->game->currentBox->state = rm;
        }
        else {
            this->game->currentBox->posX = minHx;
            this->game->currentBox->state = rHm;
        }
        */
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

        this->game->update(50);
        this->game->update(50);
        //Sleep(50);
    }
}

bool PyInterface::isSolid(int x, int y) {
    if (!this->game)
        return false;
    return this->game->mapValid(x, y);
}

bool PyInterface::gameover() {
    return this->game->gameover;
}

void PyInterface::exit() {
    delete this->game;
    this->game = nullptr;
}

void PyInterface::restart() {
    this->game->reset();
}

void PyInterface::setPos(int x, int r) {
    this->game->setPos(x, r);
}

void PyInterface::renderAvgScore(AbstractRenderer& r) {
    r.renderString(r.subWidth + 2 + r.startX + 3, 11, "avg:");
    r.renderString(r.subWidth + 2 + r.startX + 3 + 4, 11, std::to_string(average));
}

std::vector<int> PyInterface::getInfo() {
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
