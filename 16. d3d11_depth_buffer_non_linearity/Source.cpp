#ifndef	UNICODE
#define UNICODE
#endif

#include <string>
#include <iostream>

#include <windows.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#include <d3d11.h>       // D3D interface
#include <dxgi.h>        // DirectX driver interface
#include <d3dcompiler.h> // shader compiler

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler

#define _USE_MATH_DEFINES
#include <math.h>
#include <DirectXMath.h>
using namespace DirectX;

// DirectInput ( https://www.braynzarsoft.net/viewtutorial/q16390-18-direct-input ) 
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>

IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

void UpdateCamera();
void DetectInput(float time);

RECT winRect;

// Timers
#include <profileapi.h> // precision timers
#include <stdint.h>     // uint64_t etc
void init_timer();
double get_seconds();

uint64_t frequency = 1000000, offset;


// Camera ( https://www.braynzarsoft.net/viewtutorial/q16390-19-first-person-camera )
XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

XMMATRIX camRotationMatrix;
XMMATRIX groundWorld;

float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;

float camYaw = 0.0f;
float camPitch = 0.0f;

XMVECTOR camTarget;
XMVECTOR camUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
XMVECTOR camPosition;
XMMATRIX camView;

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(float time);

POINT midScreen;
bool showCursor;
bool cursorMid;
bool keepUpdate;


/* Functions for Win32 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);	// Handle window procedures here
void n_RegisterClass(HINSTANCE hInstance, const wchar_t* CLASS_NAME);
HWND n_CreateWindow(HINSTANCE hInstance, const wchar_t* CLASS_NAME);
HWND hwnd;


/* Functions & Global variables for D3D11 */

ID3D11Device* device_ptr;
ID3D11DeviceContext* device_context_ptr;
IDXGISwapChain* swap_chain_ptr;
void n_CreateDeviceAndSwapChain(HWND hwnd);

ID3D11RenderTargetView* render_target_view_ptr;
void n_CreateRenderTargetView();

ID3DBlob* vs_blob_ptr = NULL;
ID3DBlob* ps_blob_ptr = NULL;
ID3DBlob* error_blob = NULL;
ID3D11VertexShader* vertex_shader_ptr;
ID3D11PixelShader* pixel_shader_ptr;
void n_CreateAndCompileVertexShader();
void n_CreateAndCompilePixelShader();

ID3D11InputLayout* input_layout_ptr = NULL;
void n_CreateInputLayout();

ID3D11Buffer* vertex_buffer_ptr = NULL;


struct Matrixes
{
	// Note: Has to be 16-byte aligned!
	XMMATRIX m_WorldViewProj;
	XMMATRIX m_World;
};

struct Light
{
	// Note: Has to be 16-byte aligned!
	XMFLOAT3 light_pos;
	float padding;
	XMFLOAT3 camera_pos;
	float padding2;
};
ID3D11Buffer* const_buf_mov;
ID3D11Buffer* const_buf_light;
void n_CreateConstantBuffer();


// ImGUI

#include "Graphics/ImGui/imgui.h"
#include "Graphics/ImGui/imgui_impl_win32.h"
#include "Graphics/ImGui/imgui_impl_dx11.h"

float translationOffset[3] = { 0.f, 0.f, 0.f };







int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Init timer
	init_timer();

	// Initialize DirectInput
	if (!InitDirectInput(hInstance))
	{
		MessageBox(0, L"Direct Input Initialization - Failed",
			L"Error", MB_OK);
		return 0;
	}

	const wchar_t CLASS_NAME[] = L"MyWindow";

	// Window
	n_RegisterClass(hInstance, CLASS_NAME);
	hwnd = n_CreateWindow(hInstance, CLASS_NAME);
	ShowWindow(hwnd, nCmdShow);

	// D3D11 Initialization
	n_CreateDeviceAndSwapChain(hwnd);
	n_CreateRenderTargetView();
	n_CreateAndCompileVertexShader();
	n_CreateAndCompilePixelShader();
	n_CreateInputLayout();

	// Vertex Buffer
	float vertex_data_array[] =
	{
		// pos, normal, uv
		//-0.75f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.0f, 0.f,
		//0.5f, -1.f, 0.f, 0.f, 1.f, 0.f, 1.f, 1.f,
		//-0.75f, -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,

		//-0.5f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
		//0.75f, 1.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f,
		//0.75f, -1.f, 0.f, 0.f, 1.f, 0.f, 1.f, 1.f
		-1.f, -1.f, 1.f,	0.f, 1.f, 0.f,	0.0f, 0.f,
		1.f, -1.f, 0.f,		0.f, 1.f, 0.f,	1.f, 1.f,
		-1.f, -1.f, 0.f,	0.f, 1.f, 0.f,	0.f, 1.f,

		-1.f, -1.f, 1.f,	0.f, 1.f, 0.f,	0.0f, 0.f,
		1.f, -1.f, 1.f,		0.f, 1.f, 0.f,	1.f, 0.f,
		1.f, -1.f, 0.f,		0.f, 1.f, 0.f,	1.f, 1.f

	};

	UINT vertex_stride = 8 * sizeof(float);
	UINT vertex_offset = 0;
	UINT vertex_count = sizeof(vertex_data_array) / vertex_stride;

	D3D11_BUFFER_DESC vertex_buffer_desc = { 0 };
	vertex_buffer_desc.ByteWidth = sizeof(vertex_data_array);
	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA sr_data = { 0 };
	sr_data.pSysMem = vertex_data_array;

	HRESULT hr = device_ptr->CreateBuffer(&vertex_buffer_desc, &sr_data, &vertex_buffer_ptr);
	assert(SUCCEEDED(hr));

	// Constant Buffer
	n_CreateConstantBuffer();

	// Applying pipeline settings
	device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context_ptr->IASetInputLayout(input_layout_ptr);
	device_context_ptr->IASetVertexBuffers(0, 1, &vertex_buffer_ptr, &vertex_stride, &vertex_offset);

	device_context_ptr->VSSetShader(vertex_shader_ptr, NULL, 0);
	device_context_ptr->PSSetShader(pixel_shader_ptr, NULL, 0);

	device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);

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

	device_context_ptr->VSSetConstantBuffers(0, 1, &const_buf_mov);
	device_context_ptr->PSSetConstantBuffers(1, 1, &const_buf_light);

	// Setup ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device_ptr, device_context_ptr);
	ImGui::StyleColorsDark();


	double startTime = 0.f;
	double endTime = 0.f;
	double frameTime = 0.f;
	bool closed = false;
	MSG msg = { };
	showCursor = true;
	cursorMid = false;
	keepUpdate = true;

	while (!closed)
	{
		frameTime = endTime - startTime;
		/* Frame time */
		startTime = get_seconds();

		/* User input handling */
		if (PeekMessageW(&msg, hwnd, NULL, NULL, PM_REMOVE))		// PeekMessage makes sure that the program keeps running and is not waiting for input to continue
		{
			TranslateMessage(&msg);		// turns keyboard strokes to characters
			DispatchMessage(&msg);		// invokes procedure function in the right window
		}

		if (msg.message == WM_QUIT)
			closed = true;

		/* DirectInput */
		DetectInput(frameTime);

		/* Rendering */

		// Clear background
		float background_color[] = { 0.5f, 0.5f, 0.5f, 1.f };
		device_context_ptr->ClearRenderTargetView(render_target_view_ptr, background_color);

		// Calculate World, View and Projection matrix

		XMMATRIX m_World = XMMatrixTranslation(translationOffset[0], translationOffset[1], translationOffset[2]);
		m_World *= XMMatrixScaling(1.f, 1.f, 1.f);


		if (keepUpdate)
			UpdateCamera();		// camView/m_View

		float fovAngle = (180.f / (2 * M_PI)) * (70.f);
		float aspectRatio = ((float)winRect.right - (float)winRect.left) / ((float)winRect.bottom - (float)winRect.top);

		XMMATRIX m_Projection = XMMatrixPerspectiveFovLH(fovAngle, aspectRatio, 1.f, 10.f);

		XMMATRIX worldViewProj = m_World * camView * m_Projection;

		// Update Constant Buffers

		D3D11_MAPPED_SUBRESOURCE cbufMovSubres;
		device_context_ptr->Map(const_buf_mov, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cbufMovSubres);

		Matrixes* mats = (Matrixes*)(cbufMovSubres.pData);
		mats->m_WorldViewProj = XMMatrixTranspose(worldViewProj);
		mats->m_World = XMMatrixTranspose(m_World);

		device_context_ptr->Unmap(const_buf_mov, NULL);

		D3D11_MAPPED_SUBRESOURCE cbufLightSubres;
		device_context_ptr->Map(const_buf_light, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cbufLightSubres);

		Light* lights = (Light*)(cbufLightSubres.pData);
		lights->light_pos = XMFLOAT3(0.f, 0.f, 0.f);
		lights->camera_pos = XMFLOAT3(XMVectorGetX(camPosition), XMVectorGetY(camPosition), XMVectorGetZ(camPosition));

		device_context_ptr->Unmap(const_buf_light, NULL);



		// Draw
		device_context_ptr->Draw(vertex_count, 0);

		// Start ImGui Frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// Create ImGui test window
		ImGui::Begin("Information");

		std::string s_fps = "FPS: " + std::to_string((int)(1.l / frameTime));
		std::string s_frameTime = "FrameTime: " + std::to_string(frameTime * 1000.l) + " ms";
		
		ImGui::DragFloat3("Translation Offset X/Y/Z", translationOffset, 0.25f, -5.f, 5.f);

		
		ImGui::Text(s_fps.c_str());
		ImGui::Text(s_frameTime.c_str());

		ImGui::End();
		// Assemble together draw data
		ImGui::Render();
		// Render draw data
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());











		swap_chain_ptr->Present(1, 0);

		endTime = get_seconds();
	}
}



/* ImGUI input */
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);	// extern - coming from outside

/* Functions for Win32 */

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Note Rerouting WndProc to the ImGui so that it can also pickup inputs!
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;	// Why return true though?

	switch (uMsg)
	{
		// called when ALT+F4 or "X"
	case WM_CLOSE:
		if (MessageBox(hwnd, L"Quit?", L"Checking..", MB_OKCANCEL) == IDOK)
		{
			PostQuitMessage(0);		// posts a WM_QUIT message to the threads (current) message queue - so that we can exit our loop in main!
		}
		break;

	case WM_KEYDOWN:
	{
		if (wParam == 'R' && showCursor)
		{
			showCursor = false;
			ShowCursor(FALSE);
		}
		if (wParam == 'T' && !showCursor)
		{
			showCursor = true;
			ShowCursor(TRUE);
		}
		if (wParam == 'E' && cursorMid)
		{
			cursorMid = false;
		}
		if (wParam == 'Q' && !cursorMid)
		{
			cursorMid = true;
		}
		if (wParam == 'F' && keepUpdate)
		{
			keepUpdate = false;
		}
		if (wParam == 'G' && !keepUpdate)
		{
			keepUpdate = true;
		}


	}

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
		L"world war z with ya boy corona",			// text
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
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;

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
		(void**)& frameBuffer			// Receiving the address of a pointer that will point to that the ID3D11Texture2D interface
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
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

void n_CreateConstantBuffer()
{
	// Create constant buffer, one for movement of object and one for lighting
	D3D11_BUFFER_DESC buf_desc_mov;
	buf_desc_mov.Usage = D3D11_USAGE_DYNAMIC;
	buf_desc_mov.ByteWidth = sizeof(Matrixes);
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

/* DirectInput */

bool InitDirectInput(HINSTANCE hInstance)
{
	/* Initialize DirectInput */
	HRESULT hr = DirectInput8Create(
		hInstance,
		DIRECTINPUT_VERSION,			// Version of DirectInput we want to use
		IID_IDirectInput8,				// Identifier to the interface we want to use
		(void**)& DirectInput,			// Returned pointer to our DirectInput object
		NULL);							// Used for COM stuff (specify NULL)

	/* Create DirectInput Devices */
	hr = DirectInput->CreateDevice(
		GUID_SysKeyboard,				// Globally Unique ID to our System Keyboard
		&DIKeyboard,					// Returned pointer to the DirectInput Device 
		NULL);							// COM related (specify NULL)

	hr = DirectInput->CreateDevice(		// Same as above
		GUID_SysMouse, &
		DIMouse,
		NULL);

	/* Tell the device what inputs are expected - DIDATAFORMATS are things you can choose from! */
	// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee417925(v%3Dvs.85)

	hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);	// Standard keyboard structure. An array of 256 characters, one for each key.
	hr = DIMouse->SetDataFormat(&c_dfDIMouse);			// Standard mouse structure. Three axes and four buttons. Corresponds to the DIMOUSESTATE structure.

	// Format info ( https://www.braynzarsoft.net/viewtutorial/q16390-18-direct-input ) 

	return true;
}

void DetectInput(float time)
{
	// Create mouse state to store info
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	// Acquire device interface information at this moment
	DIKeyboard->Acquire();
	DIMouse->Acquire();

	// Get the respective devie state and store them to their corresponding variables
	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)& keyboardState);

	float speed = 7.f * time;

	// Keyboard state have [KEYBOARD DEVICE ENUMERATIONS] (https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee418641(v%3Dvs.85)
	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;
	}
	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}

	// Update Yaw and Pitch if mouse has moved from previous position
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseCurrState.lX * 0.002f;
		camPitch += mouseCurrState.lY * 0.002f;

		mouseLastState = mouseCurrState;

		// Update cursor to the middle
		if (cursorMid)
		{
			POINT ref_p;
			ref_p.x = (winRect.right - winRect.left) / 2;
			ref_p.y = (winRect.bottom - winRect.top) / 2;
			ClientToScreen(hwnd, &ref_p);
			SetCursorPos(ref_p.x, ref_p.y);
		}
	}

	return;
}

/* Camera Update with regards to Input*/

void UpdateCamera()
{
	// Cam rotation dependent on Pitch and Yaw which are updated by Mouse Input
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);

	// Update our new 'front target' DIRECTION with regards to the new pitch and yaw
	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	// Creating temp matrix that only rotates around Y-axis (to fix 
	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);			// New right orientation
	camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);						// Unchanged as we are rotating around the same axis 
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);		// New forward orientation

	// Make sure to move with the correct orientation
	camPosition += moveLeftRight * camRight;
	camPosition += moveBackForward * camForward;

	// 0 as KB Input decides when to move
	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	// Position the target 'infront' of the camera. (This is with correct orientation! See Line 664)
	camTarget = camPosition + camTarget;

	camView = XMMatrixLookAtLH(camPosition, camTarget, camUp);
}


/* Timers */

// initialise timer variables for build’s platform
void init_timer() {
	frequency = 1000; // QueryPerformanceCounter default
	QueryPerformanceFrequency((LARGE_INTEGER*)& frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)& offset);
}
// get the current time in seconds with up to nanosecond precision
double get_seconds() {
	uint64_t counter = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)& counter);
	return (double)(counter - offset) / frequency;
}


