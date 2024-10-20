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

    // Direct3Dの初期化


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

    while (Window::pInstance_->bRunning_)
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
