#include "ASCIIDisplayWindow.h"

DisplayWindow::DisplayWindow() : pi{} {
}

bool DisplayWindow::start() {
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    // �����ܵ�����ͨ��
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return false;
    }

    // ���ùܵ�������ɼ̳�
    if (!SetHandleInformation(hWritePipe, HANDLE_FLAG_INHERIT, 0)) {
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
    swprintf_s(commandLine, MAX_PATH + 20, L"\"%s\" --display", exePath);

    // �����½��̣���ʾ���ڣ�
    if (!CreateProcessW(
        NULL,
        commandLine,
        NULL,
        NULL,
        TRUE,                   // �̳о��
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
        SetWindowTextW(hwnd, L"��ʽ������ʾ��");
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

void runDisplayMode() {
    // ����ANSIת������֧��
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdout, &mode);
    SetConsoleMode(hStdout, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // ���ÿ���̨����
    CONSOLE_FONT_INFOEX fontInfo;
    fontInfo.cbSize = sizeof(fontInfo);
    if (GetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo)) {
        // ��������������
        fontInfo.FontFamily = FF_DONTCARE;      // ��ָ������ϵ��
        fontInfo.FontWeight = FW_NORMAL;        // ������ϸ
        wcscpy_s(fontInfo.FaceName, L"Terminal"); // ʹ��Consolas����

        // ���������С - ���������ֵ���Ըı������С
        fontInfo.dwFontSize.X = 16;  // �ַ����
        fontInfo.dwFontSize.Y = 16; // �ַ��߶ȣ���Ҫ���ƴ�С��

        SetCurrentConsoleFontEx(hStdout, FALSE, &fontInfo);
    }

    HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hConsoleInput == INVALID_HANDLE_VALUE) {
        std::cerr << "��ȡ����̨������ʧ�ܡ�������룺" << GetLastError() << std::endl;
    }
    // ��������
    DWORD consoleMode;
    if (!GetConsoleMode(hConsoleInput, &consoleMode)) {
        std::cerr << "��ȡ����̨ģʽʧ�ܡ�������룺" << GetLastError() << std::endl;
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
            return; // ����û�п���̨�����Դ���
        }

        DWORD mode;
        if (GetConsoleMode(hConsole, &mode)) {
            // ���ÿ��ٱ༭�Ͳ���ģʽ
            mode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);
            SetConsoleMode(hConsole, mode);
        }

        CloseHandle(hConsole);
    }
    else {
        // �Ƴ����ٱ༭��־ (ENABLE_QUICK_EDIT_MODE) �Ͳ���ģʽ��־ (ENABLE_INSERT_MODE)
        consoleMode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);

        if (!SetConsoleMode(hConsoleInput, consoleMode)) {
            std::cerr << "���ÿ���̨ģʽʧ�ܡ�������룺" << GetLastError() << std::endl;
        }

    }

    
    // ���ع��
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hStdout, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hStdout, &cursorInfo);

    // ���ÿ���̨����
    SetConsoleTitleW(L"��ʽ������ʾ��");
    // ���������ó�ʼ��ɫ
    std::cout << "\x1B[2J\x1B[H"; // ����
    std::cout << "\x1B[38;2;100;200;255m"; // ���ó�ʼ��ɫ
    
    // ��ȡ����ʾ����
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
    // ����Ƿ�����ʾģʽ����
    if (argc > 1 && strcmp(argv[1], "--display") == 0) {
        runDisplayMode();
        return 0;
    }

    // ���ն�����ʹ��
    std::cout << "���ն˴���: ��������������: ";
    std::string name;
    std::cin >> name;
    std::cout << "���, " << name << "! �������ն˴��ڡ�\n";

    // ������������ʾ����
    DisplayWindow display;
    if (!display.start()) {
        std::cerr << "�޷�������ʾ����\n";
        return 1;
    }

    std::cout << "��ʾ�����������������䷢������...\n";

    // ʹ����ʽ��������ʾ���ڷ�������
    display << "==================================" << std::endl;
    display << "     ��ӭʹ����ʽ������ʾ��" << std::endl;
    display << "==================================" << std::endl;
    display << std::endl;

    // ʹ����ɫ�͸�ʽ��
    display.setTextColor(255, 150, 50); // ��ɫ
    display << "����һ��֧����ʽ�����͸�ʽ������ʾ����" << std::endl;
    display << "�û��޷��ڴ˴��������κ�����" << std::endl;
    display << std::endl;
    display.resetTextColor();

    // ���ͱ��
    display << std::left << std::setw(15) << "��Ŀ"
        << std::setw(10) << "����"
        << std::setw(15) << "�۸�" << std::endl;
    display << "----------------------------------" << std::endl;

    display.setTextColor(150, 255, 150); // ��ɫ
    display << std::setw(15) << "ƻ��"
        << std::setw(10) << 5
        << std::setw(15) << "$2.99" << std::endl;

    display.setTextColor(255, 200, 100); // ��ɫ
    display << std::setw(15) << "�㽶"
        << std::setw(10) << 3
        << std::setw(15) << "$1.49" << std::endl;

    display.setTextColor(200, 150, 255); // ��ɫ
    display << std::setw(15) << "����"
        << std::setw(10) << 8
        << std::setw(15) << "$3.25" << std::endl;
    display.resetTextColor();
    display << "----------------------------------" << std::endl;

    // ���Ͷ�̬����
    display << std::endl << "���ȸ���:" << std::endl;
    for (int i = 0; i <= 10; i++) {
        display << "[";
        display.setTextColor(50, 200, 50); // ��������ɫ
        for (int j = 0; j < i; j++) display << "=";
        for (int j = i; j < 10; j++) display << " ";
        display.resetTextColor();
        display << "] " << (i * 10) << "%" << std::endl;

        // ģ����ȸ���
        if (i < 10) {
            display << "\x1B[A"; // �������һ��
        }
        Sleep(300);
    }

    // ���ͽ�����Ϣ
    display.flush(); // ȷ���������ݶ��ѷ���
    display << std::endl;
    display.setTextColor(100, 200, 255); // ��ɫ
    display << "==================================" << std::endl;
    display << "      ������ʾ���" << std::endl;
    display << "==================================" << std::endl;
    display.resetTextColor();

    std::cout << "\n�����ѷ��͵���ʾ���ڡ����س����˳�������...";
    std::cin.ignore();
    std::cin.get();

    return 0;
}
*/