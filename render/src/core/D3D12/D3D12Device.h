#pragma once
#include <cstdint>
#include <vector>
#include "Lib/d3d12.h"
#include "Lib/dxgi1_4.h"
#include "D3D12Adapter.h"
#include "../ICommandBuffer.h"
#include "../IDevice.h"
#include "../Resources/Constants.h"

namespace ducklib::Render
{
constexpr D3D_FEATURE_LEVEL DL_D3D_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_1;

class D3D12Device : public IDevice
{
public:
	friend class D3D12Adapter;

	~D3D12Device() override;

	ISwapChain* CreateSwapChain(
		uint32_t width,
		uint32_t height,
		Format format,
		uint32_t bufferCount,
		HWND windowHandle) override;
	ICommandBuffer* CreateCommandBuffer() override;

	IPass* CreatePass(const PassDescription& passDesc) override;
	IFrameBuffer* CreateFrameBuffer(ImageBuffer** imageBuffers, uint32 imageBufferCount, IPass* pass) override;

	void DestroySwapChain(ISwapChain* swapChain) override;
	void DestroyCommandBuffer(ICommandBuffer* commandBuffer) override;

	void ExecuteCommandBuffers(ICommandBuffer** commandBuffers,
		uint32_t numCommandBuffers,
		void* waitSemaphore,
		IFence* signalFence) override;

	IFence* CreateFence() override;

private:
	D3D12Device(ID3D12Device* d3dDevice, IDXGIFactory4* dxgiFactory);

	ID3D12CommandQueue* CreateQueue(D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_FLAGS flags);
	ID3D12DescriptorHeap* CreateDescriptorHeap(uint32_t numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);
	ImageBuffer* CreateImageBuffer(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		Format format,
		ID3D12Resource* apiResource,
		D3D12_CPU_DESCRIPTOR_HANDLE apiDescriptor);

	void DestroyQueue(ID3D12CommandQueue* queue);
	void DestroyDescriptorHeap(ID3D12DescriptorHeap* descriptorHeap);

	static constexpr uint32_t NUM_FRAME_BUFFERS = 4;

	IAllocator* alloc;

	IDXGIFactory4* d3dFactory;
	ID3D12Device* d3dDevice;

	ID3D12CommandQueue* commandQueue;

	ID3D12DescriptorHeap* descriptorHeap{};

	// TODO: Replace this temporary store with an allocation object
	// TODO: ALSO use TArray instead of vector
	std::vector<ISwapChain*> swapChains;
};
}
