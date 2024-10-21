#include "Renderer.h"

#include "Window.h"

Renderer::Renderer()
    : cmdAllocator_(frameBufferCount_), backBuffers_(frameBufferCount_) {
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(HWND hwnd)
{
    if (!InitializeDirect3D12(hwnd)) {
        return false;
    }
    //LoadShaders();
    //CreatePipelineState();
    //model_.Load("sample_min2.btm");
    return true;
}

void Renderer::Update()
{
    // モデルのワールド行列を更新
	// 移動行列
    const DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation(objXOffset_, objYOffset_, objZOffset_);
    wvpMatrices_.transMat = transMat;

    // 回転行列
    const DirectX::XMMATRIX rotXMat = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(objXRot_));
    const DirectX::XMMATRIX rotYMat = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(objYRot_));
    const DirectX::XMMATRIX rotZMat = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(objZRot_));
    const DirectX::XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
    wvpMatrices_.rotMat = rotMat;

    // カメラの位置を更新
    camPos_ = DirectX::XMFLOAT3{ camX_, camY_, camZ_ }; // 現在はz軸のみ移動可能

    // ビュー行列を更新
    const DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&camPos_), DirectX::XMLoadFloat3(&camTarg_), DirectX::XMLoadFloat3(&camUp_));
    wvpMatrices_.viewMat = viewMat;

    // 定数バッファーの更新 （行列の部分のみ）
    memcpy(constBuffMapAddr_, &wvpMatrices_, sizeof(WVPMatrices));
}

void Renderer::UpdatePipeline()
{
    /*HRESULT result;

    WaitForPreviousFrame();

    // コマンドアロケータをリセット
    result = cmdAllocator_[frameIndex_]->Reset();
    if (FAILED(result))
    {
        MessageBox(nullptr, L"コマンドアロケータのリセットに失敗",
            L"Command Allocator Reset", MB_OK);
        Window::GetInstance()->bRunning_ = false;
    }

    // コマンドリストをリセット
    result = cmdList_->Reset(cmdAllocator_[frameIndex_].Get(), nullptr);
    if (FAILED(result))
    {
        MessageBox(nullptr, L"コマンドリストのリセットに失敗",
            L"Command list Reset", MB_OK);
        Window::GetInstance()->bRunning_ = false;
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
    if (Window::GetInstance()->bSolidMode_)
    {
        cmdList_->SetPipelineState(psoSolid_.Get());
        if (Window::GetInstance()->bShadeSmooth_)
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
    if (Window::GetInstance()->bWireframeMode_)
    {
        cmdList_->SetPipelineState(psoWireframe_.Get());
        cmdList_->IASetVertexBuffers(0, 1, &vertBufferView_);
        cmdList_->IASetIndexBuffer(&indexBufferView_);
        cmdList_->DrawIndexedInstanced(static_cast<UINT>(trianglesBTM_.size() * 3), 1, 0, 0, 0);
    }
    if (Window::GetInstance()->bDrawNormals_)
    {
        // 法線ベクトル描画
        cmdList_->SetPipelineState(psoNormals_.Get());
        cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        if (Window::GetInstance()->bShadeSmooth_)
        {
            cmdList_->IASetVertexBuffers(0, 1, &normalBufferView_);
            cmdList_->DrawInstanced(normalBufferView_.SizeInBytes / sizeof(DirectX::XMFLOAT3), 1, 0, 0);
        }
        else
        {
            cmdList_->IASetVertexBuffers(0, 1, &flatNormalBufferView_);
            cmdList_->DrawInstanced(flatNormalBufferView_.SizeInBytes / sizeof(DirectX::XMFLOAT3), 1, 0, 0);
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
        Window::GetInstance()->bRunning_ = false;
    }*/
}

void Renderer::Render()
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
        Window::GetInstance()->SetRunning(false);
    }

    // バックバッファをフリップ
    hr = swapchain_->Present(1, 0);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"バックバッファのフリップに失敗",
            L"Swap chain present", MB_OK);
        Window::GetInstance()->SetRunning(false);
    }
}

void Renderer::Cleanup()
{
}

bool Renderer::InitializeDirect3D12(HWND hwnd)
{
    return true;
}

void Renderer::LoadShaders()
{
}

void Renderer::CreatePipelineState()
{
}

void Renderer::WaitForPreviousFrame()
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
            Window::GetInstance()->SetRunning(false);
        }

        // fenceの値がfenceValueになるまで待つ
        WaitForSingleObject(fenceEvent_, INFINITE);
    }

    // 次のフレームのためにフェンスの値を増やす
    fenceValue_++;
}
