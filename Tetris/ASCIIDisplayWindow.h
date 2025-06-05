#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// 显示窗口控制器类
class DisplayWindow {
    PROCESS_INFORMATION pi;
    HANDLE hWritePipe = nullptr;
    HANDLE hReadPipe = nullptr;
    std::ostringstream streamBuffer;

public:
    DisplayWindow();

    bool start();

    // 重载 << 运算符，支持流式输出
    template<typename T>
    DisplayWindow& operator<<(const T& value) {
        streamBuffer << value;
        return *this;
    }

    // 支持流式操纵器
    DisplayWindow& operator<<(std::ostream& (*manip)(std::ostream&));

    // 刷新缓冲区，发送内容到显示窗口
    void flush();

    // 清空显示窗口内容
    void clear();

    // 设置文本颜色
    void setTextColor(int r, int g, int b);

    // 重置文本颜色
    void resetTextColor();

    ~DisplayWindow();

private:
    // 通过进程ID查找窗口
    HWND FindWindowByProcessId(DWORD pid);
};

// 显示模式入口点
void runDisplayMode();

