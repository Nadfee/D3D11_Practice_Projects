// Namn: Nadhif Ginola
// Programkod: PAASP18

#ifndef	UNICODE
#define UNICODE
#endif

/* Win32 and Assert */
#include <windows.h>
#include <assert.h>

/* D3D11 includes */
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>       // D3D interface
#include <dxgi.h>        // DirectX driver interface
#include <d3dcompiler.h> // shader compiler

/* Linking D3D libraries */
#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler

/* DirectX Math library */
#include <DirectXMath.h>
using namespace DirectX;

/* Image data read */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* Functions for Win32 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	// Handle window procedures here
void n_RegisterClass(HINSTANCE hInstance, const wchar_t* CLASS_NAME);
HWND n_CreateWindow(HINSTANCE hInstance, const wchar_t* CLASS_NAME);


/* Functions & Global variables for D3D11 */

// 1. Create [DEVICE] and [SWAP CHAIN]
ID3D11Device* device_ptr;							
ID3D11DeviceContext* device_context_ptr;			
IDXGISwapChain* swap_chain_ptr;						
void n_CreateDeviceAndSwapChain(HWND hwnd);

// 2. Create [RENDER TARGET VIEW]
ID3D11RenderTargetView* render_target_view_ptr;		
void n_CreateRenderTargetView();

// 3. Create and Compile [VERTEX SHADER] & [PIXEL SHADER]
ID3DBlob* vs_blob_ptr = NULL;
ID3DBlob* ps_blob_ptr = NULL;
ID3DBlob* error_blob = NULL;
ID3D11VertexShader* vertex_shader_ptr;
ID3D11PixelShader* pixel_shader_ptr;
void n_CreateAndCompileVertexShader();
void n_CreateAndCompilePixelShader();

// 4. Create [INPUT LAYOUT]
ID3D11InputLayout* input_layout_ptr = NULL;
void n_CreateInputLayout();

// 5. Create [VERTEX BUFFER] in WinMain
ID3D11Buffer* vertex_buffer_ptr = NULL;

// Create [TEXTURE2D]
ID3D11Texture2D* texture2d_ptr;
void n_LoadImageAndCreateTexture2D();

// Create [SAMPLER STATE]
ID3D11SamplerState* sampler_state_ptr;
void n_CreateSamplerState();

// Create [SHADER RESOURCE VIEW]
ID3D11ShaderResourceView* shader_resource_view_ptr;

// Creating [CONSTANT BUFFER]
struct Movement
{
	// Note: Has to be 16-byte aligned!
	XMMATRIX move_mat;
};

struct Light
{
	// Note: Has to be 16-byte aligned!
	XMFLOAT3 light_pos;
	float padding;
};
ID3D11Buffer* const_buf_mov;
ID3D11Buffer* const_buf_light;
void n_CreateConstantBuffer();


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdShow)
{
	/* Win32 Window Creation */

	const wchar_t CLASS_NAME[] = L"MyWindow"; // Window Class name

	n_RegisterClass(hInstance, CLASS_NAME);				// 1. Register Window Class
	HWND hwnd = n_CreateWindow(hInstance, CLASS_NAME);	// 2. Creating the actual window
	ShowWindow(hwnd, nCmdShow);							// 3. Show window (nCmdShow: minimized/maximized, etc. state)

	/* ------------------------------------------------ */

	/* D3D11 Initialization */

	// 1. D3D11CreateDeviceAndSwapChain
	// Needs: [HWND]
	// Creates: [DEVICE], [DEVICE CONTEXT] & [SWAP CHAIN]
	n_CreateDeviceAndSwapChain(hwnd);

	// 2. CreateRenderTargetView - The output images from Direct3D are called Render Targets and we need a "Viewer" for the Render Targets D3D creates
	// Needs: [DEVICE] & [SWAP CHAIN] (to get a 2D frame buffer)
	// Creates: [RENDER TARGET VIEW]
	n_CreateRenderTargetView();

	// 3. Create and Compile Vertex and Pixel Shaders
	// Needs: [DEVICE], [VS BLOB], [PS BLOB] & [ERROR BLOB]
	// Note: ID3D11Blob for VS and PS are used in order to initialize our PS and VS respectively
	n_CreateAndCompileVertexShader();			// Creates: [VERTEX SHADER]
	n_CreateAndCompilePixelShader();			// Creates: [PIXEL SHADER]

	// 4. Create Input Layout for our Vertex buffer (in Input Assembler stage)
	// Needs: [VERTEX SHADER]
	// Creates: [INPUT LAYOUT]
	n_CreateInputLayout();

	// 5. Create a Vertex Buffer!
	// Direct3D considers by default a clockwise winding order of vertices for the front face
	UINT vertex_stride = 8 * sizeof(float);
	UINT vertex_offset = 0;
	UINT vertex_count;

	float vertex_data_array[] =
	{
		// pos, normal, uv
		-0.5f, 1.f, 0.8f, 0.f, 0.f, -1.f, 0.0f, 0.f,					// top
		0.5f, -1.f, 0.8f, 0.f, 0.f, -1.f, 1.f, 1.f,					// bottom right
		-0.5f, -1.f, 0.8f, 0.f, 0.f, -1.f, 0.f, 1.f,					// bottom left

		-0.5f, 1.f, 0.8f, 0.f, 0.f, -1.f, 0.f, 0.f,
		0.5f, 1.f, 0.8f, 0.f, 0.f, -1.f, 1.f, 0.f,
		0.5f, -1.f, 0.8f, 0.f, 0.f, -1.f, 1.f, 1.f
	};

	vertex_count = sizeof(vertex_data_array) / vertex_stride;

	D3D11_BUFFER_DESC vertex_buffer_desc = { 0 };
	vertex_buffer_desc.ByteWidth = sizeof(vertex_data_array);
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sr_data = { 0 };
	sr_data.pSysMem = vertex_data_array;

	HRESULT hr = device_ptr->CreateBuffer(
		&vertex_buffer_desc,
		&sr_data,
		&vertex_buffer_ptr
	);
	assert(SUCCEEDED(hr));

	/* ------------------------------------------------ */

	/* Texturing and Constant Buffer */

	// Load Image and Create [TEXTURE2D]
	n_LoadImageAndCreateTexture2D();

	// Create a [SHADER RESOURCE VIEW], binding the [TEXTURE2D] to the SRV
	device_ptr->CreateShaderResourceView(texture2d_ptr, NULL, &shader_resource_view_ptr);		// NULL in 2nd arg. to create a view that accesses the entire resource (using the format the resource was created with)
	assert(shader_resource_view_ptr);

	// Create a [SAMPLER STATE] - For sampling textures (filtering operations on texture resources)
	n_CreateSamplerState();

	// Create a [CONSTANT BUFFER] - To send in own data to the pipeline
	n_CreateConstantBuffer();

	/* ------------------------------------------------ */

	/* Using device context to finally set our 'settings' for the pipeline */
	/* Basic set up */

	// [INPUT ASSEMBLER] Set the Input Assembler (So it knows how to feed Vertex Data from the Vertex Buffer to Vertex Shader)
	device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);					// Every 3 vertices should form a separate triangle
	device_context_ptr->IASetInputLayout(input_layout_ptr);
	device_context_ptr->IASetVertexBuffers(0, 1, &vertex_buffer_ptr, &vertex_stride, &vertex_offset);	// Tell the device to use our vertex buffer with these properties (2nd n 3rd arg)

	// [SHADERS] We need to tell the pipeline which shaders to use (Set the Shaders)
	device_context_ptr->VSSetShader(vertex_shader_ptr, NULL, 0);
	device_context_ptr->PSSetShader(pixel_shader_ptr, NULL, 0);

	// [RTV -> OM] Set the render target for our Output Merger to the [RTV]
	device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);

	// [RS & VIEWPORT] Set the Viewport ( Fetch the "drawing surface" from our Win32 handle to set up our Viewport (drawing surface for our rasterizer) )
	RECT winRect;
	GetClientRect(hwnd, &winRect);
	D3D11_VIEWPORT viewport =
	{
		0.f,
		0.f,
		(FLOAT)(winRect.right - winRect.left),
		(FLOAT)(winRect.bottom - winRect.top),
		0.0f,
		1.f,
	};
	device_context_ptr->RSSetViewports(1, &viewport);

	/* Textures and Constant Buffers */

	// [CONSTANT BUFFERS] Bind constant buffers to corresponding stages in the pipeline
	device_context_ptr->VSSetConstantBuffers(0, 1, &const_buf_mov);
	device_context_ptr->PSSetConstantBuffers(1, 1, &const_buf_light);

	// [MIP MAPS] Generate Mips: 64x64 -> 32x32 -> 16x16.. : Builds an array of the texture resource bound to the SRV with smaller dimensions to use
	device_context_ptr->GenerateMips(shader_resource_view_ptr);
	// [SRV -> PS] Link Shader Resource View to Pixel Shader - SRV linked to PS meaning we can access the resource (Texture2D) linked to the SRV in the PS stage
	device_context_ptr->PSSetShaderResources(0, 1, &shader_resource_view_ptr);

	// [SAMPLER STATE -> PS] Link the Sampler State to our Pixel Shader 
	device_context_ptr->PSSetSamplers(0, 1, &sampler_state_ptr);


	float time = 0.f;
	bool closed = false;
	MSG msg = { };

	while (!closed)
	{
		/* User input handling */
		if (PeekMessageW(&msg, hwnd, NULL, NULL, PM_REMOVE))		// PeekMessage makes sure that the program keeps running and is not waiting for input to continue
		{
			TranslateMessage(&msg);		// turns keyboard strokes to characters
			DispatchMessage(&msg);		// invokes procedure function in the right window
		}

		if (msg.message == WM_QUIT)
			closed = true;

		/* Rendering */

		// Clear the Back Buffer & Set to some color
		float background_color[] = { 0.5f, 0.5f, 0.5f, 1.f };
		// Use our [RENDER TARGET VIEW] to access the back buffer and clear it to an RGBA colour 
		device_context_ptr->ClearRenderTargetView(render_target_view_ptr, background_color);
		
		// Update our constant buffers here - Using Map/Unmap
		
		// Update triangle vertices position
		D3D11_MAPPED_SUBRESOURCE mov_subres;
		device_context_ptr->Map(const_buf_mov, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mov_subres);

		Movement* mov_ptr = (Movement*)(mov_subres.pData);
		XMMATRIX move_m = XMMATRIX(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.5f * sinf(time), 0.f, 0.f, 1.0f
		);

		mov_ptr->move_mat = XMMatrixTranspose(move_m);
		device_context_ptr->Unmap(const_buf_mov, NULL);

		// Update light source position
		D3D11_MAPPED_SUBRESOURCE light_subres;
		device_context_ptr->Map(const_buf_light, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &light_subres);

		Light* light_ptr = (Light*)(light_subres.pData);
		light_ptr->light_pos = XMFLOAT3(1.1f * sinf(time), 0.f, 0.5f * sinf(time) + 0.5f);
		light_ptr->padding = 1.f;

		device_context_ptr->Unmap(const_buf_light, NULL);

		// Draw our Triangle (into the back buffer)
		device_context_ptr->Draw(vertex_count, 0);

		// Present frame (Swap the buffer, back to front)
		swap_chain_ptr->Present(1, 0);

		time += 0.01f;
	}
}



/* Functions for Win32 */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// called when ALT+F4 or "X"
	case WM_CLOSE:
		if (MessageBox(hwnd, L"Quit?", L"Checking..", MB_OKCANCEL) == IDOK)
		{
			PostQuitMessage(0);		// posts a WM_QUIT message to the threads (current) message queue - so that we can exit our loop in main!
		}
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);	// default

	}
}

void n_RegisterClass(HINSTANCE hInstance, const wchar_t* CLASS_NAME)
{
	WNDCLASS wc = { 0 }; // make empty WNDCLASS (B)

	// init 3 important variables in wc (C)
	wc.lpfnWndProc = WindowProc;	// pointer to window procedure func.
	wc.hInstance = hInstance;		// give handle to this application instance
	wc.lpszClassName = CLASS_NAME;	// give the window class name

	RegisterClass(&wc);
}

HWND n_CreateWindow(HINSTANCE hInstance, const wchar_t* CLASS_NAME)
{
	HWND hwnd = CreateWindowEx(
		0,						// styles
		CLASS_NAME,				// class
		L"Window 2020",			// text
		WS_OVERLAPPEDWINDOW,	// style
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,	// x, y, width, height
		NULL,				// parent window
		NULL,				// menu
		hInstance,			// instance handle
		NULL				// additional app data
	);

	assert(hwnd);	// crash the program if hwnd is NULL

	return hwnd;
}



/* Functions for D3D11 */

// Initialization for Triangle
void n_CreateDeviceAndSwapChain(HWND hwnd)
{
	// 1. Start by filling in "Swap Chain Description" struct
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = { 0 }; // Note that everything is set to '0' so anything that should be NULL are already set

	swap_chain_desc.Windowed = TRUE;
	swap_chain_desc.OutputWindow = hwnd;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 1;								// number of back buffers you want to use -> "1 back buffer and 1 front buffer" therefore using double buffering
	swap_chain_desc.SampleDesc.Quality = 0;							// default (we are not enabling multisampling antialiasing)
	swap_chain_desc.SampleDesc.Count = 1;							// default (above)
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;			// 60 hz refresh rate
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 2. Create the Device and Swap Chain!
	D3D_FEATURE_LEVEL feature_level;
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,						// pointer to the gph adapter that the device should be made for, NULL: default
		D3D_DRIVER_TYPE_HARDWARE,	// DriverType: which type of driver will be used for the device created
		NULL,						// handle to software driver dll (if you use software) - NULL if none (using hardware)
		flags,						// flags
		NULL,						// pass an array with feature levels that you would like to use	(NONE)
		0,							// how many (element in feature level array) (NONE)
		D3D11_SDK_VERSION,			// always D3D11_SDK_VERSION
		&swap_chain_desc,			// give the address to the swap chain description (has init params for the swap chain)
		&swap_chain_ptr,			// COM-object, returns the address of the pointer of the Swap Chain object
		&device_ptr,				// COM-object, returns the address of the pointer of the Device object
		NULL,						// not determining which feature level is supported
		&device_context_ptr			// COM-object, returns the address of the pointer to the Immediate Device Context
	);

	assert(SUCCEEDED(hr) && swap_chain_ptr && device_ptr && device_context_ptr);	// check that everything has initialized properly
}

void n_CreateRenderTargetView()
{

	// 1. Get our frameBuffer from our Swap Chain
	ID3D11Texture2D* frameBuffer;				// This is a D3D11Resource! (COM Object)
	HRESULT hr = swap_chain_ptr->GetBuffer(		// We are asking the IDXGISwapChain for a ID3D11Texture2D interface!
		0,
		__uuidof(ID3D11Texture2D),		// ID for the type of interface we are looking for
		(void**)&frameBuffer			// Receiving the address of a pointer that will point to that the ID3D11Texture2D interface
	);
	assert(SUCCEEDED(hr));

	// 2. Create our render target view
	hr = device_ptr->CreateRenderTargetView(
		frameBuffer,
		0,
		&render_target_view_ptr
	);

	assert(SUCCEEDED(hr));

	// We were only interested in using the Frame Buffer from our Swap Chain to create our Render Target View
	frameBuffer->Release();	// We are done with our COM-object!

}

void n_CreateAndCompileVertexShader()
{
	// compile vertex shader
	HRESULT hr = D3DCompileFromFile(
		L"shaders.hlsl",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs_main",
		"vs_5_0",		// Shader Target
		NULL,
		0,
		&vs_blob_ptr,
		&error_blob
	);

	// if fail, make sure to release
	if (FAILED(hr))
	{
		if (error_blob)
		{
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (vs_blob_ptr)
		{
			vs_blob_ptr->Release();
		}
		assert(false);
	}

	// Create the shaders
	hr = device_ptr->CreateVertexShader(
		vs_blob_ptr->GetBufferPointer(),	// pointer to the compiled shader
		vs_blob_ptr->GetBufferSize(),		// size of the compiled shader
		NULL,
		&vertex_shader_ptr
	);
	assert(SUCCEEDED(hr));
}

void n_CreateAndCompilePixelShader()
{
	// compile pixel shader
	HRESULT hr = D3DCompileFromFile(
		L"shaders.hlsl",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps_main",
		"ps_5_0",
		NULL,
		0,
		&ps_blob_ptr,
		&error_blob
	);

	// if fail, make sure to release
	if (FAILED(hr))
	{
		if (error_blob)
		{
			OutputDebugStringA((char*)error_blob->GetBufferPointer());
			error_blob->Release();
		}
		if (vs_blob_ptr)
		{
			vs_blob_ptr->Release();
		}
		assert(false);
	}


	hr = device_ptr->CreatePixelShader(
		ps_blob_ptr->GetBufferPointer(),	// pointer to the compiled shader
		ps_blob_ptr->GetBufferSize(),		// size of the compiled shader
		NULL,
		&pixel_shader_ptr
	);
	assert(SUCCEEDED(hr));
}

void n_CreateInputLayout()
{

	// # Create an Input Layout to describe how Vertex Data from the Buffer is mapped to the Input Variables for the --Vertex Shader--
	// This is done by filling out an array of D3D11_INPUT_ELEMENT_DESC structs and passing that to CreateInputLayout()

	D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// note that "POSITION" is the semantic for "position_local" in vs_in! This has to match.
		// DXGI_FORMAT_R32G32B32_FLOAT will appear as a float3 in our vertex shader
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{"TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		// UV Coordinates!
	};

	HRESULT hr = device_ptr->CreateInputLayout(
		input_element_desc,
		ARRAYSIZE(input_element_desc),
		vs_blob_ptr->GetBufferPointer(),
		vs_blob_ptr->GetBufferSize(),
		&input_layout_ptr
	);
	assert(SUCCEEDED(hr));
}

// Textures and Constant Buffer
void n_LoadImageAndCreateTexture2D()
{
	/* Load Image */

	int texel_width, texel_height, n;
	int num_channels = 4;	// RGBA for the picture!	(4 bytes = 32 bits -> 8 bit per channel)

	unsigned char* data = stbi_load("hey.jpg", &texel_width, &texel_height, &n, num_channels);

	assert(data);
	int bytes_per_row = 4 * texel_width;	// 4 bytes (one texel) * width 

	/* Create Texture */
	D3D11_TEXTURE2D_DESC texture2d_desc = { };
	texture2d_desc.Width = texel_width;
	texture2d_desc.Height = texel_height;
	texture2d_desc.MipLevels = 1;		
	texture2d_desc.ArraySize = 1;		// only one picture on one face of triangle
	texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	texture2d_desc.SampleDesc.Count = 1;			// default 1
	texture2d_desc.SampleDesc.Quality = 0;			// default 0
	texture2d_desc.Usage = D3D11_USAGE_IMMUTABLE;
	texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// Bind the texture to the shader stage
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresource_data = { };
	subresource_data.pSysMem = data;
	subresource_data.SysMemPitch = bytes_per_row;			// The distance (in bytes) from the beginning of one line of a texture to the next line. (essentially row)

	HRESULT hr = device_ptr->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d_ptr);
	assert(SUCCEEDED(hr));

	// Image data now part of our Texture2D as data (sub resource)
	free(data);
}

void n_CreateSamplerState()
{
	D3D11_SAMPLER_DESC sampler_desc = { };
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 1;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.BorderColor[0] = 1.f;
	sampler_desc.BorderColor[1] = 1.f;
	sampler_desc.BorderColor[2] = 1.f;
	sampler_desc.BorderColor[3] = 1.f;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = 0;

	device_ptr->CreateSamplerState(&sampler_desc, &sampler_state_ptr);
	assert(sampler_state_ptr);
}

void n_CreateConstantBuffer()
{
	// Create constant buffer, one for movement of object and one for lighting
	D3D11_BUFFER_DESC buf_desc_mov;
	buf_desc_mov.Usage = D3D11_USAGE_DYNAMIC;
	buf_desc_mov.ByteWidth = sizeof(Movement);
	buf_desc_mov.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// Buffer is a [CONSTANT BUFFER]
	buf_desc_mov.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buf_desc_mov.MiscFlags = NULL;
	buf_desc_mov.StructureByteStride = 0;

	D3D11_BUFFER_DESC buf_desc_light = buf_desc_mov;	// Reusing previous buffer desc.
	buf_desc_light.ByteWidth = sizeof(Light);			// Only changing size

	// Create the buffers
	HRESULT hr = device_ptr->CreateBuffer(&buf_desc_mov, NULL, &const_buf_mov);
	assert(SUCCEEDED(hr) && const_buf_mov);
	hr = device_ptr->CreateBuffer(&buf_desc_light, NULL, &const_buf_light);
	assert(SUCCEEDED(hr) && const_buf_light);
}