#include "ImGuiManager.h"

#ifdef _DEBUG
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#endif // _DEBUG


ImGuiManager* ImGuiManager::GetInstance()
{
    static ImGuiManager instance;
    return&instance;
}

void ImGuiManager::Initialize(WinApp* win, DirectXCommon* dxCommon) {

#ifdef _DEBUG


    HRESULT result;

    dxCommon_ = dxCommon;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;
    desc.NumDescriptors = 1;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    result = dxCommon_->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ret_));
    assert(SUCCEEDED(result));

    ImGui::CreateContext();

    ImGui_ImplWin32_Init(win->GetHWND());
    ImGui_ImplDX12_Init(dxCommon_->GetDevice(), 3,
        DXGI_FORMAT_R8G8B8A8_UNORM, ret_.Get(), ret_->GetCPUDescriptorHandleForHeapStart(),
        ret_->GetGPUDescriptorHandleForHeapStart());

#endif // _DEBUG

}

void ImGuiManager::Begin() {
#ifdef _DEBUG
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
#endif // _DEBUG
}

void ImGuiManager::End() {
#ifdef _DEBUG
    ImGui::Render();
#endif // _DEBUG
}

void ImGuiManager::Draw() {
#ifdef _DEBUG
    ID3D12GraphicsCommandList* cmdList = dxCommon_->GetCmdList();
    ID3D12DescriptorHeap* ppret[] = { ret_.Get() };
    cmdList->SetDescriptorHeaps(_countof(ppret), ppret);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCmdList());
#endif // _DEBUG

}

void ImGuiManager::Finalize() {
#ifdef _DEBUG
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    ret_.Reset();
#endif // _DEBUG

}
