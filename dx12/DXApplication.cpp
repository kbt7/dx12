#include "DXApplication.h"
#include "Item.h"    // Item クラスの定義
#include "Enemy.h"   // Enemy クラスの定義

DXApplication::DXApplication(unsigned int width, unsigned int height, std::wstring title)
	: title_(title)
	, windowWidth_(width)
	, windowHeight_(height)

	// ★ポリゴン描画のため追加
	, viewport_(0.0f, 0.0f, static_cast<float>(windowWidth_), static_cast<float>(windowHeight_))
	, scissorrect_(0, 0, static_cast<LONG>(windowWidth_), static_cast<LONG>(windowHeight_))
	, vertexBufferView_({})
	, indexBufferView_({})

	, fenceValue_(0)
	, fenceEvent_(nullptr)
{
}

// 初期化処理
void DXApplication::OnInit(HWND hwnd)
{
	LoadPipeline(hwnd);
	LoadAssets(); // ★ポリゴン描画のため追加
}

// ★ポリゴン描画のため追加
void DXApplication::LoadAssets()
{
	// ルートシグネチャの生成
	{
		// ルートパラメータの生成
		// ディスクリプタテーブルの実体
		CD3DX12_DESCRIPTOR_RANGE1 discriptorRanges[2];
		discriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // CBV
		discriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC); // SRV
		CD3DX12_DESCRIPTOR_RANGE1 descriptorRanges[1];
		descriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		
		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, descriptorRanges, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsConstantBufferView(0); // b0に対応
		// サンプラーの生成
		// テクスチャデータからどう色を取り出すかを決めるための設定
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
		// ルートパラメータ、サンプラーからルートシグネチャを生成
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ComPtr<ID3DBlob> rootSignatureBlob = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &errorBlob));
		ThrowIfFailed(device_->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(rootsignature_.ReleaseAndGetAddressOf())));
	}

	// パイプラインステートの生成
	{
		// シェーダーオブジェクトの生成
#if defined(_DEBUG)
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		ComPtr<ID3DBlob> vsBlob;
		ComPtr<ID3DBlob> psBlob;
		D3DCompileFromFile(L"BasicVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicVS", "vs_5_0", compileFlags, 0, &vsBlob, nullptr);
		D3DCompileFromFile(L"BasicPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "BasicPS", "ps_5_0", compileFlags, 0, &psBlob, nullptr);

		// 頂点レイアウトの生成
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// パイプラインステートオブジェクト(PSO)を生成
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = rootsignature_.Get();
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) }; // 入力レイアウトの設定
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());                       // 頂点シェーダ
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());                       // ピクセルシェーダ
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);         // ラスタライザーステート
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
		psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;                           // サンプルマスクの設定
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;   // トポロジタイプ
		psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;    // ストリップ時のカット設定
		psoDesc.NumRenderTargets = 1;                                             // レンダーターゲット数
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;                       // レンダーターゲットフォーマット
		psoDesc.SampleDesc.Count = 1;                                             // マルチサンプリングの設定
		ThrowIfFailed(device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelinestate_.ReleaseAndGetAddressOf())));
	}
}

void DXApplication::LoadPipeline(HWND hwnd)
{
	UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
	{
		// デバッグレイヤーを有効にする
		ComPtr<ID3D12Debug> debugLayer;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
			debugLayer->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	// DXGIFactoryの初期化
	ComPtr<IDXGIFactory6> dxgiFactory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

	// デバイスの初期化
	CreateD3D12Device(dxgiFactory.Get(), device_.ReleaseAndGetAddressOf());

	// コマンド関連の初期化
	{
		// コマンドキュー
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // タイムアウト無し
		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // コマンドリストと合わせる
		ThrowIfFailed(device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueue_.ReleaseAndGetAddressOf())));
		// コマンドアロケータ
		ThrowIfFailed(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator_.ReleaseAndGetAddressOf())));
		// コマンドリスト
		ThrowIfFailed(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(commandList_.ReleaseAndGetAddressOf())));
		ThrowIfFailed(commandList_->Close());
	}

	// スワップチェーンの初期化
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

	// ディスクリプタヒープの初期化
	{
		// レンダーターゲットビュー (既存)
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = kFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeaps_.ReleaseAndGetAddressOf())));

		// SRV/CBV/UAV 用ヒープ（十分な数を確保）
		D3D12_DESCRIPTOR_HEAP_DESC basicHeapDesc = {};
		basicHeapDesc.NumDescriptors = 128; // ← 余裕を持たせる（必要なら増やして）
		basicHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		basicHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(device_->CreateDescriptorHeap(&basicHeapDesc, IID_PPV_ARGS(basicHeap_.ReleaseAndGetAddressOf())));

		// ディスクリプタ増分サイズを保存
		descriptorSizeCBVSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		nextSrvIndex_ = 0; // 初期化
	}

	// スワップチェーンと関連付けてレンダーターゲットビューを生成
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeaps_->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < kFrameCount; ++i)
		{
			ThrowIfFailed(swapchain_->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(renderTargets_[i].ReleaseAndGetAddressOf())));
			device_->CreateRenderTargetView(renderTargets_[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		}
	}

	// フェンスの生成
	{
		ThrowIfFailed(device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf())));
		fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}
}

// 更新処理
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



// 終了処理
void DXApplication::OnDestroy()
{
	CloseHandle(fenceEvent_);
}

void DXApplication::CreateD3D12Device(IDXGIFactory6* dxgiFactory, ID3D12Device** d3d12device)
{
	ID3D12Device* tmpDevice = nullptr;

	// グラフィックスボードの選択
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
		// AMDを含むアダプターオブジェクトを探して格納（見つからなければnullptrでデフォルト）
		// 製品版の場合は、オプション画面から選択させて設定する必要がある
		std::wstring strAdapter = adapterDesc.Description;
		if (strAdapter.find(L"AMD") != std::string::npos)
		{
			tmpAdapter = adapter;
			break;
		}
	}

	// Direct3Dデバイスの初期化
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	for (auto level : levels) {
		// 生成可能なバージョンが見つかったらループを打ち切り
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
		// hrのエラー内容をthrowする
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

	// ★修正箇所 1: 既存のキーのチェックと早期リターンを削除し、インデックス決定に利用する
	// 既存のテクスチャが存在するかどうかを確認し、インデックスを保持
	auto existingIt = textures_.find(key);

	// --- テクスチャ読み込み ---
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	TextureResource resource = {};
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;

	// ファイル読み込みとアップロードデータの準備
	ThrowIfFailed(LoadFromWICFile(filepath.c_str(), WIC_FLAGS_FORCE_RGB | WIC_FLAGS_IGNORE_SRGB, &metadata, scratchImg));
	ThrowIfFailed(PrepareUpload(device_.Get(), scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, subresources));

	// --- GPU テクスチャ作成 ---
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

	// --- アップロードバッファ ---
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

	// --- 定数バッファ --- 
	auto cbHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto cbDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstBufferData));
	ThrowIfFailed(device_->CreateCommittedResource(
		&cbHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource.constantBuffer.ReleaseAndGetAddressOf())
	));

	// --- 転送処理 ---
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

	// ★転送処理が完了するまで待機
	WaitForGpu();

	// --- SRV 作成 ---
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1u;

	// ★修正箇所 2: SRVインデックスの決定ロジック
	UINT index;
	if (existingIt != textures_.end()) {
		// 既存の場合: 既存のインデックスを再利用
		// (TextureResourceに srvIndex メンバーが存在することを前提とします)
		index = existingIt->second.srvIndex;
	}
	else {
		// 新規の場合: 次のインデックスを使用し、インクリメント
		index = nextSrvIndex_;
		nextSrvIndex_++;
	}

	if (index >= basicHeap_->GetDesc().NumDescriptors) ThrowIfFailed(E_FAIL);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(basicHeap_->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(index), descriptorSizeCBVSRV_);
	// 既存のスロットに新しいSRVを上書き作成
	device_->CreateShaderResourceView(texture.Get(), &srvDesc, cpuHandle);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(basicHeap_->GetGPUDescriptorHandleForHeapStart(), static_cast<INT>(index), descriptorSizeCBVSRV_);
	resource.srvHandle = gpuHandle;

	// --- 頂点データ ---
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

	// --- 頂点バッファ作成 ---
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

	// ★修正: 頂点データを GPU にコピー
	void* mapped = nullptr;
	resource.vertexBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, vertices, sizeof(vertices));
	resource.vertexBuffer->Unmap(0, nullptr);

	resource.vertexView.BufferLocation = resource.vertexBuffer->GetGPUVirtualAddress();
	resource.vertexView.StrideInBytes = sizeof(Vertex);
	resource.vertexView.SizeInBytes = vbSize;

	// --- インデックスバッファ ---
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

	// ★修正: インデックスもコピー
	resource.indexBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, indices, ibSize);
	resource.indexBuffer->Unmap(0, nullptr);

	resource.indexView.BufferLocation = resource.indexBuffer->GetGPUVirtualAddress();
	resource.indexView.SizeInBytes = ibSize;
	resource.indexView.Format = DXGI_FORMAT_R16_UINT;

	// --- トランスフォーム初期化 ---
	XMMATRIX matrix = XMMatrixIdentity();
	matrix.r[0].m128_f32[0] = 2.0f / windowWidth_;
	matrix.r[1].m128_f32[1] = -2.0f / windowHeight_;
	matrix.r[3].m128_f32[0] = -1.0f;
	matrix.r[3].m128_f32[1] = 1.0f;
	resource.transformMatrix = matrix;

	// ★修正: CBData に alpha も含めて書き込む
	ConstBufferData cbData = {};
	DirectX::XMStoreFloat4x4(&cbData.gTransform, resource.transformMatrix);
	cbData.gAlpha = alpha;
	cbData.gBrightness = 1.0f; // 初期輝度
	resource.brightness = 1.0f; // 構造体にも初期輝度を設定

	void* cbMapped = nullptr;
	resource.constantBuffer->Map(0, nullptr, &cbMapped);
	memcpy(cbMapped, &cbData, sizeof(ConstBufferData));
	resource.constantBuffer->Unmap(0, nullptr);

	// --- 情報格納 ---
	resource.x = x;
	resource.y = y;
	resource.width = width;
	resource.height = height;
	resource.texture = texture;
	resource.visible = true;
	resource.alpha = alpha;

	// ★修正箇所 3: SRVインデックスをリソースに格納
	resource.srvIndex = index;

	// ★修正箇所 4: マップにリソースを上書き登録
	// 既存のキーがある場合、ここで古い TextureResource が破棄され、
	// 内部の ComPtr が安全に解放されます。
	textures_[key] = std::move(resource);

	// トランスフォーム初期化（任意）
	InitializeTextureTransform(key);
}

void DXApplication::WaitForGpu()
{
	// フェンスにシグナルを送る
	ThrowIfFailed(commandQueue_->Signal(fence_.Get(), ++fenceValue_));

	// 完了していなければイベントで待機
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

		// 座標情報を更新
		it->second.x = x;
		it->second.y = y;

		// サイズ情報も更新（ここが重要）
		it->second.width = width;
		it->second.height = height;

		// 頂点データ更新
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

		// 位置とサイズを変えたらトランスフォームも更新する
		InitializeTextureTransform(key);
	}
}

void DXApplication::SetTextureRotation(const std::wstring& key, float radians)
{
	using namespace DirectX;

	auto it = textures_.find(key);
	if (it != textures_.end()) {
		
		// 中心座標（画像の左上から中心までのオフセット）
		float centerX = it->second.x + it->second.width * 0.5f;
		float centerY = it->second.y + it->second.height * 0.5f;

		// 回転のためのワールド行列（中心→回転→元に戻す）
		XMMATRIX translateToOrigin = XMMatrixTranslation(-centerX, -centerY, 0.0f);
		XMMATRIX rotation = XMMatrixRotationZ(radians);
		XMMATRIX translateBack = XMMatrixTranslation(centerX, centerY, 0.0f);

		XMMATRIX worldMatrix = translateToOrigin * rotation * translateBack;

		// 2Dビューはアイデンティティ
		XMMATRIX viewMatrix = XMMatrixIdentity();

		// 正射影プロジェクション（左上が (0,0)、右下が (width,height)）
		XMMATRIX projMatrix = XMMatrixOrthographicOffCenterLH(
			0.0f, static_cast<float>(windowWidth_),
			static_cast<float>(windowHeight_), 0.0f,
			0.0f, 1.0f
		);

		// 合成行列
		it->second.transformMatrix = worldMatrix * viewMatrix * projMatrix;

		// 定数バッファへ転送
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

	// 固定範囲で clamp
	tex.brightness = std::clamp(brightness, 0.0f, 2.0f); // 明るさは最大2倍
	tex.alpha = std::clamp(alpha, 0.0f, 1.0f);      // 透明度は最大1.0

	// 定数バッファの brightness 部分のみ更新
	ConstBufferData cbData = {};
	cbData.gBrightness = tex.brightness;
	cbData.gAlpha = tex.alpha;

	void* mapped = nullptr;
	D3D12_RANGE readRange = { 0, 0 }; // CPU は読み取らない
	tex.constantBuffer->Map(0, &readRange, &mapped);

	// brightness と alpha のオフセット位置に書き込む
	// gTransform が 64 バイト (4x4 float) の場合、offset はその後
	char* pData = reinterpret_cast<char*>(mapped);
	std::memcpy(pData + sizeof(DirectX::XMFLOAT4X4), &cbData.gAlpha, sizeof(float));
	std::memcpy(pData + sizeof(DirectX::XMFLOAT4X4) + sizeof(float), &cbData.gBrightness, sizeof(float));

	tex.constantBuffer->Unmap(0, nullptr);
}

void DXApplication::DrawTexture(
	const std::wstring& key,
	float x, float y,
	float width, float height,
	float alpha,
	float brightness,
	float rotation,
	bool visible)
{
	// 可視状態を設定
	SetTextureVisible(key, visible);
	if (!visible) return;

	// 位置とサイズを更新
	SetTexturePosition(key, x, y, width, height);

	// 回転を更新
	if (rotation != 0.0f) {
		SetTextureRotation(key, rotation);
	}
	else {
		InitializeTextureTransform(key); // 回転しない場合でも transform を更新
	}

	// 明るさとアルファを更新
	SetTextureBrightnessAndAlpha(key, brightness, alpha);
}
