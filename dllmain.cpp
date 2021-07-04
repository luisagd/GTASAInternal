#include "stdafx.h"
#include "mem.h"
#include "Classes.h"
#include "main.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx9.h"
#include "imgui/backends/imgui_impl_win32.h"

import calc;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define USE_RADIANS//constexpr bool USE_RADIANS = true;


#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

void CPed::GetBonePosition(RwV3d& outPosition, unsigned int boneId, bool updateSkinBones)
{
	((void(__thiscall*)(CPed*, RwV3d&, unsigned int, bool))0x5E4280)(this, outPosition, boneId, updateSkinBones);
}

vec3 GetBonePosition( CPed* ped,unsigned int boneId, bool updateSkinBones)
{
	RwV3d outPosition;
	if (IsValidCPed(ped)) {
		((void(__thiscall*)(CPed*, RwV3d&, unsigned int, bool))0x5E4280)(ped, outPosition, boneId, updateSkinBones);
		return vec3(outPosition.x, outPosition.y, outPosition.z);
	}
	else
		return vec3(0,0,0);
}

class Hack {
public:
	void CleanUp() {
		if (g_font) {
			g_font->Release();
			g_font = nullptr;
		}
		if (LineL) {
			LineL->Release();
			LineL = nullptr;
		}
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		mem::Patch(d3d9Device[42], EndSceneBytes, 7);
		mem::Patch(d3d9Device[41], BeginSceneBytes, 7);
		(WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)oWndProc);
	}
	void Init() {
		//calling it with NULL also gives you the address of the .exe module
		moduleBase = (uintptr_t)GetModuleHandle(NULL);


		CPedPoolInfo = reinterpret_cast<struct CPedPoolInfo*>(*(struct CPedPoolInfo**)(moduleBase + 0x774490));

		d3d = reinterpret_cast<IDirect3D9*>(*(IDirect3D9**)0xC97C20);
		d3ddev = reinterpret_cast<IDirect3DDevice9*>(*(IDirect3DDevice9**)0xC97C28);

		D3DDEVICE_CREATION_PARAMETERS CP;
		d3ddev->GetCreationParameters(&CP);
		window = CP.hFocusWindow;


		auto hr = D3DXCreateFont(d3ddev,     //D3D Device
			10,               //Font height
			0,                //Font width
			FW_NORMAL,        //Font Weight
			1,                //MipLevels
			false,            //Italic
			DEFAULT_CHARSET,  //CharSet
			OUT_DEFAULT_PRECIS, //OutputPrecision
			ANTIALIASED_QUALITY, //Quality
			DEFAULT_PITCH | FF_DONTCARE,//PitchAndFamily
			L"Arial",          //pFacename,
			&g_font);         //ppFont

		LocalPlayerPtr = *(CPed**)(moduleBase + 0x76F5F0);
		//ViewMatrix

		pPitch = (float*)(moduleBase + 0x76F248); //Is in radians!
		pYaw = (float*)(moduleBase + 0x76F258); //Is in radians!

		pX = &LocalPlayerPtr->pViewMatrix->w1;
		pY = &LocalPlayerPtr->pViewMatrix->w2;
		pZ = &LocalPlayerPtr->pViewMatrix->w3;

		pCameraX = (float*)(moduleBase + 0x76F338);
		pCameraY = (float*)(moduleBase + 0x76F33C);
		pCameraZ = (float*)(moduleBase + 0x76F340);

		//ViewMatrix

		pMoney = (uint32_t*)(moduleBase + 0x77CE50);

		pWantedLevelPoints = *(uint32_t**)0xB7CD9C;
		pWantedLevel = ptr_add<uint32_t>(pWantedLevelPoints, 0x2C);

		pTarget = (CPed**)mem::FindDMAAddy((moduleBase + 0x76F3B8), { 0x79C });
		pRecoil = (float*)0xB7CDC8;

		pWidth = (uint32_t*)(moduleBase + 0x817044);
		pHeight = (uint32_t*)(moduleBase + 0x817048);

		BestTarget = GetBestTarget(CPedPoolInfo, vec3(*pX, *pY, *pZ), vec3(*pYaw, *pPitch, 0.0f));

		IsMenuActive = (bool*)0xBA67A4;

		CPedPool = GetCPed();

		LMB = (uint8_t*)0xB73404;
		RMB = (uint8_t*)0xB73405;

		CPedDensity = (float*)0x8D2530;


		if (GetD3D9Device(d3d9Device, sizeof(d3d9Device))) {
			memcpy(EndSceneBytes, (char*)d3d9Device[42], 7); //hook stuff using the dumped addresses
			memcpy(BeginSceneBytes, (char*)d3d9Device[41], 7); //hook stuff using the dumped addresses
		}
		else throw "Failed hooking d3d9";

		oEndScene = (Hack::fpEndScene)mem::TrampHook32((char*)d3d9Device[42], (char*)EndSceneHook, 7);
		oBeginScene = (Hack::fpEndScene)mem::TrampHook32((char*)d3d9Device[41], (char*)BeginSceneHook, 7);
		oWndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)WndProcHook);


		bAim = false;
		bEsp = true;
		bAmmo = true;
		bWanted = true;
	}
	void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t thickness, D3DCOLOR color) {
		if (!LineL) D3DXCreateLine(d3ddev, &LineL);

		D3DXVECTOR2 Line[2];
		Line[0] = D3DXVECTOR2(x1, y1);
		Line[1] = D3DXVECTOR2(x2, y2);
		LineL->SetWidth(thickness);
		LineL->Draw(Line, 2, color);
	}
	void DrawLine(const vec2& src, const vec2& dst, uint8_t thickness, D3DCOLOR color) {
		DrawLine(src.x, src.y, dst.x, dst.y, thickness, color);
	}
	void DrawEspBox2D(const vec2& top, const vec2& bot, int8_t thickness, D3DCOLOR color) {
		auto height = abs(top.y-bot.y);
		vec2 tl, tr;
		tl.x = abs(top.x - height / 4);
		//tl.x = top.x + height / 4;

		tl.y = top.y;
		tr.x = top.x + height / 4;
		tr.y = top.y;
		vec2 bl, br;
		bl.x = abs(bot.x - height / 4);
		bl.y = bot.y;
		br.x = bot.x + height / 4;
		br.y = bot.y;

		DrawLine(tl, tr, thickness, color);
		DrawLine(bl, br, thickness, color);
		DrawLine(tl, bl, thickness, color);
		DrawLine(tr, br, thickness, color);

	}
	void DrawEspBox2D(const D3DXVECTOR3& top, const D3DXVECTOR3& bot, int8_t thickness, D3DCOLOR color) {
		DrawEspBox2D(vec2(top.x, top.y), vec2(bot.x, bot.y), thickness, color);
	}
	std::vector<CPed*> GetCPed() {
		std::vector<CPed*> peds;
		for (uint8_t i = 2; i < 140; i++) {
			auto ped = ptr_add(CPedPoolInfo->FirstCPed, 0x7c4 * i);
			auto PedInUse = (*CPedPoolInfo->CPedsInUse)[i];
			if (!(PedInUse > 0x00 && 0x80 > PedInUse))
				continue;
			if (IsValidCPed(ped))
				peds.push_back(ped);
		}

		return peds;
	}

	LPDIRECT3DDEVICE9 d3ddev;
	HWND window;
	CPed* LocalPlayerPtr;
	struct CPedPoolInfo* CPedPoolInfo ;
	bool* IsMenuActive;
	uint32_t* pWidth;
	uint32_t* pHeight;
	uintptr_t moduleBase;

	void* d3d9Device[119];
	using fpEndScene = HRESULT(_stdcall*)(IDirect3DDevice9* pDevice);
	fpEndScene oEndScene;
	fpEndScene oBeginScene;
	typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
	WNDPROC oWndProc;

	ID3DXFont* g_font;
	IDirect3D9* d3d;
	void* EndSceneOld;
	uint8_t* EndSceneBytes[7];
	uint8_t* BeginSceneBytes[7];


	CPed* BestTarget;
	float* pPitch;
	float* pX ;
	float* pY ;
	float* pZ ;
	float* pYaw ;
	float* pCameraX ;
	float* pCameraY ;
	float* pCameraZ ;

	ID3DXLine* LineL;

	uint32_t* pMoney;
	uint32_t* pWantedLevel;
	uint32_t* pWantedLevelPoints;
	CPed** pTarget;
	float* pRecoil;

	uint8_t* LMB;
	uint8_t* RMB;

	float* CPedDensity;

	std::vector<CPed*>CPedPool; 

	std::atomic<bool> end;
	std::atomic<bool> imgui_init;
	std::atomic<bool> show_menu;
	std::atomic<bool> bAim;
	std::atomic<bool> bEsp;
	std::atomic<bool> bAmmo;
	std::atomic<bool> bWanted;



};


Hack* hack;


inline vec3 RadToDeg(const vec3& radians)
{
	vec3 degrees;
	degrees.x = radians.x * (180 / PI);
	degrees.y = radians.y * (180 / PI);
	degrees.z = radians.z * (180 / PI);
	return degrees;
}

inline vec3 DegToRad(const vec3& degrees)
{
	vec3 radians;
	radians.x = degrees.x * (PI / 180);
	radians.y = degrees.y * (PI / 180);
	radians.z = degrees.z * (PI / 180);
	return radians;
}


inline float Magnitude(vec3 vec)
{
	return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

inline float Distance(const vec3& src, const vec3& dst)
{
	vec3 diff = src - dst;
	return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z); //Magnitude
}


void LeftClick()
{
	INPUT    Input = { 0 };
	// left down 
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &Input, sizeof(INPUT));

	// left up
	ZeroMemory(&Input, sizeof(INPUT));
	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &Input, sizeof(INPUT));
}

template <>
struct fmt::formatter<vec3> {
	// Presentation format: 'f' - fixed, 'e' - exponential.
	char presentation = 'f';

	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx) {
		auto it = ctx.begin(), end = ctx.end();
		if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

		// Check if reached the end of the range:
		if (it != end && *it != '}')
			throw fmt::format_error("invalid format, at vec3");

		// Return an iterator past the end of the parsed range:
		return it;
	}

	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(const vec3& p, FormatContext& ctx) {
		// auto format(const point &p, FormatContext &ctx) -> decltype(ctx.out()) // c++11
		  // ctx.out() is an output iterator to write to.
		return format_to(
			ctx.out(),
			presentation == 'f' ? "({:.1f}, {:.1f}, {:.1f})" : "({:.1e}, {:.1e})",
			p.x, p.y, p.z);
	}
};


bool WorldToScreen(LPDIRECT3DDEVICE9 pDevice, D3DXVECTOR3* pos, D3DXVECTOR3* out) {
	D3DVIEWPORT9 viewPort;
	D3DXMATRIX view, projection, world;

	pDevice->GetViewport(&viewPort);
	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &projection);
	D3DXMatrixIdentity(&world);

	D3DXVec3Project(out, pos, &viewPort, &projection, &view, &world);
	if (out->z < 1.0) {
		return true;
	}
	return false;
}

#define YAW 1  //x
#define PITCH 0 //y
#define ROLL 2 //z
constexpr float PITCH_MAX_DOWN = -90;
constexpr float PITCH_MAX_UP = 90;
constexpr float YAW_MAX_LEFT = 180;
constexpr float YAW_MAX_RIGHT = -180;

#ifdef USE_RADIANS
#define ClampAngle ClampAngleR
#define CalcAngle CalcAngleR

inline vec3 ClampAngleR(const vec3& angle) {

	vec3 ClampedAngle;
	if (angle.v[PITCH] > (PI/2)) ClampedAngle.v[PITCH] = PI/2;
	if (angle.v[PITCH] < -(PI/2))
		ClampedAngle.v[PITCH] = PI/2 + fmodf(angle.v[PITCH], PI/2);

	if (angle.v[YAW] > PI) ClampedAngle.v[YAW] = -PI + (angle.v[YAW] - PI);
	if (angle.v[YAW] < -PI) ClampedAngle.v[YAW] = PI + (angle.v[YAW] + PI);

	ClampedAngle.v[ROLL] = 0.0f;
	return ClampedAngle;
}
inline vec3 CalcAngleR(const vec3& src, const vec3& dst)
{

	const vec3 delta = dst - src;

	const float opposite = delta.y;
	const float adjacent = delta.x;

	vec3 angles;
	angles.v[YAW] = atan2(-opposite, -adjacent);
	angles.v[YAW] += 0.0389;

	const float adj = delta.x;
	const float opp = delta.z;

	const float hyp = sqrtf((delta.z * delta.z) + (delta.x * delta.x) + (delta.y * delta.y));

	angles.v[PITCH] = asinf(opp / hyp);
	angles.v[PITCH] -= 0.079; //Depends of Resolution/Windows size !?


	angles.v[ROLL] = 0.0f;

	//return ClampAngle(angles);
	return angles;
}
#else
#define ClampAngle ClampAngleD
#define CalcAngle CalcAngleD

vec3 ClampAngleD(const vec3& angle) {

	vec3 ClampedAngle;
	if (angle.v[PITCH] > 89.0f) ClampedAngle.v[PITCH] = 89.f;
	if (angle.v[PITCH] < -89.0f) ClampedAngle.v[PITCH] = -89.f;

	if (angle.v[YAW] > 180.f) ClampedAngle.v[YAW] = 180.f;
	if (angle.v[YAW] < -180.f) ClampedAngle.v[YAW] = -180.f;

	ClampedAngle.v[ROLL] = 0.0f;
	return ClampedAngle;
}

vec3 CalcAngleD(vec3 src, vec3 dst)
{

	vec3 delta = dst - src;

	//float hypotenuse = sqrtf((delta.x*delta.x) +(delta.y*delta.y));
	float opposite = delta.y;
	float adjacent = delta.x;


	//std::cout << "hyp: "<<hyp<<" hypotenuse: "<<hypotenuse<<"\n";

	vec3 angles;
	//angles.v[YAW] = RadToDeg(asinf(opposite / hypotenuse));
	angles.v[YAW] = RadToDeg(atan2(-opposite, -adjacent));
	angles.v[YAW] += RadToDeg(0.0389);
	//std::cout << "opp(y):"<<opposite<<" adj(x): "<<adjacent << "atan: "<< angles.v[YAW]<<"\n";

	float adj = delta.x;
	float opp = delta.z;
	//angles.v[PITCH] = RadToDeg(atan2f(opp, adj));
	float hyp = sqrtf((delta.x * delta.x) + (delta.z * delta.z));
	//angles.v[PITCH] = RadToDeg(asin(opp/hyp));
	angles.v[PITCH] = 0.0f;
	angles.v[ROLL] = 0.0f;

	//angles = ClampAngle(angles);

	return angles;
}
#endif


/*
Remember:
	sin() = opp/hyp
	cos() = adj/hyp
	tan() = opp/adj
These are ratios, if we have an angle and we have a side, we can calculate the other sides.

But, if we have sides, and we want angles, we use the inverse functions:
	arcsin()
	arccos()
	arctan()
*/
vec3 CalcAngle2(CPed* target)
{
	vec3 angles;

	const float* xPos1 = &hack->LocalPlayerPtr->pViewMatrix->w1;//player actor X position
	const float* yPos1 = &hack->LocalPlayerPtr->pViewMatrix->w2;//player actor Y position

	float xPoint = *hack->pCameraX;
	float yPoint = *hack->pCameraY;

	if (target) {
		const float* xPos2 = &(target->pViewMatrix->w1);//X position 
		const float* yPos2 = &(target->pViewMatrix->w2);//Y position

		float xPos2a;
		float yPos2a;

		if (target->Health > 0) {

			xPos2a = *xPos2 + target->SpeedVectorX * 6;//position not far from target player sceen
			yPos2a = *yPos2 + target->SpeedVectorY * 6;

			float Aa = fabs(xPoint - xPos2a);
			float Ab = fabs(yPoint - yPos2a);
			float Ac = sqrt(Aa * Aa + Ab * Ab);
			float alpha = asin(Aa / Ac);
			float beta = acos(Aa / Ac);

			if ((*xPos1 > *xPos2) && (*yPos1 < *yPos2)) { beta = -beta; }//1 part
			if ((*xPos1 > *xPos2) && (*yPos1 > *yPos2)) { beta = beta; }//2 part
			if ((*xPos1 < *xPos2) && (*yPos1 > *yPos2)) { beta = (alpha + (1.5707)); }//3 part
			if ((*xPos1 < *xPos2) && (*yPos1 < *yPos2)) { beta = (-alpha - (1.5707)); }//4 part

			angles.v[YAW] = beta + 0.0389;//for deagle.
		}
	}

	angles.v[PITCH] = 0.0f;

	angles.v[ROLL] = 0.0f;

	return RadToDeg( angles);
}



bool IsLineOfSightClear(const CVector* vecStart, const CVector* vecEnd, bool bCheckBuildings = true, bool bCheckVehicles = false, bool bCheckPeds = false,
	bool bCheckObjects = true, bool bCheckDummies = false, bool bSeeThroughStuff = true, bool bIgnoreSomeObjectsForCamera = true)
{
	DWORD dwFunc = 0x56A490;
	bool  bReturn = false;
	_asm
	{
		push    bIgnoreSomeObjectsForCamera
		push    bSeeThroughStuff
		push    bCheckDummies
		push    bCheckObjects
		push    bCheckPeds
		push    bCheckVehicles
		push    bCheckBuildings
		push    vecEnd
		push    vecStart
		call    dwFunc
		mov     bReturn, al
		add     esp, 0x24
	}
	return bReturn;
}

inline bool IsValidCPed(CPed* ped) noexcept{
	return (ped && ped->pViewMatrix && ped->pViewMatrix->w1 != 0.0f && ped->Health > 0);
}

inline bool IsGameReady() {
	if (!(*(bool*)0xBA67A4) && (*(CPed**)(0xB6F5F0)) &&(*(CPed**)(0xB6F5F0))->Health != 0)
		return true;
	else return false;
}

CPed* GetBestTarget(struct CPedPoolInfo* CPedPoolInfo, const vec3& src, const vec3& angles) {
	float oldCoefficient = FLT_MAX;
	float newCoefficient = 0;
	CPed* BestTarget = CPedPoolInfo->FirstCPed;

	uint32_t found = 0;

	for (auto ped : hack->GetCPed()) {
		//CVector csrc(src.x, src.y, src.z);
		//CVector cdst(ped->pViewMatrix->w1, ped->pViewMatrix->w2, ped->pViewMatrix->w3);
		if (IsValidCPed(ped))// && IsLineOfSightClear(&csrc, &cdst)
		{
			
			found++;
			vec3 dst(ped->pViewMatrix->w1, ped->pViewMatrix->w2, ped->pViewMatrix->w3);
			vec3 angleTo = CalcAngle(src, dst);
			newCoefficient = Distance(angles, angleTo)* 0.2 + Distance(src, dst) * 0.8;
			if (newCoefficient < oldCoefficient)
			{
				oldCoefficient = newCoefficient;
				BestTarget = ped;
			}
		}
	}
	//fmt::print("Current CPeds: {}, Found: {}\n", CPedPoolInfo->CurrentCPedNum, found);

	return BestTarget;
}

void InitImGui(IDirect3DDevice9* pDevice) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();

	io.Fonts->AddFontDefault();

	ImGui_ImplWin32_Init(hack->window);
	ImGui_ImplDX9_Init(pDevice);
	hack->imgui_init = true;
	return;
}

HRESULT _stdcall EndSceneHook(IDirect3DDevice9 *pDevice) { //7 bytes befor calling
	if (hack->end)return hack->oEndScene(pDevice);

	//continuous writes / freeze
	*hack->CPedDensity = 100;
	*hack->pRecoil = 0.0f;
	hack->LocalPlayerPtr->Health = 10000;

	if (!hack->imgui_init) InitImGui(pDevice);
	else if(hack->show_menu){

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();



		ImGui::Text("Hello, world!");
		if (ImGui::Button("Enable/Disable Aimbot (Unstable)")) {
			hack->bAim = !hack->bAim;
		}
		if (ImGui::Button("Enable/Disable ESP")) {
			hack->bEsp = !hack->bEsp;
		}
		if (ImGui::Button("Enable/Disable Infinite Ammo")) {
			hack->bAmmo = !hack->bAmmo;
		}
		if (ImGui::Button("Enable/Disable Never Wanted")) {
			hack->bWanted = !hack->bWanted;
		}
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	ImGui::GetIO().MouseDrawCursor = hack->show_menu;

	if (!IsGameReady())			return hack->oEndScene(pDevice);

	if(hack->bWanted){
		*hack->pWantedLevel = 0;
		*hack->pWantedLevelPoints = 0;
	}
	if (hack->bAmmo) {
		//Handguns
		if (hack->LocalPlayerPtr->WeaponSlot[2].Type != 0) {
			hack->LocalPlayerPtr->WeaponSlot[2].AmmoRemaining = 999;
			hack->LocalPlayerPtr->WeaponSlot[2].AmmoInClip = 100;
		}
		//Shotguns
		if (hack->LocalPlayerPtr->WeaponSlot[3].Type != 0) {
			hack->LocalPlayerPtr->WeaponSlot[3].AmmoRemaining = 999;
			hack->LocalPlayerPtr->WeaponSlot[3].AmmoInClip = 100;
		}
		//Sub-Machineguns
		if (hack->LocalPlayerPtr->WeaponSlot[4].Type != 0) {
			hack->LocalPlayerPtr->WeaponSlot[4].AmmoRemaining = 999;
			hack->LocalPlayerPtr->WeaponSlot[4].AmmoInClip = 100;
		}
		//Machineguns
		if (hack->LocalPlayerPtr->WeaponSlot[5].Type != 0) {
			hack->LocalPlayerPtr->WeaponSlot[5].AmmoRemaining = 999;
			hack->LocalPlayerPtr->WeaponSlot[5].AmmoInClip = 100;
		}
		//Rifles
		if (hack->LocalPlayerPtr->WeaponSlot[6].Type != 0) {
			hack->LocalPlayerPtr->WeaponSlot[6].AmmoRemaining = 999;
			hack->LocalPlayerPtr->WeaponSlot[6].AmmoInClip = 100;
		}
		//Heavy Weapons
		if (hack->LocalPlayerPtr->WeaponSlot[7].Type != 0) {
			hack->LocalPlayerPtr->WeaponSlot[7].AmmoRemaining = 999;
			hack->LocalPlayerPtr->WeaponSlot[7].AmmoInClip = 100;
		}
		//Projectiles
		if (hack->LocalPlayerPtr->WeaponSlot[8].Type != 0) {
			hack->LocalPlayerPtr->WeaponSlot[8].AmmoRemaining = 20;
			hack->LocalPlayerPtr->WeaponSlot[8].AmmoInClip = 5;
		}
	}
	if (hack->bAim && *hack->RMB == 128 && IsValidCPed(hack->BestTarget))
	{
		vec3 src(*hack->pCameraX, *hack->pCameraY, *hack->pCameraZ);

		vec3 dst = GetBonePosition(hack->BestTarget, BONE_HEAD1, false);

		auto  angle = CalcAngle(src, dst);

		*hack->pPitch = (angle.v[PITCH]);
		*hack->pYaw = (angle.v[YAW]);

		hack->LocalPlayerPtr->Yaw = (angle.v[YAW]);
	}
	if (hack->bEsp) {
		auto fps_str = fmt::format("hello");
		auto fps_string = fps_str.c_str();
		RECT font_rect;

		//A pre-formatted string showing the current frames per second

		SetRect(&font_rect, 0, 0, 100, 100);

		const auto font_height = hack->g_font->DrawTextA(NULL,     //pSprite
			fps_string,  //pString
			-1,          //Count
			&font_rect,  //pRect
			DT_LEFT | DT_NOCLIP,//Format,
			0xFFFFFFFF); //Color
		for (CPed* ped : hack->GetCPed()) {
			if (IsValidCPed(ped))
			{
				auto GetColor = [](float Health, float MaxHealth) -> uint8_t{
					auto color = 255 * (Health / MaxHealth);
					if (color > 255 || color < 0)
						return 0;
					else return abs(255 - color);
				};
				D3DXVECTOR3 EntPos2D, EntHead2D;
				D3DXVECTOR3 EntPos3D(ped->pViewMatrix->w1, ped->pViewMatrix->w2, ped->pViewMatrix->w3);
				D3DXVECTOR3 EntHead3D = GetBonePosition(ped, BONE_HEAD1, false);

				auto color = D3DCOLOR_ARGB(255, GetColor(ped->Health, ped->MaxHealth), abs(255- GetColor(ped->Health, ped->MaxHealth)), 0);
				if (WorldToScreen(hack->d3ddev, &EntPos3D, &EntPos2D)) {
					//hack->DrawLine(EntPos2D.x, EntPos2D.y, (*hack->pWidth) / 2, (*hack->pHeight) / 2, 2, D3DCOLOR_ARGB(255, 255, 0, 0));
					
					WorldToScreen(hack->d3ddev, &EntHead3D, &EntHead2D);
					if(EntHead2D.y>0 && EntPos2D.x > 1 && EntHead2D.x < *hack->pWidth - 5)

					hack->DrawEspBox2D( vec2(EntHead2D.x, EntHead2D.y), vec2(EntPos2D.x, EntPos2D.y), 2, color);

				}
			}
		}
		//D3DXVECTOR3 out;
		//D3DXVECTOR3 pedXYZ(GetBonePosition(hack->BestTarget, BONE_HEAD1, false));
		//if (WorldToScreen(hack->d3ddev, &pedXYZ, &out) || true) {
		//	hack->DrawLine(out.x, out.y, (*hack->pWidth) / 2, 0, 2, D3DCOLOR_ARGB(255, 0, 255, 0));
		//}
	}

	//Update BestTarget only if is invalid or if is the player
	if (!IsValidCPed(hack->BestTarget) || hack->BestTarget == hack->LocalPlayerPtr) {
		vec3 angles(*hack->pYaw, *hack->pPitch, 0.0f);
		vec3 src(*hack->pCameraX, *hack->pCameraY, *hack->pCameraZ);
		hack->BestTarget = GetBestTarget(hack->CPedPoolInfo, src, angles);
	}

	return hack->oEndScene(pDevice);
}

HRESULT _stdcall BeginSceneHook(IDirect3DDevice9* pDevice) {
	if (hack->end)return hack->oBeginScene(pDevice);

	return hack->oBeginScene(pDevice);
};

LRESULT __stdcall WndProcHook(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if(hack->end)
		return CallWindowProc(hack->oWndProc, hWnd, uMsg, wParam, lParam);
	if (hack->show_menu) {
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;
		else return true;
	}
	else
		return CallWindowProc(hack->oWndProc, hWnd, uMsg, wParam, lParam);
}


//Copy & Paste Section

bool GetD3D9Device(void** pTable, size_t Size)
{
	static HWND window;

	auto GetProcessWindow = []()
	{
		auto EnumWindowsCallback = [](HWND handle, LPARAM lParam)
		{
			DWORD wndProcId;
			GetWindowThreadProcessId(handle, &wndProcId);

			if (GetCurrentProcessId() != wndProcId)
				return TRUE; // skip to next window

			window = handle;
			return FALSE; // window found abort search
		};
		window = NULL;
		EnumWindows(EnumWindowsCallback, NULL);
		return window;
	};

	if (!pTable)
		return false;

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!pD3D)
		return false;

	IDirect3DDevice9* pDummyDevice = NULL;

	// options to create dummy device
	D3DPRESENT_PARAMETERS d3dpp = {};
	d3dpp.Windowed = false;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = GetProcessWindow();

	HRESULT dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

	if (dummyDeviceCreated != S_OK)
	{
		// may fail in windowed fullscreen mode, trying again with windowed mode
		d3dpp.Windowed = !d3dpp.Windowed;

		dummyDeviceCreated = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);

		if (dummyDeviceCreated != S_OK)
		{
			pD3D->Release();
			return false;
		}
	}

	memcpy(pTable, *reinterpret_cast<void***>(pDummyDevice), Size);

	pDummyDevice->Release();
	pD3D->Release();
	return true;
}

//END Copy & Paste Section



DWORD WINAPI HackThread(HMODULE hModule){




	//TODO: Add more checks to validate CPeds




	//Create Console
	//AllocConsole();
	//FILE* f;
	//freopen_s(&f, "CONOUT$", "w", stdout);
	while (!IsGameReady()) {
		Sleep(10);
	}
	Sleep(1000);
	hack = new Hack;
	//fmt::print("OG for a fee, stay sippin' fam\n");

	hack->Init();
	*hack->pMoney = 10000000;
	bool bHealth = false, bAim = false, bRecoil = false;
		
	while (1)
	{
		if (GetAsyncKeyState(VK_END) & 1)       break;

		if (GetAsyncKeyState(VK_NUMPAD1) & 1)   hack->show_menu = !hack->show_menu;

		//Sleep(0);
	}

	hack->end = true;
	Sleep(15); // for now, we wait the program to be unhooked
	hack->CleanUp();
	delete hack;
	//fclose(f);
	//FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
	default:
		break;
	}
	return TRUE;
}
