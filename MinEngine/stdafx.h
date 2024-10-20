#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // あまり使われない Win32 ヘッダーを除外します。
#endif

#ifdef _DEBUG
#include <iostream>
#endif

#include <vector>
#include <fstream>
#include <stdexcept>

#include <Windows.h>
#include <d3dx12.h> // d3d12.hをインクルードする前にインクルードする必要がある
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

// PIX
#include <shlobj.h>
#include <strsafe.h>

#include <wincodec.h>

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

using namespace DirectX;

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
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
};

Header headerBTM_; // Headerデータを格納する変数
std::vector<Triangle> trianglesBTM_; // トライアングルデータを格納する変数
std::vector<Task2Vertex> verticesBTM_; // 頂点データを格納する変数
std::vector<XMFLOAT3> normalsBTM_; // XMFLOAT3型で法線ベクトルデータを格納する変数

struct WVPMatrices
{
	// ワールド行列
	XMMATRIX transMat = XMMatrixIdentity();
	XMMATRIX rotMat = XMMatrixIdentity();
	XMMATRIX scaleMat = XMMatrixIdentity();

	// ビュー・プロジェクション行列
	XMMATRIX viewMat = XMMatrixIdentity();
	XMMATRIX projMat = XMMatrixIdentity();

};

struct ConstantBuffer
{
	// 64 bytes
	//XMMATRIX wvpMat = XMMatrixIdentity(); // ワールド行列 * ビュー行列 * プロジェクション行列
	WVPMatrices wvpMat = {}; // ワールド行列 * ビュー行列 * プロジェクション行列
	// 16 bytes
	XMVECTOR lightDirection = XMVectorZero(); // ライトの方向
	XMVECTOR lightColor = XMVectorZero(); // ライトの色
	XMVECTOR ambientColor = XMVectorZero(); // アンビエントライトの色
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

void Cleanup(); // COMオブジェクトの解放

void WaitForPreviousFrame(); // フェンスでCPUとGPUを同期させる

UINT frameBufferCount_ = 2; // バックバッファの数
UINT frameIndex_ = 0; // 現在のバックバッファのインデックス

UINT rtvDescriptorSize_ = 0; // レンダーターゲットビューのディスクリプタサイズ

IDXGIFactory6* dxgiFactory_ = nullptr;
ID3D12Device* device_ = nullptr;
std::vector<ID3D12CommandAllocator*> cmdAllocator_(frameBufferCount_);
ID3D12GraphicsCommandList* cmdList_ = nullptr;
ID3D12CommandQueue* cmdQueue_ = nullptr;
IDXGISwapChain4* swapchain_ = nullptr;
ID3D12DescriptorHeap* rtvHeaps_ = nullptr;
ID3D12Resource* depthBuffer_ = nullptr;
ID3D12DescriptorHeap* dsvHeap_;
std::vector<ID3D12Resource*> backBuffers_(frameBufferCount_);
ID3D12Fence* fence_;
UINT64 fenceValue_; // フェンスがシグナルされるまでの値
HANDLE fenceEvent_;

// これらの情報をグローバルにする理由として
// これらのデータはフレーム間で共有するため、InitD3Dの中でローカル関数にすると、次のフレームにデータが引き継がれないため
ID3D12Resource* vertUploadHeap_ = nullptr;
VertexData* vertMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW vertBufferView_ = {};

ID3D12Resource* flatVertUploadHeap_ = nullptr;
VertexData* flatVertMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW flatVertBufferView_;

ID3D12Resource* indexUploadHeap_ = nullptr;
uint16_t* indexMapAddress_ = nullptr;
D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};

ID3D12Resource* normalUploadHeap_ = nullptr;
XMFLOAT3* normalMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW normalBufferView_ = {};

ID3D12Resource* flatNormalUploadHeap_ = nullptr;
XMFLOAT3* flatNormalMapAddress_ = nullptr;
D3D12_VERTEX_BUFFER_VIEW flatNormalBufferView_ = {};

ID3D12RootSignature* rootSignature_ = nullptr;
ID3D12PipelineState* psoSolid_ = nullptr;
ID3D12PipelineState* psoWireframe_ = nullptr;
ID3D12PipelineState* psoNormals_ = nullptr;
D3D12_VIEWPORT viewport_ = {};
D3D12_RECT sciRect_ = {};

ID3D12DescriptorHeap* basicDescHeap_ = nullptr;
ConstantBuffer* constBuffMapAddr_ = nullptr;

// 課題2ファイルパース
const std::string modelFilePath_ = "sample_min2.btm";

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
XMFLOAT3 camPos_ = { camX_, camY_, camZ_ }; // 視点
XMFLOAT3 camTarg_ = { 0.0f, 0.0f, 0.0f }; // 注視点
XMFLOAT3 camUp_ = { 0.0f, 1.0f, 0.0f }; // 上方向 (world up)
bool bCameraMove_ = false;
bool bCameraRotate_ = false;

// ライトの情報
XMFLOAT3 lightDirection_ = XMFLOAT3(0.0f, -1.0f, 1.0f);
XMFLOAT3 lightColor_ = XMFLOAT3(1.0f, 1.0f, 1.0f);
XMFLOAT3 ambientColor_ = XMFLOAT3(0.4f, 0.4f, 0.8f);

// インプット
POINT prevMousePos_ = { 0, 0 };
POINT currMousePos_ = { 0, 0 };

bool bSolidMode_ = true;
bool bWireframeMode_ = false;
bool bDrawNormals_ = false;
bool bShadeSmooth_ = true;