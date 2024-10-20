#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // ���܂�g���Ȃ� Win32 �w�b�_�[�����O���܂��B
#endif

#ifdef _DEBUG
#include <iostream>
#endif

#include <vector>
#include <fstream>
#include <stdexcept>

#include <Windows.h>
#include <d3dx12.h> // d3d12.h���C���N���[�h����O�ɃC���N���[�h����K�v������
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

// �I�u�W�F�N�g�����݂���ꍇ�̂݁A�������
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = 0; } }

using namespace DirectX;

// �ۑ�2�̍\����
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

Header headerBTM_; // Header�f�[�^���i�[����ϐ�
std::vector<Triangle> trianglesBTM_; // �g���C�A���O���f�[�^���i�[����ϐ�
std::vector<Task2Vertex> verticesBTM_; // ���_�f�[�^���i�[����ϐ�
std::vector<XMFLOAT3> normalsBTM_; // XMFLOAT3�^�Ŗ@���x�N�g���f�[�^���i�[����ϐ�

struct WVPMatrices
{
	// ���[���h�s��
	XMMATRIX transMat = XMMatrixIdentity();
	XMMATRIX rotMat = XMMatrixIdentity();
	XMMATRIX scaleMat = XMMatrixIdentity();

	// �r���[�E�v���W�F�N�V�����s��
	XMMATRIX viewMat = XMMatrixIdentity();
	XMMATRIX projMat = XMMatrixIdentity();

};

struct ConstantBuffer
{
	// 64 bytes
	//XMMATRIX wvpMat = XMMatrixIdentity(); // ���[���h�s�� * �r���[�s�� * �v���W�F�N�V�����s��
	WVPMatrices wvpMat = {}; // ���[���h�s�� * �r���[�s�� * �v���W�F�N�V�����s��
	// 16 bytes
	XMVECTOR lightDirection = XMVectorZero(); // ���C�g�̕���
	XMVECTOR lightColor = XMVectorZero(); // ���C�g�̐F
	XMVECTOR ambientColor = XMVectorZero(); // �A���r�G���g���C�g�̐F
};

// �A���C�����g�ɂ��낦���T�C�Y��Ԃ�
// @param size ���̃T�C�Y
// @param alignment �A���C�����g�T�C�Y
// @return �A���C�����g�����낦���T�C�Y
size_t AlignmentedSize(size_t size, size_t alignment)
{
	return size + alignment - size % alignment;
}

// �E�C���h�EHANDLE
HWND hwnd = NULL;

// �E�C���h�E�̕��ƍ���
unsigned int WINDOW_WIDTH = 1280;
unsigned int WINDOW_HEIGHT = 720;

// �t���X�N���[�����ǂ���
bool FullScreen = false;

// false�ɂȂ�ƃv���O�������I������
bool bRunning_ = true;

// �E�C���h�E�̏�����
bool InitWindow(HINSTANCE hInstance,
    int ShowWnd,
    bool fullscreen);

// �v���O�����̃��C�����[�v
void Mainloop();

// �E�C���h�E���b�Z�[�W�̏���
LRESULT CALLBACK WndProc(HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

bool InitD3D(); // Direct3D12�̏�����

void Update(); // �Q�[�����W�b�N�̍X�V

void UpdatePipeline(); // D3D12�p�C�v���C���̍X�V

void Render(); // Command list�ɕ`��R�}���h��ς�

void Cleanup(); // COM�I�u�W�F�N�g�̉��

void WaitForPreviousFrame(); // �t�F���X��CPU��GPU�𓯊�������

UINT frameBufferCount_ = 2; // �o�b�N�o�b�t�@�̐�
UINT frameIndex_ = 0; // ���݂̃o�b�N�o�b�t�@�̃C���f�b�N�X

UINT rtvDescriptorSize_ = 0; // �����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�T�C�Y

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
UINT64 fenceValue_; // �t�F���X���V�O�i�������܂ł̒l
HANDLE fenceEvent_;

// �����̏����O���[�o���ɂ��闝�R�Ƃ���
// �����̃f�[�^�̓t���[���Ԃŋ��L���邽�߁AInitD3D�̒��Ń��[�J���֐��ɂ���ƁA���̃t���[���Ƀf�[�^�������p����Ȃ�����
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

// �ۑ�2�t�@�C���p�[�X
const std::string modelFilePath_ = "sample_min2.btm";

// ���f���̃��[���h�s��̏��
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

// �J�����̏��
float camX_ = 0.0f;
float camY_ = 0.0f;
float camZ_ = -10.0f;
XMFLOAT3 camPos_ = { camX_, camY_, camZ_ }; // ���_
XMFLOAT3 camTarg_ = { 0.0f, 0.0f, 0.0f }; // �����_
XMFLOAT3 camUp_ = { 0.0f, 1.0f, 0.0f }; // ����� (world up)
bool bCameraMove_ = false;
bool bCameraRotate_ = false;

// ���C�g�̏��
XMFLOAT3 lightDirection_ = XMFLOAT3(0.0f, -1.0f, 1.0f);
XMFLOAT3 lightColor_ = XMFLOAT3(1.0f, 1.0f, 1.0f);
XMFLOAT3 ambientColor_ = XMFLOAT3(0.4f, 0.4f, 0.8f);

// �C���v�b�g
POINT prevMousePos_ = { 0, 0 };
POINT currMousePos_ = { 0, 0 };

bool bSolidMode_ = true;
bool bWireframeMode_ = false;
bool bDrawNormals_ = false;
bool bShadeSmooth_ = true;