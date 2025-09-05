#pragma once

#include <windows.h>
#include <tchar.h>
#include <string>
#include <unordered_map>

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
#include <algorithm>
#include <map>
#include "AudioEngine.h"
#include "Button.h"
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

	void SetTexturePosition(const std::wstring& key, float x, float y, float width, float height);
	void InitializeTexture(const std::wstring& key, const std::wstring& filepath, float x, float y, float width, float height, float alpha);
	void SetTextureRotation(const std::wstring& key, float radians);
	
	const WCHAR* GetTitle() const { return title_.c_str(); }
	unsigned int GetWindowWidth() const { return windowWidth_; }
	unsigned int GetWindowHeight() const { return windowHeight_; }

	bool GetTextureVisible(const std::wstring& key);
	void SetTextureVisible(const std::wstring& key, bool visible);
	void ReleaseTexture(const std::wstring& key);

	AudioEngine engine_;
	std::map<std::wstring, Button> buttons;
	void SetTextureBrightnessAndAlpha(const std::wstring& key, float brightness, float alpha);
	void DrawTexture(
		const std::wstring& key,
		float x, float y,
		float width, float height,
		float alpha,
		float brightness,
		float rotation,
		bool visible);
private:
	static const unsigned int kFrameCount = 2;

	std::wstring title_;
	unsigned int windowWidth_;
	unsigned int windowHeight_;

	CD3DX12_VIEWPORT viewport_; // �r���[�|�[�g
	CD3DX12_RECT scissorrect_;  // �V�U�[�Z�`

	// �p�C�v���C���I�u�W�F�N�g
	ComPtr<ID3D12Device> device_;
	ComPtr<ID3D12CommandAllocator> commandAllocator_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;
	ComPtr<ID3D12CommandQueue> commandQueue_;
	ComPtr<IDXGISwapChain4> swapchain_;
	ComPtr<ID3D12DescriptorHeap> rtvHeaps_;             // �����_�[�^�[�Q�b�g�q�[�v
	ComPtr<ID3D12DescriptorHeap> basicHeap_; // ���ǉ�

	UINT descriptorSizeCBVSRV_ = 0; // �ǉ�: �f�B�X�N���v�^�����T�C�Y
	UINT nextSrvIndex_ = 0;         // �ǉ�: ���Ɋ��蓖�Ă� SRV �C���f�b�N�X

	ComPtr<ID3D12Resource> renderTargets_[kFrameCount]; // �o�b�N�o�b�t�@�[

	ComPtr<ID3D12PipelineState> pipelinestate_;         // �p�C�v���C���X�e�[�g
	ComPtr<ID3D12RootSignature> rootsignature_;         // ���[�g�V�O�l�`��

	// ���|���S���`��̂��ߒǉ�
	// ���\�[�X
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	ComPtr<ID3D12Resource> textureBuffer_; // ���ǉ�
	ComPtr<ID3D12Resource> constBuffer_;

	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	// �t�F���X
	ComPtr<ID3D12Fence> fence_;
	UINT64 fenceValue_;
	HANDLE fenceEvent_;
	void LoadPipeline(HWND hwnd);
	void LoadAssets(); // ���|���S���`��̂��ߒǉ�

	void CreateD3D12Device(IDXGIFactory6* dxgiFactory, ID3D12Device** d3d12device);
	void ThrowIfFailed(HRESULT hr);

	void WaitForGpu();
	
	struct TextureResource {
		ComPtr<ID3D12Resource> texture;
		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		ComPtr<ID3D12Resource> constantBuffer;
		D3D12_VERTEX_BUFFER_VIEW vertexView;
		D3D12_INDEX_BUFFER_VIEW indexView;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
		float x, y, width, height;
		DirectX::XMMATRIX transformMatrix;
		DirectX::XMMATRIX scaleMatrix;
		DirectX::XMMATRIX rotationMatrix;
		DirectX::XMMATRIX translationMatrix;
		bool visible = true; // �\���ؑփt���O

		float alpha = 1.0f;
		float brightness = 1.0f; // ���ǉ��F���邳�W���i1.0�Œʏ�j
	};

	struct ConstBufferData
	{
		DirectX::XMFLOAT4X4 gTransform;
		float gAlpha;
		float gBrightness; // ���ǉ�
		float padding[2];  // 16�o�C�g�A���C�������g����
		float pad2[36];
	};

	void InitializeTextureTransform(const std::wstring& key);

	std::unordered_map<std::wstring, TextureResource> textures_;
};