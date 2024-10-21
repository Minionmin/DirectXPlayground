#include "Window.h"

Window* Window::pInstance_ = nullptr;

Window::Window()
{
}

Window::~Window()
{
	if(hwnd_)
	{
		DestroyWindow(hwnd_);
	}
}

bool Window::Initialize(HINSTANCE hInstance, int nShowCmd, bool fullscreen)
{
    pInstance_ = this;
    hInstance_ = hInstance;

    if (fullscreen)
    {
        HMONITOR hmon = MonitorFromWindow(NULL,
            MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hmon, &mi);

        WINDOW_WIDTH = mi.rcMonitor.right - mi.rcMonitor.left;
        WINDOW_HEIGHT = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    WNDCLASSEX wc;
    if (!RegisterWindowClass(wc)) {
        MessageBox(NULL, L"ウインドウクラスの登録に失敗しました",
            L"wc", MB_OK | MB_ICONERROR);
        return false;
    }

    hwnd_ = CreateWindowEx(NULL,
        wc.lpszClassName,
        L"Min Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        static_cast<int>(WINDOW_WIDTH),
        static_cast<int>(WINDOW_HEIGHT),
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hwnd_) {
        DWORD error = GetLastError();
        wchar_t errorMsg[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, errorMsg, 256, NULL);
        MessageBox(NULL, errorMsg, L"Window Creation Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // ウィンドウのユーザーデータに'Window'インスタンスへのポインタを格納する
    // WndProcはstatic関数なので、non-staticメンバーにアクセス出来ない
    // そのため、ウィンドウのユーザーデータに'Window'インスタンスへのポインタ（hwnd_）を格納する
    // なぜなら、WndProcはどのウィンドウで呼ばれたかを知る必要があるから（特に複数のウィンドウがある場合）
    //SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    // メッセージを受け取る度のキャストを避けるために、今回はpInstance_を使って、Windowクラスのインスタンスを取得する

    if (fullscreen)
    {
        SetWindowLong(GetHWND(), GWL_STYLE, 0);
    }

    ShowWindow(hwnd_, nShowCmd);
    UpdateWindow(hwnd_);

    // ウィンドウクラスはもういらないので登録解除
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return true;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // あいにく、メッセージを受け取る度にWindowクラスのインスタンスを取得する必要があり、毎回キャストする必要がある
    //Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    Window* pWindow = GetInstance();

    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'S': // 塗りつぶしモードの切り替え
            //pWindow->bSolidMode_ = !pWindow->bSolidMode_;
            pWindow->keyCallback_(wParam);
            break;
        case 'W': // ワイヤーフレームモードの切り替え
            //pWindow->bWireframeMode_ = !pWindow->bWireframeMode_;
            break;
        case 'M': // Flat/Smoothの切り替え
            //pWindow->bShadeSmooth_ = !pWindow->bShadeSmooth_;
            break;
        case 'N': // 法線ベクトルの描画の切り替え
            //pWindow->bDrawNormals_ = !pWindow->bDrawNormals_;
            break;
        case VK_ESCAPE: // ESCキーでプログラムを終了
            if (MessageBox(0, L"プログラムを終了しますか?",
                L"終了メッセージ", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                pWindow->bRunning_ = false;
                DestroyWindow(hwnd);
            }
            break;
        }
        return 0; // メッセージを処理したと報告するために0を返す

    case WM_RBUTTONDOWN: // マウス左クリックでカメラを回転可能状態にする
    {
        //pWindow->bCameraRotate_ = true;
        // レンダラーのbCameraRotate_をtrueにする
        pWindow->mouseCallback_(WM_RBUTTONDOWN);
        SetCapture(hwnd); // マウスのキャプチャを設定
        GetCursorPos(&pWindow->prevMousePos_); // 現時点のマウス座標を取得
        ScreenToClient(hwnd, &pWindow->prevMousePos_); // クライアント領域の座標に変換
        return 0;
    }

    case WM_LBUTTONDOWN: // マウスホイールクリックでカメラを移動可能状態にする
    {
        //pWindow->bCameraMove_ = true;
        SetCapture(hwnd);
        GetCursorPos(&pWindow->prevMousePos_); // 現時点のマウス座標を取得
        ScreenToClient(hwnd, &pWindow->prevMousePos_); // クライアント領域の座標に変換
        return 0;
    }

    case WM_MOUSEMOVE: // マウスの移動でx軸とy軸を移動する
    {
        /*GetCursorPos(&pWindow->currMousePos_);
        ScreenToClient(hwnd, &pWindow->currMousePos_);
        // マウスの移動距離を取得
        const float deltaX = static_cast<float>(pWindow->currMousePos_.x - pWindow->prevMousePos_.x);
        const float deltaY = static_cast<float>(pWindow->currMousePos_.y - pWindow->prevMousePos_.y);
        if (pWindow->bCameraMove_)
        {
            // x軸とy軸を移動（モデルをその逆方向に移動する）
            pWindow->objXOffset_ += deltaX * 0.01f; // 定数はマウスの感度
            pWindow->objYOffset_ -= deltaY * 0.01f; // -yにすることで、マウスの移動方向とモデルの移動方向を合わせる
        }
        else if (pWindow->bCameraRotate_)
        {
            // x軸とy軸を回転（モデルをその逆方向に回転する）
            pWindow->objXRot_ -= deltaY * 0.5f;
            pWindow->objYRot_ -= deltaX * 0.5f;
        }
        // 現時点のマウス座標を保存
        pWindow->prevMousePos_ = pWindow->currMousePos_;*/
        return 0;
    }

    case WM_RBUTTONUP:
    {
        //pWindow->bCameraRotate_ = false;
        ReleaseCapture(); // マウスのキャプチャを解放
        return 0;
    }

    case WM_LBUTTONUP:
    {
        //pWindow->bCameraMove_ = false;
        ReleaseCapture();
        return 0;
    }

    case WM_MOUSEWHEEL: // マウスホイールでカメラのz軸を移動する
    {
        const float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        //pWindow->camZ_ += 0.01f * zDelta; // 定数はカメラの感度
        return 0;
    }

    case WM_DESTROY:
    {
        pWindow->bRunning_ = false;
        PostQuitMessage(0);
        return 0; // メッセージを処理したと報告するために0を返す   
    }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Window::RegisterWindowClass(WNDCLASSEXW& wc)
{
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc; // ウィンドウプロシージャの関数を登録する
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance_;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"MinWindow";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    return RegisterClassEx(&wc);
}

void Window::SetKeyCallback(std::function<void(WPARAM)> callback)
{
    keyCallback_ = callback;
}

void Window::SetMouseCallback(std::function<void(UINT)> mouseCallback)
{
	mouseCallback_ = mouseCallback;
}

HWND Window::GetHWND() const
{
    return hwnd_;
}

Window* Window::GetInstance()
{
	return pInstance_;
}
