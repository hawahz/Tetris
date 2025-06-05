#pragma once
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// ��ʾ���ڿ�������
class DisplayWindow {
    PROCESS_INFORMATION pi;
    HANDLE hWritePipe = nullptr;
    HANDLE hReadPipe = nullptr;
    std::ostringstream streamBuffer;

public:
    DisplayWindow();

    bool start();

    // ���� << �������֧����ʽ���
    template<typename T>
    DisplayWindow& operator<<(const T& value) {
        streamBuffer << value;
        return *this;
    }

    // ֧����ʽ������
    DisplayWindow& operator<<(std::ostream& (*manip)(std::ostream&));

    // ˢ�»��������������ݵ���ʾ����
    void flush();

    // �����ʾ��������
    void clear();

    // �����ı���ɫ
    void setTextColor(int r, int g, int b);

    // �����ı���ɫ
    void resetTextColor();

    ~DisplayWindow();

private:
    // ͨ������ID���Ҵ���
    HWND FindWindowByProcessId(DWORD pid);
};

// ��ʾģʽ��ڵ�
void runDisplayMode();

