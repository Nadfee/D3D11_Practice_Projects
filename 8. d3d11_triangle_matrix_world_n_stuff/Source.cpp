// For win32
#ifndef	UNICODE
#define UNICODE
#endif

// win32 and assert
#include <windows.h>
#include <assert.h>

#include <string>
#include <iostream>

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

int keystate[6];

// camera (https://www.braynzarsoft.net/viewtutorial/q16390-19-first-person-camera)

POINT point_curr;
POINT point_before;

XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

XMMATRIX camRotationMatrix;
XMMATRIX groundWorld;

float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;
float moveUpDown = 0.0f;

float camYaw = 0.0f;
float camPitch = 0.0f;

XMVECTOR camTarget;
XMVECTOR camPosition;
XMMATRIX view_m;

float deltaY;
float deltaX;
POINT ref_p;

bool left_button_down;

void UpdateCamera()
{
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	// find matrix for new right and forward. Rotating around Y axis!
	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	// update our new right, forward and up
	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	//camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	// movement of camera
	camPosition += moveLeftRight * camRight;
	camPosition += moveBackForward * camForward;
	camPosition += moveUpDown * camUp;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	// make sure to always have camTarget infront of camPosition
	camTarget = camPosition + camTarget;

	view_m = XMMatrixLookAtLH(camPosition, camTarget, camUp);
}



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
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

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
		XMMATRIX another_mat;
	};

	D3D11_BUFFER_DESC buffer_desc = { };
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;				// we want to be able to change it in our message loop :)
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		// we want to bind it as a constant buffer (cbuffer type in shader)
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// cpu allowed to write to buffer

	buffer_desc.MiscFlags = 0;
	buffer_desc.ByteWidth = 2 * sizeof(const_buffer);	// why this size??
	buffer_desc.StructureByteStride = 0;

	hr = device_ptr->CreateBuffer(&buffer_desc, NULL, &constant_buffer_ptr);		// no init data, we will update this in message loop
	assert(SUCCEEDED(hr) && constant_buffer_ptr);

	float time = 1.f;
	float offset = 0.f;
	float in_out = 0.1f;

	float look_horizontal = 0.f;
	float look_vertical = 0.f;

	GetCursorPos(&point_before);	// init
	ScreenToClient(hwnd, &point_before);

	//EXTRA
	ID3D11RasterizerState* ras_state_ptr;
	D3D11_RASTERIZER_DESC ras_desc;
	ras_desc.CullMode = D3D11_CULL_NONE;
	ras_desc.FillMode = D3D11_FILL_SOLID;
	ras_desc.FrontCounterClockwise = FALSE;
	ras_desc.DepthBias = 0;
	ras_desc.SlopeScaledDepthBias = 0.0f;
	ras_desc.DepthBiasClamp = 0.f;
	ras_desc.DepthClipEnable = TRUE;
	ras_desc.ScissorEnable = FALSE;
	ras_desc.MultisampleEnable = FALSE;
	ras_desc.AntialiasedLineEnable = FALSE;

	device_ptr->CreateRasterizerState(&ras_desc, &ras_state_ptr);


	//  ## MESSAGE LOOP -------------------------------------------------
	bool closed = false;
	MSG msg = { };

	while (!closed)
	{
		// update
		GetCursorPos(&point_curr);
		ScreenToClient(hwnd, &point_curr);


		//POINT ref_p;
		RECT lol;
		// for middle of window
		GetClientRect(hwnd, &lol);

		// placing cursor in the middle
		ref_p.x = (lol.right - lol.left) / 2;
		ref_p.y = (lol.bottom - lol.top) / 2;

		ClientToScreen(hwnd, &ref_p);

		if (left_button_down)
		{
			// mouse mov
			float temp_x = (float)point_before.x;
			float temp_y = (float)point_before.y;

			deltaX = (float)point_curr.x / temp_x;
			deltaY = (float)point_curr.y / temp_y;

			POINT tmp = ref_p;
			ScreenToClient(hwnd, &tmp);

			float x_mov = (point_curr.x - tmp.x);
			float y_mov = (point_curr.y - tmp.y);

				//SetCapture(hwnd);
				if ((point_curr.x != temp_x || point_curr.y != temp_y) && point_curr.x != tmp.x && point_curr.y != tmp.y)
				{

		

					if (x_mov < -100.f)
						x_mov = -100.f;
					else if (x_mov > 100.f)
						x_mov = 100.f;

					if (y_mov < -100.f)
						y_mov = -100.f;
					else if (y_mov > 100.f)
						y_mov = 100.f;

					camYaw += x_mov * 0.001f;
					camPitch += y_mov * 0.001f;

					//if (point_curr.x > temp_x)
					//	//camYaw += deltaX * 0.04f;
					//	camYaw += (point_curr.x - tmp.x) * 0.01f;
					//else if (point_curr.x < temp_x)
					//	//camYaw -= deltaX * 0.04f;
					//	camYaw -= (point_curr.x - tmp.x) * 0.01f;

					//if (point_curr.y > temp_y)
					//	//camPitch += deltaY * 0.04f;
					//	camPitch += (point_curr.y - tmp.x) * 0.01f;
					//else if (point_curr.y < temp_y)
					//	//camPitch -= deltaY * 0.04f;
					//	camPitch -= (point_curr.y - tmp.x) * 0.01f;
				}


			point_before = point_curr;

		}
		SetCursorPos(ref_p.x, ref_p.y);




		// message handling

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
		
		device_context_ptr->RSSetState(ras_state_ptr);

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


		/* Matrix */

		// Input
		if (keystate[0])
		{
			moveLeftRight -= 0.07f;
			offset -= 0.07f;
		}
		else if (keystate[1])
		{
			moveLeftRight += 0.07f;
			offset += 0.07f;
		}
		else
			moveLeftRight = 0.f;
			

		if (keystate[2])
		{
			moveBackForward += 0.07f;
			in_out += 0.07f;
		}
		else if (keystate[3])
		{
			moveBackForward -= 0.07f;
			in_out -= 0.07f;
		}
		else
			moveBackForward = 0.f;

		if (keystate[4])
		{
			moveUpDown += 0.02f;
		}
		else if (keystate[5])
		{
			moveUpDown -= 0.02f;
		}
		else
			moveUpDown = 0.f;
		


		// World Matrix

		//XMMATRIX world_matrix = XMMATRIX(
		//	1.f, 0.f, 0.f, 0.f,
		//	0.f, 1.f, 0.f, 0.f,
		//	0.f, 0.f, 1.f, 0.f,		
		//	0.f, 0.f, 10.f, 1.f				 
		//);

		XMMATRIX world_matrix = XMMatrixTranslation(0.f, 0.f, 12.5f);


		// View Matrix

		//XMMATRIX view_matrix = XMMATRIX(
		//	1.f, 0.f, 0.f, 0.f,				// right
		//	0.f, 1.f, 0.f, 0.f,				// up
		//	0.f, 0.f, 1.f, 0.f,				// facing forward	( not -forward like PPT? )
		//	-offset, 0.f, -in_out, 1.f		// camera at origin (use -position)
		//);
		
		XMVECTOR camera_position = XMVectorSet(offset, 0.f, in_out, 0.f);
		XMVECTOR focus_position = XMVectorSet(offset, 0.f, in_out + 1.f, 0.f);
		XMVECTOR up_direction = XMVectorSet(0.f, 1.f, 0.f, 0.f);;

		//XMMATRIX view_matrix = XMMatrixLookAtLH(camera_position, focus_position, up_direction);

		UpdateCamera();

		XMMATRIX view_matrix = view_m;



		// Perspective Projection Matrix (p.387 D3D11)
		float aspect = (float)(winRect.right - winRect.left) / (float)(winRect.bottom - winRect.top);
		float zFar = 15.f;
		float zNear = 0.1f;
		float yScale = 1.f / (tanf((3.14f / 180.f) * (90.f / 2.f)));
		float xScale = yScale / aspect;

		assert(zNear > 0.f && zNear < zFar);

		//XMMATRIX projection_matrix = XMMATRIX(
		//	xScale, 0.f, 0.f, 0.f,
		//	0.f, yScale, 0.f, 0.f,
		//	0.f, 0.f, zFar/(zFar-zNear), 1.f,
		//	0.f, 0.f, (-zNear*zFar)/(zFar-zNear), 0.f
		//);

		XMMATRIX projection_matrix = XMMatrixPerspectiveFovLH(yScale, aspect, zNear, zFar);
		
		// Orthographic Projection Matrix
		//float right = 10.f;
		//float left = -10.f;
		//float top = 10.f;
		//float bottom = -10.f;
		//float zFar = 50.f;
		//float zNear = 0.f;;

		// Changed so that Z is from 0 to 1
		//XMMATRIX projection_matrix = XMMATRIX(
		//	2.f / (right - left), 0.f, 0.f, 0.f,
		//	0.f, 2.f / (top - bottom), 0.f, 0.f,
		//	0.f, 0.f, 1.f / (zNear - zFar), 0.f,
		//	-((right + left) / (right - left)), -((top + bottom) / (top - bottom)), -((zNear) / (zNear - zFar)), 1.f
		//);

		//XMMATRIX projection_matrix = XMMatrixOrthographicLH(10.f, 10.f, zNear, zFar);


		XMMATRIX worldViewProjection_matrix = XMMatrixTranspose(world_matrix * view_matrix * projection_matrix);	// Note! This order because LH coordinate system (Left-to-right)

		my_buf_ptr->mov_mat = worldViewProjection_matrix;

		XMMATRIX rot_mat = XMMatrixRotationRollPitchYaw(0.f, 0.f, 0.f);
		my_buf_ptr->another_mat = XMMatrixTranspose(rot_mat);

		device_context_ptr->Unmap(constant_buffer_ptr, NULL);


		/* Set our constant buffer to Vertex Shader pipeline! */
		device_context_ptr->VSSetConstantBuffers(0, 1, &constant_buffer_ptr);


		// 6. Draw our Triangle (into the back buffer)
		// Pipeline will use all the states we have set (vertex buffer, shaders)
		// We just need to tell it how many vertices from the buffer we want to draw (e.g 3 for one triangle)
		device_context_ptr->Draw(vertex_count, 0);

		// 7. Present frame (Swap the buffer, back to front)
		swap_chain_ptr->Present(1, 0);

		time += 0.1f;

		//std::string oss = "Mouse Pos: (" + std::to_string(point_curr.x) + "," + std::to_string(point_curr.y) + ")";
		//oss << "Mouse Position: (" << point_curr.x << "," << point_curr.y << ")";


		//SetWindowTextA(hwnd, oss.c_str());

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

	case WM_KEYDOWN:

		wchar_t msg[32];
		if (wParam == 'A')
		{
			keystate[0] = 1;
		}
		else if (wParam == 'D')
		{
			keystate[1] = 1;
		}
		else if (wParam == 'W')
		{
			keystate[2] = 1;
		}
		else if (wParam == 'S')
		{
			keystate[3] = 1;
		}
		else if (wParam == 'Q')
		{
			keystate[4] = 1;
		}
		else if (wParam == 'E')
		{
			keystate[5] = 1;
		}

		break;

	case WM_KEYUP:
		if (wParam == 'A')
		{
			keystate[0] = 0;
		}
		else if (wParam == 'D')
		{
			keystate[1] = 0;
		}
		else if (wParam == 'W')
		{
			keystate[2] = 0;
		}
		else if (wParam == 'S')
		{
			keystate[3] = 0;
		}
		else if (wParam == 'Q')
		{
			keystate[4] = 0;
		}
		else if (wParam == 'E')
		{
			keystate[5] = 0;
		}
		break;

	case WM_MOUSEMOVE:
	{

		//// mouse mov
		//float temp_x = (float)point_before.x;
		//float temp_y = (float)point_before.y;

		//deltaX = (float)point_curr.x / temp_x;
		//deltaY = (float)point_curr.y / temp_y;

		//POINT tmp = ref_p;
		//ScreenToClient(hwnd, &tmp);

		//std::string oss = "Mouse Pos: (" + std::to_string(point_curr.x) + "," + std::to_string(tmp.x) + ")";


		//SetWindowTextA(hwnd, oss.c_str());

		//if (wParam == MK_LBUTTON)
		//{
		//	//SetCapture(hwnd);
		//	if ( ( point_curr.x != temp_x || point_curr.y != temp_y ) && point_curr.x != tmp.x && point_curr.y != tmp.y )
		//	{


		//		float x_mov = (point_curr.x - tmp.x);
		//		float y_mov = (point_curr.y - tmp.y);
		//		
		//		if (x_mov < -100.f)
		//			x_mov = -100.f;
		//		else if (x_mov > 100.f)
		//			x_mov = 100.f;

		//		if (y_mov < -100.f)
		//			y_mov = -100.f;
		//		else if (y_mov > 100.f)
		//			y_mov = 100.f;

		//		camYaw += x_mov * 0.001f;
		//		camPitch += y_mov * 0.001f;

		//		//if (point_curr.x > temp_x)
		//		//	//camYaw += deltaX * 0.04f;
		//		//	camYaw += (point_curr.x - tmp.x) * 0.01f;
		//		//else if (point_curr.x < temp_x)
		//		//	//camYaw -= deltaX * 0.04f;
		//		//	camYaw -= (point_curr.x - tmp.x) * 0.01f;

		//		//if (point_curr.y > temp_y)
		//		//	//camPitch += deltaY * 0.04f;
		//		//	camPitch += (point_curr.y - tmp.x) * 0.01f;
		//		//else if (point_curr.y < temp_y)
		//		//	//camPitch -= deltaY * 0.04f;
		//		//	camPitch -= (point_curr.y - tmp.x) * 0.01f;
		//		SetCursorPos(ref_p.x, ref_p.y);
		//	}
		//}
		////else
		////	ReleaseCapture();

		//point_before = point_curr;



		break;
	}
	case WM_LBUTTONDOWN:
	{
		left_button_down = true;

		break;
	}

	case WM_LBUTTONUP:
	{
		left_button_down = false;

		break;
	}



	case WM_MOUSELEAVE:
		ReleaseCapture();

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);	// default

	}
}