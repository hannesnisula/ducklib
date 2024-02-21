#include <exception>
#include <Core/Memory/IAlloc.h>
#include "d3dx12.h"
#include "D3D12Api.h"
#include "D3D12CommandBuffer.h"
#include "D3D12ResourceCommandBuffer.h"
#include "D3D12SwapChain.h"
#include "D3D12Formats.h"

namespace DuckLib
{
namespace Render
{
D3D12Api::D3D12Api()
	: factory(nullptr)
	, device(nullptr)
{
#ifdef _DEBUG
	DL_D3D12_THROW_FAIL(
		CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)),
		"Failed to create DXGI factory");

	DL_D3D12_THROW_FAIL(
		D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)),
		"Failed to get debugInterface interface");

	debugInterface->EnableDebugLayer();
#else
	DL_D3D12_THROW_FAIL(
		CreateDXGIFactory2(IID_PPV_ARGS(&factory)),
		"Failed to create DXGI factory");
#endif

	EnumAndCreateAdapters();

	const IAdapter* adapter = nullptr;
	const std::vector<IAdapter*>& adapters = GetAdapters();

	for (const IAdapter* a : adapters)
		if (a->IsHardware())
			adapter = a;

	if (!adapter)
		throw std::exception("Failed to retrieve compatible adapter");

	DL_D3D12_THROW_FAIL(
		D3D12CreateDevice((IUnknown*)adapter->GetApiHandle(), DL_D3D_FEATURE_LEVEL,IID_PPV_ARGS(
			&device)),
		"Failed to create D3D12 device");

	commandQueue = CreateQueue(D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_FLAG_NONE);
}

D3D12Api::~D3D12Api()
{
	DestroyAdapters();

	for (ISwapChain* swapChain : swapChains)
		DestroySwapChain(swapChain);
}

const std::vector<IAdapter*>& D3D12Api::GetAdapters() const
{
	return adapters;
}

ISwapChain* D3D12Api::CreateSwapChain(
	uint32 width,
	uint32 height,
	Format format,
	uint32 bufferCount,
	HWND windowHandle)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	IDXGISwapChain1* apiSwapChain;
	ID3D12Resource* apiBuffer;

	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = MapToD3D12Format(format);
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = 0;

	DL_D3D12_THROW_FAIL(
		factory->CreateSwapChainForHwnd(commandQueue, windowHandle, &swapChainDesc, nullptr, nullptr,
			&apiSwapChain),
		"Failed to create swap chain");

	uint32 descriptorSize = device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	ID3D12DescriptorHeap* descriptorHeap = CreateDescriptorHeap(
		bufferCount,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorIterator(
		descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	ImageBuffer rtvHandles[ISwapChain::MAX_BUFFERS];

	for (uint32 i = 0; i < bufferCount; ++i)
	{
		DL_D3D12_THROW_FAIL(
			apiSwapChain->GetBuffer(i, IID_PPV_ARGS(&apiBuffer)),
			"Failed to get buffer from D3D12 swap chain");

		device->CreateRenderTargetView(apiBuffer, nullptr, descriptorIterator);

		rtvHandles[i].width = width;
		rtvHandles[i].height = height;
		rtvHandles[i].format = format;
		rtvHandles[i].apiResource = apiBuffer;
		rtvHandles[i].apiDescriptor = (void*)descriptorIterator.ptr;

		// TEST
		// ICommandBuffer* t = CreateCommandBuffer();
		// ID3D12GraphicsCommandList1* d = (ID3D12GraphicsCommandList1*)t->GetApiHandle();
		//
		// d->OMSetRenderTargets(1, &descriptorIterator, FALSE, nullptr);

		descriptorIterator.Offset(1, descriptorSize);
	}

	ID3D12Fence* apiFence;

	DL_D3D12_THROW_FAIL(
		device->CreateFence(UINT_MAX, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&apiFence)),
		"Failed to create fence for frame syncing");

	D3D12SwapChain* swapChain = DefAlloc()->New<D3D12SwapChain>(
		width,
		height,
		format,
		apiSwapChain,
		bufferCount,
		rtvHandles,
		apiFence,
		descriptorHeap,
		descriptorSize);
	swapChains.push_back(swapChain);

	return swapChain;
}

ICommandBuffer* D3D12Api::CreateCommandBuffer()
{
	ID3D12CommandAllocator* apiCommandAllocator;
	ID3D12GraphicsCommandList1* apiCommandList;

	HRESULT result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&apiCommandAllocator));

	if (result != S_OK)
		throw std::exception("Failed to create D3D12 command allocator");

	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		apiCommandAllocator,
		nullptr,
		IID_PPV_ARGS(&apiCommandList));

	if (result != S_OK)
		throw std::exception("Failed to create D3D12 command list");

	apiCommandList->Close();

	return new(DefAlloc()->Allocate<D3D12CommandBuffer>()) D3D12CommandBuffer(apiCommandList, apiCommandAllocator);
}

IResourceCommandBuffer* D3D12Api::CreateResourceCommandBuffer()
{
	ID3D12CommandAllocator* apiCommandAllocator;
	ID3D12GraphicsCommandList1* apiCommandList;

	HRESULT result = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_COPY,
		IID_PPV_ARGS(&apiCommandAllocator));

	if (result != S_OK)
		throw std::exception("Failed to create D3D12 command allocator");

	result = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_COPY,
		apiCommandAllocator,
		nullptr,
		IID_PPV_ARGS(&apiCommandList));

	if (result != S_OK)
		throw std::exception("Failed to create D3D12 command list");

	return new(DefAlloc()->Allocate<D3D12ResourceCommandBuffer>()) D3D12ResourceCommandBuffer(
		apiCommandList,
		apiCommandAllocator);
}

void D3D12Api::DestroySwapChain(ISwapChain* swapChain)
{
	swapChains.erase(std::find(swapChains.begin(), swapChains.end(), swapChain));

	DefAlloc()->Delete(swapChain);
}

void D3D12Api::DestroyCommandBuffer(ICommandBuffer* commandBuffer) {}

void D3D12Api::DestroyResourceCommandBuffer(IResourceCommandBuffer* resourceCommandBuffer) {}

void D3D12Api::ExecuteCommandBuffers(ICommandBuffer** commandBuffers, uint32 numCommandBuffers)
{
	ID3D12CommandList* apiLists[128];

	for (uint32 i = 0; i < numCommandBuffers; ++i)
		apiLists[i] = (ID3D12CommandList*)commandBuffers[i]->GetApiHandle();

	commandQueue->ExecuteCommandLists(numCommandBuffers, apiLists);
}

void D3D12Api::SignalCompletion(ISwapChain* swapChain)
{
	D3D12SwapChain* d3dSwapChain = (D3D12SwapChain*)swapChain;
	uint64 signalValue = swapChain->GetSignalValue();

	if (commandQueue->Signal(d3dSwapChain->apiFence, signalValue) != S_OK)
		throw std::exception("Failed to signal completion in D3D12 rendering");
}

void D3D12Api::EnumAndCreateAdapters()
{
	if (!factory)
		throw std::exception("Factory not initialized");

	IDXGIAdapter1* apiAdapter;
	DXGI_ADAPTER_DESC1 desc;

	for (uint32 i = 0; factory->EnumAdapters1(i, &apiAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		apiAdapter->GetDesc1(&desc);
		char descriptionBuffer[128];
		size_t dummy;

		wcstombs_s(&dummy, descriptionBuffer, desc.Description, 128);

		// S_FALSE because apparently that's what it's supposed to do on success with null device
		if (D3D12CreateDevice(
			apiAdapter,
			DL_D3D_FEATURE_LEVEL,
			_uuidof(ID3D12Device),
			nullptr) == S_FALSE)
		{
			IAdapter* adapter = DefAlloc()->New<D3D12Adapter>(
				descriptionBuffer,
				(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0,
				apiAdapter);
			adapters.push_back(adapter);
		}
	}
}

ID3D12CommandQueue* D3D12Api::CreateQueue(
	D3D12_COMMAND_LIST_TYPE type,
	D3D12_COMMAND_QUEUE_FLAGS flags)
{
	ID3D12CommandQueue* queue;
	D3D12_COMMAND_QUEUE_DESC queueDesc{};

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT result = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));

	if (result != S_OK)
		throw std::exception("Failed to create D3D12 queue");

	return queue;
}

ID3D12DescriptorHeap* D3D12Api::CreateDescriptorHeap(
	uint32 numDescriptors,
	D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	ID3D12DescriptorHeap* apiDescriptorHeap;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};

	heapDesc.NumDescriptors = numDescriptors;
	heapDesc.Type = type;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	DL_D3D12_THROW_FAIL(
		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&apiDescriptorHeap)),
		"Failed to create D3D12 descriptor heap");

	return apiDescriptorHeap;
}

ImageBuffer* D3D12Api::CreateImageBuffer(
	uint32 width,
	uint32 height,
	uint32 depth,
	Format format,
	ID3D12Resource* apiResource,
	D3D12_CPU_DESCRIPTOR_HANDLE apiDescriptor)
{
	ImageBuffer* imageBuffer = DefAlloc()->New<ImageBuffer>(
		width,
		height,
		depth,
		format,
		apiResource,
		(void*)apiDescriptor.ptr);

	return imageBuffer;
}

void D3D12Api::DestroyAdapters()
{
	for (IAdapter* adapter : adapters)
		DefAlloc()->Delete(adapter);

	adapters.clear();
}
}
}
