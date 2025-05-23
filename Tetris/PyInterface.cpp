#include "PyInterface.h"

PYBIND11_MODULE(Tetris, m) {
    m.doc() = "The general function";
    m.def("init", &pyinter::init);
    m.def("run", &pyinter::run);
    m.def("step", &pyinter::step);
    m.def("get_map", &pyinter::getMap);
    m.def("auto_play", &pyinter::autoPlay);
    //m.def("set_sub_window", &setSubWindow);
}

void PyInterface::init() {
}

void pyinter::init() {
    if (!::pyInter) {
        ::pyInter = new PyInterface();
    }
    if (::pyInter->game) {
        delete ::pyInter->game;
    }
    if (!::renderer) {
        renderer = new ASCIIRenderer(40, 34);
    }
    maxScore = 0;
    ::pyInter->game = new tetris::Tetris(renderer);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    if (!GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo)) {
        std::cerr << "��ȡ������Ϣʧ�ܡ�������룺" << GetLastError() << std::endl;
        return;
    }

    // ��������ΪConsolas���ɸ�����Ҫ����Ϊ�����ȿ�����
    wcscpy_s(fontInfo.FaceName, L"Terminal");
    // ��ѡ�����������С��ȷ���ȿ�ȸ�
    fontInfo.dwFontSize.X = 16;  // ���
    fontInfo.dwFontSize.Y = 16; // �߶�
    if (!SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo)) {
        std::cerr << "��������ʧ�ܡ�������룺" << GetLastError() << std::endl;
    }
    ::pyInter->game->setSubWindow(3, 3, 10, 20);
}

void pyinter::run() {
    for (int p = 0; ; p++) {
        ::pyInter->game->update(UPDATE_RATE);
        Sleep(UPDATE_RATE);
    }
}

void pyinter::step() {
    ::pyInter->game->update(50);
}

std::vector<int> pyinter::getMap() {
    std::vector<int> ret;
    for (int i = 0; i < ::pyInter->game->subHeight; i++) {
        ret.push_back(::pyInter->game->map[i]);
    }

    return ret;
}

void pyinter::autoPlay() {
    ::pyInter->game->renderEvents.push_back(renderMaxScore);
    while (true) {
        if (::pyInter->game->gameover) {
            maxScore = max(::pyInter->game->score, maxScore);
            ::pyInter->game->gameover = false;
            ::pyInter->game->reset();
        }

        int x = 0, score = 0, minHx = 0, minH = ::pyInter->game->subHeight;
        tetris::Rotate rm = ::pyInter->game->currentBox->state, rHm = ::pyInter->game->currentBox->state;
        ::pyInter->game->currentBox->posX = 0;
        for (int j = 0; j < 4; j++) {
            ::pyInter->game->currentBox->state = tetris::Rotate(j);
            for (int i = 0; i < ::pyInter->game->subWidth; i++, ::pyInter->game->currentBox->posX++) {
                if (!::pyInter->game->currentBox->isValidPos())
                    continue;
                int scEx = ::pyInter->game->scoreExtruder(), hEx = ::pyInter->game->heightExtruder();
                if (score < scEx) {
                    x = ::pyInter->game->currentBox->posX;
                    score = scEx;
                    rm = tetris::Rotate(::pyInter->game->currentBox->state);
                }

                if (minH > hEx) {
                    minHx = ::pyInter->game->currentBox->posX;
                    minH = hEx;
                    rHm = tetris::Rotate(::pyInter->game->currentBox->state);
                }
            }
        }
        if (score) {
            ::pyInter->game->currentBox->posX = x;
            ::pyInter->game->currentBox->state = rm;
        }
        else {
            ::pyInter->game->currentBox->posX = minHx;
            ::pyInter->game->currentBox->state = rHm;
        }

        while (::pyInter->game->fall()) {
        }

        ::pyInter->game->update(50);

        ::pyInter->game->update(50);
    }
}

void renderMaxScore(AbstractRenderer& r) {
    r.renderString(::pyInter->game->subWidth + 2 + ::pyInter->game->renderer->startX + 3, 11, "max:");
    r.renderString(::pyInter->game->subWidth + 2 + ::pyInter->game->renderer->startX + 3 + 4, 11, std::to_string(maxScore));
}
