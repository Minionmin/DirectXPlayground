#pragma once

#include <Windows.h>
#include <string>
#include <DirectXMath.h>
#include <functional>

class Window {
public:
    Window();
    ~Window();

    bool Initialize(HINSTANCE hInstance, int nShowCmd, bool fullscreen);
    HWND GetHWND() const;
	static Window* GetInstance();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // �E�B���h�E�C���v�b�g����̃R�[���o�b�N
    void SetKeyCallback(std::function<void(WPARAM)> callback);
    void SetMouseCallback(std::function<void(UINT)> mouseCallback);

    size_t WINDOW_WIDTH = 1280;
    size_t WINDOW_HEIGHT = 720;

    void SetRunning(bool bRunning) { bRunning_ = bRunning; }
    void SetPrevMousePos(POINT pos) { prevMousePos_ = pos; }
    void SetCurrMousePos(POINT pos) { currMousePos_ = pos; }

    POINT GetPrevMousePos() const { return prevMousePos_; }
    POINT GetCurrMousePos() const { return currMousePos_; }
    bool GetRunning() const { return bRunning_; }

private:

    bool RegisterWindowClass(WNDCLASSEXW& wc);

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // �V���O���g��
    static Window* pInstance_;
    std::function<void(WPARAM)> keyCallback_;
    std::function<void(UINT)> mouseCallback_;

    // �t���O
    bool bRunning_ = true; // false�ɂȂ�ƃv���O�������I������
    // �C���v�b�g
    POINT prevMousePos_ = { 0, 0 };
    POINT currMousePos_ = { 0, 0 };

    HWND hwnd_ = NULL;
    HINSTANCE hInstance_ = NULL;
};