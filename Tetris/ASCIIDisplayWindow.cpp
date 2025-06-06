#include "ASCIIDisplayWindow.h"
#include <thread>

DisplayWindow::DisplayWindow() : pi{} {
}

bool DisplayWindow::start() {
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    int code = rand();
    
    // �����ܵ�����ͨ��
    std::wstring pipeName = L"\\\\.\\pipe\\DisplayPipe" + std::to_wstring(code);

    // ���������ܵ�
    HANDLE hPipe = CreateNamedPipeW(
        pipeName.c_str(),
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1, 4096, 4096, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "���������ܵ�ʧ��\n";
        return false;
    }

    hWritePipe = hPipe;

    // ������->���ܵ�
    std::wstring inputPipeName = L"\\\\.\\pipe\\InputPipe" + std::to_wstring(code);

    hInputRead = CreateNamedPipeW(
        inputPipeName.c_str(),
        PIPE_ACCESS_INBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1, 4096, 4096, 0, NULL
    );
    if (hInputRead == INVALID_HANDLE_VALUE) {
        std::cerr << "�����ӡ������������ܵ�ʧ��" << std::endl;
        return false;
    }

    // �����½���
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hReadPipe;  // ���ӽ��̵������ض��򵽹ܵ�
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    // ��ȡ��ǰ��ִ���ļ�·��
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    // ���������в���
    wchar_t commandLine[MAX_PATH + 20];
    swprintf_s(commandLine, MAX_PATH + 20, L"\"%s\" --display \"%s\" \"%s\"", exePath, pipeName.c_str(), inputPipeName.c_str());

    // �����½��̣���ʾ���ڣ�
    if (!CreateProcessW(
        NULL,
        commandLine,
        NULL,
        NULL,
        FALSE,                   // �̳о��
        CREATE_NEW_CONSOLE,     // �����µĿ���̨����
        NULL,
        NULL,
        &si,
        &pi)) {
        return false;
    }

    // �رղ�����Ҫ�ľ��
    CloseHandle(hReadPipe);
    hReadPipe = nullptr;

    // ���ô��ڱ���
    Sleep(100); // �ȴ����ڴ���
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
        streamBuffer.str(""); // ��ջ�����
    }
}

void DisplayWindow::clear() {
    if (hWritePipe) {
        DWORD written;
        // ANSI��������
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
    flush(); // ȷ���������ݶ��ѷ���
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
                    return FALSE; // ֹͣö��
                }
            }
        }
        return TRUE; // ����ö��
        }, reinterpret_cast<LPARAM>(&data));

    return data.hwnd;
}

void runDisplayMode(const wchar_t* pipeName, const wchar_t* inputPipeName) {
    // 1. ���������ܵ�����ȡ��ʾ���ݣ�
    HANDLE hPipe = CreateFileW(
        pipeName,
        GENERIC_READ,
        0, NULL, OPEN_EXISTING, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        std::cerr << "�޷����������ܵ�\n";
        return;
    }

    HANDLE hInputWrite = CreateFileW(
        inputPipeName,         // ���� argv[3]
        GENERIC_WRITE,
        0, NULL,
        OPEN_EXISTING,
        0, NULL
    );
    if (hInputWrite == INVALID_HANDLE_VALUE) {
        std::cerr << "�ӽ����޷���������������ܵ�������: " << GetLastError() << std::endl;
        return;
    }

    // 2. ���ÿ���̨�����ʽ
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    if (GetConsoleMode(hStdout, &mode)) {
        SetConsoleMode(hStdout, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    // ��������
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

    // 3. �������ѡ��ȸ�����Ϊ
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

    // 4. ���ع��
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hStdout, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hStdout, &cursorInfo);

    // 5. ���ñ��⡢��������ʼ��ɫ
    SetConsoleTitleW(L"Tetris Game");
    std::cout << "\x1B[2J\x1B[H";                // ����
    std::cout << "\x1B[38;2;100;200;255m";       // ��ʼ��ɫ

    // 6. �������̼߳����������루ͨ����ʵ����̨��
    std::thread keyThread([hInputWrite]() {
        HANDLE hKeyboard = CreateFileW(
            L"CONIN$", GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, 0, nullptr
        );

        if (hInputWrite == INVALID_HANDLE_VALUE) {
            std::cerr << "��ȡд��Handleʧ��" << std::endl;
            return;
        }

        if (hKeyboard == INVALID_HANDLE_VALUE) {
            std::cerr << "�ӽ��̣��� CONIN$ ʧ�ܣ�������: " << GetLastError() << std::endl;
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

                    // д��������
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
    // 7. ����ʾѭ��
    char buffer[4096];
    DWORD read;
    while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &read, NULL)) {
        if (read > 0) {
            buffer[read] = '\0';
            std::cout << buffer;
        }
    }

    // 8. ����
    CloseHandle(hPipe);
    if (hConsoleInput != INVALID_HANDLE_VALUE)
        CloseHandle(hConsoleInput);

    
}