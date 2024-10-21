#include "Application.h"

Application::Application(){}

Application::~Application(){}

bool Application::Initialize(HINSTANCE hInstance, int nShowCmd)
{
    // ウィンドウの初期化
	if(!window_.Initialize(hInstance, nShowCmd, false))
	{
		return false;
	}

    // Applicationをオブザーバーにして、Windowからのインプットを受け取って、別のクラスとの処理をする
    window_.SetKeyCallback([this](WPARAM wParam) { HandleKeyInput(wParam); });
    window_.SetMouseCallback([this](UINT msg) { HandleMouseInput(msg); });

    // Direct3Dの初期化
    if (!renderer_.Initialize(window_.GetHWND())) {
        return false;
    }

	return true;
}

void Application::Run()
{
	Mainloop();
}

void Application::Mainloop()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (Window::GetInstance()->GetRunning())
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg); // ウィンドウプロシージャにメッセージを送る
        }
        else {
            // ゲームロジック
            //Update();
            //Render();
        }
    }
}

void Application::HandleKeyInput(WPARAM wParam)
{
    switch (wParam) {
    case 'S':
        renderer_.SetSolidMode(!renderer_.GetSolidMode());
        break;
    case 'W':
        renderer_.SetWireframeMode(!renderer_.GetWireframeMode());
        break;
    case 'N':
        renderer_.SetDrawNormals(!renderer_.GetDrawNormals());
        break;
    }
}

void Application::HandleMouseInput(UINT msg)
{
	switch (msg) {
    case WM_LBUTTONDOWN:
    {
    }
    case WM_RBUTTONDOWN: // マウス左クリックでカメラを回転可能状態にする
    {
        renderer_.SetCameraRotate(true);
        break;
    }
	}
}
