#include "ShaderMain.h"
#include <stdio.h>

#include <assert.h>

// D3D 관련
LPDIRECT3D9			gpD3D			= NULL;
LPDIRECT3DDEVICE9	gpD3DDevice		= NULL;

// 폰트
ID3DXFont*			gpFont			= NULL;

// 모델
LPD3DXMESH			gpSphere		= NULL;
LPD3DXMESH			gpTeapot		= NULL;
// Ch 07
LPD3DXMESH			gpSphereTangent	= NULL;
// Ch 09
LPD3DXMESH			gpTorus = NULL;
// Ch 10
LPD3DXMESH			gpDisc = NULL;

// 세이더
// Ch 02
LPD3DXEFFECT		gpColorShader	= NULL; 
// Ch 03
LPD3DXEFFECT		gpTextureMappingShader = NULL;
// Ch 04
LPD3DXEFFECT		gpLightingShader = NULL;
// Ch 05
LPD3DXEFFECT		gpSpecularMappingShader = NULL;
// Ch 06
LPD3DXEFFECT		gpToonShader = NULL;
// Ch 07
LPD3DXEFFECT		gpNormalMappingShader = NULL;
// Ch 08
LPD3DXEFFECT		gpEnvironmentMappingShader = NULL;
// Ch 09
LPD3DXEFFECT		gpUVAnimationShader = NULL;
// Ch 10
LPD3DXEFFECT		gpCreateShadowShader = NULL;
LPD3DXEFFECT		gpApplyShadowShader = NULL;
// Ch 11
LPD3DXEFFECT		gpNoEffectShader = NULL;
LPD3DXEFFECT		gpGrayscaleShader = NULL;
LPD3DXEFFECT		gpSepiaShader = NULL;
// Ch 12
LPD3DXEFFECT		gpEdgeDetectionShader = NULL;
LPD3DXEFFECT		gpEmbossShader = NULL;

// 텍스쳐
// Ch 03
LPDIRECT3DTEXTURE9	gpEarthDM = NULL;
// Ch 05
LPDIRECT3DTEXTURE9 gpStoneDM = NULL;
LPDIRECT3DTEXTURE9 gpStoneSM = NULL;
// Ch 07
LPDIRECT3DTEXTURE9 gpStoneNM = NULL;
// Ch 08
LPDIRECT3DCUBETEXTURE9 gpSnowENV = NULL;
// Ch 10
LPDIRECT3DTEXTURE9 gpShadowRenderTarget = NULL;
LPDIRECT3DSURFACE9 gpShadowDepthStencil = NULL;
// Ch 11
LPDIRECT3DTEXTURE9 gpSceneRenderTarget = NULL;

// 프로그램 이름
const char* gAppName = "초 간단 셰이더 데모 프레임워크";

// 회전 값
float gRotationY = 0.f;

// 빛의 위치
D3DXVECTOR4 gWorldLightPosition(500.0f, 500.0f, -500.0f, 1.0f);

// 빛의 색상 05
D3DXVECTOR4 gLightColor(0.7f, 0.7f, 1.0f, 1.0f);

// 카메라 위치
D3DXVECTOR4 gWorldCameraPosition(0.0f, 0.0f, -200.0f, 1.0f);

// 표면 색상 06
D3DXVECTOR4 gSurfaceColor(0.0f, 1.0f, 0.f, 1.0f);

// 물체 색상 10
D3DXVECTOR4 gTorusColor(1.0f, 1.0f, 0.f, 1.0f);
D3DXVECTOR4 gDiscColor(0.0f, 1.0f, 1.0f, 1.0f);

// 사각형 메쉬 11
LPDIRECT3DVERTEXDECLARATION9 gpFullscreenQuadDecl = NULL;
LPDIRECT3DVERTEXBUFFER9 gpFullscreenQuadVB = NULL;
LPDIRECT3DINDEXBUFFER9 gpFullscreenQuadIB = NULL;

// 사용할 포스트프로세스 쉐이더 인덱스
int gPostProcessIndex = 0;

enum CHAPTER
{
	CH02,
	CH03,
	CH04,
	CH05,
	CH06,
	CH07,
	CH08,
	CH09,
	CH10,
	CH11,
	CH12,
};
CHAPTER Chapter = CH02;

// 매크로
#define SAFE_RELEASE(x) \
if(x) \
{ x->Release(); x = NULL; }

#define LOAD_SHADER_CHECK(shader, path) \
shader = LoadShader(path); \
if(!shader) { assert(false); return false; }

#define LOAD_TEXTURE_CHECK(texture, path) \
texture = LoadTexture(path); \
if(!texture) { assert(false); return false; }

#define LOAD_CUBE_TEXTURE_CHECK(texture, path) \
texture = LoadCubeTexture(path); \
if(!texture) { assert(false); return false; }

#define LOAD_MODEL_CHECK(model, path) \
model = LoadModel(path); \
if(!model) { assert(false); return false; }

// 진입점
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	// 윈도우 클래스 등록
	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		gAppName, NULL
	};

	RegisterClassEx(&wc);
	
	// 프로그램 창을 생성한다.
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(
		gAppName, gAppName,
		style, CW_USEDEFAULT, 0, WIN_WIDTH, WIN_HEIGHT,
		GetDesktopWindow(), NULL, wc.hInstance, NULL
	);

	// Client Rect 크기가 WIN_WIDTH, WIN_HEIGHT 와 같도록 크기를 조정한다.
	POINT ptDiff;
	RECT rcClient, rcWindow;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(hWnd, rcWindow.left, rcWindow.top, WIN_WIDTH + ptDiff.x, WIN_HEIGHT + ptDiff.y, TRUE);

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);


	// D3D를 비롯한 모든 것을 초기화한다.
	if (!InitEverything(hWnd))
	{
		PostQuitMessage(1);
	}

	// 메세지 루프
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			PlayDemo();
		}
	}

	UnregisterClass(gAppName, wc.hInstance);
	return 0;
}

// 메세지 처리기
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		ProcessInput(hWnd, wParam);
		break;
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// 키보드 입력처리
void ProcessInput(HWND hWnd, WPARAM keyPress)
{
	switch (keyPress)
	{
	case VK_ESCAPE:
		PostMessage(hWnd, WM_DESTROY, 0L, 0L);
		break;
	// 0x30 - 0x39
	case 0x31:
		Chapter = CH02;
		break;
	case 0x32:
		Chapter = CH03;
		break;
	case 0x33:
		Chapter = CH04;
		break;
	case 0x34:
		Chapter = CH05;
		break;
	case 0x35:
		Chapter = CH06;
		break;
	case 0x36:
		Chapter = CH07;
		break;
	case 0x37:
		Chapter = CH08;
		break;
	case 0x38:
		Chapter = CH09;
		break;
	case 0x39:
		Chapter = CH10;
		break;
	case 81: //'q'
		Chapter = CH11;
		break;
	case 90: //'z': // Ch11
		gPostProcessIndex = 0;
		break;
	case 88: //'x': // Ch11
		gPostProcessIndex = 1;
		break;
	case 67: //'c': // Ch11
		gPostProcessIndex = 2;
		break;
	case 87: //'w'
		Chapter = CH12;
		break;
	case 'e':
		Chapter = CH12;
		break;
	default:
		break;
	}
}

// 초기화 코드
bool InitEverything(HWND hWnd)
{
	// D3D 초기화
	if (!InitD3D(hWnd))
	{
		return false;
	}

	// 화면을 가득 채우는 사각형을 만듬
	InitFullScreenQuad();

	// 렌더 타겟을 만든다
	const int shadowMapSize = 2048;
	if (FAILED(gpD3DDevice->CreateTexture(shadowMapSize, shadowMapSize, 1, 
		D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &gpShadowRenderTarget, NULL)))
	{
		return false;
	}

	// 그림자맵과 동일한 크기의 깊이버퍼도 만들어줘야 한다.
	if (FAILED(gpD3DDevice->CreateDepthStencilSurface(shadowMapSize, shadowMapSize, 
		D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 0, TRUE, &gpShadowDepthStencil, NULL)))
	{
		return false;
	}

	// 렌더 타겟 생성
	if (FAILED(gpD3DDevice->CreateTexture(WIN_WIDTH, WIN_HEIGHT, 1, 
		D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gpSceneRenderTarget, NULL)))
	{
		return false;
	}


	// 모델, 셰이더, 텍스쳐 등을 로딩
	if (!LoadAssets())
	{
		return false;
	}

	// 폰트를 로딩
	if (FAILED(D3DXCreateFont(gpD3DDevice, 20, 10, FW_BOLD, 1, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		(DEFAULT_PITCH | FF_DONTCARE), "Arial", &gpFont)))
	{
		return false;
	}

	return true;
}

// D3D 객체 및 장치 초기화
bool InitD3D(HWND hWnd)
{
	// D3D 객체
	gpD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!gpD3D)
	{
		return false;
	}

	// D3D 장치를 생성하는데 필요한 구조체를 채워 넣는다.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferWidth = WIN_WIDTH;
	d3dpp.BackBufferHeight = WIN_HEIGHT;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.MultiSampleQuality = 0;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.Windowed = TRUE;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24X8;
	d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	d3dpp.FullScreen_RefreshRateInHz = 0;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	// D3D 장치를 생성한다.
	if (FAILED(gpD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, 
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &gpD3DDevice)))
	{
		return false;
	}

	return true;
}

void InitFullScreenQuad()
{
	// 정점 선언을 만든다
	D3DVERTEXELEMENT9 vtxDesc[3];

	int offset = 0;
	int i = 0;

	// 위치
	vtxDesc[i].Stream = 0;
	vtxDesc[i].Offset = offset;
	vtxDesc[i].Type = D3DDECLTYPE_FLOAT3;
	vtxDesc[i].Method = D3DDECLMETHOD_DEFAULT;
	vtxDesc[i].Usage = D3DDECLUSAGE_POSITION;
	vtxDesc[i].UsageIndex = 0;

	offset += sizeof(float) * 3;
	++i;

	// UV 좌표 0
	vtxDesc[i].Stream = 0;
	vtxDesc[i].Offset = offset;
	vtxDesc[i].Type = D3DDECLTYPE_FLOAT2;
	vtxDesc[i].Method = D3DDECLMETHOD_DEFAULT;
	vtxDesc[i].Usage = D3DDECLUSAGE_TEXCOORD;
	vtxDesc[i].UsageIndex = 0;

	offset += sizeof(float) * 2;
	++i;

	// 정점 포맷의 끝임을 표현
	vtxDesc[i].Stream = 0xFF;
	vtxDesc[i].Offset = 0;
	vtxDesc[i].Type = D3DDECLTYPE_UNUSED;
	vtxDesc[i].Method = 0;
	vtxDesc[i].Usage = 0;
	vtxDesc[i].UsageIndex = 0;
	// D3DDECL_END()

	gpD3DDevice->CreateVertexDeclaration(vtxDesc, &gpFullscreenQuadDecl);

	// 정점 버퍼를 만듬
	gpD3DDevice->CreateVertexBuffer(offset * 4, 0, 0, D3DPOOL_MANAGED, &gpFullscreenQuadVB, NULL);

	void* vertexData = NULL;
	gpFullscreenQuadVB->Lock(0, 0, &vertexData, 0);
	{
		// 정점 위치, UV 좌표 순
		float* data = (float*)vertexData;
		*data++ = -1.0f; *data++ = 1.0f; *data++ = 0.0f;
		*data++ = 0.0f; *data++ = 0.0f;

		*data++ = 1.0f; *data++ = 1.0f; *data++ = 0.0f;
		*data++ = 1.0f; *data++ = 0.0f;

		*data++ = 1.0f; *data++ = -1.0f; *data++ = 0.0f;
		*data++ = 1.0f; *data++ = 1.0f;

		*data++ = -1.0f; *data++ = -1.0f; *data++ = 0.0f;
		*data++ = 0.0f; *data++ = 1.0f;
	}
	gpFullscreenQuadVB->Unlock();

	// 색인 버퍼를 만듬
	gpD3DDevice->CreateIndexBuffer(sizeof(short) * 6, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &gpFullscreenQuadIB, NULL);
	
	void* indexData = NULL;
	gpFullscreenQuadIB->Lock(0, 0, &indexData, 0);
	{
		unsigned short* data = (unsigned short*)indexData;
		*data++ = 0; *data++ = 1; *data++ = 3;
		*data++ = 3; *data++ = 1; *data++ = 2;
	}
	gpFullscreenQuadIB->Unlock();
}

bool LoadAssets()
{
	//  텍스쳐 로딩
	LOAD_TEXTURE_CHECK(gpEarthDM, "../RenderMonkey/Earth.jpg");
	LOAD_TEXTURE_CHECK(gpStoneDM, "../RenderMonkey/Fieldstone_DM.tga");
	LOAD_TEXTURE_CHECK(gpStoneSM, "../RenderMonkey/fieldstone_SM.tga");
	LOAD_TEXTURE_CHECK(gpStoneNM, "../RenderMonkey/fieldstone_NM.tga");
	LOAD_CUBE_TEXTURE_CHECK(gpSnowENV, "../RenderMonkey/Snow_ENV.dds");

	// 셰이더 로딩
	/*gpColorShader = LoadShader("../RenderMonkey/ColorShader.fx");
	if (!gpColorShader)
	{
		return false;
	}*/
	LOAD_SHADER_CHECK(gpColorShader, "../RenderMonkey/ColorShader.fx");
	LOAD_SHADER_CHECK(gpTextureMappingShader, "../RenderMonkey/TextureMapping.fx");
	LOAD_SHADER_CHECK(gpLightingShader, "../RenderMonkey/Lighting.fx");
	LOAD_SHADER_CHECK(gpSpecularMappingShader, "../RenderMonkey/SpecularMapping.fx");
	LOAD_SHADER_CHECK(gpToonShader, "../RenderMonkey/ToonShader.fx");
	LOAD_SHADER_CHECK(gpNormalMappingShader, "../RenderMonkey/NormalMapping.fx");
	LOAD_SHADER_CHECK(gpEnvironmentMappingShader, "../RenderMonkey/EnvironmentMapping.fx");
	LOAD_SHADER_CHECK(gpUVAnimationShader, "../RenderMonkey/UVAnimation.fx");
	LOAD_SHADER_CHECK(gpCreateShadowShader, "../RenderMonkey/CreateShadow.fx");
	LOAD_SHADER_CHECK(gpApplyShadowShader, "../RenderMonkey/ApplyShadow.fx");
	LOAD_SHADER_CHECK(gpNoEffectShader, "../RenderMonkey/NoEffect.fx");
	LOAD_SHADER_CHECK(gpGrayscaleShader, "../RenderMonkey/Grayscale.fx");
	LOAD_SHADER_CHECK(gpSepiaShader, "../RenderMonkey/Sepia.fx");
	LOAD_SHADER_CHECK(gpEdgeDetectionShader, "../RenderMonkey/EdgeDetection.fx");
	LOAD_SHADER_CHECK(gpEmbossShader, "../RenderMonkey/Emboss.fx");

	// 모델 로딩
	/*gpSphere = LoadModel("../RenderMonkey/sphere.x");
	if (!gpSphere)
	{
		return false;
	}*/
	LOAD_MODEL_CHECK(gpSphere, "../RenderMonkey/sphere.x");
	LOAD_MODEL_CHECK(gpTeapot, "../RenderMonkey/teapot.x");
	LOAD_MODEL_CHECK(gpSphereTangent, "../RenderMonkey/SphereWithTangent.x");
	LOAD_MODEL_CHECK(gpTorus, "../RenderMonkey/Torus.x");
	LOAD_MODEL_CHECK(gpDisc, "../RenderMonkey/Disc.x");

	return true;
}

// 셰이더 로딩
LPD3DXEFFECT LoadShader(const char* filename)
{
	LPD3DXEFFECT ret = NULL;
	LPD3DXBUFFER pError = NULL;
	DWORD dwShaderFlags = 0;

#if _DEBUG
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

	D3DXCreateEffectFromFile(gpD3DDevice, filename, 
		NULL, NULL, dwShaderFlags, NULL, &ret, &pError);

	// 셰이더 로딩에 실패한 경우 output 창에 셰이더 컴파일 에러를 출력한다.
	if (!ret && pError)
	{
		int size = pError->GetBufferSize();
		void* ack = pError->GetBufferPointer();

		if (ack)
		{
			char* str = new char[size];
			sprintf_s(str, size, (const char*)ack);
			OutputDebugString(str);
			delete[] str;
		}
	}

	return ret;
}

// 모델 로딩
LPD3DXMESH LoadModel(const char* filename)
{
	LPD3DXMESH ret = NULL;
	if (FAILED(D3DXLoadMeshFromX(filename, D3DXMESH_SYSTEMMEM, gpD3DDevice, 
		NULL, NULL, NULL, NULL, &ret)))
	{
		OutputDebugString("모델 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	}

	return ret;
}

// 텍스쳐 로딩
LPDIRECT3DTEXTURE9 LoadTexture(const char* filename)
{
	LPDIRECT3DTEXTURE9 ret = NULL;
	if (FAILED(D3DXCreateTextureFromFile(gpD3DDevice, filename, &ret)))
	{
		OutputDebugString("텍스쳐 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	}

	return ret;
}

LPDIRECT3DCUBETEXTURE9 LoadCubeTexture(const char* filename)
{
	LPDIRECT3DCUBETEXTURE9 ret = NULL;
	if (FAILED(D3DXCreateCubeTextureFromFile(gpD3DDevice, filename, &ret)))
	{
		OutputDebugString("텍스쳐 로딩 실패: ");
		OutputDebugString(filename);
		OutputDebugString("\n");
	}

	return ret;
}


// 게임 루프
void PlayDemo()
{
	Update();
	RenderFrame();
}

// 게임 로직 업데이트
void Update()
{
	switch (Chapter)
	{
	case CH02:
		break;
	case CH03:
	case CH04:
	case CH05:
	case CH06:
	case CH07:
	case CH08:
	case CH09:
	case CH10:
	case CH11:
	case CH12:
	{
		// 프레임 마다 0.4도씩 회전 시킴
		gRotationY += 0.4f * PI / 180.0f;
		if (gRotationY > 2 * PI)
		{
			gRotationY -= 2 * PI;
		}
	}
		break;
	default:
		break;
	}
}


// 렌더링
void RenderFrame()
{
	D3DCOLOR bgColor = 0xff0000ff; // blue
	gpD3DDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), bgColor, 1.0f, 0);

	gpD3DDevice->BeginScene();
	{
		RenderScene();
		RenderInfo();
	}
	gpD3DDevice->EndScene();

	gpD3DDevice->Present(NULL, NULL, NULL, NULL);
}

// 3D 물체 등을 그린다.
void RenderScene()
{
	switch (Chapter)
	{
	case CH02:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(0.0f, 0.0f, -200.0f);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 쉐이더 전역 변수들을 설정
		gpColorShader->SetMatrix("gWorldMatrix", &matWorld);
		gpColorShader->SetMatrix("gViewMatrix", &matView);
		gpColorShader->SetMatrix("gProjectionMatrix", &matProjection);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpColorShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpColorShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpSphere->DrawSubset(0);
			}
			gpColorShader->EndPass();
		}
		gpColorShader->End();
	}
		break;
	case CH03:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(0.0f, 0.0f, -200.0f);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixRotationY(&matWorld, gRotationY);

		// 쉐이더 전역 변수들을 설정
		gpTextureMappingShader->SetMatrix("gWorldMatrix", &matWorld);
		gpTextureMappingShader->SetMatrix("gViewMatrix", &matView);
		gpTextureMappingShader->SetMatrix("gProjectionMatrix", &matProjection);
		// 텍스쳐 설정
		gpTextureMappingShader->SetTexture("DiffuseMap_Tex", gpEarthDM); // _Tex를 붙인 이유는 렌더몽키에서 자동으로 붙여주기 때문에

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpTextureMappingShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpTextureMappingShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpSphere->DrawSubset(0);
			}
			gpTextureMappingShader->EndPass();
		}
		gpTextureMappingShader->End();
	}
		break;
	case CH04:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 쉐이더 전역 변수들을 설정
		gpLightingShader->SetMatrix("gWorldMatrix", &matWorld);
		gpLightingShader->SetMatrix("gViewMatrix", &matView);
		gpLightingShader->SetMatrix("gProjectionMatrix", &matProjection);

		gpLightingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpLightingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpLightingShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpLightingShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpSphere->DrawSubset(0);
			}
			gpLightingShader->EndPass();
		}
		gpLightingShader->End();
	}
		break;
	case CH05:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 쉐이더 전역 변수들을 설정
		gpSpecularMappingShader->SetMatrix("gWorldMatrix", &matWorld);
		gpSpecularMappingShader->SetMatrix("gViewMatrix", &matView);
		gpSpecularMappingShader->SetMatrix("gProjectionMatrix", &matProjection);

		gpSpecularMappingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpSpecularMappingShader->SetVector("gLightColor", &gLightColor);
		gpSpecularMappingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);

		gpSpecularMappingShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
		gpSpecularMappingShader->SetTexture("SpecularMap_Tex", gpStoneSM);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpSpecularMappingShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpSpecularMappingShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpSphere->DrawSubset(0);
			}
			gpSpecularMappingShader->EndPass();
		}
		gpSpecularMappingShader->End();
	}
		break;
	case CH06:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 월드의 역행렬을 구한다
		D3DXMATRIXA16 matInvWorld;
		D3DXMatrixTranspose(&matInvWorld, &matWorld); // 전치행렬 구하는 함수
		// D3DXMatrixInverse() : 역행렬 구하는 함수

		// 월드/뷰/투영행렬을 미리 곱한다.
		D3DXMATRIXA16 matWorldView;
		D3DXMATRIXA16 matWorldViewProjection;
		D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
		D3DXMatrixMultiply(&matWorldViewProjection, &matWorldView, &matProjection);

		// 쉐이더 전역 변수들을 설정
		gpToonShader->SetMatrix("gWorldViewProjectionMatrix", &matWorldViewProjection);
		gpToonShader->SetMatrix("gInvWorldMatrix", &matInvWorld);

		gpToonShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpToonShader->SetVector("gSurfaceColor", &gSurfaceColor);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpToonShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpToonShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpTeapot->DrawSubset(0);
			}
			gpToonShader->EndPass();
		}
		gpToonShader->End();
	}
		break;
	case CH07:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 월드/뷰/투영행렬을 미리 곱한다.
		D3DXMATRIXA16 matWorldView;
		D3DXMATRIXA16 matWorldViewProjection;
		D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
		D3DXMatrixMultiply(&matWorldViewProjection, &matWorldView, &matProjection);

		// 쉐이더 전역 변수들을 설정
		gpNormalMappingShader->SetMatrix("gWorldMatrix", &matWorld);
		gpNormalMappingShader->SetMatrix("gWorldViewProjectionMatrix", &matWorldViewProjection);

		gpNormalMappingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpNormalMappingShader->SetVector("gLightColor", &gLightColor);
		gpNormalMappingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);

		gpNormalMappingShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
		gpNormalMappingShader->SetTexture("SpecularMap_Tex", gpStoneSM);
		gpNormalMappingShader->SetTexture("NormalMap_Tex", gpStoneNM);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpNormalMappingShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpNormalMappingShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpSphereTangent->DrawSubset(0);
			}
			gpNormalMappingShader->EndPass();
		}
		gpNormalMappingShader->End();
	}
		break;
	case CH08:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 월드/뷰/투영행렬을 미리 곱한다.
		D3DXMATRIXA16 matWorldView;
		D3DXMATRIXA16 matWorldViewProjection;
		D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
		D3DXMatrixMultiply(&matWorldViewProjection, &matWorldView, &matProjection);

		// 쉐이더 전역 변수들을 설정
		gpEnvironmentMappingShader->SetMatrix("gWorldMatrix", &matWorld);
		gpEnvironmentMappingShader->SetMatrix("gWorldViewProjectionMatrix", &matWorldViewProjection);

		gpEnvironmentMappingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpEnvironmentMappingShader->SetVector("gLightColor", &gLightColor);
		gpEnvironmentMappingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);

		gpEnvironmentMappingShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
		gpEnvironmentMappingShader->SetTexture("SpecularMap_Tex", gpStoneSM);
		gpEnvironmentMappingShader->SetTexture("NormalMap_Tex", gpStoneNM);
		gpEnvironmentMappingShader->SetTexture("EnvironmentMap_Tex", gpSnowENV);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpEnvironmentMappingShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpEnvironmentMappingShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpTeapot->DrawSubset(0);
			}
			gpEnvironmentMappingShader->EndPass();
		}
		gpEnvironmentMappingShader->End();
	}
		break;
	case CH09:
	{
		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 쉐이더 전역 변수들을 설정
		gpUVAnimationShader->SetMatrix("gWorldMatrix", &matWorld);
		gpUVAnimationShader->SetMatrix("gViewMatrix", &matView);
		gpUVAnimationShader->SetMatrix("gProjectionMatrix", &matProjection);

		gpUVAnimationShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpUVAnimationShader->SetVector("gLightColor", &gLightColor);
		gpUVAnimationShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);

		gpUVAnimationShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
		gpUVAnimationShader->SetTexture("SpecularMap_Tex", gpStoneSM);

		// 시스템 시간을 구함
		ULONGLONG tick = GetTickCount64();
		gpUVAnimationShader->SetFloat("gTime", tick/1000.0f);
		gpUVAnimationShader->SetFloat("gWaveHeight", 3);
		gpUVAnimationShader->SetFloat("gSpeed", 2);
		gpUVAnimationShader->SetFloat("gWaveFrequency", 10);
		gpUVAnimationShader->SetFloat("gUVSpeed", 0.25f);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpUVAnimationShader->Begin(&numPasses, NULL);
		for (UINT i = 0; i < numPasses; ++i)
		{
			gpUVAnimationShader->BeginPass(i);
			{
				// 구체를 그린다.
				gpTorus->DrawSubset(0);
			}
			gpUVAnimationShader->EndPass();
		}
		gpUVAnimationShader->End();
	}
		break;
	case CH10:
	{
		// 광원-뷰행렬을 만든다.
		D3DXMATRIXA16 matLightView;
		{
			D3DXVECTOR3 vEyePt(gWorldLightPosition.x, gWorldLightPosition.y, gWorldLightPosition.z);
			D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
			D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
			D3DXMatrixLookAtLH(&matLightView, &vEyePt, &vLookatPt, &vUpVec);
		}

		// 광원-투영행렬을 만든다.
		D3DXMATRIXA16 matLightProjection;
		{
			D3DXMatrixPerspectiveFovLH(&matLightProjection, D3DX_PI / 4.0f, 1, 1, 3000);
		}

		// 뷰/투영행렬을 만든다.
		D3DXMATRIXA16 matViewProjection;
		{
			// 뷰 행렬을 만든다.
			D3DXMATRIXA16 matView;
			D3DXVECTOR3 vEyePt(gWorldCameraPosition.x, gWorldCameraPosition.y, gWorldCameraPosition.z);
			D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
			D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
			D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

			// 투영 행렬을 만든다.
			D3DXMATRIXA16 matProjection;
			D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

			D3DXMatrixMultiply(&matViewProjection, &matView, &matProjection);
		}

		// 원환체의 월드행렬을 만든다.
		D3DXMATRIXA16 matTorusWorld;
		{
			D3DXMatrixRotationY(&matTorusWorld, gRotationY);
		}

		// 디스크의 월드 행렬을 만든다.
		D3DXMATRIXA16 matDiscWorld;
		{
			D3DXMATRIXA16 matScale;
			D3DXMatrixScaling(&matScale, 2.f, 2.f, 2.f);
			D3DXMATRIXA16 matTrans;
			D3DXMatrixTranslation(&matTrans, 0, -40, 0);

			D3DXMatrixMultiply(&matDiscWorld, &matScale, &matTrans);
		}

		// 현재 하드웨어 백버퍼와 깊이버퍼
		LPDIRECT3DSURFACE9 pHWBackBuffer = NULL;
		LPDIRECT3DSURFACE9 pHWDepthStencilBuffer = NULL;
		gpD3DDevice->GetRenderTarget(0, &pHWBackBuffer);
		gpD3DDevice->GetDepthStencilSurface(&pHWDepthStencilBuffer);


		/// 그림자 만들기

		// 그림자맵의 렌더타겟과 깊이 버퍼를 사용한다.
		LPDIRECT3DSURFACE9 pShadowSurface = NULL;
		if (SUCCEEDED(gpShadowRenderTarget->GetSurfaceLevel(0, &pShadowSurface)))
		{
			gpD3DDevice->SetRenderTarget(0, pShadowSurface);
			SAFE_RELEASE(pShadowSurface);
		}
		gpD3DDevice->SetDepthStencilSurface(gpShadowDepthStencil);

		// 저번 프레임에 그렸던 그림자 정보를 지움
		gpD3DDevice->Clear(0, NULL, (D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER), 0xffffffff, 1.0f, 0);

		// 그림자 만들기 쉐이더 전역변수 설정
		gpCreateShadowShader->SetMatrix("gWorldMatrix", &matTorusWorld);
		gpCreateShadowShader->SetMatrix("gLightViewMatrix", &matLightView);
		gpCreateShadowShader->SetMatrix("gLightProjectionMatrix", &matLightProjection);

		// 그림자 만들기 시작
		{
			UINT numPasses = 0;
			gpCreateShadowShader->Begin(&numPasses, NULL);
			{
				for (UINT i = 0; i < numPasses; ++i)
				{
					gpCreateShadowShader->BeginPass(i);
					{
						// 원환체를 그린다
						gpTorus->DrawSubset(0);
					}
					gpCreateShadowShader->EndPass();
				}
			}
			gpCreateShadowShader->End();
		}

		/// 그림자 입히기

		// 하드웨어 백버퍼/ 깊이 버퍼 사용
		gpD3DDevice->SetRenderTarget(0, pHWBackBuffer);
		gpD3DDevice->SetDepthStencilSurface(pHWDepthStencilBuffer);

		SAFE_RELEASE(pHWBackBuffer);
		SAFE_RELEASE(pHWDepthStencilBuffer);

		// 그림자 입히기 쉐이더 전역변수들을 설정
		gpApplyShadowShader->SetMatrix("gWorldMatrix", &matTorusWorld); // 원환체
		gpApplyShadowShader->SetMatrix("gViewProjectionMatrix", &matViewProjection);

		gpApplyShadowShader->SetMatrix("gLightViewMatrix", &matLightView);
		gpApplyShadowShader->SetMatrix("gLightProjectionMatrix", &matLightProjection);

		gpApplyShadowShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpApplyShadowShader->SetVector("gObjectColor", &gTorusColor);

		gpApplyShadowShader->SetTexture("ShadowMap_Tex", gpShadowRenderTarget);

		// 쉐이더를 시작
		{
			UINT numPasses = 0;
			gpApplyShadowShader->Begin(&numPasses, NULL);
			{
				for (UINT i = 0; i < numPasses; ++i)
				{
					gpApplyShadowShader->BeginPass(i);
					{
						// 원환체를 그린다
						gpTorus->DrawSubset(0);

						gpApplyShadowShader->SetMatrix("gWorldMatrix", &matDiscWorld);
						gpApplyShadowShader->SetVector("gObjectColor", &gDiscColor);

						gpApplyShadowShader->CommitChanges();

						gpDisc->DrawSubset(0);
					}
					gpApplyShadowShader->EndPass();
				}
			}
			gpApplyShadowShader->End();
		}
	}
		break;
	case CH11:
	{
		// 1. 장면을 렌더타겟 안에 그린다.
		//////////////////////////////////////////
		// 현재 하드웨어 백버퍼
		LPDIRECT3DSURFACE9 pHWBackBuffer = NULL;
		gpD3DDevice->GetRenderTarget(0, &pHWBackBuffer);

		// 렌더타셋 위에 그린다.
		LPDIRECT3DSURFACE9 pSceneSurface = NULL;
		if (SUCCEEDED(gpSceneRenderTarget->GetSurfaceLevel(0, &pSceneSurface)))
		{
			gpD3DDevice->SetRenderTarget(0, pSceneSurface);
			SAFE_RELEASE(pSceneSurface);
		}

		// 저번 프레임에 그렸던 장면을 지운다.
		gpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);

		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 월드/뷰/투영행렬을 미리 곱한다.
		D3DXMATRIXA16 matWorldView;
		D3DXMATRIXA16 matWorldViewProjection;
		D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
		D3DXMatrixMultiply(&matWorldViewProjection, &matWorldView, &matProjection);

		// 쉐이더 전역 변수들을 설정
		gpEnvironmentMappingShader->SetMatrix("gWorldMatrix", &matWorld);
		gpEnvironmentMappingShader->SetMatrix("gWorldViewProjectionMatrix", &matWorldViewProjection);

		gpEnvironmentMappingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpEnvironmentMappingShader->SetVector("gLightColor", &gLightColor);
		gpEnvironmentMappingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);

		gpEnvironmentMappingShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
		gpEnvironmentMappingShader->SetTexture("SpecularMap_Tex", gpStoneSM);
		gpEnvironmentMappingShader->SetTexture("NormalMap_Tex", gpStoneNM);
		gpEnvironmentMappingShader->SetTexture("EnvironmentMap_Tex", gpSnowENV);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpEnvironmentMappingShader->Begin(&numPasses, NULL);
		{
			for (UINT i = 0; i < numPasses; ++i)
			{
				gpEnvironmentMappingShader->BeginPass(i);
				{
					// 구체를 그린다.
					gpTeapot->DrawSubset(0);
				}
				gpEnvironmentMappingShader->EndPass();
			}
		}
		gpEnvironmentMappingShader->End();

		// 2. 포스트프로세싱을 적용한다.
		/////////////////////////////////////////
		// 하드웨어 백버퍼를 사용한다.
		gpD3DDevice->SetRenderTarget(0, pHWBackBuffer);
		SAFE_RELEASE(pHWBackBuffer);

		// 사용할 포스트프로세스 효과
		LPD3DXEFFECT effectToUse = gpNoEffectShader;
		switch (gPostProcessIndex)
		{
		case 0:
			effectToUse = gpNoEffectShader;
			break;
		case 1:
			effectToUse = gpGrayscaleShader;
			break;
		case 2:
			effectToUse = gpSepiaShader;
			break;
		default:
			break;
		}


		effectToUse->SetTexture("SceneTexture_Tex", gpSceneRenderTarget);
		effectToUse->Begin(&numPasses, NULL);
		{
			for (UINT i = 0; i < numPasses; ++i)
			{
				effectToUse->BeginPass(i);
				{
					// 화면 가득 사각형을 그린다.
					gpD3DDevice->SetStreamSource(0, gpFullscreenQuadVB, 0, sizeof(float) * 5);
					gpD3DDevice->SetIndices(gpFullscreenQuadIB);
					gpD3DDevice->SetVertexDeclaration(gpFullscreenQuadDecl);

					gpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2);
				}
				effectToUse->EndPass();
			}
		}
		effectToUse->End();
	}
		break;
	case CH12:
	{
		// 1. 장면을 렌더타겟 안에 그린다.
		//////////////////////////////////////////
		// 현재 하드웨어 백버퍼
		LPDIRECT3DSURFACE9 pHWBackBuffer = NULL;
		gpD3DDevice->GetRenderTarget(0, &pHWBackBuffer);

		// 렌더타셋 위에 그린다.
		LPDIRECT3DSURFACE9 pSceneSurface = NULL;
		if (SUCCEEDED(gpSceneRenderTarget->GetSurfaceLevel(0, &pSceneSurface)))
		{
			gpD3DDevice->SetRenderTarget(0, pSceneSurface);
			SAFE_RELEASE(pSceneSurface);
		}

		// 저번 프레임에 그렸던 장면을 지운다.
		gpD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0);

		// 뷰 행렬을 만든다.
		D3DXMATRIXA16 rotMat;
		D3DXMatrixRotationY(&rotMat, gRotationY);

		D3DXVECTOR4 CameraPosition;
		D3DXVec4Transform(&CameraPosition, &gWorldCameraPosition, &rotMat);

		D3DXMATRIXA16 matView;
		D3DXVECTOR3 vEyePt(CameraPosition.x, CameraPosition.y, CameraPosition.z);
		D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);

		// 투영 행렬을 만든다.
		D3DXMATRIXA16 matProjection;
		D3DXMatrixPerspectiveFovLH(&matProjection, FOV, ASPECT_RATIO, NEAR_PLANE, FAR_PLANE);

		// 월드 행렬을 만든다.
		D3DXMATRIXA16 matWorld;
		D3DXMatrixIdentity(&matWorld);

		// 월드/뷰/투영행렬을 미리 곱한다.
		D3DXMATRIXA16 matWorldView;
		D3DXMATRIXA16 matWorldViewProjection;
		D3DXMatrixMultiply(&matWorldView, &matWorld, &matView);
		D3DXMatrixMultiply(&matWorldViewProjection, &matWorldView, &matProjection);

		// 쉐이더 전역 변수들을 설정
		gpEnvironmentMappingShader->SetMatrix("gWorldMatrix", &matWorld);
		gpEnvironmentMappingShader->SetMatrix("gWorldViewProjectionMatrix", &matWorldViewProjection);

		gpEnvironmentMappingShader->SetVector("gWorldLightPosition", &gWorldLightPosition);
		gpEnvironmentMappingShader->SetVector("gLightColor", &gLightColor);
		gpEnvironmentMappingShader->SetVector("gWorldCameraPosition", &gWorldCameraPosition);

		gpEnvironmentMappingShader->SetTexture("DiffuseMap_Tex", gpStoneDM);
		gpEnvironmentMappingShader->SetTexture("SpecularMap_Tex", gpStoneSM);
		gpEnvironmentMappingShader->SetTexture("NormalMap_Tex", gpStoneNM);
		gpEnvironmentMappingShader->SetTexture("EnvironmentMap_Tex", gpSnowENV);

		// 쉐이더를 시작한다.
		UINT numPasses = 0;
		gpEnvironmentMappingShader->Begin(&numPasses, NULL);
		{
			for (UINT i = 0; i < numPasses; ++i)
			{
				gpEnvironmentMappingShader->BeginPass(i);
				{
					// 구체를 그린다.
					gpTeapot->DrawSubset(0);
				}
				gpEnvironmentMappingShader->EndPass();
			}
		}
		gpEnvironmentMappingShader->End();

		// 2. 포스트프로세싱을 적용한다.
		/////////////////////////////////////////
		// 하드웨어 백버퍼를 사용한다.
		gpD3DDevice->SetRenderTarget(0, pHWBackBuffer);
		SAFE_RELEASE(pHWBackBuffer);

		// 사용할 포스트프로세스 효과
		LPD3DXEFFECT effectToUse = gpEdgeDetectionShader;
		switch (gPostProcessIndex)
		{
		case 0:
			effectToUse = gpEdgeDetectionShader;
			break;
		case 1:
			effectToUse = gpEmbossShader;
			break;
		default:
			break;
		}

		D3DXVECTOR4 pixelOffset(1 / (float)WIN_WIDTH, 1 / (float)WIN_HEIGHT, 0, 0);
		effectToUse->SetVector("gPixelOffset", &pixelOffset);

		effectToUse->SetTexture("SceneTexture_Tex", gpSceneRenderTarget);
		effectToUse->Begin(&numPasses, NULL);
		{
			for (UINT i = 0; i < numPasses; ++i)
			{
				effectToUse->BeginPass(i);
				{
					// 화면 가득 사각형을 그린다.
					gpD3DDevice->SetStreamSource(0, gpFullscreenQuadVB, 0, sizeof(float) * 5);
					gpD3DDevice->SetIndices(gpFullscreenQuadIB);
					gpD3DDevice->SetVertexDeclaration(gpFullscreenQuadDecl);

					gpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 6, 0, 2);
				}
				effectToUse->EndPass();
			}
		}
		effectToUse->End();
	}
		break;
	default:
		break;
	}

	
}

// 디버그 정보를 출력
void RenderInfo()
{
	// 텍스트 색상
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 255);

	// 텍스트를 출력할 위치
	RECT rct;
	rct.left = 5;
	rct.right = WIN_WIDTH / 3;
	rct.top = 5;
	rct.bottom = WIN_HEIGHT / 3;

	// 키 입력 정보를 출력
	gpFont->DrawText(NULL, "데모 프레임워크\n\nESC: 데모종료", -1, &rct, 0, fontColor);
}


// 뒷정리 코드
void Cleanup()
{
	// 폰트를 Release 한다.
	/*if (gpFont)
	{
		gpFont->Release();
		gpFont = NULL;
	}*/
	SAFE_RELEASE(gpFont);

	// 모델을 Release
	SAFE_RELEASE(gpSphere);
	SAFE_RELEASE(gpTeapot);
	SAFE_RELEASE(gpSphereTangent);
	SAFE_RELEASE(gpTorus);
	SAFE_RELEASE(gpDisc);

	// 세이더를 Release
	SAFE_RELEASE(gpColorShader);
	SAFE_RELEASE(gpTextureMappingShader);
	SAFE_RELEASE(gpLightingShader);
	SAFE_RELEASE(gpSpecularMappingShader);
	SAFE_RELEASE(gpToonShader);
	SAFE_RELEASE(gpNormalMappingShader);
	SAFE_RELEASE(gpUVAnimationShader);
	SAFE_RELEASE(gpCreateShadowShader);
	SAFE_RELEASE(gpApplyShadowShader);
	SAFE_RELEASE(gpNoEffectShader);
	SAFE_RELEASE(gpGrayscaleShader);
	SAFE_RELEASE(gpSepiaShader);

	// 텍스쳐를 Release
	SAFE_RELEASE(gpEarthDM);
	SAFE_RELEASE(gpStoneDM);
	SAFE_RELEASE(gpStoneSM);
	SAFE_RELEASE(gpStoneNM);
	SAFE_RELEASE(gpShadowRenderTarget);
	SAFE_RELEASE(gpShadowDepthStencil);
	SAFE_RELEASE(gpSceneRenderTarget);

	// 화면크기 사각형을 해제
	SAFE_RELEASE(gpFullscreenQuadDecl);
	SAFE_RELEASE(gpFullscreenQuadVB);
	SAFE_RELEASE(gpFullscreenQuadIB);

	// D3D를 Release
	SAFE_RELEASE(gpD3DDevice);
	SAFE_RELEASE(gpD3D);
}
