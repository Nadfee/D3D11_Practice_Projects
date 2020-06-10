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

/* For Matrix n Stuff */
#include <DirectXMath.h>
using namespace DirectX;

/* Constant Buffer Pointer */
ID3D11Buffer* constant_buffer_ptr;


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
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		// note that "POSITION" is the semantic for "position_local" in vs_in! This has to match.
		// DXGI_FORMAT_R32G32B32_FLOAT will appear as a float3 in our vertex shader
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

	// 1. We start by defining an array of vertex points
	float vertex_data_array[] =
	{
		0.0f, 0.5f, 0.f,		// top
		0.5f, -0.5f, 0.f,		// bottom right
		0.0f, -0.5f, 0.f,		// bottom left
	};

	UINT vertex_stride = 3 * sizeof(float);		// (what is stride?)
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

	hr = device_ptr->CreateBuffer(&vertex_buffer_desc, &sr_data, &vertex_buffer_ptr);
	assert(SUCCEEDED(hr));




	/* Create a Constant Buffer */
	struct const_buffer
	{
		XMMATRIX mov_mat;	// 16 * 4 bytes = 64 bytes (multiple of 16 byte, O.K for HLSL!)
		XMMATRIX another_mov;

	};

	//const_buffer const_buf;
	//const_buf.mov_mat = XMFLOAT4X4(
	//	0.3f, 0.f, 0.f, 0.f,
	//	0.f, 0.3f, 0.f, 0.f,
	//	0.f, 0.f, 0.3f, 0.f,
	//	0.f, 0.f, 0.f, 1.f);

	D3D11_BUFFER_DESC buffer_desc = { };
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;				// we want to be able to change it in our message loop :)
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		// we want to bind it to the constant buffer (cbuffer type in shader)
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// ..
	buffer_desc.MiscFlags = 0;
	buffer_desc.ByteWidth = 2 * sizeof(const_buffer);	// why this size??
	buffer_desc.StructureByteStride = 0;

	//D3D11_SUBRESOURCE_DATA subres_data = { };
	//subres_data.pSysMem = &const_buf;
	//subres_data.SysMemPitch = 0;
	//subres_data.SysMemSlicePitch = 0;

	hr = device_ptr->CreateBuffer(&buffer_desc, NULL, &constant_buffer_ptr);		// no init data, we will do this in message loop
	assert(SUCCEEDED(hr) && constant_buffer_ptr);


	/* Second cbuffer */
	ID3D11Buffer* other_const_buf;

	hr = device_ptr->CreateBuffer(&buffer_desc, NULL, &other_const_buf);		// no init data, we will do this in message loop
	assert(SUCCEEDED(hr) && other_const_buf);


	/* Create an array with our cbuffer pointers */
	ID3D11Buffer** my_bufs = (ID3D11Buffer * *)malloc(2 * sizeof(ID3D11Buffer*));
	my_bufs[0] = constant_buffer_ptr;
	my_bufs[1] = other_const_buf;


	float time = 1.f;

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

		/* Set our constant buffer */
		D3D11_MAPPED_SUBRESOURCE mapped_subresource;
		// Gets a pointer to the data contained in a subresource, and denies the GPU access to that subresource.
		device_context_ptr->Map(constant_buffer_ptr, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_subresource);

		// Edit our constant buffer
		const_buffer* my_buf_ptr = (const_buffer*)(mapped_subresource.pData);

		XMMATRIX mat = XMMATRIX(
			sinf(time), 0.f, 0.f, 0.f,
			0.f, sinf(time), 0.f, 0.f,
			0.f, 0.f, sinf(time), 0.f,
			0.f, 0.f, 0.f, 1.f);

		XMMATRIX mat2 = XMMATRIX(
			3 * sinf(time), 0.f, 0.f, 0.f,
			3 * sinf(time), 0.7f, 0.f, 0.f,
			3 * sinf(time), 0.f, 0.7f, 0.f,
			0.f, 0.f, 0.f, 1.f);

		my_buf_ptr->mov_mat = mat;
		my_buf_ptr->another_mov = mat2;
		time += 0.1f;

		device_context_ptr->Unmap(constant_buffer_ptr, NULL);

		/* Other const buf edit */
		D3D11_MAPPED_SUBRESOURCE oth_subresource;
		device_context_ptr->Map(other_const_buf, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &oth_subresource);

		const_buffer* new_buf_ptr = (const_buffer*)(oth_subresource.pData);

		XMMATRIX new_mat = XMMATRIX(
			3.0f, 0.f, 0.f, 0.f,
			0.f, 3.0f, 0.f, 0.f,
			0.f, 0.f, 3.f, 0.f,
			0.f, 0.f, 0.f, 1.f);

		new_buf_ptr->mov_mat = new_mat;
		new_buf_ptr->another_mov = mat2;

		device_context_ptr->Unmap(other_const_buf, NULL);

		/* Set our constant buffer to Vertex Shader pipeline! */
		device_context_ptr->VSSetConstantBuffers(0, 2, my_bufs);






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