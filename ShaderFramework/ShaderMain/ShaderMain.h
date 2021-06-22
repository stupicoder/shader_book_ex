#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#define WIN_WIDTH	800
#define WIN_HEIGHT	600

#define PI 3.14159265f
#define FOV (PI/4.0f)
#define ASPECT_RATIO (WIN_WIDTH/(float)WIN_HEIGHT)
#define NEAR_PLANE 1
#define FAR_PLANE 10000

// �޼��� ó����
LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ProcessInput(HWND hWnd, WPARAM keyPress);

// �ʱ�ȭ ����
bool InitEverything(HWND hWnd);
bool InitD3D(HWND hWnd);
void InitFullScreenQuad();
bool LoadAssets();
LPD3DXEFFECT LoadShader(const char* filename);
LPDIRECT3DTEXTURE9 LoadTexture(const char* filename);
LPDIRECT3DCUBETEXTURE9 LoadCubeTexture(const char* filename);
LPD3DXMESH LoadModel(const char* filename);

// ���� ���� ����
void PlayDemo();
void Update();

// ������ ����
void RenderFrame();
void RenderScene();
void RenderInfo();

// ������ ����
void Cleanup();

