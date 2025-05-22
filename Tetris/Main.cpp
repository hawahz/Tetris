#include <iostream>
#include <string>
#include <Windows.h>
#include <stdlib.h>
#include "ASCIIRenderer.h"
#include "Tetris.h"
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#define UPDATE_RATE 5

void init();
void aiPlay();

void init() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    if (!GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo)) {
        std::cerr << "获取字体信息失败。错误代码：" << GetLastError() << std::endl;
        return;
    }

    // 设置字体为Consolas，可根据需要更改为其他等宽字体
    wcscpy_s(fontInfo.FaceName, L"Terminal");
    // 可选：设置字体大小，确保等宽等高
    fontInfo.dwFontSize.X = 16;  // 宽度
    fontInfo.dwFontSize.Y = 16; // 高度
    if (!SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo)) {
        std::cerr << "设置字体失败。错误代码：" << GetLastError() << std::endl;
    }
}



ASCIIRenderer renderer(40, 34);
tetris::Tetris game(&::renderer);
bool initialized = false;

int main() {
    
    //std::string charList = " .,-'`:!1+*abcdefghijklmnopqrstuvwxyz<>()\\/{}[]?234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ%&@#$";
    /*
    std::srand(std::time(nullptr));
    init();
    
    
    ::game.setSubWindow(3, 3, 10, 20);
    init();
    */
    if (true) {
        ::game.update(50);
        aiPlay();
    }
    for (int p = 0; ; p++) {
        ::game.update(UPDATE_RATE);
        
        Sleep(UPDATE_RATE);
    }
    
    return 0;
}

void run() {
    main();
}

void pyInit() {
    initialized = true;
    init();
    ::game.setSubWindow(3, 3, 10, 20);
}

void step() {
    if (!initialized) {
        pyInit();
    }
    ::game.update(50);
}

void setSubWindow(int startX, int startY, int width, int height) {
    ::game.setSubWindow(startX, startY, width, height);
}

std::vector<int> getMap() {

    std::vector<int> ret;
    for (int i = 0; i < ::game.subHeight; i++) {
        ret.push_back(::game.map[i]);
    }

    return ret;
}
int maxScore = 0;
void r() {
    ::game.renderer->renderString(::game.subWidth + 2 + ::game.renderer->startX + 3, 11, "m:");
    ::game.renderer->renderString(::game.subWidth + 2 + ::game.renderer->startX + 3 + 2, 11, std::to_string(maxScore));
}

void aiPlay() {
    if (!initialized) {
        pyInit();
    }
    
    ::game.renderEvent = r;
    while (true) {
        if (::game.gameover) {
            maxScore = max(::game.score, maxScore);
            ::game.gameover = false;
            ::game.reset();
        }

        int x = 0, score = 0, minHx = 0, minH = ::game.subHeight;
        tetris::Rotate rm = ::game.currentBox->state, rHm = ::game.currentBox->state;
        ::game.currentBox->posX = 0;
        for (int j = 0; j < 4; j++) {
            ::game.currentBox->state = tetris::Rotate(j);
            for (int i = 0; i < ::game.subWidth; i++, ::game.currentBox->posX++) {
                if (!::game.currentBox->isValidPos())
                    continue;
                int scEx = ::game.scoreExtruder(), hEx = ::game.heightExtruder();
                if (score < scEx) {
                    x = ::game.currentBox->posX;
                    score = scEx;
                    rm = tetris::Rotate(::game.currentBox->state);
                }

                if (minH > hEx) {
                    minHx = ::game.currentBox->posX;
                    minH = hEx;
                    rHm = tetris::Rotate(::game.currentBox->state);
                }
            }
        }
        if (score) {
            ::game.currentBox->posX = x;
            ::game.currentBox->state = rm;
        }
        else {
            ::game.currentBox->posX = minHx;
            ::game.currentBox->state = rHm;
        }
        
        while (::game.fall()) {
        }
        
        ::game.update(50);
        
        ::game.update(50);
        //Sleep(1);
    }
}

PYBIND11_MODULE(Tetris, m) {
    m.doc() = "The general function";
    m.def("init", &pyInit);
    m.def("run", &main);
    m.def("step", &step);
    m.def("get_map", &getMap);
    //m.def("set_sub_window", &setSubWindow);
}