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
    // ���f���̃��[���h�s����X�V
	// �ړ��s��
    const DirectX::XMMATRIX transMat = DirectX::XMMatrixTranslation(objXOffset_, objYOffset_, objZOffset_);
    wvpMatrices_.transMat = transMat;

    // ��]�s��
    const DirectX::XMMATRIX rotXMat = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(objXRot_));
    const DirectX::XMMATRIX rotYMat = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(objYRot_));
    const DirectX::XMMATRIX rotZMat = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(objZRot_));
    const DirectX::XMMATRIX rotMat = rotXMat * rotYMat * rotZMat;
    wvpMatrices_.rotMat = rotMat;

    // �J�����̈ʒu���X�V
    camPos_ = DirectX::XMFLOAT3{ camX_, camY_, camZ_ }; // ���݂�z���݈̂ړ��\

    // �r���[�s����X�V
    const DirectX::XMMATRIX viewMat = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&camPos_), DirectX::XMLoadFloat3(&camTarg_), DirectX::XMLoadFloat3(&camUp_));
    wvpMatrices_.viewMat = viewMat;

    // �萔�o�b�t�@�[�̍X�V �i�s��̕����̂݁j
    memcpy(constBuffMapAddr_, &wvpMatrices_, sizeof(WVPMatrices));
}

void Renderer::UpdatePipeline()
{
    /*HRESULT result;

    WaitForPreviousFrame();

    // �R�}���h�A���P�[�^�����Z�b�g
    result = cmdAllocator_[frameIndex_]->Reset();
    if (FAILED(result))
    {
        MessageBox(nullptr, L"�R�}���h�A���P�[�^�̃��Z�b�g�Ɏ��s",
            L"Command Allocator Reset", MB_OK);
        Window::GetInstance()->bRunning_ = false;
    }

    // �R�}���h���X�g�����Z�b�g
    result = cmdList_->Reset(cmdAllocator_[frameIndex_].Get(), nullptr);
    if (FAILED(result))
    {
        MessageBox(nullptr, L"�R�}���h���X�g�̃��Z�b�g�Ɏ��s",
            L"Command list Reset", MB_OK);
        Window::GetInstance()->bRunning_ = false;
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
        // �@���x�N�g���`��
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
        Window::GetInstance()->bRunning_ = false;
    }*/
}

void Renderer::Render()
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
        Window::GetInstance()->SetRunning(false);
    }

    // �o�b�N�o�b�t�@���t���b�v
    hr = swapchain_->Present(1, 0);
    if (FAILED(hr))
    {
        MessageBox(nullptr, L"�o�b�N�o�b�t�@�̃t���b�v�Ɏ��s",
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
            Window::GetInstance()->SetRunning(false);
        }

        // fence�̒l��fenceValue�ɂȂ�܂ő҂�
        WaitForSingleObject(fenceEvent_, INFINITE);
    }

    // ���̃t���[���̂��߂Ƀt�F���X�̒l�𑝂₷
    fenceValue_++;
}
