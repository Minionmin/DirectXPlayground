#include "stdafx.h"

using namespace DirectX;

// �ۑ�2�̃t�@�C����ǂݍ��ފ֐�
// �o�b�t�@�[��char*�^�ɃL���X�g���ăo�C�i���[�f�[�^�𒼐ړǂݍ��܂Ȃ��Ƃ����Ȃ��Aconst�͂��Ȃ�
bool LoadBTMFile(const std::string& filePath, Header& header, std::vector<Triangle>& triangles, std::vector<Task2Vertex>& vertices, std::vector<XMFLOAT3>& normals)
{
    std::ifstream inFile(filePath, std::ios::binary); // �p�[�X���̃t�@�C�����o�C�i�����[�h�ŊJ��
    if (!inFile.is_open())
    {
        MessageBox(nullptr, L".btm�t�@�C�����J�����ƂɎ��s���܂���", L".btm is_open", MB_OK | MB_ICONERROR);
        return false;
    }

    // Header�̓ǂݍ���
    // reinterpret_cast<char*>���g���āAchar�^�̃|�C���^�ɃL���X�g���āA�o�C�g�𒼐ړǂݍ��߂�悤�ɂ���
    inFile.read(reinterpret_cast<char*>(&header), sizeof(Header)); // ���߂��T�C�Y���̃o�C�g��ǂݍ���
    if (inFile.gcount() != sizeof(Header)) // gcount()�͓ǂݍ��񂾃o�C�g�̐���Ԃ��A��v���Ȃ���΃G���[���b�Z�[�W��Ԃ�
    {
        MessageBox(nullptr, L".btm�t�@�C��Header�̓ǂݍ��݂Ɏ��s���܂���", L"Header�ǂݍ��݃G���[", MB_OK | MB_ICONERROR);
        return false;
    }

    // �g���C�A���O���̓ǂݍ���
    inFile.seekg(header.triangleOffset, std::ios::beg); // seekg�Ńt�@�C���|�C���^���ŏ�����triangleOffset�����ړ�������
    triangles.resize(header.triangleCount); // �g���C�A���O���̐��������T�C�Y�i�X�y�[�X���m�ہj
    // .data()�Ńx�N�g���̍ŏ��̗v�f�̃|�C���^���擾
    inFile.read(reinterpret_cast<char*>(triangles.data()), sizeof(Triangle) * header.triangleCount); // triangles�o�b�t�@�[�Ƀo�C�i���[�f�[�^�𒼐ړǂݍ���
    if (inFile.gcount() != sizeof(Triangle) * header.triangleCount)
    {
        MessageBox(nullptr, L".btm�t�@�C���̃g���C�A���O���f�[�^�̓ǂݍ��݂Ɏ��s���܂���", L"�g���C�A���O���̓ǂݍ��݃G���[", MB_OK | MB_ICONERROR);
        return false;
    }

    // ���_�̂̓ǂݍ���
    inFile.seekg(header.vertexOffset, std::ios::beg);
    vertices.resize(header.vertexCount);
    inFile.read(reinterpret_cast<char*>(vertices.data()), sizeof(Task2Vertex) * header.vertexCount);
    if (inFile.gcount() != sizeof(Task2Vertex) * header.vertexCount)
    {
        MessageBox(nullptr, L".btm�t�@�C���̒��_�f�[�^�̓ǂݍ��݂Ɏ��s���܂���", L"���_�̓ǂݍ��݃G���[", MB_OK | MB_ICONERROR);
        return false;
    }

    // �@���x�N�g���̂̓ǂݍ���
    inFile.seekg(header.normalOffset, std::ios::beg);
    normals.resize(header.normalCount);
    inFile.read(reinterpret_cast<char*>(normals.data()), sizeof(XMFLOAT3) * header.normalCount);
    if (inFile.gcount() != sizeof(XMFLOAT3) * header.normalCount)
    {
        MessageBox(nullptr, L".btm�t�@�C���̖@���x�N�g���f�[�^�̓ǂݍ��݂Ɏ��s���܂���", L"�@���x�N�g���̓ǂݍ��݃G���[", MB_OK | MB_ICONERROR);
        return false;
    }

    inFile.close();
    return true;
}

// �v���O�����̃G���g���[�|�C���g
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // �E�C���h�E�̏�����
    if (!InitWindow(hInstance, nShowCmd, FullScreen))
    {
        MessageBox(0, L"�E�C���h�E�̏����������s���܂���",
            L"InitWindow", MB_OK);
        return 1;
    }

    if (!InitD3D())
    {
        MessageBox(0, L"Direct3D12�̏������Ɏ��s���܂���",
            L"InitD3D", MB_OK);
        return 1;
    }

    // �v���O�����̃��C�����[�v
    Mainloop();

    WaitForPreviousFrame();

    CloseHandle(fenceEvent_);

    return 0;
}

bool InitWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen)
{
    if (fullscreen)
    {
        HMONITOR hmon = MonitorFromWindow(hwnd,
            MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hmon, &mi);

        WINDOW_WIDTH = mi.rcMonitor.right - mi.rcMonitor.left;
        WINDOW_HEIGHT = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc; // �E�B���h�E�v���V�[�W���̊֐���o�^����
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"MinWindow";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"�E�C���h�E�N���X�̓o�^�Ɏ��s���܂���",
            L"wc", MB_OK | MB_ICONERROR);
        return false;
    }

    hwnd = CreateWindowEx(NULL,
        wc.lpszClassName,
        L"Min Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (!hwnd)
    {
        MessageBox(NULL, L"�E�C���h�E�̏������Ɏ��s���܂���",
            L"hwnd", MB_OK | MB_ICONERROR);
        return false;
    }

    if (fullscreen)
    {
        SetWindowLong(hwnd, GWL_STYLE, 0);
    }

    ShowWindow(hwnd, ShowWnd);
    UpdateWindow(hwnd);

    // �E�B���h�E�N���X�͂�������Ȃ��̂œo�^����
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return true;
}

// �E�C���h�E�v���V�[�W���EWndClass����̃��b�Z�[�W����������
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'S': // �h��Ԃ����[�h�̐؂�ւ�
            bSolidMode_ = !bSolidMode_;
            break;
        case 'W': // ���C���[�t���[�����[�h�̐؂�ւ�
            bWireframeMode_ = !bWireframeMode_;
            break;
		case 'M': // Flat/Smooth�̐؂�ւ�
			bShadeSmooth_ = !bShadeSmooth_;
			break;
        case 'N': // �@���x�N�g���̕`��̐؂�ւ�
            bDrawNormals_ = !bDrawNormals_;
            break;
		case VK_ESCAPE: // ESC�L�[�Ńv���O�������I��
            if (MessageBox(0, L"�v���O�������I�����܂���?",
                L"�I�����b�Z�[�W", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                bRunning_ = false;
                DestroyWindow(hwnd);
            }
            break;
        }
        return 0; // ���b�Z�[�W�����������ƕ񍐂��邽�߂�0��Ԃ�

    case WM_RBUTTONDOWN : // �}�E�X���N���b�N�ŃJ��������]�\��Ԃɂ���
	{
        bCameraRotate_ = true;
        SetCapture(hwnd); // �}�E�X�̃L���v�`����ݒ�
        GetCursorPos(&prevMousePos_); // �����_�̃}�E�X���W���擾
        ScreenToClient(hwnd, &prevMousePos_); // �N���C�A���g�̈�̍��W�ɕϊ�
        return 0;
	}

    case WM_LBUTTONDOWN: // �}�E�X�z�C�[���N���b�N�ŃJ�������ړ��\��Ԃɂ���
	{
        bCameraMove_ = true;
        SetCapture(hwnd);
        GetCursorPos(&prevMousePos_); // �����_�̃}�E�X���W���擾
        ScreenToClient(hwnd, &prevMousePos_); // �N���C�A���g�̈�̍��W�ɕϊ�
        return 0;
	}

    case WM_MOUSEMOVE: // �}�E�X�̈ړ���x����y�����ړ�����
	{
        GetCursorPos(&currMousePos_);
        ScreenToClient(hwnd, &currMousePos_);
        // �}�E�X�̈ړ��������擾
        const float deltaX = static_cast<float>(currMousePos_.x - prevMousePos_.x);
        const float deltaY = static_cast<float>(currMousePos_.y - prevMousePos_.y);
        if (bCameraMove_)
        {
            // x����y�����ړ��i���f�������̋t�����Ɉړ�����j
            objXOffset_ += deltaX * 0.01f; // �萔�̓}�E�X�̊��x
            objYOffset_ -= deltaY * 0.01f; // -y�ɂ��邱�ƂŁA�}�E�X�̈ړ������ƃ��f���̈ړ����������킹��
        }
        else if (bCameraRotate_)
        {
            // x����y������]�i���f�������̋t�����ɉ�]����j
            objXRot_ -= deltaY * 0.5f;
            objYRot_ -= deltaX * 0.5f;
        }
        // �����_�̃}�E�X���W��ۑ�
        prevMousePos_ = currMousePos_;
        return 0;
	    }

    case WM_RBUTTONUP:
	{
        bCameraRotate_ = false;
        ReleaseCapture(); // �}�E�X�̃L���v�`�������
        return 0;
	}

    case WM_LBUTTONUP:
	    {
            bCameraMove_ = false;
            ReleaseCapture();
            return 0;
	    }

    case WM_MOUSEWHEEL: // �}�E�X�z�C�[���ŃJ������z�����ړ�����
        {
            const float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            camZ_ += 0.01f * zDelta; // �萔�̓J�����̊��x
            return 0;
        }
    
    case WM_DESTROY:
	    {
            bRunning_ = false;
            PostQuitMessage(0);
            return 0; // ���b�Z�[�W�����������ƕ񍐂��邽�߂�0��Ԃ�   
	    }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Mainloop() {
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));

    while (bRunning_)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg); // �E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
        }
        else {
            // �Q�[�����W�b�N
            Update();
            Render();
        }
    }
}

void AverageNormals(const std::vector<VertexData>& verticesData, const std::vector<uint16_t>& indices, const std::vector<XMFLOAT3>& normals, std::vector<XMFLOAT3>& accumulatedNormals)
{
	std::vector<int> normalCount(verticesData.size(), 0); // �w�肵�����_�ɑ΂���@���̐����J�E���g����z��

	// �e���_�̖@���x�N�g�����v�Z����
	for (size_t i = 0; i < indices.size(); i += 3) // 1�̃g���C�A���O���ɑ΂���3�̒��_�����邽�߁A+3����i���[�v�łP�̃g���C�A���O������������j
	{
		const int triangleNormalIndex = static_cast<int>(i) / 3; // 3�̒��_�ɑ΂��ăg���C�A���O��1�̖@���ɂ���

		// 1�̃g���C�A���O���ɑ΂��āA���_�̖@����~�ς���
		for (int j = 0; j < 3; ++j)
		{
			const int vertexIndex = indices[i + j];  // ���_�̃C���f�b�N�X���擾�A�i��j0, 1, 2 �Ȃ�
			accumulatedNormals[vertexIndex].x += normals[triangleNormalIndex].x;
			accumulatedNormals[vertexIndex].y += normals[triangleNormalIndex].y;
			accumulatedNormals[vertexIndex].z += normals[triangleNormalIndex].z;
			normalCount[vertexIndex]++;  // ���̒��_�ɂ����̖@��������̂��𐔂���
		}
	}

	// �v�Z�����@���𐳋K�����ĕ��ω�����
	for (size_t i = 0; i < accumulatedNormals.size(); ++i)
	{
		if (normalCount[i] > 0)
		{
			// ���ω�
			accumulatedNormals[i].x /= normalCount[i];
			accumulatedNormals[i].y /= normalCount[i];
			accumulatedNormals[i].z /= normalCount[i];

			// ���K��
			XMVECTOR normalVec = XMLoadFloat3(&accumulatedNormals[i]);
			normalVec = XMVector3Normalize(normalVec);
			XMStoreFloat3(&accumulatedNormals[i], normalVec);
		}
	}
}

bool InitD3D()
{
    // Pix GPU Capturer
    // Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
    // This may happen if the application is launched through the PIX UI. 
    if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
    {
        LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
    }
    // End Pix GPU Capturer

    // ** D3D�f�o�C�X�̐��� START
    //�t�B�[�`�����x����
    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    // �f�o�C�X�̂̌���
    HRESULT result = S_OK;
    if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory_)))) {
        if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory_)))) {
            return false;
        }
    }

    // GPU���W�߂�
    std::vector <IDXGIAdapter1*> adapters;
    IDXGIAdapter1* tmpAdapter = nullptr;
    for (int i = 0; dxgiFactory_->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        adapters.push_back(tmpAdapter);
    }

    // NVIDIA��GPU��I��
    for (IDXGIAdapter1* adpt : adapters) {
        DXGI_ADAPTER_DESC adesc = {};
        adpt->GetDesc(&adesc); // GPU�̏����擾
        std::wstring strDesc = adesc.Description;

        // NVIDIA�Ƃ��������񂪊܂܂�Ă��邩
        if (strDesc.find(L"NVIDIA") != std::string::npos) {
            tmpAdapter = adpt;
            break;
        }
    }

    // �l������GPU�ōœK�ȃ��x�����`�F�b�N���āADirect3D�f�o�C�X�̏�����
    bool bDeviceCreated = false;
    for (D3D_FEATURE_LEVEL l : levels) {
        if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(device_.ReleaseAndGetAddressOf())) == S_OK) {
            bDeviceCreated = true;
            break;
        }
    }

    if (!bDeviceCreated) {
        MessageBox(nullptr, L"D3D�f�o�C�X�̐����Ɏ��s",
            L"D3D�f�o�C�X", MB_OK);
        return false;
    }
    // ** D3D�f�o�C�X�̐��� END

    // ** Command Allocator��Command List�̐��� START
    for (unsigned int i = 0; i < frameBufferCount_; ++i)
    {
        result = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdAllocator_[i].ReleaseAndGetAddressOf()));
        if (FAILED(result))
        {
            MessageBox(nullptr, L"CommandAllocator�̐����Ɏ��s",
                L"Command Allocator", MB_OK);
            return false;
        }
    }

    result = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator_[0].Get(),
        nullptr, IID_PPV_ARGS(cmdList_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"CommandAllocator�̐����Ɏ��s",
            L"Command Allocator", MB_OK);
        return false;
    }
    // �R�}���h���X�g�̓��R�[�h���Ă����ԂŐ�������邽�߁AClose���Ă���
    // ��Ńe�N�X�`���[�̓]���ɂ��A���̂܂܎g����悤�ɂ��Ă���
    //cmdList_->Close();
    // ** Command Allocator��Command List�̐��� END

    // ** Command Queue�̐��� START
    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.NodeMask = 0;
    cqDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    result = device_->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(cmdQueue_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"CommandQueue�̐����Ɏ��s",
            L"Command Queue", MB_OK);
        return false;
    }
    // ** Command Queue�̐��� End

    // ** Swap Chain�̐��� START
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = WINDOW_WIDTH;
    swapchainDesc.Height = WINDOW_HEIGHT;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = false;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapchainDesc.BufferCount = frameBufferCount_; // �_�u���o�b�t�@
    swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    result = dxgiFactory_->CreateSwapChainForHwnd(cmdQueue_.Get(),
        hwnd,
        &swapchainDesc,
        nullptr,
        nullptr,
        reinterpret_cast<IDXGISwapChain1**>(swapchain_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"Swap Chain�̐����Ɏ��s",
            L"Swap Chain", MB_OK);
        return false;
    }

    // �����_�[�^�[�Q�b�g�r���[�p��Descriptor Heap�𐶐�
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //�����_�[�^�[�Q�b�g�r���[�Ȃ̂œ��RRTV
    heapDesc.NodeMask = 0;
    heapDesc.NumDescriptors = 2; // �\���̂Q��
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // ���Ɏw��Ȃ�
    result = device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps_.ReleaseAndGetAddressOf()));

    // RTV Descriptor�̐���
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    //rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �K���}�␳����
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // �K���}�␳�Ȃ�
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // RTV�̐���
    DXGI_SWAP_CHAIN_DESC swcDesc = {};
    result = swapchain_->GetDesc(&swcDesc); // �X���b�v�`�F�C���̐ݒ���擾
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeaps_->GetCPUDescriptorHandleForHeapStart());
    rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // RTV�f�B�X�N���v�^�̃T�C�Y���擾
    for (size_t i = 0; i < frameBufferCount_; ++i) {
        result = swapchain_->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(backBuffers_[i].ReleaseAndGetAddressOf())); // Swap Chain����o�b�N�o�b�t�@���擾
        if (FAILED(result))
        {
            MessageBox(nullptr, L"�o�b�N�o�b�t�@�̐����Ɏ��s",
                L"Back Buffer", MB_OK);
            return false;
        }
        device_->CreateRenderTargetView(backBuffers_[i].Get(), &rtvDesc, rtvHandle); // Swap Chain����擾�����o�b�N�o�b�t�@��RTV�ɕϊ����ARTV��Descriptor Heap�Ɋi�[
        //rtvHandle.ptr += rtvDescriptorSize_; // ���̃n���h���ֈړ�
        rtvHandle.Offset(1, rtvDescriptorSize_);
    }
    // ** Swap Chain�̐��� END

    // ** Depth Buffer�̐��� START
    // �[�x�o�b�t�@�[�r���[
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    ID3D12DescriptorHeap* dsvHeap = nullptr;
    device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap_.ReleaseAndGetAddressOf()));

    // �[�x�o�b�t�@�̐���
    D3D12_RESOURCE_DESC depthResDesc = {};
    depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResDesc.Width = WINDOW_WIDTH;
    depthResDesc.Height = WINDOW_HEIGHT;
    depthResDesc.DepthOrArraySize = 1;
    depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthResDesc.SampleDesc.Count = 1;
    depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    depthResDesc.MipLevels = 1;
    depthResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthResDesc.Alignment = 0;
    CD3DX12_HEAP_PROPERTIES depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    // �N���A�o�����[�̐ݒ�
    D3D12_CLEAR_VALUE _depthClearValue = {};
    _depthClearValue.DepthStencil.Depth = 1.0f;      //�[���P(�ő�l)�ŃN���A
    _depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; //32bit�[�x�l�Ƃ��ăN���A

    device_->CreateCommittedResource(
        &depthHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &depthResDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
        &_depthClearValue,
        IID_PPV_ARGS(depthBuffer_.ReleaseAndGetAddressOf()));

    // �[�x�o�b�t�@�[�r���[�쐬
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    device_->CreateDepthStencilView(depthBuffer_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());
    // ** Depth Buffer�̐��� END

    // ** Fence�̐��� START
    result = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"�t�F���X�̐����Ɏ��s",
            L"Fence", MB_OK);
        return false;
    }
    fenceValue_ = 0;

    // Fence Event�̐���
    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent_ == nullptr)
    {
        MessageBox(nullptr, L"�t�F���X�C�x���g�̐����Ɏ��s",
            L"FenceEvent", MB_OK);
        bRunning_ = false;
    }
    // ** Fence�̐��� END

    // �ۑ�2: .mqo -> .btm�̃o�C�i���[�t�@�C���ɕϊ����Ă���ǂݍ��݁A�r���[�|�[�g�ɕ\������
    // .btm�̃t�H�[�}�b�g��
    // Header����
    //    �f�[�^�t�@�C����			4Byte
    //    �ϊ������R���o�[�^�̃o�[�W����		1Byte
    //    �g���C�A���O���\���̐�			2Byte
    //    ���_��					2Byte
    //    �@����					2Byte
    //    �g���C�A���O���\���̂ւ̃I�t�Z�b�g	4Byte
    //    ���_�f�[�^�ւ̃I�t�Z�b�g		4Byte
    //    �@���f�[�^�ւ̃I�t�Z�b�g		4Byte
    // Body����
    //    �g���C�A���O���\���̃f�[�^		2Byte*4 (���_�̃C���f�b�N�X��3�Ɩ@���̃C���f�b�N�X1�j 
	//    ���_�f�[�^				float*3 ( x, y, z ) 
	//    �@���f�[�^				float*3 ( x, y, z )

	// .btm�t�@�C���̓ǂݍ���
    // �t�@�C���p�X�A�w�b�_�o�b�t�@�A�g���C�A���O���o�b�t�@�A���_�o�b�t�@�A�@���x�N�g���o�b�t�@
    if (!LoadBTMFile(modelFilePath_, headerBTM_, trianglesBTM_, verticesBTM_, normalsBTM_))
    {
        MessageBox(nullptr, L"Failed to load .btm file.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }
    // 12 tris 8 verts 12 normals
    // 36 indices (12 tris * 3), 8 vertices, 12 normals

    // ���_�̃f�[�^�iPOSITION��NORMAL�j
    std::vector<VertexData> verticesData;
    verticesData.reserve(verticesBTM_.size()); // �ǂݍ��񂾒��_�����̃��������m��
    for (size_t i = 0; i < verticesBTM_.size(); ++i)
    {
        VertexData vd;
        vd.position = XMFLOAT3(verticesBTM_[i].x, verticesBTM_[i].y, verticesBTM_[i].z);
        vd.normal = normalsBTM_[i];
        verticesData.push_back(vd);
    }
    const size_t vertexBufferSize = sizeof(VertexData) * verticesData.size();

	// �C���f�b�N�X�̃f�[�^
    size_t indexCount = trianglesBTM_.size() * 3; // �g���C�A���O����3�̃C���f�b�N�X�����邽��
    // �g���C�A���O���\���̃f�[�^��2Byte*4�Ȃ̂�uint16_t���g��
    size_t indexBufferSize = sizeof(uint16_t) * indexCount;
    std::vector<uint16_t> indices;
    indices.reserve(indexCount);
    for (const Triangle& tri : trianglesBTM_)
    {
        indices.push_back(tri.v1);
        indices.push_back(tri.v2);
        indices.push_back(tri.v3);
    }

    // �@���x�N�g���̃f�[�^
    const float normalLineLength = 0.2f; // �@���x�N�g���̒���
    std::vector<XMFLOAT3> normalLineVertices; // �@���x�N�g���̒��_�f�[�^
    normalLineVertices.reserve(indices.size() * 2); // �@���x�N�g���̒��_���̓C���f�b�N�X����2�{ �i�x�[�X���_�ƃ^�[�Q�b�g���_�j
    // �@���x�N�g���𕽋ω����Ďg���ƁA�h��Ԃ���Smooth�X�^�C���ɂȂ�
    // �e���_�ɑ΂��Ė@���𕽋ω����Ė@�����C���𐶐�
    std::vector<XMFLOAT3> accumulatedNormals(verticesData.size(), XMFLOAT3(0.0f, 0.0f, 0.0f)); // �e���_�̖@����~�ς���z��
    AverageNormals(verticesData, indices, normalsBTM_, accumulatedNormals); // �C���f�b�N�X�ɉe������e�g���C�A���O���̖@�������킹�ĕ��ω�����

    // �v�Z���ꂽ�@���x�N�g�����g���Ė@���f�[�^�𐶐�
    for (size_t i = 0; i < indices.size(); ++i)
    {
        // �x�[�X���_
        XMFLOAT3 startPos = verticesData[indices[i]].position;

        // ���̒��_�ɑ΂��镽�ω����ꂽ�@��
        XMFLOAT3 averagedNormal = accumulatedNormals[indices[i]];

        // �^�[�Q�b�g���_
        XMFLOAT3 endPos = XMFLOAT3(
            startPos.x + averagedNormal.x * normalLineLength,
            startPos.y + averagedNormal.y * normalLineLength,
            startPos.z + averagedNormal.z * normalLineLength
        );

        // �@���x�N�g���f�[�^�̃R�s�[
        normalLineVertices.push_back(startPos);
        normalLineVertices.push_back(endPos);
    }

    // ���ω����ꂽ�@�����g���Ē��_�f�[�^�𒲐�
    for (size_t i = 0; i < verticesData.size(); ++i)
    {
        verticesData[i].normal = accumulatedNormals[i];
    }
    const size_t normalLineVerticesBufferSize = sizeof(XMFLOAT3) * normalLineVertices.size();

    // Flat Shading�p�̒��_�f�[�^�𐶐�
    std::vector<VertexData> flatShadedVertices;  // Flat Shading�p�̒��_�f�[�^
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        // �g���C�A���O����3�̒��_���擾
		VertexData v1 = verticesData[indices[i]];
		VertexData v2 = verticesData[indices[i + 1]];
		VertexData v3 = verticesData[indices[i + 2]];

		// �o�C�i���t�@�C������̃g���C�A���O���̖@�����Q��
    	const XMFLOAT3 normal = normalsBTM_[i / 3];

		// �g���C�A���O����3�̒��_�ɖ@����ݒ�
        v1.normal = normal;
        v2.normal = normal;
		v3.normal = normal;

		// �@����ݒ肵�����_��ǉ�
		flatShadedVertices.push_back(v1);
		flatShadedVertices.push_back(v2);
		flatShadedVertices.push_back(v3);
    }
    const size_t flatVertBufferSize = sizeof(VertexData) * flatShadedVertices.size();

    // Flat Shading�p�̖@���x�N�g���̃f�[�^�𐶐�
    std::vector<XMFLOAT3> flatNormalLineVertices;
    flatNormalLineVertices.reserve(indices.size() * 2);
    for(size_t i = 0; i < indices.size(); i++)
    {
    	// �x�[�X���_
		XMFLOAT3 startPos = flatShadedVertices[i].position;

		// ���̒��_�ɑ΂���@��
		XMFLOAT3 normal = flatShadedVertices[i].normal;

		// �^�[�Q�b�g���_
		XMFLOAT3 endPos = XMFLOAT3(
			startPos.x + normal.x * normalLineLength,
			startPos.y + normal.y * normalLineLength,
			startPos.z + normal.z * normalLineLength
		);

		// �@���x�N�g���f�[�^�̃R�s�[
		flatNormalLineVertices.push_back(startPos);
		flatNormalLineVertices.push_back(endPos);
    }
    const size_t flatNormalLineVerticesBufferSize = sizeof(XMFLOAT3) * flatNormalLineVertices.size();

    // ** ���_�̃q�[�v�𐶐� START
    D3D12_HEAP_PROPERTIES uploadHeapProp = {};
    uploadHeapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC vertBufferDesc = {};
    vertBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertBufferDesc.Width = vertexBufferSize;
	vertBufferDesc.Height = 1;
    vertBufferDesc.DepthOrArraySize = 1;
    vertBufferDesc.MipLevels = 1;
    vertBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertBufferDesc.SampleDesc.Count = 1;
    vertBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    vertBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    result = device_->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &vertBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(vertUploadHeap_.ReleaseAndGetAddressOf())
    );
    if (FAILED(result)) {
        MessageBox(nullptr, L"���_�̃q�[�v�̐����Ɏ��s",
            L"Vertex Heap", MB_OK);
        bRunning_ = false;
    }

    D3D12_RESOURCE_DESC flatVertBufferDesc = vertBufferDesc;
    flatVertBufferDesc.Width = flatVertBufferSize;
    result = device_->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &vertBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(flatVertUploadHeap_.ReleaseAndGetAddressOf())
    );
    if (FAILED(result)) {
        MessageBox(nullptr, L"Flat���_�̃q�[�v�̐����Ɏ��s",
            L"Flat Vertex Heap", MB_OK);
        bRunning_ = false;
    }
	// ** ���_�̃q�[�v�𐶐� END

    // ** �C���f�b�N�X�̃q�[�v�𐶐� START
    D3D12_RESOURCE_DESC indexBufferDesc = {};
    indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // ���\�[�X���o�b�t�@�[�ł��邱�Ƃ�����
    indexBufferDesc.Width = indexBufferSize;
    indexBufferDesc.Height = 1;
    indexBufferDesc.DepthOrArraySize = 1;
    indexBufferDesc.MipLevels = 1;
    indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    indexBufferDesc.SampleDesc.Count = 1;
    indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    result = device_->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &indexBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(indexUploadHeap_.ReleaseAndGetAddressOf())
    );
    if (FAILED(result)) {
        MessageBox(nullptr, L"�C���f�b�N�X�̃q�[�v�̐����Ɏ��s",
            L"Index Heap", MB_OK);
        bRunning_ = false;
    }
    // ** �C���f�b�N�X�̃q�[�v�𐶐� END

    // ** �@���x�N�g���̃q�[�v�𐶐� START
    D3D12_RESOURCE_DESC normalLineBufferDesc = {};
    normalLineBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    normalLineBufferDesc.Width = normalLineVerticesBufferSize;
    normalLineBufferDesc.Height = 1;
    normalLineBufferDesc.DepthOrArraySize = 1;
    normalLineBufferDesc.MipLevels = 1;
    normalLineBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    normalLineBufferDesc.SampleDesc.Count = 1;
    normalLineBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    result = device_->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &normalLineBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(normalUploadHeap_.ReleaseAndGetAddressOf())
    );
    if (FAILED(result)) {
        MessageBox(nullptr, L"�@���x�N�g���̃q�[�v�̐����Ɏ��s",
            L"Normal Line Heap", MB_OK);
        bRunning_ = false;
    }

    D3D12_RESOURCE_DESC flatNormalLineBufferDesc = normalLineBufferDesc;
    flatNormalLineBufferDesc.Width = flatNormalLineVerticesBufferSize;
    result = device_->CreateCommittedResource(
        &uploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &flatNormalLineBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(flatNormalUploadHeap_.ReleaseAndGetAddressOf())
    );
    if (FAILED(result)) {
        MessageBox(nullptr, L"Flat Shading�p�@���x�N�g���̃q�[�v�̐����Ɏ��s",
            L"Flat Normal Line Heap", MB_OK);
        bRunning_ = false;
    }
    // ** �@���x�N�g���̃q�[�v�𐶐� END

    // ** �f�[�^�̃R�s�[ START
    // GPU�̉��z�A�h���X�ɒ��_�f�[�^���R�s�[
    vertUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&vertMapAddress_)); // ���_�̃}�b�s���O
    memcpy(vertMapAddress_, verticesData.data(), vertexBufferSize); // ���_�f�[�^�̃R�s�[�Amemcpy�̓f�[�^��raw�|�C���^�[��v������
	vertUploadHeap_->Unmap(0, nullptr); // �}�b�s���O����

    // Flat Shading�p�̒��_�f�[�^���R�s�[
    flatVertUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&flatVertMapAddress_));
    memcpy(flatVertMapAddress_, flatShadedVertices.data(), flatVertBufferSize);
    flatVertUploadHeap_->Unmap(0, nullptr);

    // �C���f�b�N�X�f�[�^���R�s�[
    indexUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&indexMapAddress_));
    memcpy(indexMapAddress_, indices.data(), indexBufferSize);
	indexUploadHeap_->Unmap(0, nullptr);

    // �@���x�N�g���f�[�^���R�s�[
    normalUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&normalMapAddress_));
    memcpy(normalMapAddress_, normalLineVertices.data(), normalLineVerticesBufferSize);
    normalUploadHeap_->Unmap(0, nullptr);

    // Flat Shading�p�@���x�N�g���f�[�^���R�s�[
    flatNormalUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&flatNormalMapAddress_));
    memcpy(flatNormalMapAddress_, flatNormalLineVertices.data(), flatNormalLineVerticesBufferSize);
    flatNormalUploadHeap_->Unmap(0, nullptr);
    // ** �f�[�^�̃R�s�[ END

    // ** �f�[�^�r���[�̐��� START
    // GPU���ǂ߂�悤�Ƀq�[�v�ɃR�s�[������ɁA�r���[���쐬����
    vertBufferView_.BufferLocation = vertUploadHeap_->GetGPUVirtualAddress();
    vertBufferView_.StrideInBytes = sizeof(VertexData); // 1���_�̃T�C�Y
    vertBufferView_.SizeInBytes = static_cast<UINT>(vertexBufferSize); // ���_�f�[�^�S�̂̃T�C�Y

    // Flat Shading�p�̒��_�r���[���쐬
    flatVertBufferView_.BufferLocation = flatVertUploadHeap_->GetGPUVirtualAddress();
    flatVertBufferView_.StrideInBytes = sizeof(VertexData);
    flatVertBufferView_.SizeInBytes = static_cast<UINT>(flatVertBufferSize);

    // �C���f�b�N�X�o�b�t�@�̃r���[���쐬
    indexBufferView_.BufferLocation = indexUploadHeap_->GetGPUVirtualAddress();
    indexBufferView_.Format = DXGI_FORMAT_R16_UINT;
    indexBufferView_.SizeInBytes = static_cast<UINT>(indexBufferSize);

    // �@���x�N�g���̃r���[���쐬
    normalBufferView_.BufferLocation = normalUploadHeap_->GetGPUVirtualAddress();
    normalBufferView_.StrideInBytes = sizeof(XMFLOAT3); // 1���_�̃T�C�Y
    normalBufferView_.SizeInBytes = static_cast<UINT>(normalLineVerticesBufferSize); // �@���x�N�g���f�[�^�S�̂̃T�C�Y

    // �@���x�N�g���̃r���[���쐬
    flatNormalBufferView_.BufferLocation = flatNormalUploadHeap_->GetGPUVirtualAddress();
    flatNormalBufferView_.StrideInBytes = sizeof(XMFLOAT3);
    flatNormalBufferView_.SizeInBytes = static_cast<UINT>(flatNormalLineVerticesBufferSize);
    // ** �f�[�^�r���[�̐��� END

    // �f�[�^�\���̒�`
	// �Z�}���e�B�b�N���A�Z�}���e�B�b�N�C���f�b�N�X�A�t�H�[�}�b�g�A�X���b�g�ԍ��A�o�C�g�I�t�Z�b�g�A���͕��ށA�C���X�^���X�f�[�^�X�e�b�v���[�g
	// ���_�̍\��
    D3D12_INPUT_ELEMENT_DESC _modelInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexData, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexData, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_ELEMENT_DESC _normalLineInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } // �@���x�N�g���̃f�[�^�͒��_�̂�
    };

    // ** Shader�̃R���p�C�� START
    // ���f���p�̃V�F�[�_�[
    ID3DBlob* vertexShaderBlob = nullptr;
    ID3DBlob* pixelShaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    result = D3DCompileFromFile(
        L"MyHLSL/BasicVertexShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicVS",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vertexShaderBlob,
        &errorBlob
    );
    if (FAILED(result)) {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            ::OutputDebugStringA("�t�@�C������������܂���");
        }
        else {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        bRunning_ = false;
    }

    result = D3DCompileFromFile(
        L"MyHLSL/BasicPixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicPS",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &pixelShaderBlob, 
        &errorBlob
    );
    if (FAILED(result)) {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            ::OutputDebugStringA("�t�@�C������������܂���");
        }
        else {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        bRunning_ = false;
    }

	// �f�o�b�O�p�̃V�F�[�_�[
    ID3DBlob* debugVertexShaderBlob = nullptr;
    ID3DBlob* debugPixelShaderBlob = nullptr;

    result = D3DCompileFromFile(
        L"MyHLSL/DebugVertexShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "DebugVS",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &debugVertexShaderBlob,
        &errorBlob
    );
    if (FAILED(result)) {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            ::OutputDebugStringA("�t�@�C������������܂���");
        }
        else {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        bRunning_ = false;
    }

    result = D3DCompileFromFile(
        L"MyHLSL/DebugPixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "DebugPS",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &debugPixelShaderBlob,
        &errorBlob
    );
    if (FAILED(result)) {
        if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            ::OutputDebugStringA("�t�@�C������������܂���");
        }
        else {
            std::string errstr;
            errstr.resize(errorBlob->GetBufferSize());
            std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errstr.begin());
            errstr += "\n";
            OutputDebugStringA(errstr.c_str());
        }
        bRunning_ = false;
    }
    // ** Shader�̃R���p�C�� END

    // ** Root Signature�̐��� START
    // RootSigDesc -> Serialize -> Create

    // ���[�g�V�O�l�`���[�̃��C���f�B�X�N���v�^�[�q�[�v�𐶐�
    D3D12_DESCRIPTOR_HEAP_DESC basicDescHeapDesc = {};
    basicDescHeapDesc.NumDescriptors = 1; // ����͒萔�o�b�t�@�r���[�̂�
    basicDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    basicDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    basicDescHeapDesc.NodeMask = 0;
    result = device_->CreateDescriptorHeap(&basicDescHeapDesc, IID_PPV_ARGS(basicDescHeap_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"���[�g�V�O�l�`���[�p�̃f�B�X�N���v�^�[�q�[�v�̐����Ɏ��s",
            L"Root Signature Descriptor Heap", MB_OK);
        bRunning_ = false;
    }

    // ���[�g�p�����[�^�i�f�B�X�N���v�^�[�e�[�u���j�̐���
    D3D12_ROOT_PARAMETER rootParam[1] = {};

    // �f�B�X�N���v�^�[�e�[�u���̐ݒ�i�f�B�X�N���v�^�[�����W�j
    //      ����̓f�B�X�N���v�^�q�[�v���1�̃r���[�i�f�B�X�N���v�^�j�����������Ă��܂��񂪁A
    //      �q�[�v��Ɏ�ނ������f�B�X�N���v�^���A�����đ��݂��Ă���ꍇ�ɂ́A
    //      �������܂Ƃ߂Ďg�p�ł���悤�Ƀf�B�X�N���v�^�����W���g���Ďw�肵�܂��B
    D3D12_DESCRIPTOR_RANGE basicDescRange[1] = {};
    basicDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // �萔�o�b�t�@�r���[(�s��)
    basicDescRange[0].NumDescriptors = 1; // �萔�o�b�t�@�̐�
    basicDescRange[0].BaseShaderRegister = 0; // ���W�X�^�ԍ� 0�� (b0)
    basicDescRange[0].RegisterSpace = 0; // ���W�X�^�X�y�[�X 0��
    basicDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // �e�[�u���̍Ō�ɒǉ�

    // ���[�g�p�����[�^�̒�`�ɖ߂�
    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    //rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam[0].DescriptorTable.pDescriptorRanges = &basicDescRange[0];
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;

    // �T���v���[�Ƃ́Auv�l�ɂ���ăe�N�X�`���f�[�^����ǂ��F�����o���������߂邽�߂̐ݒ�ł��B
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // �������̌J��Ԃ�
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // �c�����̌J��Ԃ�
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // ���s�����̌J��Ԃ�
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; // �{�[�_�[�J���[�͍�
    //samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // �e�N�X�`���̊g��k���ɂ̓��j�A�t�B���^�����O���g�p(���`���)
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; // ��Ԃ��Ȃ��i�j�A���X�g�l�C�o�[�@�F�ŋߖT��ԁj
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; // �~�b�v�}�b�v�̍ő僌�x��
    samplerDesc.MinLOD = 0.0f; // �~�b�v�}�b�v�̍ŏ����x��
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // �s�N�Z���V�F�[�_�[�Ŏg�p
    samplerDesc.ShaderRegister = 0; // ���W�X�^�ԍ� 0�� (s0)
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // ���T���v�����O���Ȃ�

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParam;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;

    ID3DBlob* rootSigBlob = nullptr;
    result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);
    if (FAILED(result)) {
        MessageBox(nullptr, L"Root Signature�̃V���A���C�Y�Ɏ��s",
            L"Root Signature", MB_OK);
        bRunning_ = false;
    }

    result = device_->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
        IID_PPV_ARGS(rootSignature_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"Root Signature�̐����Ɏ��s",
            L"Root Signature", MB_OK);
        bRunning_ = false;
    }
    // ** Root Signature�̐��� END

    // ** �O���t�B�b�N�p�C�v���C���X�e�[�g�̃I�u�W�F�N�g�𐶐� START
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature_.Get();
    // Shader�ݒ�
    psoDesc.InputLayout.pInputElementDescs = _modelInputLayout; // ���C�A�E�g�z��
    psoDesc.InputLayout.NumElements = _countof(_modelInputLayout); // ���C�A�E�g�z��
    psoDesc.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
    psoDesc.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();
    psoDesc.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
    psoDesc.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

    // RasterizerState�ݒ�
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //�J�����O���Ȃ�
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; //���g��h��Ԃ�
    psoDesc.RasterizerState.DepthClipEnable = TRUE; //�[�x�����̃N���b�s���O�͗L����
    psoDesc.RasterizerState.MultisampleEnable = FALSE; // �܂��A���`�F���͎g��Ȃ�
    psoDesc.RasterizerState.FrontCounterClockwise = false;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.AntialiasedLineEnable = false;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // BlendState�ݒ�
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
    renderTargetBlendDesc.BlendEnable = FALSE; // �ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
    renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    renderTargetBlendDesc.LogicOpEnable = FALSE; // �ЂƂ܂��_�����Z�͎g�p���Ȃ�
    psoDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;

    // DepthStencilState�ݒ�
    psoDesc.DepthStencilState.DepthEnable = true;                             // �[�x�o�b�t�@�[���g�p���邩
    psoDesc.DepthStencilState.StencilEnable = false;                          // �X�e���V���e�X�g���s����
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;                                // �[�x�o�b�t�@�[�Ŏg�p����t�H�[�}�b�g
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;         // ��������
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;    // �����������̗p����

    // RTV�ݒ�
    psoDesc.NumRenderTargets = 1;//���͂P�̂�
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//0�`1�ɐ��K�����ꂽRGBA

    // ���̑�
    psoDesc.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
    psoDesc.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

    // ** �p�C�v���C���X�e�[�g�̐��� START
    // psoDesc���ė��p���āA�h��Ԃ��p�̃p�C�v���C���X�e�[�g�𐶐�
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescSolid = psoDesc;
    result = device_->CreateGraphicsPipelineState(&psoDescSolid, IID_PPV_ARGS(psoSolid_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"�h��Ԃ��O���t�B�b�N�p�C�v���C���X�e�[�g�̐����Ɏ��s",
            L"Graphics Pipeline State", MB_OK);
        bRunning_ = false;
    }

    // psoDesc���ė��p���āA���C���[�t���[���p�̃p�C�v���C���X�e�[�g�𐶐�
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescWireframe = psoDesc;
    psoDescWireframe.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; // ���C���[�t���[�����[�h
    psoDescWireframe.VS.pShaderBytecode = debugVertexShaderBlob->GetBufferPointer();
    psoDescWireframe.VS.BytecodeLength = debugVertexShaderBlob->GetBufferSize();
    psoDescWireframe.PS.pShaderBytecode = debugPixelShaderBlob->GetBufferPointer();
    psoDescWireframe.PS.BytecodeLength = debugPixelShaderBlob->GetBufferSize();
    result = device_->CreateGraphicsPipelineState(&psoDescWireframe, IID_PPV_ARGS(psoWireframe_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"���C���[�t���[���O���t�B�b�N�p�C�v���C���X�e�[�g�̐����Ɏ��s",
            L"Graphics Pipeline State", MB_OK);
        bRunning_ = false;
    }

    // psoDesc���ė��p���āA�@���x�N�g���`��p�̃p�C�v���C���X�e�[�g�𐶐�
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescNormalLine = psoDesc;
    psoDescNormalLine.InputLayout.pInputElementDescs = _normalLineInputLayout; // �@���x�N�g���̃��C�A�E�g
    psoDescNormalLine.InputLayout.NumElements = _countof(_normalLineInputLayout); // ���C�A�E�g�z��
    psoDescNormalLine.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; // ���C���ō\��
    psoDescNormalLine.VS.pShaderBytecode = debugVertexShaderBlob->GetBufferPointer();
    psoDescNormalLine.VS.BytecodeLength = debugVertexShaderBlob->GetBufferSize();
    psoDescNormalLine.PS.pShaderBytecode = debugPixelShaderBlob->GetBufferPointer();
    psoDescNormalLine.PS.BytecodeLength = debugPixelShaderBlob->GetBufferSize();
    result = device_->CreateGraphicsPipelineState(&psoDescNormalLine, IID_PPV_ARGS(psoNormals_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"�@���x�N�g���̃O���t�B�b�N�p�C�v���C���X�e�[�g�̐����Ɏ��s",
            L"Graphics Pipeline State", MB_OK);
        bRunning_ = false;
    }
    // ** �p�C�v���C���X�e�[�g�̐��� END

    // �����g��Ȃ��̂ŉ��
    vertexShaderBlob->Release();
    pixelShaderBlob->Release();
	debugVertexShaderBlob->Release();
    debugPixelShaderBlob->Release();
    rootSigBlob->Release();
    if(errorBlob) errorBlob->Release();
    // ** �O���t�B�b�N�p�C�v���C���X�e�[�g�̃I�u�W�F�N�g�𐶐� END

    // ** �r���[�|�[�g�ƃV�U�[�̐ݒ� START
    viewport_.Width = static_cast<FLOAT>(WINDOW_WIDTH); // �o�͐�̕�(�s�N�Z����)
    viewport_.Height = static_cast<FLOAT>(WINDOW_HEIGHT); // �o�͐�̍���(�s�N�Z����)
    viewport_.TopLeftX = 0; // �o�͐�̍�����WX
    viewport_.TopLeftY = 0; // �o�͐�̍�����WY
    viewport_.MaxDepth = 1.0f; // �[�x�ő�l
    viewport_.MinDepth = 0.0f; // �[�x�ŏ��l

    sciRect_.top = 0;//�؂蔲������W
    sciRect_.left = 0;//�؂蔲�������W
    sciRect_.right = sciRect_.left + WINDOW_WIDTH; // �؂蔲���E���W
    sciRect_.bottom = sciRect_.top + WINDOW_HEIGHT; // �؂蔲�������W
    // ** �r���[�|�[�g�ƃV�U�[�̐ݒ� END

    cmdList_->Close();
    // �R�}���h���X�g�̎��s
    ID3D12CommandList* cmdlists[] = { cmdList_.Get() };
    cmdQueue_->ExecuteCommandLists(_countof(cmdlists), cmdlists);
    cmdQueue_->Signal(fence_.Get(), ++fenceValue_);

    // ** �萔�o�b�t�@�[�i���W�Ԋҍs��j�̐��� START
    // ���@1: �萔�o�b�t�@�[�̐��� �[�� �f�B�X�N���v�^�[�q�[�v �[�� �r���[(CBV) �[�� ���[�g�p�����[�^ �[�� ���[�g�V�O�l�`���[
    // ���@2: �萔�o�b�t�@�[�̐��� �[�� �r���[(CBV) �[�� ���[�g�p�����[�^ �[�� ���[�g�V�O�l�`���[ (�����̃f�B�X�N���v�^�[�q�[�v���g�p �[�� ���[�g�p�����[�^��ǉ�)

    // �蓮�ŃA���C�����g���w�肷��K�v������̂́A���\�[�X���������Ɏ蓮�Ŕz�u����ꍇ��A
    // ���ɓ���̍œK���P�[�X�Ń��\�[�X�������A���C�����g�𖾎��I�ɐ��䂷��K�v������ꍇ�����ł��B

    // ���̏ꍇ�ADirect3D 12�́A���\�[�X�^�C�v�i�e�N�X�`���Ȃǁj�ƃ��C�A�E�g�ݒ�iD3D12_TEXTURE_LAYOUT_UNKNOWN�Ȃǁj�Ɋ�Â��āA�������A���C�����g�������I�ɓK�p���܂��B 

    // wvpMat = world * view * projection
    // ���[���h�s��i�ړ��s�� * ��]�s�� * �g��s��j
    // �ړ��s��
    XMMATRIX tmpTransMat = XMMatrixTranslation(objXOffset_, objYOffset_, objZOffset_);
    wvpMatrices_.transMat = tmpTransMat;

    // ��]�s��
    XMMATRIX tmpRotMat = XMMatrixRotationY(XMConvertToRadians(15.0f));
    wvpMatrices_.rotMat = tmpRotMat;

    // �r���[�s��
    XMMATRIX viewMat = XMMatrixLookAtLH(XMLoadFloat3(&camPos_), XMLoadFloat3(&camTarg_), XMLoadFloat3(&camUp_));
    wvpMatrices_.viewMat = viewMat;

    // �v���W�F�N�V�����s��
    // ������F������͈͂��`��ɂ����ǂ�������(�N���b�s���O�{�����[��)
    XMMATRIX projMat = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f), // ����p
        static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
        0.1f,
        100.0f
    );
    wvpMatrices_.projMat = projMat;

    // �萔�o�b�t�@�[�̐���
    ID3D12Resource* constBuff = nullptr;
    CD3DX12_HEAP_PROPERTIES constUploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC constResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstantBuffer) + 0xff) & ~0xff); // 256�o�C�g�A���C�����g
    result = device_->CreateCommittedResource(
        &constUploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &constResDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constBuff)
    );
    if (FAILED(result)) {
        MessageBox(nullptr, L"�萔�o�b�t�@�[�̐����Ɏ��s",
            L"Constant Buffer", MB_OK);
        bRunning_ = false;
    }

    // ������Z�q���g����! �u�R�s�[�Ȃ̂�memcpy���g���v�Ǝv�����l�����邩������܂��񂪁A���̂悤�ɑ�����Z�q���g�����Ƃ��ł��܂��B
    constBuff->Map(0, nullptr, reinterpret_cast<void**>(&constBuffMapAddr_));
    //constBuffMapAddr_->wvpMat = XMMatrixTranspose(wvpMat); // HLSL�͗�D��Ȃ̂œ]�u���Ă���
    constBuffMapAddr_->wvpMat = wvpMatrices_;
	constBuffMapAddr_->lightDirection = XMLoadFloat3(&lightDirection_); // ���C�g�̕���
    constBuffMapAddr_->lightColor = XMLoadFloat3(&lightColor_); // ���C�g�̐F
    constBuffMapAddr_->ambientColor = XMLoadFloat3(&ambientColor_); // �A���r�G���g���C�g�̐F
	// �킴��Unmap���Ȃ���Update()�Ŗ��t���[���X�V����

    // CBV�̐���
    // CBV,SRV,UAV��3��HandleIncrementSize�������ƂȂ��Ă��܂��B�Ȃ������_�[�^�[�Q�b�g�r���[����ѐ[�x�X�e���V���r���[�̑傫���͂����Ƃ͈قȂ�̂ŁA���ӂ��܂��傤�B
    // CPU���Ńr���[���쐬���Ă��邽�߁AGetCPUDescriptorHandleForHeapStart()���g���܂��B
    D3D12_CPU_DESCRIPTOR_HANDLE basicHeapHandle = basicDescHeap_->GetCPUDescriptorHandleForHeapStart(); // basicDescHeap�̓��[�g�V�O�l�`���[�ɓo�^����Ă��郁�C���̃f�B�X�N���v�^�[�q�[�v

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);
	device_->CreateConstantBufferView(&cbvDesc, basicHeapHandle);
    // ** �萔�o�b�t�@�[�i���W�Ԋҍs��j�̐��� END

    return true;
}

void Update()
{
    // ���f���̃��[���h�s����X�V
    // �ړ��s��
    const XMMATRIX transMat = XMMatrixTranslation(objXOffset_, objYOffset_, objZOffset_);
    wvpMatrices_.transMat = transMat;

    // ��]�s��
    const XMMATRIX rotXMat = XMMatrixRotationX(XMConvertToRadians(objXRot_));
    const XMMATRIX rotYMat = XMMatrixRotationY(XMConvertToRadians(objYRot_));
    const XMMATRIX rotZMat = XMMatrixRotationZ(XMConvertToRadians(objZRot_));
    const XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
    wvpMatrices_.rotMat = rotMat;

    // �J�����̈ʒu���X�V
    camPos_ = XMFLOAT3{ camX_, camY_, camZ_ }; // ���݂�z���݈̂ړ��\

    // �r���[�s����X�V
    const XMMATRIX viewMat = XMMatrixLookAtLH(XMLoadFloat3(&camPos_), XMLoadFloat3(&camTarg_), XMLoadFloat3(&camUp_));
    wvpMatrices_.viewMat = viewMat;

    // �萔�o�b�t�@�[�̍X�V �i�s��̕����̂݁j
    memcpy(constBuffMapAddr_, &wvpMatrices_, sizeof(WVPMatrices));
}

void UpdatePipeline()
{
    HRESULT result;

    WaitForPreviousFrame();

    // �R�}���h�A���P�[�^�����Z�b�g
    result = cmdAllocator_[frameIndex_]->Reset();
    if (FAILED(result))
    {
        MessageBox(nullptr, L"�R�}���h�A���P�[�^�̃��Z�b�g�Ɏ��s",
            L"Command Allocator Reset", MB_OK);
        bRunning_ = false;
    }

    // �R�}���h���X�g�����Z�b�g
    result = cmdList_->Reset(cmdAllocator_[frameIndex_].Get(), nullptr);
    if (FAILED(result))
    {
        MessageBox(nullptr, L"�R�}���h���X�g�̃��Z�b�g�Ɏ��s",
            L"Command list Reset", MB_OK);
        bRunning_ = false;
    }

    // �o�b�N�o�b�t�@�ɕ`���O�Ƀ��\�[�X�̏�Ԃ�J��
    D3D12_RESOURCE_BARRIER _barrierDesc = {};
    _barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    _barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    _barrierDesc.Transition.pResource = backBuffers_[frameIndex_].Get();
    _barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    _barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    _barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    cmdList_->ResourceBarrier(1, &_barrierDesc);

    // �����_�[�^�[�Q�b�g���Z�b�g
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeaps_->GetCPUDescriptorHandleForHeapStart(), frameIndex_, rtvDescriptorSize_);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
    cmdList_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // �����_�[�^�[�Q�b�g�̃N���A
    float clearColor[] = { 0.8f, 0.8f, 1.0f, 1.0f };
    cmdList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ���[�g�V�O�l�`�����Z�b�g
    // GPU���Ƀf�B�X�N���v�^�[�e�[�u����ݒ肵�Ă���̂ŁAGetGPUDescriptorHandleForHeapStart()���g���܂�
    cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
    cmdList_->SetDescriptorHeaps(1, basicDescHeap_.GetAddressOf());
    D3D12_GPU_DESCRIPTOR_HANDLE basicDescHeapHandle = basicDescHeap_->GetGPUDescriptorHandleForHeapStart();
    cmdList_->SetGraphicsRootDescriptorTable(0, basicDescHeapHandle); // �萔�o�b�t�@�[�r���[

    // �r���[�|�[�g�ƃV�U�[���Z�b�g
    cmdList_->RSSetViewports(1, &viewport_);
    cmdList_->RSSetScissorRects(1, &sciRect_);

    // 3D���f���`��
    // �O���t�B�b�N�p�C�v���C���X�e�[�g���Z�b�g
	// Input Assembler�̐ݒ�
    cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    if (bSolidMode_)
    {
        cmdList_->SetPipelineState(psoSolid_.Get());
        if(bShadeSmooth_)
        {
            cmdList_->IASetVertexBuffers(0, 1, &vertBufferView_);
            cmdList_->IASetIndexBuffer(&indexBufferView_);
            cmdList_->DrawIndexedInstanced(static_cast<UINT>(trianglesBTM_.size() * 3), 1, 0, 0, 0);
        }
        else
        {
            cmdList_->IASetVertexBuffers(0, 1, &flatVertBufferView_);
            cmdList_->DrawInstanced(flatVertBufferView_.SizeInBytes / sizeof(VertexData), 1, 0, 0);
        }
    }
    if (bWireframeMode_)
    {
        cmdList_->SetPipelineState(psoWireframe_.Get());
        cmdList_->IASetVertexBuffers(0, 1, &vertBufferView_);
        cmdList_->IASetIndexBuffer(&indexBufferView_);
        cmdList_->DrawIndexedInstanced(static_cast<UINT>(trianglesBTM_.size() * 3), 1, 0, 0, 0);
    }
    if(bDrawNormals_)
    {
        // �@���x�N�g���`��
        cmdList_->SetPipelineState(psoNormals_.Get());
        cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        if(bShadeSmooth_)
        {
            cmdList_->IASetVertexBuffers(0, 1, &normalBufferView_);
            cmdList_->DrawInstanced(normalBufferView_.SizeInBytes / sizeof(XMFLOAT3), 1, 0, 0);
        }
        else
        {
	        cmdList_->IASetVertexBuffers(0, 1, &flatNormalBufferView_);
			cmdList_->DrawInstanced(flatNormalBufferView_.SizeInBytes / sizeof(XMFLOAT3), 1, 0, 0);
        }
    }

    // �o�b�N�o�b�t�@��؂�ւ���O�Ƀ��\�[�X�̏�Ԃ�J��
    _barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    _barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList_->ResourceBarrier(1, &_barrierDesc);

    // �R�}���h���X�g�����
    result = cmdList_->Close();
    if (FAILED(result))
    {
        MessageBox(nullptr, L"�R�}���h���X�g�̃N���[�Y�Ɏ��s",
            L"Command list Close", MB_OK);
        bRunning_ = false;
    }
}

void Render()
{
    HRESULT hr;

    UpdatePipeline();

    // �R�}���h�����s
    ID3D12CommandList* ppCommandLists[] = { cmdList_.Get() };
    cmdQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    hr = cmdQueue_->Signal(fence_.Get(), fenceValue_);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"�t�F���X�̃V�O�i���Ɏ��s",
            L"Fence Signal", MB_OK);
        bRunning_ = false;
    }

    // �o�b�N�o�b�t�@���t���b�v
    hr = swapchain_->Present(1, 0);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"�o�b�N�o�b�t�@�̃t���b�v�Ɏ��s",
            L"Swap chain present", MB_OK);
        bRunning_ = false;
    }
}

void WaitForPreviousFrame()
{
    HRESULT hr;

    // ���̃t���[���Ŏg���o�b�N�o�b�t�@�̃C���f�b�N�X���擾
    frameIndex_ = swapchain_->GetCurrentBackBufferIndex();

    // fence�̒l��fenceValue��菬�����ꍇ�́AGPU���܂��R�}���h���������Ă���ƈӖ�����
    if (fence_->GetCompletedValue() < fenceValue_)
    {
        // fence�̒l��fenceValue�ɂȂ鎞�̃C�x���g�R�[���o�b�N
        hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        if (FAILED(hr))
        {
            MessageBox(nullptr, L"fence�C�x���g�̐����Ɏ��s",
                L"Fence Event Completion", MB_OK);
            bRunning_ = false;
        }

        // fence�̒l��fenceValue�ɂȂ�܂ő҂�
        WaitForSingleObject(fenceEvent_, INFINITE);
    }

    // ���̃t���[���̂��߂Ƀt�F���X�̒l�𑝂₷
    fenceValue_++;
}