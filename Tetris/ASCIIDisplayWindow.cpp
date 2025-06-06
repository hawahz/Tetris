#include "ASCIIDisplayWindow.h"
#include <thread>

DisplayWindow::DisplayWindow() : pi{} {
}

bool DisplayWindow::start() {
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    int code = rand();
    
    // 创建管道用于通信
    std::wstring pipeName = L"\\\\.\\pipe\\DisplayPipe" + std::to_wstring(code);

    // 创建命名管道
    HANDLE hPipe = CreateNamedPipeW(
        pipeName.c_str(),
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1, 4096, 4096, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "创建命名管道失败\n";
        return false;
    }

    hWritePipe = hPipe;

    // 创建子->主管道
    std::wstring inputPipeName = L"\\\\.\\pipe\\InputPipe" + std::to_wstring(code);

    hInputRead = CreateNamedPipeW(
        inputPipeName.c_str(),
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1, 4096, 4096, 0, NULL
    );
    if (hInputRead == INVALID_HANDLE_VALUE) {
        std::cerr << "创建子→主输入命名管道失败" << std::endl;
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
    swprintf_s(commandLine, MAX_PATH + 20, L"\"%s\" --display \"%s\" \"%s\"", exePath, pipeName.c_str(), inputPipeName.c_str());

    // 创建新进程（显示窗口）
    if (!CreateProcessW(
        NULL,
        commandLine,
        NULL,
        NULL,
        FALSE,                   // 继承句柄
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
        SetWindowTextW(hwnd, L"Tetris Game");
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

void runDisplayMode(const wchar_t* pipeName, const wchar_t* inputPipeName) {
    // 1. 连接命名管道（读取显示内容）
    HANDLE hPipe = CreateFileW(
        pipeName,
        GENERIC_READ,
        0, NULL, OPEN_EXISTING, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "无法连接命名管道\n";
        return;
    }

    HANDLE hInputWrite = CreateFileW(
        inputPipeName,         // 来自 argv[3]
        GENERIC_WRITE,
        0, NULL,
        OPEN_EXISTING,
        0, NULL
    );
    if (hInputWrite == INVALID_HANDLE_VALUE) {
        std::cerr << "子进程无法连接主进程输入管道，错误: " << GetLastError() << std::endl;
        return;
    }

    // 2. 设置控制台输出样式
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(hStdout, &mode)) {
        SetConsoleMode(hStdout, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    // 设置字体
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    if (GetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo)) {
        fontInfo.FontFamily = FF_DONTCARE;
        fontInfo.FontWeight = FW_NORMAL;
        wcscpy_s(fontInfo.FaceName, L"Terminal");
        fontInfo.dwFontSize.X = 16;
        fontInfo.dwFontSize.Y = 16;
        SetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo);
    }

    // 3. 禁用鼠标选择等干扰行为
    HANDLE hConsoleInput = CreateFileW(
        L"CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr
    );

    if (hConsoleInput != INVALID_HANDLE_VALUE) {
        DWORD inputMode;
        if (GetConsoleMode(hConsoleInput, &inputMode)) {
            inputMode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);
            SetConsoleMode(hConsoleInput, inputMode);
        }
    }

    // 4. 隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hStdout, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hStdout, &cursorInfo);

    // 5. 设置标题、清屏、初始颜色
    SetConsoleTitleW(L"Tetris Game");
    std::cout << "\x1B[2J\x1B[H";                // 清屏
    std::cout << "\x1B[38;2;100;200;255m";       // 初始颜色

    // 6. 启动子线程监听键盘输入（通过真实控制台）
    std::thread keyThread([hInputWrite]() {
        HANDLE hKeyboard = CreateFileW(
            L"CONIN$", GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, 0, nullptr
        );

        if (hInputWrite == INVALID_HANDLE_VALUE) {
            std::cerr << "获取写入Handle失败" << std::endl;
            return;
        }

        if (hKeyboard == INVALID_HANDLE_VALUE) {
            std::cerr << "子进程：打开 CONIN$ 失败，错误码: " << GetLastError() << std::endl;
            return;
        }

        DWORD mode;
        GetConsoleMode(hKeyboard, &mode);
        SetConsoleMode(hKeyboard, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

        INPUT_RECORD record;
        DWORD read;
        while (true) {
            
            if (ReadConsoleInput(hKeyboard, &record, 1, &read)) {
                if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
                    int keyCode = record.Event.KeyEvent.wVirtualKeyCode;

                    // 写入主进程
                    DWORD written;
                    WriteFile(hInputWrite, &keyCode, sizeof(keyCode), &written, NULL);
                    if (keyCode == VK_ESCAPE) break;
                }
            }
            
        }

        CloseHandle(hKeyboard);
        CloseHandle(hInputWrite);
        });
    keyThread.detach();
    // 7. 主显示循环
    char buffer[4096];
    DWORD read;
    while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &read, NULL)) {
        if (read > 0) {
            buffer[read] = '\0';
            std::cout << buffer;
        }
    }

    // 8. 清理
    CloseHandle(hPipe);
    if (hConsoleInput != INVALID_HANDLE_VALUE)
        CloseHandle(hConsoleInput);

    
}