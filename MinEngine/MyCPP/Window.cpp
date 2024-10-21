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
        MessageBox(NULL, L"�E�C���h�E�N���X�̓o�^�Ɏ��s���܂���",
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

    // �E�B���h�E�̃��[�U�[�f�[�^��'Window'�C���X�^���X�ւ̃|�C���^���i�[����
    // WndProc��static�֐��Ȃ̂ŁAnon-static�����o�[�ɃA�N�Z�X�o���Ȃ�
    // ���̂��߁A�E�B���h�E�̃��[�U�[�f�[�^��'Window'�C���X�^���X�ւ̃|�C���^�ihwnd_�j���i�[����
    // �Ȃ��Ȃ�AWndProc�͂ǂ̃E�B���h�E�ŌĂ΂ꂽ����m��K�v�����邩��i���ɕ����̃E�B���h�E������ꍇ�j
    //SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    // ���b�Z�[�W���󂯎��x�̃L���X�g������邽�߂ɁA�����pInstance_���g���āAWindow�N���X�̃C���X�^���X���擾����

    if (fullscreen)
    {
        SetWindowLong(GetHWND(), GWL_STYLE, 0);
    }

    ShowWindow(hwnd_, nShowCmd);
    UpdateWindow(hwnd_);

    // �E�B���h�E�N���X�͂�������Ȃ��̂œo�^����
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return true;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // �����ɂ��A���b�Z�[�W���󂯎��x��Window�N���X�̃C���X�^���X���擾����K�v������A����L���X�g����K�v������
    //Window* pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    Window* pWindow = GetInstance();

    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'S': // �h��Ԃ����[�h�̐؂�ւ�
            //pWindow->bSolidMode_ = !pWindow->bSolidMode_;
            pWindow->keyCallback_(wParam);
            break;
        case 'W': // ���C���[�t���[�����[�h�̐؂�ւ�
            //pWindow->bWireframeMode_ = !pWindow->bWireframeMode_;
            break;
        case 'M': // Flat/Smooth�̐؂�ւ�
            //pWindow->bShadeSmooth_ = !pWindow->bShadeSmooth_;
            break;
        case 'N': // �@���x�N�g���̕`��̐؂�ւ�
            //pWindow->bDrawNormals_ = !pWindow->bDrawNormals_;
            break;
        case VK_ESCAPE: // ESC�L�[�Ńv���O�������I��
            if (MessageBox(0, L"�v���O�������I�����܂���?",
                L"�I�����b�Z�[�W", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                pWindow->bRunning_ = false;
                DestroyWindow(hwnd);
            }
            break;
        }
        return 0; // ���b�Z�[�W�����������ƕ񍐂��邽�߂�0��Ԃ�

    case WM_RBUTTONDOWN: // �}�E�X���N���b�N�ŃJ��������]�\��Ԃɂ���
    {
        //pWindow->bCameraRotate_ = true;
        // �����_���[��bCameraRotate_��true�ɂ���
        pWindow->mouseCallback_(WM_RBUTTONDOWN);
        SetCapture(hwnd); // �}�E�X�̃L���v�`����ݒ�
        GetCursorPos(&pWindow->prevMousePos_); // �����_�̃}�E�X���W���擾
        ScreenToClient(hwnd, &pWindow->prevMousePos_); // �N���C�A���g�̈�̍��W�ɕϊ�
        return 0;
    }

    case WM_LBUTTONDOWN: // �}�E�X�z�C�[���N���b�N�ŃJ�������ړ��\��Ԃɂ���
    {
        //pWindow->bCameraMove_ = true;
        SetCapture(hwnd);
        GetCursorPos(&pWindow->prevMousePos_); // �����_�̃}�E�X���W���擾
        ScreenToClient(hwnd, &pWindow->prevMousePos_); // �N���C�A���g�̈�̍��W�ɕϊ�
        return 0;
    }

    case WM_MOUSEMOVE: // �}�E�X�̈ړ���x����y�����ړ�����
    {
        /*GetCursorPos(&pWindow->currMousePos_);
        ScreenToClient(hwnd, &pWindow->currMousePos_);
        // �}�E�X�̈ړ��������擾
        const float deltaX = static_cast<float>(pWindow->currMousePos_.x - pWindow->prevMousePos_.x);
        const float deltaY = static_cast<float>(pWindow->currMousePos_.y - pWindow->prevMousePos_.y);
        if (pWindow->bCameraMove_)
        {
            // x����y�����ړ��i���f�������̋t�����Ɉړ�����j
            pWindow->objXOffset_ += deltaX * 0.01f; // �萔�̓}�E�X�̊��x
            pWindow->objYOffset_ -= deltaY * 0.01f; // -y�ɂ��邱�ƂŁA�}�E�X�̈ړ������ƃ��f���̈ړ����������킹��
        }
        else if (pWindow->bCameraRotate_)
        {
            // x����y������]�i���f�������̋t�����ɉ�]����j
            pWindow->objXRot_ -= deltaY * 0.5f;
            pWindow->objYRot_ -= deltaX * 0.5f;
        }
        // �����_�̃}�E�X���W��ۑ�
        pWindow->prevMousePos_ = pWindow->currMousePos_;*/
        return 0;
    }

    case WM_RBUTTONUP:
    {
        //pWindow->bCameraRotate_ = false;
        ReleaseCapture(); // �}�E�X�̃L���v�`�������
        return 0;
    }

    case WM_LBUTTONUP:
    {
        //pWindow->bCameraMove_ = false;
        ReleaseCapture();
        return 0;
    }

    case WM_MOUSEWHEEL: // �}�E�X�z�C�[���ŃJ������z�����ړ�����
    {
        const float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        //pWindow->camZ_ += 0.01f * zDelta; // �萔�̓J�����̊��x
        return 0;
    }

    case WM_DESTROY:
    {
        pWindow->bRunning_ = false;
        PostQuitMessage(0);
        return 0; // ���b�Z�[�W�����������ƕ񍐂��邽�߂�0��Ԃ�   
    }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Window::RegisterWindowClass(WNDCLASSEXW& wc)
{
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc; // �E�B���h�E�v���V�[�W���̊֐���o�^����
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
