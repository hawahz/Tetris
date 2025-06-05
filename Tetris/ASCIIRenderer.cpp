#include "ASCIIRenderer.h"

void DisableQuickEditMode() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE) {
        std::cerr << "��ȡ����̨������ʧ�ܡ��������: " << GetLastError() << std::endl;
        return;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(hInput, &mode)) {
        std::cerr << "��ȡ����̨ģʽʧ�ܡ��������: " << GetLastError() << std::endl;
        return;
    }

    // ���ÿ��ٱ༭ģʽ��ENABLE_QUICK_EDIT_MODE��
    // ��ȷ��������չ��־��ENABLE_EXTENDED_FLAGS��
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS;

    if (!SetConsoleMode(hInput, mode)) {
        std::cerr << "���ÿ���̨ģʽʧ�ܡ��������: " << GetLastError() << std::endl;
    }
}

void setConsoleWindowSize(SHORT width, SHORT height) {
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE); //��ȡ������
    SMALL_RECT wrt = { 0, 0, width, height - 1 }; //���ÿ��
    bool ret = SetConsoleWindowInfo(hOutput, TRUE, &wrt); // ���ô���ߴ�
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
        std::cerr << "�޷�������ʾ����\n";
    }
    /*
    setConsoleWindowSize(width, height);
    
    DisableQuickEditMode();
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cInfo{};
    GetConsoleCursorInfo(hOutput, &cInfo); //��ȡ���й����Ϣ
    cInfo.bVisible = false; //falseΪ���ش���,trueΪ��ʾ����
    SetConsoleCursorInfo(hOutput, &cInfo);  //�������ù����Ϣ

    HWND hWnd = GetConsoleWindow(); //��ȡ���ھ��
    LONG_PTR sty = GetWindowLongPtrA(hWnd, GWL_STYLE); //��ȡ������ʽ
    sty = sty & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX; //ȥ���ɱ仯��С,���,��С����ť,~��ȡ��,&����,����λ������ص�֪ʶ��
    SetWindowLongPtrA(hWnd, GWL_STYLE, sty); //���ô��岻�ɸ��Ĵ�С,�������
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
