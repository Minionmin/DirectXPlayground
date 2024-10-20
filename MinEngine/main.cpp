#include "stdafx.h"

using namespace DirectX;

// 課題2のファイルを読み込む関数
// バッファーをchar*型にキャストしてバイナリーデータを直接読み込まないといけない、constはつけない
bool LoadBTMFile(const std::string& filePath, Header& header, std::vector<Triangle>& triangles, std::vector<Task2Vertex>& vertices, std::vector<XMFLOAT3>& normals)
{
    std::ifstream inFile(filePath, std::ios::binary); // パース元のファイルをバイナリモードで開く
    if (!inFile.is_open())
    {
        MessageBox(nullptr, L".btmファイルを開くことに失敗しました", L".btm is_open", MB_OK | MB_ICONERROR);
        return false;
    }

    // Headerの読み込み
    // reinterpret_cast<char*>を使って、char型のポインタにキャストして、バイトを直接読み込めるようにする
    inFile.read(reinterpret_cast<char*>(&header), sizeof(Header)); // 決めたサイズ分のバイトを読み込む
    if (inFile.gcount() != sizeof(Header)) // gcount()は読み込んだバイトの数を返す、一致しなければエラーメッセージを返す
    {
        MessageBox(nullptr, L".btmファイルHeaderの読み込みに失敗しました", L"Header読み込みエラー", MB_OK | MB_ICONERROR);
        return false;
    }

    // トライアングルの読み込み
    inFile.seekg(header.triangleOffset, std::ios::beg); // seekgでファイルポインタを最初からtriangleOffset分を移動させる
    triangles.resize(header.triangleCount); // トライアングルの数だけリサイズ（スペースを確保）
    // .data()でベクトルの最初の要素のポインタを取得
    inFile.read(reinterpret_cast<char*>(triangles.data()), sizeof(Triangle) * header.triangleCount); // trianglesバッファーにバイナリーデータを直接読み込む
    if (inFile.gcount() != sizeof(Triangle) * header.triangleCount)
    {
        MessageBox(nullptr, L".btmファイルのトライアングルデータの読み込みに失敗しました", L"トライアングルの読み込みエラー", MB_OK | MB_ICONERROR);
        return false;
    }

    // 頂点のの読み込み
    inFile.seekg(header.vertexOffset, std::ios::beg);
    vertices.resize(header.vertexCount);
    inFile.read(reinterpret_cast<char*>(vertices.data()), sizeof(Task2Vertex) * header.vertexCount);
    if (inFile.gcount() != sizeof(Task2Vertex) * header.vertexCount)
    {
        MessageBox(nullptr, L".btmファイルの頂点データの読み込みに失敗しました", L"頂点の読み込みエラー", MB_OK | MB_ICONERROR);
        return false;
    }

    // 法線ベクトルのの読み込み
    inFile.seekg(header.normalOffset, std::ios::beg);
    normals.resize(header.normalCount);
    inFile.read(reinterpret_cast<char*>(normals.data()), sizeof(XMFLOAT3) * header.normalCount);
    if (inFile.gcount() != sizeof(XMFLOAT3) * header.normalCount)
    {
        MessageBox(nullptr, L".btmファイルの法線ベクトルデータの読み込みに失敗しました", L"法線ベクトルの読み込みエラー", MB_OK | MB_ICONERROR);
        return false;
    }

    inFile.close();
    return true;
}

// プログラムのエントリーポイント
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    // ウインドウの初期化
    if (!InitWindow(hInstance, nShowCmd, FullScreen))
    {
        MessageBox(0, L"ウインドウの初期化が失敗しました",
            L"InitWindow", MB_OK);
        return 1;
    }

    if (!InitD3D())
    {
        MessageBox(0, L"Direct3D12の初期化に失敗しました",
            L"InitD3D", MB_OK);
        return 1;
    }

    // プログラムのメインループ
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
    wc.lpfnWndProc = WndProc; // ウィンドウプロシージャの関数を登録する
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
        MessageBox(NULL, L"ウインドウクラスの登録に失敗しました",
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
        MessageBox(NULL, L"ウインドウの初期化に失敗しました",
            L"hwnd", MB_OK | MB_ICONERROR);
        return false;
    }

    if (fullscreen)
    {
        SetWindowLong(hwnd, GWL_STYLE, 0);
    }

    ShowWindow(hwnd, ShowWnd);
    UpdateWindow(hwnd);

    // ウィンドウクラスはもういらないので登録解除
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return true;
}

// ウインドウプロシージャ・WndClassからのメッセージを処理する
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 'S': // 塗りつぶしモードの切り替え
            bSolidMode_ = !bSolidMode_;
            break;
        case 'W': // ワイヤーフレームモードの切り替え
            bWireframeMode_ = !bWireframeMode_;
            break;
		case 'M': // Flat/Smoothの切り替え
			bShadeSmooth_ = !bShadeSmooth_;
			break;
        case 'N': // 法線ベクトルの描画の切り替え
            bDrawNormals_ = !bDrawNormals_;
            break;
		case VK_ESCAPE: // ESCキーでプログラムを終了
            if (MessageBox(0, L"プログラムを終了しますか?",
                L"終了メッセージ", MB_YESNO | MB_ICONQUESTION) == IDYES)
            {
                bRunning_ = false;
                DestroyWindow(hwnd);
            }
            break;
        }
        return 0; // メッセージを処理したと報告するために0を返す

    case WM_RBUTTONDOWN : // マウス左クリックでカメラを回転可能状態にする
	{
        bCameraRotate_ = true;
        SetCapture(hwnd); // マウスのキャプチャを設定
        GetCursorPos(&prevMousePos_); // 現時点のマウス座標を取得
        ScreenToClient(hwnd, &prevMousePos_); // クライアント領域の座標に変換
        return 0;
	}

    case WM_LBUTTONDOWN: // マウスホイールクリックでカメラを移動可能状態にする
	{
        bCameraMove_ = true;
        SetCapture(hwnd);
        GetCursorPos(&prevMousePos_); // 現時点のマウス座標を取得
        ScreenToClient(hwnd, &prevMousePos_); // クライアント領域の座標に変換
        return 0;
	}

    case WM_MOUSEMOVE: // マウスの移動でx軸とy軸を移動する
	{
        GetCursorPos(&currMousePos_);
        ScreenToClient(hwnd, &currMousePos_);
        // マウスの移動距離を取得
        const float deltaX = static_cast<float>(currMousePos_.x - prevMousePos_.x);
        const float deltaY = static_cast<float>(currMousePos_.y - prevMousePos_.y);
        if (bCameraMove_)
        {
            // x軸とy軸を移動（モデルをその逆方向に移動する）
            objXOffset_ += deltaX * 0.01f; // 定数はマウスの感度
            objYOffset_ -= deltaY * 0.01f; // -yにすることで、マウスの移動方向とモデルの移動方向を合わせる
        }
        else if (bCameraRotate_)
        {
            // x軸とy軸を回転（モデルをその逆方向に回転する）
            objXRot_ -= deltaY * 0.5f;
            objYRot_ -= deltaX * 0.5f;
        }
        // 現時点のマウス座標を保存
        prevMousePos_ = currMousePos_;
        return 0;
	    }

    case WM_RBUTTONUP:
	{
        bCameraRotate_ = false;
        ReleaseCapture(); // マウスのキャプチャを解放
        return 0;
	}

    case WM_LBUTTONUP:
	    {
            bCameraMove_ = false;
            ReleaseCapture();
            return 0;
	    }

    case WM_MOUSEWHEEL: // マウスホイールでカメラのz軸を移動する
        {
            const float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            camZ_ += 0.01f * zDelta; // 定数はカメラの感度
            return 0;
        }
    
    case WM_DESTROY:
	    {
            bRunning_ = false;
            PostQuitMessage(0);
            return 0; // メッセージを処理したと報告するために0を返す   
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
            DispatchMessage(&msg); // ウィンドウプロシージャにメッセージを送る
        }
        else {
            // ゲームロジック
            Update();
            Render();
        }
    }
}

void AverageNormals(const std::vector<VertexData>& verticesData, const std::vector<uint16_t>& indices, const std::vector<XMFLOAT3>& normals, std::vector<XMFLOAT3>& accumulatedNormals)
{
	std::vector<int> normalCount(verticesData.size(), 0); // 指定した頂点に対する法線の数をカウントする配列

	// 各頂点の法線ベクトルを計算する
	for (size_t i = 0; i < indices.size(); i += 3) // 1つのトライアングルに対して3つの頂点があるため、+3する（ループで１つのトライアングルを処理する）
	{
		const int triangleNormalIndex = static_cast<int>(i) / 3; // 3つの頂点に対してトライアングル1つの法線にする

		// 1つのトライアングルに対して、頂点の法線を蓄積する
		for (int j = 0; j < 3; ++j)
		{
			const int vertexIndex = indices[i + j];  // 頂点のインデックスを取得、（例）0, 1, 2 など
			accumulatedNormals[vertexIndex].x += normals[triangleNormalIndex].x;
			accumulatedNormals[vertexIndex].y += normals[triangleNormalIndex].y;
			accumulatedNormals[vertexIndex].z += normals[triangleNormalIndex].z;
			normalCount[vertexIndex]++;  // この頂点にいくつの法線があるのかを数える
		}
	}

	// 計算した法線を正規化して平均化する
	for (size_t i = 0; i < accumulatedNormals.size(); ++i)
	{
		if (normalCount[i] > 0)
		{
			// 平均化
			accumulatedNormals[i].x /= normalCount[i];
			accumulatedNormals[i].y /= normalCount[i];
			accumulatedNormals[i].z /= normalCount[i];

			// 正規化
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

    // ** D3Dデバイスの生成 START
    //フィーチャレベル列挙
    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    // デバイスのの検索
    HRESULT result = S_OK;
    if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory_)))) {
        if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory_)))) {
            return false;
        }
    }

    // GPUを集める
    std::vector <IDXGIAdapter1*> adapters;
    IDXGIAdapter1* tmpAdapter = nullptr;
    for (int i = 0; dxgiFactory_->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        adapters.push_back(tmpAdapter);
    }

    // NVIDIAのGPUを選択
    for (IDXGIAdapter1* adpt : adapters) {
        DXGI_ADAPTER_DESC adesc = {};
        adpt->GetDesc(&adesc); // GPUの情報を取得
        std::wstring strDesc = adesc.Description;

        // NVIDIAという文字列が含まれているか
        if (strDesc.find(L"NVIDIA") != std::string::npos) {
            tmpAdapter = adpt;
            break;
        }
    }

    // 獲得したGPUで最適なレベルをチェックして、Direct3Dデバイスの初期化
    bool bDeviceCreated = false;
    for (D3D_FEATURE_LEVEL l : levels) {
        if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(device_.ReleaseAndGetAddressOf())) == S_OK) {
            bDeviceCreated = true;
            break;
        }
    }

    if (!bDeviceCreated) {
        MessageBox(nullptr, L"D3Dデバイスの生成に失敗",
            L"D3Dデバイス", MB_OK);
        return false;
    }
    // ** D3Dデバイスの生成 END

    // ** Command AllocatorとCommand Listの生成 START
    for (unsigned int i = 0; i < frameBufferCount_; ++i)
    {
        result = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdAllocator_[i].ReleaseAndGetAddressOf()));
        if (FAILED(result))
        {
            MessageBox(nullptr, L"CommandAllocatorの生成に失敗",
                L"Command Allocator", MB_OK);
            return false;
        }
    }

    result = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator_[0].Get(),
        nullptr, IID_PPV_ARGS(cmdList_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"CommandAllocatorの生成に失敗",
            L"Command Allocator", MB_OK);
        return false;
    }
    // コマンドリストはレコードしている状態で生成されるため、Closeしておく
    // 後でテクスチャーの転送により、そのまま使えるようにしておく
    //cmdList_->Close();
    // ** Command AllocatorとCommand Listの生成 END

    // ** Command Queueの生成 START
    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.NodeMask = 0;
    cqDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    result = device_->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(cmdQueue_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"CommandQueueの生成に失敗",
            L"Command Queue", MB_OK);
        return false;
    }
    // ** Command Queueの生成 End

    // ** Swap Chainの生成 START
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
    swapchainDesc.Width = WINDOW_WIDTH;
    swapchainDesc.Height = WINDOW_HEIGHT;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.Stereo = false;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;
    swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    swapchainDesc.BufferCount = frameBufferCount_; // ダブルバッファ
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
        MessageBox(nullptr, L"Swap Chainの生成に失敗",
            L"Swap Chain", MB_OK);
        return false;
    }

    // レンダーターゲットビュー用のDescriptor Heapを生成
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //レンダーターゲットビューなので当然RTV
    heapDesc.NodeMask = 0;
    heapDesc.NumDescriptors = 2; // 表裏の２つ
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // 特に指定なし
    result = device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(rtvHeaps_.ReleaseAndGetAddressOf()));

    // RTV Descriptorの生成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    //rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ガンマ補正あり
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; // ガンマ補正なし
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    // RTVの生成
    DXGI_SWAP_CHAIN_DESC swcDesc = {};
    result = swapchain_->GetDesc(&swcDesc); // スワップチェインの設定を取得
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeaps_->GetCPUDescriptorHandleForHeapStart());
    rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // RTVディスクリプタのサイズを取得
    for (size_t i = 0; i < frameBufferCount_; ++i) {
        result = swapchain_->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(backBuffers_[i].ReleaseAndGetAddressOf())); // Swap Chainからバックバッファを取得
        if (FAILED(result))
        {
            MessageBox(nullptr, L"バックバッファの生成に失敗",
                L"Back Buffer", MB_OK);
            return false;
        }
        device_->CreateRenderTargetView(backBuffers_[i].Get(), &rtvDesc, rtvHandle); // Swap Chainから取得したバックバッファをRTVに変換し、RTVのDescriptor Heapに格納
        //rtvHandle.ptr += rtvDescriptorSize_; // 次のハンドルへ移動
        rtvHandle.Offset(1, rtvDescriptorSize_);
    }
    // ** Swap Chainの生成 END

    // ** Depth Bufferの生成 START
    // 深度バッファービュー
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    ID3D12DescriptorHeap* dsvHeap = nullptr;
    device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap_.ReleaseAndGetAddressOf()));

    // 深度バッファの生成
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

    // クリアバリューの設定
    D3D12_CLEAR_VALUE _depthClearValue = {};
    _depthClearValue.DepthStencil.Depth = 1.0f;      //深さ１(最大値)でクリア
    _depthClearValue.Format = DXGI_FORMAT_D32_FLOAT; //32bit深度値としてクリア

    device_->CreateCommittedResource(
        &depthHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &depthResDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
        &_depthClearValue,
        IID_PPV_ARGS(depthBuffer_.ReleaseAndGetAddressOf()));

    // 深度バッファービュー作成
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    device_->CreateDepthStencilView(depthBuffer_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());
    // ** Depth Bufferの生成 END

    // ** Fenceの生成 START
    result = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"フェンスの生成に失敗",
            L"Fence", MB_OK);
        return false;
    }
    fenceValue_ = 0;

    // Fence Eventの生成
    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (fenceEvent_ == nullptr)
    {
        MessageBox(nullptr, L"フェンスイベントの生成に失敗",
            L"FenceEvent", MB_OK);
        bRunning_ = false;
    }
    // ** Fenceの生成 END

    // 課題2: .mqo -> .btmのバイナリーファイルに変換してから読み込み、ビューポートに表示する
    // .btmのフォーマットは
    // Header部分
    //    データファイル名			4Byte
    //    変換したコンバータのバージョン		1Byte
    //    トライアングル構造体数			2Byte
    //    頂点数					2Byte
    //    法線数					2Byte
    //    トライアングル構造体へのオフセット	4Byte
    //    頂点データへのオフセット		4Byte
    //    法線データへのオフセット		4Byte
    // Body部分
    //    トライアングル構造体データ		2Byte*4 (頂点のインデックスが3つと法線のインデックス1つ） 
	//    頂点データ				float*3 ( x, y, z ) 
	//    法線データ				float*3 ( x, y, z )

	// .btmファイルの読み込み
    // ファイルパス、ヘッダバッファ、トライアングルバッファ、頂点バッファ、法線ベクトルバッファ
    if (!LoadBTMFile(modelFilePath_, headerBTM_, trianglesBTM_, verticesBTM_, normalsBTM_))
    {
        MessageBox(nullptr, L"Failed to load .btm file.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }
    // 12 tris 8 verts 12 normals
    // 36 indices (12 tris * 3), 8 vertices, 12 normals

    // 頂点のデータ（POSITIONとNORMAL）
    std::vector<VertexData> verticesData;
    verticesData.reserve(verticesBTM_.size()); // 読み込んだ頂点数分のメモリを確保
    for (size_t i = 0; i < verticesBTM_.size(); ++i)
    {
        VertexData vd;
        vd.position = XMFLOAT3(verticesBTM_[i].x, verticesBTM_[i].y, verticesBTM_[i].z);
        vd.normal = normalsBTM_[i];
        verticesData.push_back(vd);
    }
    const size_t vertexBufferSize = sizeof(VertexData) * verticesData.size();

	// インデックスのデータ
    size_t indexCount = trianglesBTM_.size() * 3; // トライアングルに3つのインデックスがあるため
    // トライアングル構造体データが2Byte*4なのでuint16_tを使う
    size_t indexBufferSize = sizeof(uint16_t) * indexCount;
    std::vector<uint16_t> indices;
    indices.reserve(indexCount);
    for (const Triangle& tri : trianglesBTM_)
    {
        indices.push_back(tri.v1);
        indices.push_back(tri.v2);
        indices.push_back(tri.v3);
    }

    // 法線ベクトルのデータ
    const float normalLineLength = 0.2f; // 法線ベクトルの長さ
    std::vector<XMFLOAT3> normalLineVertices; // 法線ベクトルの頂点データ
    normalLineVertices.reserve(indices.size() * 2); // 法線ベクトルの頂点数はインデックス数の2倍 （ベース頂点とターゲット頂点）
    // 法線ベクトルを平均化して使うと、塗りつぶしがSmoothスタイルになる
    // 各頂点に対して法線を平均化して法線ラインを生成
    std::vector<XMFLOAT3> accumulatedNormals(verticesData.size(), XMFLOAT3(0.0f, 0.0f, 0.0f)); // 各頂点の法線を蓄積する配列
    AverageNormals(verticesData, indices, normalsBTM_, accumulatedNormals); // インデックスに影響する各トライアングルの法線を合わせて平均化する

    // 計算された法線ベクトルを使って法線データを生成
    for (size_t i = 0; i < indices.size(); ++i)
    {
        // ベース頂点
        XMFLOAT3 startPos = verticesData[indices[i]].position;

        // この頂点に対する平均化された法線
        XMFLOAT3 averagedNormal = accumulatedNormals[indices[i]];

        // ターゲット頂点
        XMFLOAT3 endPos = XMFLOAT3(
            startPos.x + averagedNormal.x * normalLineLength,
            startPos.y + averagedNormal.y * normalLineLength,
            startPos.z + averagedNormal.z * normalLineLength
        );

        // 法線ベクトルデータのコピー
        normalLineVertices.push_back(startPos);
        normalLineVertices.push_back(endPos);
    }

    // 平均化された法線を使って頂点データを調整
    for (size_t i = 0; i < verticesData.size(); ++i)
    {
        verticesData[i].normal = accumulatedNormals[i];
    }
    const size_t normalLineVerticesBufferSize = sizeof(XMFLOAT3) * normalLineVertices.size();

    // Flat Shading用の頂点データを生成
    std::vector<VertexData> flatShadedVertices;  // Flat Shading用の頂点データ
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        // トライアングルの3つの頂点を取得
		VertexData v1 = verticesData[indices[i]];
		VertexData v2 = verticesData[indices[i + 1]];
		VertexData v3 = verticesData[indices[i + 2]];

		// バイナリファイルからのトライアングルの法線を参照
    	const XMFLOAT3 normal = normalsBTM_[i / 3];

		// トライアングルの3つの頂点に法線を設定
        v1.normal = normal;
        v2.normal = normal;
		v3.normal = normal;

		// 法線を設定した頂点を追加
		flatShadedVertices.push_back(v1);
		flatShadedVertices.push_back(v2);
		flatShadedVertices.push_back(v3);
    }
    const size_t flatVertBufferSize = sizeof(VertexData) * flatShadedVertices.size();

    // Flat Shading用の法線ベクトルのデータを生成
    std::vector<XMFLOAT3> flatNormalLineVertices;
    flatNormalLineVertices.reserve(indices.size() * 2);
    for(size_t i = 0; i < indices.size(); i++)
    {
    	// ベース頂点
		XMFLOAT3 startPos = flatShadedVertices[i].position;

		// この頂点に対する法線
		XMFLOAT3 normal = flatShadedVertices[i].normal;

		// ターゲット頂点
		XMFLOAT3 endPos = XMFLOAT3(
			startPos.x + normal.x * normalLineLength,
			startPos.y + normal.y * normalLineLength,
			startPos.z + normal.z * normalLineLength
		);

		// 法線ベクトルデータのコピー
		flatNormalLineVertices.push_back(startPos);
		flatNormalLineVertices.push_back(endPos);
    }
    const size_t flatNormalLineVerticesBufferSize = sizeof(XMFLOAT3) * flatNormalLineVertices.size();

    // ** 頂点のヒープを生成 START
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
        MessageBox(nullptr, L"頂点のヒープの生成に失敗",
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
        MessageBox(nullptr, L"Flat頂点のヒープの生成に失敗",
            L"Flat Vertex Heap", MB_OK);
        bRunning_ = false;
    }
	// ** 頂点のヒープを生成 END

    // ** インデックスのヒープを生成 START
    D3D12_RESOURCE_DESC indexBufferDesc = {};
    indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; // リソースがバッファーであることを示す
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
        MessageBox(nullptr, L"インデックスのヒープの生成に失敗",
            L"Index Heap", MB_OK);
        bRunning_ = false;
    }
    // ** インデックスのヒープを生成 END

    // ** 法線ベクトルのヒープを生成 START
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
        MessageBox(nullptr, L"法線ベクトルのヒープの生成に失敗",
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
        MessageBox(nullptr, L"Flat Shading用法線ベクトルのヒープの生成に失敗",
            L"Flat Normal Line Heap", MB_OK);
        bRunning_ = false;
    }
    // ** 法線ベクトルのヒープを生成 END

    // ** データのコピー START
    // GPUの仮想アドレスに頂点データをコピー
    vertUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&vertMapAddress_)); // 頂点のマッピング
    memcpy(vertMapAddress_, verticesData.data(), vertexBufferSize); // 頂点データのコピー、memcpyはデータのrawポインターを要求する
	vertUploadHeap_->Unmap(0, nullptr); // マッピング解除

    // Flat Shading用の頂点データをコピー
    flatVertUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&flatVertMapAddress_));
    memcpy(flatVertMapAddress_, flatShadedVertices.data(), flatVertBufferSize);
    flatVertUploadHeap_->Unmap(0, nullptr);

    // インデックスデータをコピー
    indexUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&indexMapAddress_));
    memcpy(indexMapAddress_, indices.data(), indexBufferSize);
	indexUploadHeap_->Unmap(0, nullptr);

    // 法線ベクトルデータをコピー
    normalUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&normalMapAddress_));
    memcpy(normalMapAddress_, normalLineVertices.data(), normalLineVerticesBufferSize);
    normalUploadHeap_->Unmap(0, nullptr);

    // Flat Shading用法線ベクトルデータをコピー
    flatNormalUploadHeap_->Map(0, nullptr, reinterpret_cast<void**>(&flatNormalMapAddress_));
    memcpy(flatNormalMapAddress_, flatNormalLineVertices.data(), flatNormalLineVerticesBufferSize);
    flatNormalUploadHeap_->Unmap(0, nullptr);
    // ** データのコピー END

    // ** データビューの生成 START
    // GPUが読めるようにヒープにコピーした後に、ビューを作成する
    vertBufferView_.BufferLocation = vertUploadHeap_->GetGPUVirtualAddress();
    vertBufferView_.StrideInBytes = sizeof(VertexData); // 1頂点のサイズ
    vertBufferView_.SizeInBytes = static_cast<UINT>(vertexBufferSize); // 頂点データ全体のサイズ

    // Flat Shading用の頂点ビューを作成
    flatVertBufferView_.BufferLocation = flatVertUploadHeap_->GetGPUVirtualAddress();
    flatVertBufferView_.StrideInBytes = sizeof(VertexData);
    flatVertBufferView_.SizeInBytes = static_cast<UINT>(flatVertBufferSize);

    // インデックスバッファのビューを作成
    indexBufferView_.BufferLocation = indexUploadHeap_->GetGPUVirtualAddress();
    indexBufferView_.Format = DXGI_FORMAT_R16_UINT;
    indexBufferView_.SizeInBytes = static_cast<UINT>(indexBufferSize);

    // 法線ベクトルのビューを作成
    normalBufferView_.BufferLocation = normalUploadHeap_->GetGPUVirtualAddress();
    normalBufferView_.StrideInBytes = sizeof(XMFLOAT3); // 1頂点のサイズ
    normalBufferView_.SizeInBytes = static_cast<UINT>(normalLineVerticesBufferSize); // 法線ベクトルデータ全体のサイズ

    // 法線ベクトルのビューを作成
    flatNormalBufferView_.BufferLocation = flatNormalUploadHeap_->GetGPUVirtualAddress();
    flatNormalBufferView_.StrideInBytes = sizeof(XMFLOAT3);
    flatNormalBufferView_.SizeInBytes = static_cast<UINT>(flatNormalLineVerticesBufferSize);
    // ** データビューの生成 END

    // データ構造の定義
	// セマンティック名、セマンティックインデックス、フォーマット、スロット番号、バイトオフセット、入力分類、インスタンスデータステップレート
	// 頂点の構造
    D3D12_INPUT_ELEMENT_DESC _modelInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexData, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexData, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_ELEMENT_DESC _normalLineInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 } // 法線ベクトルのデータは頂点のみ
    };

    // ** Shaderのコンパイル START
    // モデル用のシェーダー
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
            ::OutputDebugStringA("ファイルが見当たりません");
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
            ::OutputDebugStringA("ファイルが見当たりません");
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

	// デバッグ用のシェーダー
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
            ::OutputDebugStringA("ファイルが見当たりません");
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
            ::OutputDebugStringA("ファイルが見当たりません");
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
    // ** Shaderのコンパイル END

    // ** Root Signatureの生成 START
    // RootSigDesc -> Serialize -> Create

    // ルートシグネチャーのメインディスクリプターヒープを生成
    D3D12_DESCRIPTOR_HEAP_DESC basicDescHeapDesc = {};
    basicDescHeapDesc.NumDescriptors = 1; // 今回は定数バッファビューのみ
    basicDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    basicDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    basicDescHeapDesc.NodeMask = 0;
    result = device_->CreateDescriptorHeap(&basicDescHeapDesc, IID_PPV_ARGS(basicDescHeap_.ReleaseAndGetAddressOf()));
    if (FAILED(result))
    {
        MessageBox(nullptr, L"ルートシグネチャー用のディスクリプターヒープの生成に失敗",
            L"Root Signature Descriptor Heap", MB_OK);
        bRunning_ = false;
    }

    // ルートパラメータ（ディスクリプターテーブル）の生成
    D3D12_ROOT_PARAMETER rootParam[1] = {};

    // ディスクリプターテーブルの設定（ディスクリプターレンジ）
    //      今回はディスクリプタヒープ上に1つのビュー（ディスクリプタ）しか生成していませんが、
    //      ヒープ上に種類が同じディスクリプタが連続して存在している場合には、
    //      それらをまとめて使用できるようにディスクリプタレンジを使って指定します。
    D3D12_DESCRIPTOR_RANGE basicDescRange[1] = {};
    basicDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // 定数バッファビュー(行列)
    basicDescRange[0].NumDescriptors = 1; // 定数バッファの数
    basicDescRange[0].BaseShaderRegister = 0; // レジスタ番号 0番 (b0)
    basicDescRange[0].RegisterSpace = 0; // レジスタスペース 0番
    basicDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // テーブルの最後に追加

    // ルートパラメータの定義に戻る
    rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    //rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam[0].DescriptorTable.pDescriptorRanges = &basicDescRange[0];
    rootParam[0].DescriptorTable.NumDescriptorRanges = 1;

    // サンプラーとは、uv値によってテクスチャデータからどう色を取り出すかを決めるための設定です。
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 横方向の繰り返し
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 縦方向の繰り返し
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 奥行方向の繰り返し
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK; // ボーダーカラーは黒
    //samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // テクスチャの拡大縮小にはリニアフィルタリングを使用(線形補間)
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT; // 補間しない（ニアレストネイバー法：最近傍補間）
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; // ミップマップの最大レベル
    samplerDesc.MinLOD = 0.0f; // ミップマップの最小レベル
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // ピクセルシェーダーで使用
    samplerDesc.ShaderRegister = 0; // レジスタ番号 0番 (s0)
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // リサンプリングしない

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = rootParam;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &samplerDesc;

    ID3DBlob* rootSigBlob = nullptr;
    result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);
    if (FAILED(result)) {
        MessageBox(nullptr, L"Root Signatureのシリアライズに失敗",
            L"Root Signature", MB_OK);
        bRunning_ = false;
    }

    result = device_->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(),
        IID_PPV_ARGS(rootSignature_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"Root Signatureの生成に失敗",
            L"Root Signature", MB_OK);
        bRunning_ = false;
    }
    // ** Root Signatureの生成 END

    // ** グラフィックパイプラインステートのオブジェクトを生成 START
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature_.Get();
    // Shader設定
    psoDesc.InputLayout.pInputElementDescs = _modelInputLayout; // レイアウト配列
    psoDesc.InputLayout.NumElements = _countof(_modelInputLayout); // レイアウト配列数
    psoDesc.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
    psoDesc.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();
    psoDesc.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
    psoDesc.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

    // RasterizerState設定
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE; //カリングしない
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID; //中身を塗りつぶす
    psoDesc.RasterizerState.DepthClipEnable = TRUE; //深度方向のクリッピングは有効に
    psoDesc.RasterizerState.MultisampleEnable = FALSE; // まだアンチェリは使わない
    psoDesc.RasterizerState.FrontCounterClockwise = false;
    psoDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    psoDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    psoDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    psoDesc.RasterizerState.AntialiasedLineEnable = false;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // BlendState設定
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
    psoDesc.BlendState.IndependentBlendEnable = FALSE;

    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
    renderTargetBlendDesc.BlendEnable = FALSE; // ひとまず加算や乗算やαブレンディングは使用しない
    renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    renderTargetBlendDesc.LogicOpEnable = FALSE; // ひとまず論理演算は使用しない
    psoDesc.BlendState.RenderTarget[0] = renderTargetBlendDesc;

    // DepthStencilState設定
    psoDesc.DepthStencilState.DepthEnable = true;                             // 深度バッファーを使用するか
    psoDesc.DepthStencilState.StencilEnable = false;                          // ステンシルテストを行うか
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;                                // 深度バッファーで使用するフォーマット
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;         // 書き込む
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;    // 小さい方を採用する

    // RTV設定
    psoDesc.NumRenderTargets = 1;//今は１つのみ
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//0～1に正規化されたRGBA

    // その他
    psoDesc.SampleDesc.Count = 1;//サンプリングは1ピクセルにつき１
    psoDesc.SampleDesc.Quality = 0;//クオリティは最低
    psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//ストリップ時のカットなし
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//三角形で構成

    // ** パイプラインステートの生成 START
    // psoDescを再利用して、塗りつぶし用のパイプラインステートを生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescSolid = psoDesc;
    result = device_->CreateGraphicsPipelineState(&psoDescSolid, IID_PPV_ARGS(psoSolid_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"塗りつぶしグラフィックパイプラインステートの生成に失敗",
            L"Graphics Pipeline State", MB_OK);
        bRunning_ = false;
    }

    // psoDescを再利用して、ワイヤーフレーム用のパイプラインステートを生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescWireframe = psoDesc;
    psoDescWireframe.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; // ワイヤーフレームモード
    psoDescWireframe.VS.pShaderBytecode = debugVertexShaderBlob->GetBufferPointer();
    psoDescWireframe.VS.BytecodeLength = debugVertexShaderBlob->GetBufferSize();
    psoDescWireframe.PS.pShaderBytecode = debugPixelShaderBlob->GetBufferPointer();
    psoDescWireframe.PS.BytecodeLength = debugPixelShaderBlob->GetBufferSize();
    result = device_->CreateGraphicsPipelineState(&psoDescWireframe, IID_PPV_ARGS(psoWireframe_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"ワイヤーフレームグラフィックパイプラインステートの生成に失敗",
            L"Graphics Pipeline State", MB_OK);
        bRunning_ = false;
    }

    // psoDescを再利用して、法線ベクトル描画用のパイプラインステートを生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescNormalLine = psoDesc;
    psoDescNormalLine.InputLayout.pInputElementDescs = _normalLineInputLayout; // 法線ベクトルのレイアウト
    psoDescNormalLine.InputLayout.NumElements = _countof(_normalLineInputLayout); // レイアウト配列数
    psoDescNormalLine.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE; // ラインで構成
    psoDescNormalLine.VS.pShaderBytecode = debugVertexShaderBlob->GetBufferPointer();
    psoDescNormalLine.VS.BytecodeLength = debugVertexShaderBlob->GetBufferSize();
    psoDescNormalLine.PS.pShaderBytecode = debugPixelShaderBlob->GetBufferPointer();
    psoDescNormalLine.PS.BytecodeLength = debugPixelShaderBlob->GetBufferSize();
    result = device_->CreateGraphicsPipelineState(&psoDescNormalLine, IID_PPV_ARGS(psoNormals_.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        MessageBox(nullptr, L"法線ベクトルのグラフィックパイプラインステートの生成に失敗",
            L"Graphics Pipeline State", MB_OK);
        bRunning_ = false;
    }
    // ** パイプラインステートの生成 END

    // もう使わないので解放
    vertexShaderBlob->Release();
    pixelShaderBlob->Release();
	debugVertexShaderBlob->Release();
    debugPixelShaderBlob->Release();
    rootSigBlob->Release();
    if(errorBlob) errorBlob->Release();
    // ** グラフィックパイプラインステートのオブジェクトを生成 END

    // ** ビューポートとシザーの設定 START
    viewport_.Width = static_cast<FLOAT>(WINDOW_WIDTH); // 出力先の幅(ピクセル数)
    viewport_.Height = static_cast<FLOAT>(WINDOW_HEIGHT); // 出力先の高さ(ピクセル数)
    viewport_.TopLeftX = 0; // 出力先の左上座標X
    viewport_.TopLeftY = 0; // 出力先の左上座標Y
    viewport_.MaxDepth = 1.0f; // 深度最大値
    viewport_.MinDepth = 0.0f; // 深度最小値

    sciRect_.top = 0;//切り抜き上座標
    sciRect_.left = 0;//切り抜き左座標
    sciRect_.right = sciRect_.left + WINDOW_WIDTH; // 切り抜き右座標
    sciRect_.bottom = sciRect_.top + WINDOW_HEIGHT; // 切り抜き下座標
    // ** ビューポートとシザーの設定 END

    cmdList_->Close();
    // コマンドリストの実行
    ID3D12CommandList* cmdlists[] = { cmdList_.Get() };
    cmdQueue_->ExecuteCommandLists(_countof(cmdlists), cmdlists);
    cmdQueue_->Signal(fence_.Get(), ++fenceValue_);

    // ** 定数バッファー（座標返還行列）の生成 START
    // 方法1: 定数バッファーの生成 ー＞ ディスクリプターヒープ ー＞ ビュー(CBV) ー＞ ルートパラメータ ー＞ ルートシグネチャー
    // 方法2: 定数バッファーの生成 ー＞ ビュー(CBV) ー＞ ルートパラメータ ー＞ ルートシグネチャー (既存のディスクリプターヒープを使用 ー＞ ルートパラメータを追加)

    // 手動でアライメントを指定する必要があるのは、リソースをメモリに手動で配置する場合や、
    // 非常に特定の最適化ケースでリソースメモリアライメントを明示的に制御する必要がある場合だけです。

    // この場合、Direct3D 12は、リソースタイプ（テクスチャなど）とレイアウト設定（D3D12_TEXTURE_LAYOUT_UNKNOWNなど）に基づいて、正しいアライメントを自動的に適用します。 

    // wvpMat = world * view * projection
    // ワールド行列（移動行列 * 回転行列 * 拡大行列）
    // 移動行列
    XMMATRIX tmpTransMat = XMMatrixTranslation(objXOffset_, objYOffset_, objZOffset_);
    wvpMatrices_.transMat = tmpTransMat;

    // 回転行列
    XMMATRIX tmpRotMat = XMMatrixRotationY(XMConvertToRadians(15.0f));
    wvpMatrices_.rotMat = tmpRotMat;

    // ビュー行列
    XMMATRIX viewMat = XMMatrixLookAtLH(XMLoadFloat3(&camPos_), XMLoadFloat3(&camTarg_), XMLoadFloat3(&camUp_));
    wvpMatrices_.viewMat = viewMat;

    // プロジェクション行列
    // 視錐台：見える範囲を台形状にかたどったもの(クリッピングボリューム)
    XMMATRIX projMat = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f), // 視野角
        static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT),
        0.1f,
        100.0f
    );
    wvpMatrices_.projMat = projMat;

    // 定数バッファーの生成
    ID3D12Resource* constBuff = nullptr;
    CD3DX12_HEAP_PROPERTIES constUploadHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC constResDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstantBuffer) + 0xff) & ~0xff); // 256バイトアライメント
    result = device_->CreateCommittedResource(
        &constUploadHeapProp,
        D3D12_HEAP_FLAG_NONE,
        &constResDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constBuff)
    );
    if (FAILED(result)) {
        MessageBox(nullptr, L"定数バッファーの生成に失敗",
            L"Constant Buffer", MB_OK);
        bRunning_ = false;
    }

    // 代入演算子が使える! 「コピーなのでmemcpyを使う」と思った人もいるかもしれませんが、このように代入演算子を使うこともできます。
    constBuff->Map(0, nullptr, reinterpret_cast<void**>(&constBuffMapAddr_));
    //constBuffMapAddr_->wvpMat = XMMatrixTranspose(wvpMat); // HLSLは列優先なので転置しておく
    constBuffMapAddr_->wvpMat = wvpMatrices_;
	constBuffMapAddr_->lightDirection = XMLoadFloat3(&lightDirection_); // ライトの方向
    constBuffMapAddr_->lightColor = XMLoadFloat3(&lightColor_); // ライトの色
    constBuffMapAddr_->ambientColor = XMLoadFloat3(&ambientColor_); // アンビエントライトの色
	// わざとUnmapしないでUpdate()で毎フレーム更新する

    // CBVの生成
    // CBV,SRV,UAVの3つはHandleIncrementSizeが同じとなっています。なおレンダーターゲットビューおよび深度ステンシルビューの大きさはそれらとは異なるので、注意しましょう。
    // CPU側でビューを作成しているため、GetCPUDescriptorHandleForHeapStart()を使います。
    D3D12_CPU_DESCRIPTOR_HANDLE basicHeapHandle = basicDescHeap_->GetCPUDescriptorHandleForHeapStart(); // basicDescHeapはルートシグネチャーに登録されているメインのディスクリプターヒープ

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = static_cast<UINT>(constBuff->GetDesc().Width);
	device_->CreateConstantBufferView(&cbvDesc, basicHeapHandle);
    // ** 定数バッファー（座標返還行列）の生成 END

    return true;
}

void Update()
{
    // モデルのワールド行列を更新
    // 移動行列
    const XMMATRIX transMat = XMMatrixTranslation(objXOffset_, objYOffset_, objZOffset_);
    wvpMatrices_.transMat = transMat;

    // 回転行列
    const XMMATRIX rotXMat = XMMatrixRotationX(XMConvertToRadians(objXRot_));
    const XMMATRIX rotYMat = XMMatrixRotationY(XMConvertToRadians(objYRot_));
    const XMMATRIX rotZMat = XMMatrixRotationZ(XMConvertToRadians(objZRot_));
    const XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
    wvpMatrices_.rotMat = rotMat;

    // カメラの位置を更新
    camPos_ = XMFLOAT3{ camX_, camY_, camZ_ }; // 現在はz軸のみ移動可能

    // ビュー行列を更新
    const XMMATRIX viewMat = XMMatrixLookAtLH(XMLoadFloat3(&camPos_), XMLoadFloat3(&camTarg_), XMLoadFloat3(&camUp_));
    wvpMatrices_.viewMat = viewMat;

    // 定数バッファーの更新 （行列の部分のみ）
    memcpy(constBuffMapAddr_, &wvpMatrices_, sizeof(WVPMatrices));
}

void UpdatePipeline()
{
    HRESULT result;

    WaitForPreviousFrame();

    // コマンドアロケータをリセット
    result = cmdAllocator_[frameIndex_]->Reset();
    if (FAILED(result))
    {
        MessageBox(nullptr, L"コマンドアロケータのリセットに失敗",
            L"Command Allocator Reset", MB_OK);
        bRunning_ = false;
    }

    // コマンドリストをリセット
    result = cmdList_->Reset(cmdAllocator_[frameIndex_].Get(), nullptr);
    if (FAILED(result))
    {
        MessageBox(nullptr, L"コマンドリストのリセットに失敗",
            L"Command list Reset", MB_OK);
        bRunning_ = false;
    }

    // バックバッファに描く前にリソースの状態を遷移
    D3D12_RESOURCE_BARRIER _barrierDesc = {};
    _barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    _barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    _barrierDesc.Transition.pResource = backBuffers_[frameIndex_].Get();
    _barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    _barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    _barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    cmdList_->ResourceBarrier(1, &_barrierDesc);

    // レンダーターゲットをセット
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeaps_->GetCPUDescriptorHandleForHeapStart(), frameIndex_, rtvDescriptorSize_);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(dsvHeap_->GetCPUDescriptorHandleForHeapStart());
    cmdList_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // レンダーターゲットのクリア
    float clearColor[] = { 0.8f, 0.8f, 1.0f, 1.0f };
    cmdList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ルートシグネチャをセット
    // GPU側にディスクリプターテーブルを設定しているので、GetGPUDescriptorHandleForHeapStart()を使います
    cmdList_->SetGraphicsRootSignature(rootSignature_.Get());
    cmdList_->SetDescriptorHeaps(1, basicDescHeap_.GetAddressOf());
    D3D12_GPU_DESCRIPTOR_HANDLE basicDescHeapHandle = basicDescHeap_->GetGPUDescriptorHandleForHeapStart();
    cmdList_->SetGraphicsRootDescriptorTable(0, basicDescHeapHandle); // 定数バッファービュー

    // ビューポートとシザーをセット
    cmdList_->RSSetViewports(1, &viewport_);
    cmdList_->RSSetScissorRects(1, &sciRect_);

    // 3Dモデル描画
    // グラフィックパイプラインステートをセット
	// Input Assemblerの設定
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
        // 法線ベクトル描画
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

    // バックバッファを切り替える前にリソースの状態を遷移
    _barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    _barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList_->ResourceBarrier(1, &_barrierDesc);

    // コマンドリストを閉じる
    result = cmdList_->Close();
    if (FAILED(result))
    {
        MessageBox(nullptr, L"コマンドリストのクローズに失敗",
            L"Command list Close", MB_OK);
        bRunning_ = false;
    }
}

void Render()
{
    HRESULT hr;

    UpdatePipeline();

    // コマンドを実行
    ID3D12CommandList* ppCommandLists[] = { cmdList_.Get() };
    cmdQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    hr = cmdQueue_->Signal(fence_.Get(), fenceValue_);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"フェンスのシグナルに失敗",
            L"Fence Signal", MB_OK);
        bRunning_ = false;
    }

    // バックバッファをフリップ
    hr = swapchain_->Present(1, 0);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"バックバッファのフリップに失敗",
            L"Swap chain present", MB_OK);
        bRunning_ = false;
    }
}

void WaitForPreviousFrame()
{
    HRESULT hr;

    // このフレームで使うバックバッファのインデックスを取得
    frameIndex_ = swapchain_->GetCurrentBackBufferIndex();

    // fenceの値がfenceValueより小さい場合は、GPUがまだコマンドを処理していると意味する
    if (fence_->GetCompletedValue() < fenceValue_)
    {
        // fenceの値がfenceValueになる時のイベントコールバック
        hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        if (FAILED(hr))
        {
            MessageBox(nullptr, L"fenceイベントの生成に失敗",
                L"Fence Event Completion", MB_OK);
            bRunning_ = false;
        }

        // fenceの値がfenceValueになるまで待つ
        WaitForSingleObject(fenceEvent_, INFINITE);
    }

    // 次のフレームのためにフェンスの値を増やす
    fenceValue_++;
}