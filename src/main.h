#pragma once

#ifndef __MODMAIN_H
#define __MODMAIN_H

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS 1

// let's do a precompiled header, why not
#pragma message( "Compiling precompiled header.\n" )

// handler not registered as safe handler
#pragma warning( disable : 4733 )

// API/SDK includes
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <shellapi.h>
#include <d3dx9.h>
#include <Gdiplus.h>
#include <assert.h>
#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <CDirect3DData.h>
#include "stdtypes.h"
#include "d3drender.h"
#include "proxyIDirect3D9.h"
#include "proxyIDirect3DDevice9.h"

#include <iostream>
#include <sstream>
#include <istream>
#include <ostream>
#include <fstream>
#include <glm/gtx/quaternion.hpp>
//#include <patch.h>
#include <chrono>
#include <deque>

#include <network/ActiveSocket.h>
#include <network/PassiveSocket.h>
#include <network/SimpleSocket.h>

// externals
extern HMODULE					g_hDllModule;
extern char						g_szWorkingDirectory[MAX_PATH];
extern t_WindowsInfo			WindowsInfo;
extern HINSTANCE				g_hOrigDll;
extern HWND					g_WindowHandle;

int process_init ( void );

std::string string_format(const std::string fmt, ...) ;

//let the "hacking" begin :)

#define PI 3.1415926535897932384626433f

extern POINT MouseCursorPosition;

struct KeyManager
{
	bool Pressed;
	bool Down;
	bool Released;
	bool Up;
	bool ConsumePressed()
	{
		if(Pressed)
		{
			Pressed = false;
			return true;
		}
		return false;
	}
	bool ConsumeReleased()
	{
		if(Released)
		{
			Released = false;
			return true;
		}
		return false;
	}
	bool ConsumeDown()
	{
		if(Down)
		{
			Down = false;
			return true;
		}
		return false;
	}
	bool ConsumeUp()
	{
		if(Up)
		{
			Up = false;
			return true;
		}
		return false;
	}
};

extern KeyManager Keys[256];

void KeyManagerRun();

#define Keys(a) Keys[a]

///
//#define AddVar(Type,Name,Address) Type& Name = *reinterpret_cast<Type*>(Address)

struct Point4D { float x, y, z, w; };
struct Point3D { float x, y, z; };
struct Point2D { float x, y; };

struct CameraInfo
{
	Point3D Position;
	Point4D Rotation;
	float Other[3];
};

struct QuatRot
{
	Point4D Front;//frontvector
	Point4D Right;//rightvector
	Point4D Up;//upvector
};

#define NFS_MAX_VEHICLES (14)//seriously just 14.. (including the player)

//vehicle structure which is recreated by me, 
//floats... 
//floats everywhere
//85 variables in total, 352 bytes.. maybe 512?
struct VehicleInfo
{
	//type		name									array	offset
	float		unknown_01								[8];	//0x0
	Point3D		Pos;											//0x20
	float		unknown_02;										//0x2C
	QuatRot		Rotation;										//0x30
	Point3D		unknown_03;										//0x60
	float		unknown_04;										//0x6C
	Point3D		Velocity;										//0x70
	float		unknown_05;										//0x7C
	Point3D		TurnSpeed;										//0x80
	float		unknown_06;										//0x8C
	float		SpeedAndBrakeAccelerator;						//0x90
	float		unknown_07								[3];	//0x94
	Point3D		unknown_08;										//0xA0
	float		unknown_09;										//0xAC
	Point4D		unknown_10								[5];	//0xB0
	float		unknown_11								[20];	//0x100
	float		unknown_12_is_zero_when_accelerating;			//0x150
	//end of structure...?
};

std::ostream& operator<<(std::ostream& stream, const VehicleInfo &info);
std::ofstream& operator<<(std::ofstream& stream, VehicleInfo &info);
std::ifstream& operator>>(std::ifstream& stream, VehicleInfo &info);

namespace Recorder
{
	struct FrameInfo//1392 bytes / frame | max 167040 bytes @ 120 fps | 9.56 MB / min max
	{
		std::chrono::high_resolution_clock::duration time;
		VehicleInfo Vehicle;
		int Nitro;
		float RPM;
		float TURBO;
		int CurrentGear;
		KeyManager Keys[256];
	};
};

std::ofstream& operator<<(std::ofstream& stream, Recorder::FrameInfo &info);
std::ifstream& operator>>(std::ifstream& stream, Recorder::FrameInfo &info);

#define ASSIGN(a,b,x) a->x = b->x
#define VehicleInfoAssign(a,b) \
{\
	ASSIGN(a,b,unknown_12_is_zero_when_accelerating);\
	ASSIGN(a,b,SpeedAndBrakeAccelerator);\
	ASSIGN(a,b,Pos.x);\
	ASSIGN(a,b,Pos.y);\
	ASSIGN(a,b,Pos.z);\
	ASSIGN(a,b,Velocity.x);\
	ASSIGN(a,b,Velocity.y);\
	ASSIGN(a,b,Velocity.z);\
	ASSIGN(a,b,TurnSpeed.x);\
	ASSIGN(a,b,TurnSpeed.y);\
	ASSIGN(a,b,TurnSpeed.z);\
	ASSIGN(a,b,Rotation.Front.w);\
	ASSIGN(a,b,Rotation.Front.x);\
	ASSIGN(a,b,Rotation.Front.y);\
	ASSIGN(a,b,Rotation.Front.z);\
	ASSIGN(a,b,Rotation.Right.w);\
	ASSIGN(a,b,Rotation.Right.x);\
	ASSIGN(a,b,Rotation.Right.y);\
	ASSIGN(a,b,Rotation.Right.z);\
	ASSIGN(a,b,Rotation.Up.w);\
	ASSIGN(a,b,Rotation.Up.x);\
	ASSIGN(a,b,Rotation.Up.y);\
	ASSIGN(a,b,Rotation.Up.z);\
}

#define SAVEASSIGN(x) SAVED.x = Vehicles[0]->x

#define SAVEPlayerVehicleInfo() \
{\
	SAVEASSIGN(unknown_01[0]);\
	SAVEASSIGN(unknown_01[1]);\
	SAVEASSIGN(unknown_01[2]);\
	SAVEASSIGN(unknown_01[3]);\
	SAVEASSIGN(unknown_01[4]);\
	SAVEASSIGN(unknown_01[5]);\
	SAVEASSIGN(unknown_01[6]);\
	SAVEASSIGN(unknown_01[7]);\
	SAVEASSIGN(unknown_02);\
	SAVEASSIGN(unknown_03.x);\
	SAVEASSIGN(unknown_03.y);\
	SAVEASSIGN(unknown_03.z);\
	SAVEASSIGN(unknown_04);\
	SAVEASSIGN(unknown_05);\
	SAVEASSIGN(unknown_06);\
	SAVEASSIGN(unknown_07[0]);\
	SAVEASSIGN(unknown_07[1]);\
	SAVEASSIGN(unknown_07[2]);\
	SAVEASSIGN(unknown_08.x);\
	SAVEASSIGN(unknown_08.y);\
	SAVEASSIGN(unknown_08.z);\
	SAVEASSIGN(unknown_09);\
	SAVEASSIGN(unknown_10[0].w);\
	SAVEASSIGN(unknown_10[0].x);\
	SAVEASSIGN(unknown_10[0].y);\
	SAVEASSIGN(unknown_10[0].z);\
	SAVEASSIGN(unknown_10[1].w);\
	SAVEASSIGN(unknown_10[1].x);\
	SAVEASSIGN(unknown_10[1].y);\
	SAVEASSIGN(unknown_10[1].z);\
	SAVEASSIGN(unknown_10[2].w);\
	SAVEASSIGN(unknown_10[2].x);\
	SAVEASSIGN(unknown_10[2].y);\
	SAVEASSIGN(unknown_10[2].z);\
	SAVEASSIGN(unknown_10[3].w);\
	SAVEASSIGN(unknown_10[3].x);\
	SAVEASSIGN(unknown_10[3].y);\
	SAVEASSIGN(unknown_10[3].z);\
	SAVEASSIGN(unknown_10[4].w);\
	SAVEASSIGN(unknown_10[4].x);\
	SAVEASSIGN(unknown_10[4].y);\
	SAVEASSIGN(unknown_10[4].z);\
	SAVEASSIGN(unknown_12_is_zero_when_accelerating);\
	SAVEASSIGN(SpeedAndBrakeAccelerator);\
	SAVEASSIGN(Pos.x);\
	SAVEASSIGN(Pos.y);\
	SAVEASSIGN(Pos.z);\
	SAVEASSIGN(Velocity.x);\
	SAVEASSIGN(Velocity.y);\
	SAVEASSIGN(Velocity.z);\
	SAVEASSIGN(TurnSpeed.x);\
	SAVEASSIGN(TurnSpeed.y);\
	SAVEASSIGN(TurnSpeed.z);\
	SAVEASSIGN(Rotation.Front.w);\
	SAVEASSIGN(Rotation.Front.x);\
	SAVEASSIGN(Rotation.Front.y);\
	SAVEASSIGN(Rotation.Front.z);\
	SAVEASSIGN(Rotation.Right.w);\
	SAVEASSIGN(Rotation.Right.x);\
	SAVEASSIGN(Rotation.Right.y);\
	SAVEASSIGN(Rotation.Right.z);\
	SAVEASSIGN(Rotation.Up.w);\
	SAVEASSIGN(Rotation.Up.x);\
	SAVEASSIGN(Rotation.Up.y);\
	SAVEASSIGN(Rotation.Up.z);\
}

#define LOADASSIGN(x) Vehicles[0]->x = SAVED.x
#define LOADPlayerVehicleInfo() \
{\
	LOADASSIGN(Pos.x);\
	LOADASSIGN(Pos.y);\
	LOADASSIGN(Pos.z);\
	LOADASSIGN(Rotation.Front.w);\
	LOADASSIGN(Rotation.Front.x);\
	LOADASSIGN(Rotation.Front.y);\
	LOADASSIGN(Rotation.Front.z);\
	LOADASSIGN(Rotation.Right.w);\
	LOADASSIGN(Rotation.Right.x);\
	LOADASSIGN(Rotation.Right.y);\
	LOADASSIGN(Rotation.Right.z);\
	LOADASSIGN(Rotation.Up.w);\
	LOADASSIGN(Rotation.Up.x);\
	LOADASSIGN(Rotation.Up.y);\
	LOADASSIGN(Rotation.Up.z);\
	LOADASSIGN(unknown_12_is_zero_when_accelerating);\
	LOADASSIGN(SpeedAndBrakeAccelerator);\
	LOADASSIGN(Velocity.x);\
	LOADASSIGN(Velocity.y);\
	LOADASSIGN(Velocity.z);\
	LOADASSIGN(TurnSpeed.x);\
	LOADASSIGN(TurnSpeed.y);\
	LOADASSIGN(TurnSpeed.z);\
	LOADASSIGN(unknown_01[0]);\
	LOADASSIGN(unknown_01[1]);\
	LOADASSIGN(unknown_01[2]);\
	LOADASSIGN(unknown_01[3]);\
	LOADASSIGN(unknown_01[4]);\
	LOADASSIGN(unknown_01[5]);\
	LOADASSIGN(unknown_01[6]);\
	LOADASSIGN(unknown_01[7]);\
	LOADASSIGN(unknown_02);\
	LOADASSIGN(unknown_03.x);\
	LOADASSIGN(unknown_03.y);\
	LOADASSIGN(unknown_03.z);\
	LOADASSIGN(unknown_04);\
	LOADASSIGN(unknown_05);\
	LOADASSIGN(unknown_06);\
	LOADASSIGN(unknown_07[0]);\
	LOADASSIGN(unknown_07[1]);\
	LOADASSIGN(unknown_07[2]);\
	LOADASSIGN(unknown_08.x);\
	LOADASSIGN(unknown_08.y);\
	LOADASSIGN(unknown_08.z);\
	LOADASSIGN(unknown_09);\
	LOADASSIGN(unknown_10[0].w);\
	LOADASSIGN(unknown_10[0].x);\
	LOADASSIGN(unknown_10[0].y);\
	LOADASSIGN(unknown_10[0].z);\
	LOADASSIGN(unknown_10[1].w);\
	LOADASSIGN(unknown_10[1].x);\
	LOADASSIGN(unknown_10[1].y);\
	LOADASSIGN(unknown_10[1].z);\
	LOADASSIGN(unknown_10[2].w);\
	LOADASSIGN(unknown_10[2].x);\
	LOADASSIGN(unknown_10[2].y);\
	LOADASSIGN(unknown_10[2].z);\
	LOADASSIGN(unknown_10[3].w);\
	LOADASSIGN(unknown_10[3].x);\
	LOADASSIGN(unknown_10[3].y);\
	LOADASSIGN(unknown_10[3].z);\
	LOADASSIGN(unknown_10[4].w);\
	LOADASSIGN(unknown_10[4].x);\
	LOADASSIGN(unknown_10[4].y);\
	LOADASSIGN(unknown_10[4].z);\
}

#define PLAYER_VEHICLE (0)

#define PROTECT try{
#define UNPROTECT }catch(...){}
#define POINTER(type,addr) (*(type*)(addr))

namespace DirectXFont
{
	extern std::map<std::pair<std::string,std::pair<int,DWORD>>,CD3DFont> fonts;
	extern std::map<unsigned int,std::pair<std::string,std::pair<int,DWORD>>> font_id;
	extern unsigned int FontCounter;
	int Add(std::string fontname, int size, DWORD flags);
	bool Initialize(unsigned int ID);
	bool Remove(unsigned int ID);
	void InitializeAll();
	void InvalidateAll();
	CD3DFont * Access(int ID);
};

/*
AddVar(unsigned int,VehiclePoolBaseAddr,0x0089CDA8);
AddVar(unsigned int,CameraBaseAddress,0x0086E6FC);
AddVar(unsigned int,CameraPosBaseAddr,0x0089CCF8);
AddVar(unsigned int,DistanceBaseAddr,0x0089CD04);
AddVar(Point3D,PlayerCameraPosition,0x00876120);
AddVar(unsigned int,OtherCameraStuff,0x0086E6FC);
AddVar(float,FOV,0x0078721C);
AddVar(unsigned int,PoolsBaseAddr,0x1002A140);
AddVar(unsigned int,DriftingBaseAddr,0x00890118);
AddVar(bool,RearMirrorEnabled,0x00832F38);
AddVar(unsigned int,CurrentCameraMode,0x0083AA74);
AddVar(unsigned int,ExploreMapMode,0x0083AA1C);
*/
std::stringstream& operator>>(std::stringstream& stream, Recorder::FrameInfo &info);
std::stringstream& operator<<(std::stringstream& stream, Recorder::FrameInfo &info);
std::stringstream& operator>>(std::stringstream& stream, VehicleInfo &info);
std::stringstream& operator<<(std::stringstream& stream, VehicleInfo &info);
#endif
