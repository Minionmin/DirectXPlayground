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

    // falseになるとプログラムが終了する
    bool bRunning_ = true;

    // フラグ
    bool bSolidMode_ = true;
    bool bWireframeMode_ = false;
    bool bDrawNormals_ = false;
    bool bShadeSmooth_ = true;

    // インプット
    POINT prevMousePos_ = { 0, 0 };
    POINT currMousePos_ = { 0, 0 };

    // モデルのワールド行列の情報
    float objXOffset_ = 0.0f;
    float objYOffset_ = 0.0f;
    float objZOffset_ = 0.0f;
    float objXRot_ = 0.0f;
    float objYRot_ = 0.0f;
    float objZRot_ = 0.0f;

    // カメラの情報
    float camX_ = 0.0f;
    float camY_ = 0.0f;
    float camZ_ = -10.0f;

    DirectX::XMFLOAT3 camPos_ = { camX_, camY_, camZ_ }; // 視点
    DirectX::XMFLOAT3 camTarg_ = { 0.0f, 0.0f, 0.0f }; // 注視点
    DirectX::XMFLOAT3 camUp_ = { 0.0f, 1.0f, 0.0f }; // 上方向 (world up)
    bool bCameraMove_ = false;
    bool bCameraRotate_ = false;

private:
    HWND hwnd_ = NULL;
    HINSTANCE hInstance_ = NULL;

    bool RegisterWindowClass(WNDCLASSEXW& wc);
};