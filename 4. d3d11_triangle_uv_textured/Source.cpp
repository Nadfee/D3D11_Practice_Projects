// For win32
#ifndef	UNICODE
#define UNICODE
#endif

// win32 and assert
#include <windows.h>
#include <assert.h>

// d3d stuff
#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>       // D3D interface
#include <dxgi.h>        // DirectX driver interface
#include <d3dcompiler.h> // shader compiler

// linking d3d libraries
#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler

// global pointers for D3D11 initialization
ID3D11Device* device_ptr;						// device
ID3D11DeviceContext* device_context_ptr;		// immediate device context
IDXGISwapChain* swap_chain_ptr;					// swap chain
ID3D11RenderTargetView* render_target_view_ptr;	// render target view

// global pointers for shaders
ID3D11VertexShader* vertex_shader_ptr;
ID3D11PixelShader* pixel_shader_ptr;

// global pointer for vertex buffer
ID3D11Buffer* vertex_buffer_ptr = NULL;

/* read image */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* global pointer for sampler state*/
ID3D11SamplerState* sampler_state_ptr;

/* global pointer for texture2d */
ID3D11Texture2D* texture2d_ptr;

/* global pointer to our shader resource view */
ID3D11ShaderResourceView* shader_resource_view_ptr;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// # CREATING THE WINDOW ----------------------------------------

	// 1. Register window (A)
	const wchar_t CLASS_NAME[] = L"MyWindow"; // window class name

	WNDCLASS wc = { 0 }; // make empty WNDCLASS (B)

	// init 3 important variables in wc (C)
	wc.lpfnWndProc = WindowProc;	// pointer to window procedure func.
	wc.hInstance = hInstance;		// give handle to this application instance
	wc.lpszClassName = CLASS_NAME;	// give the window class name

	RegisterClass(&wc);

	// 2. Creating the actual window (don't try to remember parameters, just check 'em up)
	HWND hwnd = CreateWindowEx(
		0,				// window styles
		CLASS_NAME,		// window class
		L"Window 2020",		// window text
		WS_OVERLAPPEDWINDOW,	// window style
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,	// x,y,width,height
		NULL,		// parent window
		NULL,		// menu
		hInstance,	// instance handle
		NULL		// additional application data
	);

	assert(hwnd);	// crash the program if hwnd is NULL

	// 3. Show window (nCmdShow is kinda like a state value where it should know whether to be minimized or maximized)
	ShowWindow(hwnd, nCmdShow);








	// ## INITIALIZING D3D11 --------------------------------------------------------------

	// # D3D11CreateDeviceAndSwapChain

	// 1. Start by filling in "Swap Chain Description" struct
	DXGI_SWAP_CHAIN_DESC swap_chain_desc = { 0 }; // Note that everything is set to '0' so anything that should be NULL are already set
	// Not initializing SwapEffect and Flags

	swap_chain_desc.Windowed = TRUE;
	swap_chain_desc.OutputWindow = hwnd;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = 1;								// number of back buffers you want to use -> "1 back buffer and 1 front buffer" therefore using double buffering
	swap_chain_desc.SampleDesc.Quality = 0;							// default (we are not enabling multisampling antialiasing)
	swap_chain_desc.SampleDesc.Count = 1;							// default (above)
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;			// 60 hz refresh rate
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	// color output without gamma correction (?)

	// 2. Create the Device and Swap Chain!
	D3D_FEATURE_LEVEL feature_level;
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

	// added flag for debug
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,						// pointer to the gph adapter that the device should be made for, NULL: default
		D3D_DRIVER_TYPE_HARDWARE,	// DriverType: which type of driver will be used for the device created
		NULL,						// handle to software driver dll(if you use software).NULL if non - software(using hardware)
		flags,						// flags
		NULL,						// pass an array with feature levels that you would like to use	(NONE)
		0,							// how many (element in feature level array) (NONE)
		D3D11_SDK_VERSION,			// always D3D11_SDK_VERSION
		&swap_chain_desc,			// give the address to the swap chain description (has init params for the swap chain)
		&swap_chain_ptr,			// COM-object output, returns the address of the pointer of the Swap Chain object
		&device_ptr,				// COM-object output, returns the address of the pointer of the Device object
		&feature_level,				// non-COM-object, returns the adress of the feature level object
		&device_context_ptr			// COM-object output, returns the address of the pointer to the Immediate Device Context
	);

	// swap_chain_ptr, device_ptr and device_context_ptr should all now point to active data (as we have initialized Device and Swap Chain!

	assert(SUCCEEDED(hr) && swap_chain_ptr && device_ptr && device_context_ptr);	// check that everything has initialized properly






	// # CreateRenderTargetView - The output images from Direct3D are called Render Targets

	// 1. Get our frameBuffer from our Swap Chain
	ID3D11Texture2D* frameBuffer;		// this is a D3D11Resource! (COM Object)
	hr = swap_chain_ptr->GetBuffer(		// We are asking the IDXGISwapChain for an ID3D11Texture2D interface!
		0,
		__uuidof(ID3D11Texture2D),		// ID for the type of interface we are looking for
		(void**)& frameBuffer			// Pointer that will point to that interface
	);
	assert(SUCCEEDED(hr));

	// 2. Create our render target view
	hr = device_ptr->CreateRenderTargetView(
		frameBuffer,
		0,
		&render_target_view_ptr
	);

	assert(SUCCEEDED(hr));

	frameBuffer->Release();	// we are done with our COM-object!
	// We were only interested in using the Frame Buffer from our Swap Chain to create our Render Target View (? Check pipeline on how this works )







	// # Create and Compile Shaders (D3DCompileFromFile)

	flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

	ID3DBlob* vs_blob_ptr = NULL;
	ID3DBlob* ps_blob_ptr = NULL;
	ID3DBlob* error_blob = NULL;

	// compile vertex shader
	hr = D3DCompileFromFile(
		L"shaders.hlsl",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"vs_main",
		"vs_5_0",		// Shader Target (check msdn)
		flags,
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

	// compile pixel shader
	hr = D3DCompileFromFile(
		L"shaders.hlsl",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"ps_main",
		"ps_5_0",
		flags,
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

	// Create the shaders
	hr = device_ptr->CreateVertexShader(
		vs_blob_ptr->GetBufferPointer(),		// what are these (?)
		vs_blob_ptr->GetBufferSize(),
		NULL,
		&vertex_shader_ptr
	);
	assert(SUCCEEDED(hr));

	hr = device_ptr->CreatePixelShader(
		ps_blob_ptr->GetBufferPointer(),
		ps_blob_ptr->GetBufferSize(),
		NULL,
		&pixel_shader_ptr
	);
	assert(SUCCEEDED(hr));

	// We have now compiled the shaders and created them (they are now bound to device)







	// # Create an Input Layout to describe how Vertex Data from the Buffer is mapped to the Input Variables for the --Vertex Shader--
	// This is done by filling out an array of D3D11_INPUT_ELEMENT_DESC structs and passing that to CreateInputLayout()

	ID3D11InputLayout* input_layout_ptr = NULL;
	D3D11_INPUT_ELEMENT_DESC input_element_desc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// note that "POSITION" is the semantic for "position_local" in vs_in! This has to match.
		// DXGI_FORMAT_R32G32B32_FLOAT will appear as a float3 in our vertex shader
		{"TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		// UV Coordinates!
	};

	hr = device_ptr->CreateInputLayout(
		input_element_desc,
		ARRAYSIZE(input_element_desc),
		vs_blob_ptr->GetBufferPointer(),
		vs_blob_ptr->GetBufferSize(),
		&input_layout_ptr
	);
	assert(SUCCEEDED(hr));








	// # Create a Vertex Buffer!

	// A vertex shader will set the final position of each vertex in homogeneous clip space
	// We won't transform our original vertex values, so we will define them in the visible range for clip space (for this)
	// X, Y : -1 to 1
	// Z: 0 to 1 (into the screen)
	// Direct3D considers by default a clockwise winding order of vertices to be the front face

	/* We start by defining an array of vertex points AND ALSO put in UV coordinates */
	float vertex_data_array[] =
	{
		-0.6f, 1.f, 0.f, 0.0f, 0.f,					// top
		0.4f, -1.f, 0.f, 1.f, 1.f,					// bottom right
		-0.6f, -1.f, 0.f, 0.f, 1.f,					// bottom left

		-0.4f, 1.f, 0.f, 0.f, 0.f,
		0.6f, 1.f, 0.f, 1.f, 0.f,
		0.6f, -1.f, 0.f, 1.f, 1.f
	};

	UINT vertex_stride = 5 * sizeof(float);		// (what is stride?)
	UINT vertex_offset = 0;
	UINT vertex_count = sizeof(vertex_data_array) / vertex_stride;

	// 2. We want to load this into the Vertex Buffer!
	// It requires a struct of type D3D11_BUFFER_DESC with parameters
	// and a struct D3D11_SUBRESOURCE_DATA which points to the actual vertex array data

	D3D11_BUFFER_DESC vertex_buffer_desc = { 0 };
	vertex_buffer_desc.ByteWidth = sizeof(vertex_data_array);
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sr_data = { 0 };
	sr_data.pSysMem = vertex_data_array;

	hr = device_ptr->CreateBuffer(
		&vertex_buffer_desc,
		&sr_data,
		&vertex_buffer_ptr
	);
	assert(SUCCEEDED(hr));






	/* Load Image */

	int texel_width, texel_height, n;		// n just to hold value? (?)
	int num_channels = 4;	// RGBA for the picture!	(4 bytes = 32 bits -> 8 bit per channel

	unsigned char *data = stbi_load("hey.jpg", &texel_width, &texel_height, &n, num_channels);

	assert(data);
	int texel_bytes_per_row = 4 * texel_width;	// 4 bytes * width (channel for each pixel times how many texels(?))


	/* Create Texture */
	D3D11_TEXTURE2D_DESC texture2d_desc = { };
	texture2d_desc.Width = texel_width;
	texture2d_desc.Height = texel_height;
	texture2d_desc.MipLevels = 1;		// what is multisampled texture?
	texture2d_desc.ArraySize = 1;		// only one picture on one face of triangle (?)
	texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	texture2d_desc.SampleDesc.Count = 1;			// default 1 (?)
	texture2d_desc.SampleDesc.Quality = 0;			// default 0 (?)
	texture2d_desc.Usage = D3D11_USAGE_IMMUTABLE;
	texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;	// Bind a buffer or texture to a shader stage (MSDN)
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresource_data = { };
	subresource_data.pSysMem = data;
	subresource_data.SysMemPitch = texel_bytes_per_row;			// pitch = row?

	hr = device_ptr->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d_ptr);
	assert(SUCCEEDED(hr));
	// Image buffer already in texture now! We used subresource data

	/* Create ShaderResourceView and bind texture to it */
	device_ptr->CreateShaderResourceView(texture2d_ptr, NULL, &shader_resource_view_ptr);		// NULL to create a view that accesses the entire resource (using the format the resource was created with)
	assert(shader_resource_view_ptr);

	/* Generate Mips (WHAT IS MIPS?? WHY GENERATE?) */
	//device_context_ptr->GenerateMips(shader_resource_view_ptr);

	/* Link Shader Resource View to Pixel Shader*/
	device_context_ptr->PSSetShaderResources(0, 1, &shader_resource_view_ptr);




	/* Create a Sampler State*/
	D3D11_SAMPLER_DESC sampler_desc = { };
	sampler_desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
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

	/* Link the Sampler State to our Pixel Shader */
	device_context_ptr->PSSetSamplers(0, 1, &sampler_state_ptr);


	free(data);






	//  ## MESSAGE LOOP -------------------------------------------------
	bool closed = false;
	MSG msg = { };

	while (!closed)
	{
		if (PeekMessageW(&msg, hwnd, NULL, NULL, PM_REMOVE))		// PeekMessage makes sure that the program is NOT waiting for input to run
		{
			TranslateMessage(&msg);		// turns keyboard strokes to characters
			DispatchMessage(&msg);		// invokes procedure function in the right window
		}

		if (msg.message == WM_QUIT)		// msg.message are "Window Notifications"
			closed = true;

		// DRAWING HERE!! (RENDERING)

		// 1. Clear the Back Buffer and Set the Viewport
		float background_color[] = {
			0.5f, 0.5f, 0.5f, 1.f			// rgba from 0 to 1
		};

		// Use our render target view pointer to access the back buffer and clear it to an RGBA colour 
		device_context_ptr->ClearRenderTargetView(render_target_view_ptr, background_color);

		// 2. Fetch the "drawing surface" from our Win32 handle -> Set up our Viewport
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
		device_context_ptr->RSSetViewports(1, &viewport);		// Rasteriser Set function! 

		// 5. Set the Output Merger (tell our Output Merger to use our Render Target
		device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);

		// 4. Set the Input Assembler (So it knows how to feed Vertex Data from the Vertex Buffer to Vertex Shader) (IA = Input Assembler)
		device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// Every 3 vertices should form a separate triangle
		device_context_ptr->IASetInputLayout(input_layout_ptr);		// We pass on the Input Layout we initialized earlier! (Tell the device to use our input layout)
		device_context_ptr->IASetVertexBuffers(						// Tell the device to use our vertex buffer
			0,
			1,
			&vertex_buffer_ptr,
			&vertex_stride,
			&vertex_offset
		);

		// 5. We need to tell the pipeline which shaders to use (Set the Shaders)
		device_context_ptr->VSSetShader(vertex_shader_ptr, NULL, 0);
		device_context_ptr->PSSetShader(pixel_shader_ptr, NULL, 0);

		// 6. Draw our Triangle (into the back buffer)
		// Pipeline will use all the states we have set (vertex buffer, shaders)
		// We just need to tell it how many vertices from the buffer we want to draw (e.g 3 for one triangle)
		device_context_ptr->Draw(vertex_count, 0);

		// 7. Present frame (Swap the buffer, back to front)
		swap_chain_ptr->Present(1, 0);

	}

}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		// called when ALT+F4 or "X to Close"
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