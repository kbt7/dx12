#include "DXApplication.h"

DXApplication::DXApplication(unsigned int width, unsigned int height, std::wstring title)
	: title_(title)
	, windowWidth_(width)
	, windowHeight_(height)

	// ���|���S���`��̂��ߒǉ�
	, viewport_(0.0f, 0.0f, static_cast<float>(windowWidth_), static_cast<float>(windowHeight_))
	, scissorrect_(0, 0, static_cast<LONG>(windowWidth_), static_cast<LONG>(windowHeight_))
	, vertexBufferView_({})
	, indexBufferView_({})

	, fenceValue_(0)
	, fenceEvent_(nullptr)
{
}

// ����������
void DXApplication::OnInit(HWND hwnd)
{
	LoadPipeline(hwnd);
	LoadAssets(); // ���|���S���`��̂��ߒǉ�
}

// ���|���S���`��̂��ߒǉ�
void DXApplication::LoadAssets()
{
	// ���[�g�V�O�l�`���̐���
	{
		// ���[�g�p�����[�^�̐���
		// �f�B�X�N���v�^�e�[�u���̎���
		CD3DX12_DESCRIPTOR_RANGE1 discriptorRanges[2];
		discriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // CBV
		discriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // SRV
		CD3DX12_DESCRIPTOR_RANGE1 descriptorRanges[1];
		descriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		
		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, descriptorRanges, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsConstantBufferView(0); // b0�ɑΉ�
		// �T���v���[�̐���
		// �e�N�X�`���f�[�^����ǂ��F�����o���������߂邽�߂̐ݒ�
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		// ���[�g�p�����[�^�A�T���v���[���烋�[�g�V�O�l�`���𐶐�
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &errorBlob));
		ThrowIfFailed(device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(rootsignature_.ReleaseAndGetAddressOf())));
	}

	// �p�C�v���C���X�e�[�g�̐���
	{
		// �V�F�[�_�[�I�u�W�F�N�g�̐���
#if defined(_DEBUG)
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ComPtr<ID3DBlob> vsBlob;
		ComPtr<ID3DBlob> psBlob;
		D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", compileFlags, 0, &vsBlob, nullptr);
		D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", compileFlags, 0, &psBlob, nullptr);

		// ���_���C�A�E�g�̐���
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// �p�C�v���C���X�e�[�g�I�u�W�F�N�g(PSO)�𐶐�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = rootsignature_.Get();
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) }; // ���̓��C�A�E�g�̐ݒ�
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());                       // ���_�V�F�[�_
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());                       // �s�N�Z���V�F�[�_
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);         // ���X�^���C�U�[�X�e�[�g
		D3D12_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		auto& rtBlend = blendDesc.RenderTarget[0];
		rtBlend.BlendEnable = TRUE;
		rtBlend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rtBlend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rtBlend.BlendOp = D3D12_BLEND_OP_ADD;
		rtBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
		rtBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
		rtBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rtBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		psoDesc.BlendState = blendDesc;
		psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;                           // �T���v���}�X�N�̐ݒ�
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;   // �g�|���W�^�C�v
		psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;    // �X�g���b�v���̃J�b�g�ݒ�
		psoDesc.NumRenderTargets = 1;                                             // �����_�[�^�[�Q�b�g��
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                       // �����_�[�^�[�Q�b�g�t�H�[�}�b�g
		psoDesc.SampleDesc.Count = 1;                                             // �}���`�T���v�����O�̐ݒ�
		ThrowIfFailed(device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelinestate_.ReleaseAndGetAddressOf())));
	}
}

void DXApplication::LoadPipeline(HWND hwnd)
{
	UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
	{
		// �f�o�b�O���C���[��L���ɂ���
		ComPtr<ID3D12Debug> debugLayer;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
			debugLayer->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// DXGIFactory�̏�����
	ComPtr<IDXGIFactory6> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

	// �f�o�C�X�̏�����
	CreateD3D12Device(dxgiFactory.Get(), device_.ReleaseAndGetAddressOf());

	// �R�}���h�֘A�̏�����
	{
		// �R�}���h�L���[
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // �^�C���A�E�g����
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // �R�}���h���X�g�ƍ��킹��
		ThrowIfFailed(device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueue_.ReleaseAndGetAddressOf())));
		// �R�}���h�A���P�[�^
		ThrowIfFailed(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator_.ReleaseAndGetAddressOf())));
		// �R�}���h���X�g
		ThrowIfFailed(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(commandList_.ReleaseAndGetAddressOf())));
		ThrowIfFailed(commandList_->Close());
	}

	// �X���b�v�`�F�[���̏�����
	{
		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
		swapchainDesc.BufferCount = kFrameCount;
		swapchainDesc.Width = windowWidth_;
		swapchainDesc.Height = windowHeight_;
		swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.SampleDesc.Count = 1;
		ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
			commandQueue_.Get(),
			hwnd,
			&swapchainDesc,
			nullptr,
			nullptr,
			(IDXGISwapChain1**)swapchain_.ReleaseAndGetAddressOf()));
	}

	// �f�B�X�N���v�^�q�[�v�̏�����
	{
		// �����_�[�^�[�Q�b�g�r���[ (����)
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = kFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeaps_.ReleaseAndGetAddressOf())));

		// SRV/CBV/UAV �p�q�[�v�i�\���Ȑ����m�ہj
		D3D12_DESCRIPTOR_HEAP_DESC basicHeapDesc = {};
		basicHeapDesc.NumDescriptors = 128; // �� �]�T����������i�K�v�Ȃ瑝�₵�āj
		basicHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		basicHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(device_->CreateDescriptorHeap(&basicHeapDesc, IID_PPV_ARGS(basicHeap_.ReleaseAndGetAddressOf())));

		// �f�B�X�N���v�^�����T�C�Y��ۑ�
		descriptorSizeCBVSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		nextSrvIndex_ = 0; // ������
	}

	// �X���b�v�`�F�[���Ɗ֘A�t���ă����_�[�^�[�Q�b�g�r���[�𐶐�
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeaps_->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < kFrameCount; ++i)
		{
			ThrowIfFailed(swapchain_->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(renderTargets_[i].ReleaseAndGetAddressOf())));
			device_->CreateRenderTargetView(renderTargets_[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		}
	}

	// �t�F���X�̐���
	{
		ThrowIfFailed(device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf())));
		fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}
}

// �X�V����
void DXApplication::OnUpdate()
{
}

void DXApplication::OnRender()
{
	auto frameIndex = swapchain_->GetCurrentBackBufferIndex();

	ThrowIfFailed(commandAllocator_->Reset());
	ThrowIfFailed(commandList_->Reset(commandAllocator_.Get(), pipelinestate_.Get()));

	auto barrierStart = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTargets_[frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList_->ResourceBarrier(1, &barrierStart);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		rtvHeaps_->GetCPUDescriptorHandleForHeapStart(),
		frameIndex,
		device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
	);
	commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	commandList_->SetPipelineState(pipelinestate_.Get());
	commandList_->SetGraphicsRootSignature(rootsignature_.Get());
	commandList_->RSSetViewports(1, &viewport_);
	commandList_->RSSetScissorRects(1, &scissorrect_);

	ID3D12DescriptorHeap* ppHeaps[] = { basicHeap_.Get() };
	commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	for (auto& [key, tex] : textures_) {
		if (!tex.visible) continue;

		commandList_->SetGraphicsRootConstantBufferView(1, tex.constantBuffer->GetGPUVirtualAddress());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList_->IASetVertexBuffers(0, 1, &tex.vertexView);
		commandList_->IASetIndexBuffer(&tex.indexView);
		commandList_->SetGraphicsRootDescriptorTable(0, tex.srvHandle);
		commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

	auto barrierEnd = CD3DX12_RESOURCE_BARRIER::Transition(
		renderTargets_[frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);
	commandList_->ResourceBarrier(1, &barrierEnd);
	ThrowIfFailed(commandList_->Close());

	ID3D12CommandList* cmds[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, cmds);
	swapchain_->Present(1, 0);
	WaitForGpu();
}



// �I������
void DXApplication::OnDestroy()
{
	CloseHandle(fenceEvent_);
}

void DXApplication::CreateD3D12Device(IDXGIFactory6* dxgiFactory, ID3D12Device** d3d12device)
{
	ID3D12Device* tmpDevice = nullptr;

	// �O���t�B�b�N�X�{�[�h�̑I��
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; SUCCEEDED(dxgiFactory->EnumAdapters(i, &tmpAdapter)); ++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adapter : adapters)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		adapter->GetDesc(&adapterDesc);
		// AMD���܂ރA�_�v�^�[�I�u�W�F�N�g��T���Ċi�[�i������Ȃ����nullptr�Ńf�t�H���g�j
		// ���i�ł̏ꍇ�́A�I�v�V������ʂ���I�������Đݒ肷��K�v������
		std::wstring strAdapter = adapterDesc.Description;
		if (strAdapter.find(L"AMD") != std::string::npos)
		{
			tmpAdapter = adapter;
			break;
		}
	}

	// Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	for (auto level : levels) {
		// �����\�ȃo�[�W���������������烋�[�v��ł��؂�
		if (SUCCEEDED(D3D12CreateDevice(tmpAdapter, level, IID_PPV_ARGS(&tmpDevice)))) {
			break;
		}
	}
	*d3d12device = tmpDevice;
}

void DXApplication::ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		// hr�̃G���[���e��throw����
		char s_str[64] = {};
		sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
		std::string errMessage = std::string(s_str);
		throw std::runtime_error(errMessage);
	}
}

void DXApplication::InitializeTexture(
	const std::wstring& key,
	const std::wstring& filepath,
	float x, float y, float width, float height,
	float alpha = 1.0f)
{
	using namespace DirectX;

	if (textures_.find(key) != textures_.end()) {
		return;
	}

	// --- �e�N�X�`���ǂݍ��� ---
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	TextureResource resource = {};
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;

	ThrowIfFailed(LoadFromWICFile(filepath.c_str(), WIC_FLAGS_FORCE_RGB | WIC_FLAGS_IGNORE_SRGB, &metadata, scratchImg));
	ThrowIfFailed(PrepareUpload(device_.Get(), scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, subresources));

	// --- GPU �e�N�X�`���쐬 ---
	ComPtr<ID3D12Resource> texture;
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R8G8B8A8_UNORM,
		static_cast<UINT>(metadata.width),
		static_cast<UINT>(metadata.height));

	ThrowIfFailed(device_->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(texture.ReleaseAndGetAddressOf())
	));

	// --- �A�b�v���[�h�o�b�t�@ ---
	ComPtr<ID3D12Resource> uploadBuffer;
	UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);
	auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	ThrowIfFailed(device_->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&uploadDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
	));

	// --- �萔�o�b�t�@ --- 
	auto cbHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstBufferData)); // ���C��: XMMATRIX �ł͂Ȃ� CBData
	ThrowIfFailed(device_->CreateCommittedResource(
		&cbHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource.constantBuffer.ReleaseAndGetAddressOf())
	));

	// --- �]������ ---
	ThrowIfFailed(commandAllocator_->Reset());
	ThrowIfFailed(commandList_->Reset(commandAllocator_.Get(), pipelinestate_.Get()));
	UpdateSubresources(commandList_.Get(), texture.Get(), uploadBuffer.Get(), 0, 0, subresources.size(), subresources.data());
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList_->ResourceBarrier(1, &barrier);
	ThrowIfFailed(commandList_->Close());
	ID3D12CommandList* lists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(_countof(lists), lists);
	WaitForGpu();

	// --- SRV �쐬 ---
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1u;

	const UINT index = nextSrvIndex_;
	if (index >= basicHeap_->GetDesc().NumDescriptors) ThrowIfFailed(E_FAIL);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(basicHeap_->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(index), descriptorSizeCBVSRV_);
	device_->CreateShaderResourceView(texture.Get(), &srvDesc, cpuHandle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(basicHeap_->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(index), descriptorSizeCBVSRV_);
	resource.srvHandle = gpuHandle;
	nextSrvIndex_++;

	// --- ���_�f�[�^ ---
	float left = x;
	float right = x + width;
	float top = y;
	float bottom = y + height;
	Vertex vertices[] = {
		{{ left,  bottom, 0.0f }, { 0.0f, 1.0f }},
		{{ left,  top,    0.0f }, { 0.0f, 0.0f }},
		{{ right, bottom, 0.0f }, { 1.0f, 1.0f }},
		{{ right, top,    0.0f }, { 1.0f, 0.0f }},
	};

	// --- ���_�o�b�t�@�쐬 ---
	UINT vbSize = sizeof(vertices);
	auto vbHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vbDesc = CD3DX12_RESOURCE_DESC::Buffer(vbSize);
	ThrowIfFailed(device_->CreateCommittedResource(
		&vbHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&vbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource.vertexBuffer.ReleaseAndGetAddressOf())
	));

	// ���C��: ���_�f�[�^�� GPU �ɃR�s�[
	void* mapped = nullptr;
	resource.vertexBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));
	resource.vertexBuffer->Unmap(0, nullptr);

	resource.vertexView.BufferLocation = resource.vertexBuffer->GetGPUVirtualAddress();
	resource.vertexView.StrideInBytes = sizeof(Vertex);
	resource.vertexView.SizeInBytes = vbSize;

	// --- �C���f�b�N�X�o�b�t�@ ---
	uint16_t indices[] = { 0, 1, 2, 2, 1, 3 };
	UINT ibSize = sizeof(indices);
	auto ibHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto ibDesc = CD3DX12_RESOURCE_DESC::Buffer(ibSize);
	ThrowIfFailed(device_->CreateCommittedResource(
		&ibHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&ibDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource.indexBuffer.ReleaseAndGetAddressOf())
	));

	// ���C��: �C���f�b�N�X���R�s�[
	resource.indexBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, indices, ibSize);
	resource.indexBuffer->Unmap(0, nullptr);

	resource.indexView.BufferLocation = resource.indexBuffer->GetGPUVirtualAddress();
	resource.indexView.SizeInBytes = ibSize;
	resource.indexView.Format = DXGI_FORMAT_R16_UINT;

	// --- �g�����X�t�H�[�������� ---
	XMMATRIX matrix = XMMatrixIdentity();
	matrix.r[0].m128_f32[0] = 2.0f / windowWidth_;
	matrix.r[1].m128_f32[1] = -2.0f / windowHeight_;
	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = 1.0f;
	resource.transformMatrix = matrix;

	// ���C��: CBData �� alpha ���܂߂ď�������
	ConstBufferData cbData = {};
	DirectX::XMStoreFloat4x4(&cbData.gTransform, resource.transformMatrix);
	cbData.gAlpha = alpha;
	cbData.gBrightness = resource.brightness;

	void* cbMapped = nullptr;
	resource.constantBuffer->Map(0, nullptr, &cbMapped);
	memcpy(cbMapped, &cbData, sizeof(ConstBufferData));
	resource.constantBuffer->Unmap(0, nullptr);

	// --- ���i�[ ---
	resource.x = x;
	resource.y = y;
	resource.width = width;
	resource.height = height;
	resource.texture = texture;
	resource.visible = true;
	resource.alpha = alpha;

	textures_[key] = std::move(resource);

	// �g�����X�t�H�[���������i�C�Ӂj
	InitializeTextureTransform(key);
}


void DXApplication::WaitForGpu()
{
	// �t�F���X�ɃV�O�i���𑗂�
	ThrowIfFailed(commandQueue_->Signal(fence_.Get(), ++fenceValue_));

	// �������Ă��Ȃ���΃C�x���g�őҋ@
	if (fence_->GetCompletedValue() < fenceValue_)
	{
		ThrowIfFailed(fence_->SetEventOnCompletion(fenceValue_, fenceEvent_));
		WaitForSingleObjectEx(fenceEvent_, INFINITE, FALSE);
	}
}

void DXApplication::SetTexturePosition(const std::wstring& key, float x, float y, float width, float height)
{
	auto it = textures_.find(key);
	if (it != textures_.end()) {

		// ���W�����X�V
		it->second.x = x;
		it->second.y = y;

		// �T�C�Y�����X�V�i�������d�v�j
		it->second.width = width;
		it->second.height = height;

		// ���_�f�[�^�X�V
		float left = x;
		float right = x + width;
		float top = y;
		float bottom = y + height;

		Vertex vertices[] = {
			{{ left,  bottom, 0.0f }, { 0.0f, 1.0f }},
			{{ left,  top,    0.0f }, { 0.0f, 0.0f }},
			{{ right, bottom, 0.0f }, { 1.0f, 1.0f }},
			{{ right, top,    0.0f }, { 1.0f, 0.0f }},
		};

		void* mapped = nullptr;
		it->second.vertexBuffer->Map(0, nullptr, &mapped);
		memcpy(mapped, vertices, sizeof(vertices));
		it->second.vertexBuffer->Unmap(0, nullptr);

		// �ʒu�ƃT�C�Y��ς�����g�����X�t�H�[�����X�V����
		InitializeTextureTransform(key);
	}
}

void DXApplication::SetTextureRotation(const std::wstring& key, float radians)
{
	using namespace DirectX;

	auto it = textures_.find(key);
	if (it != textures_.end()) {
		
		// ���S���W�i�摜�̍��ォ�璆�S�܂ł̃I�t�Z�b�g�j
		float centerX = it->second.x + it->second.width * 0.5f;
		float centerY = it->second.y + it->second.height * 0.5f;

		// ��]�̂��߂̃��[���h�s��i���S����]�����ɖ߂��j
		XMMATRIX translateToOrigin = XMMatrixTranslation(-centerX, -centerY, 0.0f);
		XMMATRIX rotation = XMMatrixRotationZ(radians);
		XMMATRIX translateBack = XMMatrixTranslation(centerX, centerY, 0.0f);

		XMMATRIX worldMatrix = translateToOrigin * rotation * translateBack;

		// 2D�r���[�̓A�C�f���e�B�e�B
		XMMATRIX viewMatrix = XMMatrixIdentity();

		// ���ˉe�v���W�F�N�V�����i���オ (0,0)�A�E���� (width,height)�j
		XMMATRIX projMatrix = XMMatrixOrthographicOffCenterLH(
			0.0f, static_cast<float>(windowWidth_),
			static_cast<float>(windowHeight_), 0.0f,
			0.0f, 1.0f
		);

		// �����s��
		it->second.transformMatrix = worldMatrix * viewMatrix * projMatrix;

		// �萔�o�b�t�@�֓]��
		void* mapped = nullptr;
		it->second.constantBuffer->Map(0, nullptr, &mapped);
		memcpy(mapped, &it->second.transformMatrix, sizeof(XMMATRIX));
		it->second.constantBuffer->Unmap(0, nullptr);
	}
}

void DXApplication::InitializeTextureTransform(const std::wstring& key)
{
	auto it = textures_.find(key);
	if (it == textures_.end()) return;
	auto& resource = it->second;

	using namespace DirectX;
	resource.scaleMatrix = XMMatrixScaling(
		2.0f * resource.width / windowWidth_,
		-2.0f * resource.height / windowHeight_,
		1.0f
	);
	resource.rotationMatrix = XMMatrixIdentity();

	float centerX = resource.x + resource.width * 0.5f;
	float centerY = resource.y + resource.height * 0.5f;
	float ndcX = (2.0f * centerX / windowWidth_) - 1.0f;
	float ndcY = 1.0f - (2.0f * centerY / windowHeight_);

	resource.translationMatrix = XMMatrixTranslation(ndcX, ndcY, 0.0f);
	resource.transformMatrix =
		resource.scaleMatrix * resource.rotationMatrix * resource.translationMatrix;
}

bool DXApplication::GetTextureVisible(const std::wstring& key) {
	auto it = textures_.find(key);
	if (it != textures_.end()) {
		return it->second.visible;
	}
	return false;
}

void DXApplication::SetTextureVisible(const std::wstring& key, bool visible)
{
	auto it = textures_.find(key);
	if (it != textures_.end()) {
		it->second.visible = visible;
	}
}

void DXApplication::ReleaseTexture(const std::wstring& key)
{
	auto it = textures_.find(key);
	if (it != textures_.end()) {
		it->second.texture.Reset();
		it->second.vertexBuffer.Reset();
		it->second.indexBuffer.Reset();
		it->second.constantBuffer.Reset();
		textures_.erase(it);
	}
}

void DXApplication::SetTextureBrightnessAndAlpha(const std::wstring& key, float brightness, float alpha)
{
	auto it = textures_.find(key);
	if (it == textures_.end()) return;

	auto& tex = it->second;

	// �Œ�͈͂� clamp
	tex.brightness = std::clamp(brightness, 0.0f, 2.0f); // ���邳�͍ő�2�{
	tex.alpha = std::clamp(alpha, 0.0f, 1.0f);      // �����x�͍ő�1.0

	// �萔�o�b�t�@�� brightness �����̂ݍX�V
	ConstBufferData cbData = {};
	cbData.gBrightness = tex.brightness;
	cbData.gAlpha = tex.alpha;

	void* mapped = nullptr;
	D3D12_RANGE readRange = { 0, 0 }; // CPU �͓ǂݎ��Ȃ�
	tex.constantBuffer->Map(0, &readRange, &mapped);

	// brightness �� alpha �̃I�t�Z�b�g�ʒu�ɏ�������
	// gTransform �� 64 �o�C�g (4x4 float) �̏ꍇ�Aoffset �͂��̌�
	char* pData = reinterpret_cast<char*>(mapped);
	std::memcpy(pData + sizeof(DirectX::XMFLOAT4X4), &cbData.gAlpha, sizeof(float));
	std::memcpy(pData + sizeof(DirectX::XMFLOAT4X4) + sizeof(float), &cbData.gBrightness, sizeof(float));

	tex.constantBuffer->Unmap(0, nullptr);
}