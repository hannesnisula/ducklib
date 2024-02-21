#include <cstdint>
#include <exception>
#include <Windows.h>
#include "Render/Core/ICommandBuffer.h"
#include "Render/Core/IDevice.h"
#include "Render/Core/D3D12/D3D12RHI.h"
#include "Render/Core/ISwapChain.h"
#include "Render/Core/Vulkan/VulkanRHI.h"

using namespace ducklib;
using namespace Render;

constexpr int WND_WIDTH = 800;
constexpr int WND_HEIGHT = 600;

bool runTest = true;
HWND window;

// ReSharper disable once CppInconsistentNaming
#define DL_D3D_API 0
#define DL_VK_API 1
#define DL_TEST_API DL_VK_API

#if DL_TEST_API == DL_D3D_API
IRHI* rhi = D3D12RHI::GetInstance();
#elif DL_TEST_API == DL_VK_API
IRHI* rhi = VulkanRHI::GetInstance();
#endif

float clearColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };

IAdapter* adapter;
IDevice* device;
ISwapChain* swapChain;
ICommandBuffer* cmdBuffer;
IPass* pass;
IFrameBuffer* frameBuffers[2];
IFence* frameRenderFence;

void InitRender(uint32_t width, uint32_t height, HWND windowHandle)
{
	const TArray<IAdapter*>& adapters = rhi->GetAdapters();

	if (adapters.IsEmpty())
		throw std::runtime_error("No adapters found");

	FrameBufferDesc frameBufferDesc(Format::B8G8R8A8_UNORM);
	FrameBufferDescRef frameBufferDescRef(0, ImageBufferLayout::COLOR);
	SubPassDescription subPassDesc(PipelineBindPoint::GRAPHICS, &frameBufferDescRef, 1);
	const PassDescription passDesc(&frameBufferDesc, 1, &subPassDesc, 1);

	adapter = adapters[0];
	device = adapter->CreateDevice(); // No parameters? Seems odd
	swapChain = device->CreateSwapChain(width, height, Format::B8G8R8A8_UNORM, 2, windowHandle);
	cmdBuffer = device->CreateCommandBuffer();
	pass = device->CreatePass(passDesc);
	frameRenderFence = device->CreateFence();

	ImageBuffer* imageBuffer1 = swapChain->GetBuffer(0);
	ImageBuffer* imageBuffer2 = swapChain->GetBuffer(1);

	frameBuffers[0] = device->CreateFrameBuffer(&imageBuffer1, 1, pass);
	frameBuffers[1] = device->CreateFrameBuffer(&imageBuffer2, 1, pass);

	cmdBuffer->SetClearColor(clearColor);
}

void RenderFrame()
{
	Sleep(20);

	frameRenderFence->Wait();
	swapChain->PrepareFrame();

	cmdBuffer->Reset();
	cmdBuffer->Begin();

	// TODO: Add passes and stuff here to see if Vulkan path can be tested yet (before adding emulated passes for D3D12) - done?
	cmdBuffer->BeginPass(pass, frameBuffers[swapChain->GetCurrentBufferIndex()]);

	constexpr Rect scissorRect{ 0, 0, WND_WIDTH, WND_HEIGHT };
	constexpr Viewport viewport{ 0.0f, 0.0f, (float)WND_WIDTH, (float)WND_HEIGHT, 0, 1 };

	cmdBuffer->SetScissorRect(scissorRect);
	cmdBuffer->SetViewport(viewport);
	// cmdBuffer->SetRT(swapChain->GetCurrentBuffer());

	// cmdBuffer->Draw();

	cmdBuffer->EndPass();
	cmdBuffer->End();

	device->ExecuteCommandBuffers(&cmdBuffer, 1, swapChain->GetImageAvailabilitySemaphore(), frameRenderFence);

	swapChain->Present();
}

void DestroyRender()
{
	device->DestroySwapChain(swapChain);
}

LRESULT __stdcall WndProc(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		runTest = false;
		return 0;
	default:
		return DefWindowProcA(hwnd, msg, wParam, lParam);
	}
}

HWND InitWindow(uint32_t width, uint32_t height, HINSTANCE hInstance)
{
	WNDCLASSEXA wndClass;
	RECT clientAreaRect;

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hIcon = nullptr;
	wndClass.hIconSm = nullptr;
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hInstance = hInstance;
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = "renderWindowClass";

	clientAreaRect.left = 0;
	clientAreaRect.top = 0;
	clientAreaRect.right = (LONG)width;
	clientAreaRect.bottom = (LONG)height;

	RegisterClassExA(&wndClass);
	AdjustWindowRectEx(&clientAreaRect, WS_OVERLAPPEDWINDOW, false, 0);

	const uint32_t windowWidth = clientAreaRect.right - clientAreaRect.left;
	const uint32_t windowHeight = clientAreaRect.bottom - clientAreaRect.top;

	const HWND newWindow = CreateWindowExA(
		0,
		"renderWindowClass",
		"Hello Triangle sample",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(int)windowWidth,
		(int)windowHeight,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	if (newWindow == nullptr)
		throw std::runtime_error("maddafakka");

	return newWindow;
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* cmdLine, int32_t cmdShow)
{
	MSG msg;
	constexpr uint32_t width = WND_WIDTH;
	constexpr uint32_t height = WND_HEIGHT;

	window = InitWindow(width, height, hInstance);

	InitRender(width, height, window);

	while (runTest)
	{
		while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		RenderFrame();
	}

	DestroyRender();

	return 0;
}
