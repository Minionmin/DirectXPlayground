#pragma once

#include <Windows.h>
#include <string>
#include <DirectXMath.h>

class Window {
public:
    Window();
    ~Window();

    bool Initialize(HINSTANCE hInstance, int nShowCmd, bool fullscreen);
    HWND GetHWND() const;

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static Window* pInstance_;

    size_t WINDOW_WIDTH = 1280;
    size_t WINDOW_HEIGHT = 720;

    // false�ɂȂ�ƃv���O�������I������
    bool bRunning_ = true;

    // �t���O
    bool bSolidMode_ = true;
    bool bWireframeMode_ = false;
    bool bDrawNormals_ = false;
    bool bShadeSmooth_ = true;

    // �C���v�b�g
    POINT prevMousePos_ = { 0, 0 };
    POINT currMousePos_ = { 0, 0 };

    // ���f���̃��[���h�s��̏��
    float objXOffset_ = 0.0f;
    float objYOffset_ = 0.0f;
    float objZOffset_ = 0.0f;
    float objXRot_ = 0.0f;
    float objYRot_ = 0.0f;
    float objZRot_ = 0.0f;

    // �J�����̏��
    float camX_ = 0.0f;
    float camY_ = 0.0f;
    float camZ_ = -10.0f;

    DirectX::XMFLOAT3 camPos_ = { camX_, camY_, camZ_ }; // ���_
    DirectX::XMFLOAT3 camTarg_ = { 0.0f, 0.0f, 0.0f }; // �����_
    DirectX::XMFLOAT3 camUp_ = { 0.0f, 1.0f, 0.0f }; // ����� (world up)
    bool bCameraMove_ = false;
    bool bCameraRotate_ = false;

private:
    HWND hwnd_ = NULL;
    HINSTANCE hInstance_ = NULL;

    bool RegisterWindowClass(WNDCLASSEXW& wc);
};