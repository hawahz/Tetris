#include "ASCIIDisplayWindow.h"

DisplayWindow::DisplayWindow() : pi{} {
}

bool DisplayWindow::start() {
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    // 创建管道用于通信
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return false;
    }

    // 设置管道句柄不可继承
    if (!SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 0)) {
        return false;
    }

    // 创建新进程
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hReadPipe;  // 将子进程的输入重定向到管道
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    // 获取当前可执行文件路径
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // 构建命令行参数
    wchar_t commandLine[MAX_PATH + 20];
    swprintf_s(commandLine, MAX_PATH + 20, L"\"%s\" --display", exePath);

    // 创建新进程（显示窗口）
    if (!CreateProcessW(
        NULL,
        commandLine,
        NULL,
        NULL,
        TRUE,                   // 继承句柄
        CREATE_NEW_CONSOLE,     // 创建新的控制台窗口
        NULL,
        NULL,
        &si,
        &pi)) {
        return false;
    }

    // 关闭不再需要的句柄
    CloseHandle(hReadPipe);
    hReadPipe = nullptr;

    // 设置窗口标题
    Sleep(100); // 等待窗口创建
    HWND hwnd = FindWindowByProcessId(pi.dwProcessId);
    if (hwnd) {
        SetWindowTextW(hwnd, L"流式内容显示器");
    }

    return true;
}

DisplayWindow& DisplayWindow::operator<<(std::ostream& (*manip)(std::ostream&)) {
    manip(streamBuffer);
    if (manip == static_cast<std::ostream & (*)(std::ostream&)>(std::endl) ||
        manip == static_cast<std::ostream & (*)(std::ostream&)>(std::flush)) {
        flush();
    }
    return *this;
}

void DisplayWindow::flush() {
    if (hWritePipe && !streamBuffer.str().empty()) {
        std::string text = streamBuffer.str();
        DWORD written;
        WriteFile(hWritePipe, text.c_str(), static_cast<DWORD>(text.size()), &written, NULL);
        streamBuffer.str(""); // 清空缓冲区
    }
}

void DisplayWindow::clear() {
    if (hWritePipe) {
        DWORD written;
        // ANSI清屏序列
        const char* clearSeq = "\x1B[2J\x1B[H";
        WriteFile(hWritePipe, clearSeq, static_cast<DWORD>(strlen(clearSeq)), &written, NULL);
    }
}

void DisplayWindow::setTextColor(int r, int g, int b) {
    if (hWritePipe) {
        DWORD written;
        std::string colorSeq = "\x1B[38;2;" +
            std::to_string(r) + ";" +
            std::to_string(g) + ";" +
            std::to_string(b) + "m";
        WriteFile(hWritePipe, colorSeq.c_str(), static_cast<DWORD>(colorSeq.size()), &written, NULL);
    }
}

void DisplayWindow::resetTextColor() {
    if (hWritePipe) {
        DWORD written;
        const char* resetSeq = "\x1B[0m";
        WriteFile(hWritePipe, resetSeq, static_cast<DWORD>(strlen(resetSeq)), &written, NULL);
    }
}

bool DisplayWindow::running() {
    return pi.dwProcessId;
}

DisplayWindow::~DisplayWindow() {
    flush(); // 确保所有内容都已发送
    if (hWritePipe) {
        CloseHandle(hWritePipe);
    }
    if (pi.hProcess) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

HWND DisplayWindow::FindWindowByProcessId(DWORD pid) {
    struct EnumData {
        DWORD pid;
        HWND hwnd;
    } data = { pid, NULL };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        EnumData& data = *reinterpret_cast<EnumData*>(lParam);

        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);

        if (windowPid == data.pid) {
            wchar_t className[256];
            if (GetClassNameW(hwnd, className, 256)) {
                if (wcscmp(className, L"ConsoleWindowClass") == 0) {
                    data.hwnd = hwnd;
                    return FALSE; // 停止枚举
                }
            }
        }
        return TRUE; // 继续枚举
        }, reinterpret_cast<LPARAM>(&data));

    return data.hwnd;
}

void runDisplayMode() {
    // 启用ANSI转义序列支持
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdout, &mode);
    SetConsoleMode(hStdout, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // 设置控制台字体
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    if (GetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo)) {
        // 设置新字体属性
        fontInfo.FontFamily = FF_DONTCARE;      // 不指定字体系列
        fontInfo.FontWeight = FW_NORMAL;        // 正常粗细
        wcscpy_s(fontInfo.FaceName, L"Terminal"); // 使用Consolas字体

        // 设置字体大小 - 调整这里的值可以改变字体大小
        fontInfo.dwFontSize.X = 16;  // 字符宽度
        fontInfo.dwFontSize.Y = 16; // 字符高度（主要控制大小）

        SetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo);
    }

    HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hConsoleInput == INVALID_HANDLE_VALUE) {
        std::cerr << "获取控制台输入句柄失败。错误代码：" << GetLastError() << std::endl;
    }
    // 禁用输入
    DWORD consoleMode;
    if (!GetConsoleMode(hConsoleInput, &consoleMode)) {
        std::cerr << "获取控制台模式失败。错误代码：" << GetLastError() << std::endl;
        HANDLE hConsole = CreateFile(
            TEXT("CONIN$"),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (hConsole == INVALID_HANDLE_VALUE) {
            return; // 可能没有控制台，忽略错误
        }

        DWORD mode;
        if (GetConsoleMode(hConsole, &mode)) {
            // 禁用快速编辑和插入模式
            mode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);
            SetConsoleMode(hConsole, mode);
        }

        CloseHandle(hConsole);
    }
    else {
        // 移除快速编辑标志 (ENABLE_QUICK_EDIT_MODE) 和插入模式标志 (ENABLE_INSERT_MODE)
        consoleMode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);

        if (!SetConsoleMode(hConsoleInput, consoleMode)) {
            std::cerr << "设置控制台模式失败。错误代码：" << GetLastError() << std::endl;
        }

    }

    
    // 隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hStdout, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hStdout, &cursorInfo);

    // 设置控制台标题
    SetConsoleTitleW(L"流式内容显示器");
    // 清屏并设置初始颜色
    std::cout << "\x1B[2J\x1B[H"; // 清屏
    std::cout << "\x1B[38;2;100;200;255m"; // 设置初始颜色
    
    // 读取并显示内容
    char buffer[4096];
    DWORD read;
    while (ReadFile(GetStdHandle(STD_INPUT_HANDLE), buffer, sizeof(buffer) - 1, &read, NULL)) {
        if (read > 0) {
            buffer[read] = '\0';
            std::cout << buffer;
        }
    }


}

/*
int main(int argc, char* argv[]) {
    // 检查是否以显示模式启动
    if (argc > 1 && strcmp(argv[1], "--display") == 0) {
        runDisplayMode();
        return 0;
    }

    // 主终端正常使用
    std::cout << "主终端窗口: 请输入您的名字: ";
    std::string name;
    std::cin >> name;
    std::cout << "你好, " << name << "! 这是主终端窗口。\n";

    // 创建并启动显示窗口
    DisplayWindow display;
    if (!display.start()) {
        std::cerr << "无法创建显示窗口\n";
        return 1;
    }

    std::cout << "显示窗口已启动。将向其发送内容...\n";

    // 使用流式操作向显示窗口发送内容
    display << "==================================" << std::endl;
    display << "     欢迎使用流式内容显示器" << std::endl;
    display << "==================================" << std::endl;
    display << std::endl;

    // 使用颜色和格式化
    display.setTextColor(255, 150, 50); // 橙色
    display << "这是一个支持流式操作和格式化的显示窗口" << std::endl;
    display << "用户无法在此窗口输入任何内容" << std::endl;
    display << std::endl;
    display.resetTextColor();

    // 发送表格
    display << std::left << std::setw(15) << "项目"
        << std::setw(10) << "数量"
        << std::setw(15) << "价格" << std::endl;
    display << "----------------------------------" << std::endl;

    display.setTextColor(150, 255, 150); // 绿色
    display << std::setw(15) << "苹果"
        << std::setw(10) << 5
        << std::setw(15) << "$2.99" << std::endl;

    display.setTextColor(255, 200, 100); // 黄色
    display << std::setw(15) << "香蕉"
        << std::setw(10) << 3
        << std::setw(15) << "$1.49" << std::endl;

    display.setTextColor(200, 150, 255); // 紫色
    display << std::setw(15) << "橙子"
        << std::setw(10) << 8
        << std::setw(15) << "$3.25" << std::endl;
    display.resetTextColor();
    display << "----------------------------------" << std::endl;

    // 发送动态内容
    display << std::endl << "进度更新:" << std::endl;
    for (int i = 0; i <= 10; i++) {
        display << "[";
        display.setTextColor(50, 200, 50); // 进度条绿色
        for (int j = 0; j < i; j++) display << "=";
        for (int j = i; j < 10; j++) display << " ";
        display.resetTextColor();
        display << "] " << (i * 10) << "%" << std::endl;

        // 模拟进度更新
        if (i < 10) {
            display << "\x1B[A"; // 光标上移一行
        }
        Sleep(300);
    }

    // 发送结束消息
    display.flush(); // 确保所有内容都已发送
    display << std::endl;
    display.setTextColor(100, 200, 255); // 蓝色
    display << "==================================" << std::endl;
    display << "      内容显示完成" << std::endl;
    display << "==================================" << std::endl;
    display.resetTextColor();

    std::cout << "\n内容已发送到显示窗口。按回车键退出主程序...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}
*/