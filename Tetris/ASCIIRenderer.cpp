#include "ASCIIRenderer.h"

void DisableQuickEditMode() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE) {
        std::cerr << "获取控制台输入句柄失败。错误代码: " << GetLastError() << std::endl;
        return;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(hInput, &mode)) {
        std::cerr << "获取控制台模式失败。错误代码: " << GetLastError() << std::endl;
        return;
    }

    // 禁用快速编辑模式（ENABLE_QUICK_EDIT_MODE）
    // 并确保启用扩展标志（ENABLE_EXTENDED_FLAGS）
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS;

    if (!SetConsoleMode(hInput, mode)) {
        std::cerr << "设置控制台模式失败。错误代码: " << GetLastError() << std::endl;
    }
}

void setConsoleWindowSize(SHORT width, SHORT height) {
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE); //获取输出句柄
    SMALL_RECT wrt = { 0, 0, width, height - 1 }; //设置宽高
    bool ret = SetConsoleWindowInfo(hOutput, TRUE, &wrt); // 设置窗体尺寸
    if (!ret) return;
    COORD coord = { width + 1, height };
    ret = SetConsoleScreenBufferSize(hOutput, coord);
    return;
}

ASCIIRenderer::ASCIIRenderer(int width, int height) :AbstractRenderer(width, height), biStorage(width, height) {
    this->wallColorSide = 8;
    this->wallColorBottom = 5;
    this->blockColor = 7;
    this->shadowColor = 9;
    this->wallColorTop = 10;
    if (!window.start()) {
        std::cerr << "无法创建显示窗口\n";
    }
    /*
    setConsoleWindowSize(width, height);
    
    DisableQuickEditMode();
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cInfo{};
    GetConsoleCursorInfo(hOutput, &cInfo); //获取现有光标信息
    cInfo.bVisible = false; //false为隐藏窗口,true为显示窗口
    SetConsoleCursorInfo(hOutput, &cInfo);  //重新设置光标信息

    HWND hWnd = GetConsoleWindow(); //获取窗口句柄
    LONG_PTR sty = GetWindowLongPtrA(hWnd, GWL_STYLE); //获取窗口样式
    sty = sty & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX; //去除可变化大小,最大,最小化按钮,~是取反,&是与,这是位运算相关的知识了
    SetWindowLongPtrA(hWnd, GWL_STYLE, sty); //设置窗体不可更改大小,不可最大化
    */
    
    this->displayText = std::string(height * width, ' ');
}

void ASCIIRenderer::renderBox(int x, int y, abr::RenderType type) {
}

void ASCIIRenderer::clear() {
    for (int i = 0; i < this->biStorage.chainLen; i++) {
        if (unsigned int g = this->biStorage.biIntChain[i]) {
            for (int j = 0; j < 32; j++) {
                if (((g >> j) & 1) && i * 32 + j < this->width * this->height) {
                    this->displayText[i * 32 + j] = ' ';
                    this->biStorage.bi()[{i * 32 + j % this->width, (i * 32 + j) / this->width}];
                }
            }
        }
    }
    //this->displayText = std::string(height * width, ' ');
}

void ASCIIRenderer::update() {
    //setConsoleWindowSize(width, height + 1);
    //HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    //COORD pos;
    //pos.X = 0;
    //pos.Y = 0;
    //SetConsoleCursorPosition(hConsoleOutput, pos);
    std::string o(this->displayText);
    for (int i = this->height; i ; i--) {
        o.insert(i * this->width, "\n");
    }
    out << o << std::endl;
    out.flush();
    
}

void ASCIIRenderer::renderPixel(int x, int y, short color) {
    renderPixel(x, y, ::charList[color % charSize]);
}

void ASCIIRenderer::renderPixel(int x, int y, char type) {
    if (x < 0 || y < 0 || x >= this->width || y >= this->height || type == ' ')
        return;
    this->displayText[y * width + x] = type == '\0'? '@' : type;
    this->biStorage.bi()[{ (unsigned int)x, (unsigned int)y}] = 1;
}

void ASCIIRenderer::renderSubPixel(int x, int y, short color) {
    renderPixel(x + startX, y + startY, color);
}

void ASCIIRenderer::renderCharacter(int x, int y, char c) {
    renderPixel(x, y, c);
}

void ASCIIRenderer::renderString(int x, int y, std::string message) {
    for (int i = 0; i < message.size(); i++) {
        renderPixel(x + i, y, message[i]);
    }
}

bool ASCIIRenderer::running() {
    
    return window.FindWindowByProcessId(window.pi.dwProcessId);
}

BiStorage::BiStorage(int width, int height) : width(width), height(height), chainLen(width* height / 32 + ((width * height) % 8 ? 1 : 0)) {
    this->biIntChain = new unsigned int[chainLen] {0};
}

void BiStorage::turnOn(int x, int y) {
    int idx = y * this->width + x;
    this->biIntChain[idx / 32] |= 1 << (idx % 32);
}

void BiStorage::turnOff(int x, int y) {
    int idx = y * this->width + x;
    this->biIntChain[idx / 32] &= ~(1 << (idx % 32));
}

bool BiStorage::operator[](coord pos) {
    int idx = pos.x + pos.y * this->width;
    return (this->biIntChain[idx / 32] >> (idx % 32)) & 1;
}

BiStorage::BiOp BiStorage::bi() {
    return BiOp(-1, -1, *this);
}

BiStorage::BiOp::BiOp(int x, int y, BiStorage& par) : x(x), y(y), par(par){
}

BiStorage::BiOp BiStorage::BiOp::operator[](coord pos) {
    return BiOp(pos.x, pos.y, par);
}

void BiStorage::BiOp::operator=(bool flag) {
    if (flag) {
        par.turnOn(x, y);
    }
    else {
        par.turnOff(x, y);
    }
}
