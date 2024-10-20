#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // あまり使われない Win32 ヘッダーを除外します。
#endif

#ifdef _DEBUG
#include <iostream>
#endif

#include <vector>
#include <fstream>

#include <Windows.h>
#include <d3dx12.h> // ヘルパー関数・構造体（d3d12.hをインクルードする前にインクルードする必要がある）
#include <d3d12.h> // DirectX12のコアAPI
#include <d3dcompiler.h> // シェーダーコンパイラ（ランタイムでシェーダをコンパイルすれば必須）
#include <dxgi1_6.h> // DXGI（DirectX Graphics Infrastructure）のインターフェース（アダプターとSwap chain用）
#include <DirectXMath.h>
#include <wrl.h> // ComPtr

// PIX
#include <shlobj.h>
#include <strsafe.h>

// PIX support
static std::wstring GetLatestWinPixGpuCapturerPath()
{
	LPWSTR programFilesPath = nullptr;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

	std::wstring pixSearchPath = programFilesPath + std::wstring(L"\\Microsoft PIX\\*");

	WIN32_FIND_DATA findData;
	bool foundPixInstallation = false;
	wchar_t newestVersionFound[MAX_PATH];

	HANDLE hFind = FindFirstFile(pixSearchPath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
				(findData.cFileName[0] != '.'))
			{
				if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
				{
					foundPixInstallation = true;
					StringCchCopy(newestVersionFound, _countof(newestVersionFound), findData.cFileName);
				}
			}
		} while (FindNextFile(hFind, &findData) != 0);
	}

	FindClose(hFind);

	if (!foundPixInstallation)
	{
		// TODO: Error, no PIX installation found
	}

	wchar_t output[MAX_PATH];
	StringCchCopy(output, pixSearchPath.length(), pixSearchPath.data());
	StringCchCat(output, MAX_PATH, &newestVersionFound[0]);
	StringCchCat(output, MAX_PATH, L"\\WinPixGpuCapturer.dll");

	return &output[0];
}

// オブジェクトが存在する場合のみ、解放する
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

// 課題2の構造体
#pragma pack(push, 1)
struct Header
{
	char fileName[4] = "";           // 4 bytes
	uint8_t version = 0;            // 1 byte
	uint16_t triangleCount = 0;     // 2 bytes
	uint16_t vertexCount = 0;       // 2 bytes
	uint16_t normalCount = 0;       // 2 bytes
	uint32_t triangleOffset = 0;    // 4 bytes
	uint32_t vertexOffset = 0;      // 4 bytes
	uint32_t normalOffset = 0;      // 4 bytes
};
#pragma pack(pop)

struct Task2Vertex
{
	float x = 0, y = 0, z = 0;
};

struct Triangle
{
	uint16_t v1 = 0, v2 = 0, v3 = 0, normalIndex = 0;
};

struct VertexData
{
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
};

Header headerBTM_; // Headerデータを格納する変数
std::vector<Triangle> trianglesBTM_; // トライアングルデータを格納する変数
std::vector<Task2Vertex> verticesBTM_; // 頂点データを格納する変数
std::vector<DirectX::XMFLOAT3> normalsBTM_; // XMFLOAT3型で法線ベクトルデータを格納する変数

struct WVPMatrices
{
	// ワールド行列
	DirectX::XMMATRIX transMat = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX rotMat = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX scaleMat = DirectX::XMMatrixIdentity();

	// ビュー・プロジェクション行列
	DirectX::XMMATRIX viewMat = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX projMat = DirectX::XMMatrixIdentity();

};

struct ConstantBuffer
{
	// 64 bytes
	//XMMATRIX wvpMat = XMMatrixIdentity(); // ワールド行列 * ビュー行列 * プロジェクション行列
	WVPMatrices wvpMat = {}; // ワールド行列 * ビュー行列 * プロジェクション行列
	// 16 bytes
	DirectX::XMVECTOR lightDirection = DirectX::XMVectorZero(); // ライトの方向
	DirectX::XMVECTOR lightColor = DirectX::XMVectorZero(); // ライトの色
	DirectX::XMVECTOR ambientColor = DirectX::XMVectorZero(); // アンビエントライトの色
};

// アライメントにそろえたサイズを返す
// @param size 元のサイズ
// @param alignment アライメントサイズ
// @return アライメントをそろえたサイズ
size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
}

// ウインドウHANDLE
HWND hwnd = NULL;

// ウインドウの幅と高さ
unsigned int WINDOW_WIDTH = 1280;
unsigned int WINDOW_HEIGHT = 720;

// フルスクリーンかどうか
bool FullScreen = false;

// falseになるとプログラムが終了する
bool bRunning_ = true;

// ウインドウの初期化
bool InitWindow(HINSTANCE hInstance,
    int ShowWnd,
    bool fullscreen);

// プログラムのメインループ
void Mainloop();

// ウインドウメッセージの処理
LRESULT CALLBACK WndProc(HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

bool InitD3D(); // Direct3D12の初期化

void Update(); // ゲームロジックの更新

void UpdatePipeline(); // D3D12パイプラインの更新

void Render(); // Command listに描画コマンドを積む

void WaitForPreviousFrame(); // フェンスでCPUとGPUを同期させる

UINT frameBufferCount_ = 2; // バックバッファの数
UINT frameIndex_ = 0; // 現在のバックバッファのインデックス

UINT rtvDescriptorSize_ = 0; // レンダーターゲットビューのディスクリプタサイズ

IDXGIFactory6* dxgiFactory_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12Device> device_;
std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> cmdAllocator_(frameBufferCount_);
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue_ = nullptr;
Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain_ = nullptr;
std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_(frameBufferCount_);
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeaps_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
UINT64 fenceValue_; // フェンスがシグナルされるまでの値
HANDLE fenceEvent_;

// これらの情報をグローバルにする理由として
// これらのデータはフレーム間で共有するため、InitD3Dの中でローカル関数にすると、次のフレームにデータが引き継がれないため
Microsoft::WRL::ComPtr<ID3D12Resource> vertUploadHeap_ = nullptr;
VertexData* vertMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW vertBufferView_ = {};

Microsoft::WRL::ComPtr<ID3D12Resource> flatVertUploadHeap_ = nullptr;
VertexData* flatVertMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW flatVertBufferView_;

Microsoft::WRL::ComPtr<ID3D12Resource> indexUploadHeap_ = nullptr;
uint16_t* indexMapAddress_ = nullptr;
D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};

Microsoft::WRL::ComPtr<ID3D12Resource> normalUploadHeap_ = nullptr;
DirectX::XMFLOAT3* normalMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW normalBufferView_ = {};

Microsoft::WRL::ComPtr<ID3D12Resource> flatNormalUploadHeap_ = nullptr;
DirectX::XMFLOAT3* flatNormalMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW flatNormalBufferView_ = {};

Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12PipelineState> psoSolid_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12PipelineState> psoWireframe_ = nullptr;
Microsoft::WRL::ComPtr<ID3D12PipelineState> psoNormals_ = nullptr;
D3D12_VIEWPORT viewport_ = {};
D3D12_RECT sciRect_ = {};

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> basicDescHeap_ = nullptr;
ConstantBuffer* constBuffMapAddr_ = nullptr;

// 課題2ファイルパース
const std::string modelFilePath_ = "models/sample_min.btm";

// モデルのワールド行列の情報
float objXOffset_ = 0.0f;
float objYOffset_ = 0.0f;
float objZOffset_ = 0.0f;
float objXRot_ = 0.0f;
float objYRot_ = 0.0f;
float objZRot_ = 0.0f;

WVPMatrices wvpMatrices_ = {};
//XMFLOAT4X4 transMat_ = {
//	1.0f, 0.0f, 0.0f, 0.0f,
//	0.0f, 1.0f, 0.0f, 0.0f,
//	0.0f, 0.0f, 1.0f, 0.0f,
//	0.0f, 0.0f, 0.0f, 1.0f
//};
//XMFLOAT4X4 rotMat_ = {
//	1.0f, 0.0f, 0.0f, 0.0f,
//	0.0f, 1.0f, 0.0f, 0.0f,
//	0.0f, 0.0f, 1.0f, 0.0f,
//	0.0f, 0.0f, 0.0f, 1.0f
//};
//XMFLOAT4X4 viewMat_ = {
//	1.0f, 0.0f, 0.0f, 0.0f,
//	0.0f, 1.0f, 0.0f, 0.0f,
//	0.0f, 0.0f, 1.0f, 0.0f,
//	0.0f, 0.0f, 0.0f, 1.0f
//};
//XMFLOAT4X4 projMat_ = {
//	1.0f, 0.0f, 0.0f, 0.0f,
//		0.0f, 1.0f, 0.0f, 0.0f,
//		0.0f, 0.0f, 1.0f, 0.0f,
//		0.0f, 0.0f, 0.0f, 1.0f
//};

// カメラの情報
float camX_ = 0.0f;
float camY_ = 0.0f;
float camZ_ = -10.0f;
DirectX::XMFLOAT3 camPos_ = { camX_, camY_, camZ_ }; // 視点
DirectX::XMFLOAT3 camTarg_ = { 0.0f, 0.0f, 0.0f }; // 注視点
DirectX::XMFLOAT3 camUp_ = { 0.0f, 1.0f, 0.0f }; // 上方向 (world up)
bool bCameraMove_ = false;
bool bCameraRotate_ = false;

// ライトの情報
DirectX::XMFLOAT3 lightDirection_ = DirectX::XMFLOAT3(0.0f, -1.0f, 1.0f);
DirectX::XMFLOAT3 lightColor_ = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
DirectX::XMFLOAT3 ambientColor_ = DirectX::XMFLOAT3(0.6f, 0.6f, 1.0f);

// インプット
POINT prevMousePos_ = { 0, 0 };
POINT currMousePos_ = { 0, 0 };

bool bSolidMode_ = true;
bool bWireframeMode_ = false;
bool bDrawNormals_ = false;
bool bShadeSmooth_ = true;