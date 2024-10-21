#pragma once

#include <d3dx12.h> // ヘルパー関数・構造体（d3d12.hをインクルードする前にインクルードする必要がある）
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <vector>
#include <wrl/client.h>
#include "Model.h"

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

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(HWND hwnd);
    void Update();
    void UpdatePipeline();
    void Render();
    void Cleanup();

    const UINT frameBufferCount_ = 2;
    UINT frameIndex_ = 0;


    void SetCameraMove(bool bCameraMove) { bCameraMove_ = bCameraMove; }
    void SetCameraRotate(bool bCameraRotate) { bCameraRotate_ = bCameraRotate; }
    void SetSolidMode(bool bSolidMode) { bSolidMode_ = bSolidMode; }
    void SetWireframeMode(bool bWireframeMode) { bWireframeMode_ = bWireframeMode; }
    void SetDrawNormals(bool bDrawNormals) { bDrawNormals_ = bDrawNormals; }
    void SetShadeSmooth(bool bShadeSmooth) { bShadeSmooth_ = bShadeSmooth; }

    bool GetCameraMove() const { return bCameraMove_; }
    bool GetCameraRotate() const { return bCameraRotate_; }
    bool GetSolidMode() const { return bSolidMode_; }
    bool GetWireframeMode() const { return bWireframeMode_; }
    bool GetDrawNormals() const { return bDrawNormals_; }
    bool GetShadeSmooth() const { return bShadeSmooth_; }

private:
    bool InitializeDirect3D12(HWND hwnd);
    void LoadShaders();
    void CreatePipelineState();
    void WaitForPreviousFrame();

    IDXGIFactory6* dxgiFactory_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> cmdAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> cmdQueue_ = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapchain_ = nullptr;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeaps_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
    UINT64 fenceValue_; // フェンスがシグナルされるまでの値
    HANDLE fenceEvent_;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> psoSolid_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> psoWireframe_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> psoNormals_ = nullptr;

    D3D12_VIEWPORT viewport_ = {};
    D3D12_RECT sciRect_ = {};

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> basicDescHeap_ = nullptr;
    ConstantBuffer* constBuffMapAddr_ = nullptr;

    WVPMatrices wvpMatrices_ = {};

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

    // フラグ
    bool bSolidMode_ = true;
    bool bWireframeMode_ = false;
    bool bDrawNormals_ = false;
    bool bShadeSmooth_ = true;
};
