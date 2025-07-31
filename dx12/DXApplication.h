#pragma once

#include <windows.h>
#include <tchar.h>
#include <string>

#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXTex.h>

#include <wrl.h>
#include "d3dx12.h"
#include <stdexcept>
#include <vector>
#include "AudioEngine.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib") 
using Microsoft::WRL::ComPtr;

class DXApplication
{
public:
	DXApplication(unsigned int width, unsigned int height, std::wstring title);
	void OnInit(HWND hwnd);
	void OnUpdate();
	void OnRender();
	void OnDestroy();

	void SetTexturePosition(size_t index, float x, float y, float width, float height);
	void InitializeTexture(const std::wstring& filepath, float x, float y, float width, float height);
	void SetTextureRotation(size_t index, float radians);
	
	const WCHAR* GetTitle() const { return title_.c_str(); }
	unsigned int GetWindowWidth() const { return windowWidth_; }
	unsigned int GetWindowHeight() const { return windowHeight_; }

	AudioEngine engine_;

private:
	static const unsigned int kFrameCount = 2;

	std::wstring title_;
	unsigned int windowWidth_;
	unsigned int windowHeight_;

	CD3DX12_VIEWPORT viewport_; // ビューポート
	CD3DX12_RECT scissorrect_;  // シザー短形

	// パイプラインオブジェクト
	ComPtr<ID3D12Device> device_;
	ComPtr<ID3D12CommandAllocator> commandAllocator_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;
	ComPtr<ID3D12CommandQueue> commandQueue_;
	ComPtr<IDXGISwapChain4> swapchain_;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps_;             // レンダーターゲットヒープ
	ComPtr<ID3D12DescriptorHeap> basicHeap_; // ★追加
	ComPtr<ID3D12Resource> renderTargets_[kFrameCount]; // バックバッファー

	ComPtr<ID3D12PipelineState> pipelinestate_;         // パイプラインステート
	ComPtr<ID3D12RootSignature> rootsignature_;         // ルートシグネチャ

	// ★ポリゴン描画のため追加
	// リソース
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	ComPtr<ID3D12Resource> textureBuffer_; // ★追加
	ComPtr<ID3D12Resource> constBuffer_;

	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	// フェンス
	ComPtr<ID3D12Fence> fence_;
	UINT64 fenceValue_;
	HANDLE fenceEvent_;
	void LoadPipeline(HWND hwnd);
	void LoadAssets(); // ★ポリゴン描画のため追加

	void CreateD3D12Device(IDXGIFactory6* dxgiFactory, ID3D12Device** d3d12device);
	void ThrowIfFailed(HRESULT hr);

	void WaitForGpu();
	
	struct TextureResource {
		ComPtr<ID3D12Resource> texture;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
		D3D12_VERTEX_BUFFER_VIEW vertexView;
		D3D12_INDEX_BUFFER_VIEW indexView;
		ComPtr<ID3D12Resource> constantBuffer;
		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		DirectX::XMMATRIX transformMatrix;

		float x, y, width, height;

		// 変換行列
		DirectX::XMMATRIX scaleMatrix;
		DirectX::XMMATRIX rotationMatrix;
		DirectX::XMMATRIX translationMatrix;

	};

	void InitializeTextureTransform(size_t index);

	std::vector<TextureResource> textures_;
};