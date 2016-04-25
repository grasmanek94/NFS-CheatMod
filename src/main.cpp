#include <main.h>

extern proxyIDirect3DDevice9	*pDirect3DDevice9;

HINSTANCE				g_hOrigDll = NULL;
HMODULE					g_hDllModule = NULL;
HWND					g_WindowHandle = NULL;
char					g_szWorkingDirectory[MAX_PATH];
t_WindowsInfo			WindowsInfo;

/////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI unhandledExceptionFilter ( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
	return EXCEPTION_CONTINUE_SEARCH;
}
BOOL DI_HOOK_INITIALIZE( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 );
BOOL APIENTRY DllMain ( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	//DI_HOOK_INITIALIZE(hModule,ul_reason_for_call,lpReserved);
	switch ( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls( hModule );
		g_hDllModule = hModule;
		SetUnhandledExceptionFilter( unhandledExceptionFilter );
		break;

	case DLL_PROCESS_DETACH:

		if ( g_hOrigDll != NULL )
		{
			FreeLibrary( g_hOrigDll );
			g_hOrigDll = NULL;
		}
		break;
	}

	return true;
}

int process_init ( void )
{
	if ( g_hOrigDll == NULL )
	{
		if ( GetModuleFileName(g_hDllModule, g_szWorkingDirectory, sizeof(g_szWorkingDirectory) - 32) != 0 )
		{
			if ( strrchr(g_szWorkingDirectory, '\\') != NULL )
				*strrchr( g_szWorkingDirectory, '\\' ) = 0;
			else
				strcpy( g_szWorkingDirectory, "." );
		}
		else
		{
			strcpy( g_szWorkingDirectory, "." );
		}

		g_hOrigDll = LoadLibrary( "proxyloader_d3d9.dll" );
		char	filename[MAX_PATH];
		if ( g_hOrigDll == NULL )
		{	
			GetSystemDirectory( filename, (UINT) (MAX_PATH - strlen("\\d3d9.dll") - 1) );

			strcat( filename, "\\d3d9.dll" );
			g_hOrigDll = LoadLibrary( filename );
			if ( g_hOrigDll == NULL )
			{
				return 0;
			}
		}
		orig_Direct3DCreate9 = ( D3DC9 ) GetProcAddress( g_hOrigDll, "Direct3DCreate9" );
		if ( orig_Direct3DCreate9 == NULL )
		{
			FreeLibrary( g_hOrigDll );
			return 0;
		}
	}
	return 1;
}

namespace DirectXFont
{
	std::map<std::pair<std::string,std::pair<int,DWORD>>,CD3DFont> fonts;
	std::map<unsigned int,std::pair<std::string,std::pair<int,DWORD>>> font_id;
	unsigned int FontCounter = -1;
	int Add(std::string fontname, int size, DWORD flags)
	{
		if(fonts.find(std::make_pair(fontname,std::make_pair(size,flags))) == fonts.end())
		{
			for(auto it = font_id.begin(); it != font_id.end(); ++it)
			{
				if(it->second == std::make_pair(fontname,std::make_pair(size,flags)))
				{
					return it->first;//guaranteed to happen
				}
			}
		}
		fonts.emplace(std::make_pair(fontname,std::make_pair(size,flags)),CD3DFont(fontname.c_str(),size,flags));
		font_id[++FontCounter] = std::make_pair(fontname,std::make_pair(size,flags));
		return FontCounter;
	}
	bool Initialize(unsigned int ID)
	{
		if(ID < 0)
			return false;
		if(fonts.find(font_id[ID]) == fonts.end())
			return false;
		return fonts.at(font_id[ID]).Initialize( pDirect3DDevice9 ) == S_OK;
	}
	bool Remove(unsigned int ID)
	{
		if(ID < 0)
			return false;
		if(fonts.find(font_id[ID]) == fonts.end())
			return false;
		fonts.at(font_id[ID]).Invalidate();
		fonts.erase(font_id[ID]);
		return true;
	}
	void InitializeAll()
	{
		for(auto it = fonts.begin(); it != fonts.end(); ++it)
			it->second.Initialize( pDirect3DDevice9 );
	}
	void InvalidateAll()
	{
		for(auto it = fonts.begin(); it != fonts.end(); ++it)
			it->second.Invalidate();
	}
	CD3DFont * Access(int ID)
	{
		if(ID < 0)
			return 0;
		if(fonts.find(font_id[ID]) == fonts.end())
			return 0;
		return &fonts.at(font_id[ID]);
	}
};

std::string string_format(const std::string fmt, ...) 
{
	int size = 512;
	std::string str;
	va_list ap;
	while (1) {
		str.resize(size);
		va_start(ap, fmt);
		int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
		va_end(ap);
		if (n > -1 && n < size) {
			str.resize(n);
			return str;
		}
		if (n > -1)
			size = n + 1;
		else
			size *= 2;
	}
	return str;
}

KeyManager Keys[256];

void KeyManagerRun()
{
	for(unsigned int i = 0; i < 256; ++i)
	{
		if(GetAsyncKeyState(i))
		{
			if(!Keys[i].Down)
			{
				Keys[i].Down = true;
				Keys[i].Pressed = true;
				Keys[i].Released = false;
				Keys[i].Up = false;
			}
			else if(Keys[i].Pressed)
			{
				Keys[i].Pressed = false;
			}
		}
		else
		{
			if(!Keys[i].Up)
			{
				Keys[i].Released = true;
				Keys[i].Up = true;
				Keys[i].Down = false;
				Keys[i].Pressed = false;
			}
			else if(Keys[i].Released)
			{
				Keys[i].Released = false;
			}
		}
	}
}

POINT MouseCursorPosition;

std::ofstream& operator<<(std::ofstream& stream, VehicleInfo &info)
{
	stream.write(reinterpret_cast<char*>(&info.unknown_01[0]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[1]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[2]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[3]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[4]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[5]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[6]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[7]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[8]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Pos.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Pos.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Pos.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_03.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_03.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_03.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_04), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Velocity.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Velocity.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Velocity.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_05), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.TurnSpeed.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.TurnSpeed.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.TurnSpeed.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_06), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.SpeedAndBrakeAccelerator), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_07[0]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_07[1]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_07[2]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_08.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_08.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_08.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_09), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[0]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[1]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[2]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[3]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[4]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[5]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[6]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[7]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[8]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[9]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[10]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[11]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[12]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[13]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[14]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[15]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[16]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[17]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[18]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[19]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[20]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_12_is_zero_when_accelerating), sizeof(float));
	return stream;
}

std::ifstream& operator>>(std::ifstream& stream, VehicleInfo &info)
{
	stream.read(reinterpret_cast<char*>(&info.unknown_01[0]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[1]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[2]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[3]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[4]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[5]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[6]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[7]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[8]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Pos.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Pos.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Pos.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_03.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_03.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_03.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_04), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Velocity.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Velocity.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Velocity.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_05), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.TurnSpeed.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.TurnSpeed.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.TurnSpeed.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_06), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.SpeedAndBrakeAccelerator), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_07[0]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_07[1]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_07[2]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_08.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_08.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_08.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_09), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[0]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[1]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[2]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[3]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[4]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[5]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[6]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[7]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[8]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[9]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[10]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[11]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[12]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[13]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[14]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[15]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[16]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[17]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[18]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[19]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[20]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_12_is_zero_when_accelerating), sizeof(float));
	return stream;
}

std::ofstream& operator<<(std::ofstream& stream, Recorder::FrameInfo &info)
{
	stream.write(reinterpret_cast<char*>(&info.time), sizeof(std::chrono::high_resolution_clock::duration));
	stream << info.Vehicle;
	stream.write(reinterpret_cast<char*>(&info.Nitro), sizeof(int));
	stream.write(reinterpret_cast<char*>(&info.RPM), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.CurrentGear), sizeof(int));
	stream.write(reinterpret_cast<char*>(&info.TURBO), sizeof(int));
	for(int i = 0; i < 256; ++i)
	{
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Pressed), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Released), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Down), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Up), sizeof(bool));
	}
	return stream;
}

std::ifstream& operator>>(std::ifstream& stream, Recorder::FrameInfo &info)
{
	stream.read(reinterpret_cast<char*>(&info.time), sizeof(std::chrono::high_resolution_clock::duration));
	stream >> info.Vehicle;
	stream.read(reinterpret_cast<char*>(&info.Nitro), sizeof(int));
	stream.read(reinterpret_cast<char*>(&info.RPM), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.CurrentGear), sizeof(int));
	stream.read(reinterpret_cast<char*>(&info.TURBO), sizeof(int));
	for(int i = 0; i < 256; ++i)
	{
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Pressed), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Released), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Down), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Up), sizeof(bool));
	}
	return stream;
}

///

std::stringstream& operator<<(std::stringstream& stream, VehicleInfo &info)
{
	stream.write(reinterpret_cast<char*>(&info.unknown_01[0]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[1]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[2]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[3]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[4]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[5]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[6]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[7]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_01[8]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Pos.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Pos.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Pos.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Front.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Right.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Rotation.Up.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_03.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_03.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_03.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_04), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Velocity.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Velocity.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.Velocity.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_05), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.TurnSpeed.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.TurnSpeed.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.TurnSpeed.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_06), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.SpeedAndBrakeAccelerator), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_07[0]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_07[1]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_07[2]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_08.x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_08.y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_08.z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_09), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[0].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[1].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[2].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[3].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].w), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].x), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].y), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_10[4].z), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[0]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[1]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[2]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[3]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[4]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[5]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[6]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[7]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[8]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[9]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[10]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[11]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[12]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[13]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[14]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[15]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[16]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[17]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[18]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[19]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_11[20]), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.unknown_12_is_zero_when_accelerating), sizeof(float));
	return stream;
}

std::stringstream& operator>>(std::stringstream& stream, VehicleInfo &info)
{
	stream.read(reinterpret_cast<char*>(&info.unknown_01[0]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[1]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[2]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[3]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[4]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[5]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[6]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[7]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_01[8]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Pos.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Pos.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Pos.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Front.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Right.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Rotation.Up.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_03.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_03.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_03.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_04), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Velocity.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Velocity.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.Velocity.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_05), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.TurnSpeed.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.TurnSpeed.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.TurnSpeed.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_06), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.SpeedAndBrakeAccelerator), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_07[0]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_07[1]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_07[2]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_08.x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_08.y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_08.z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_09), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[0].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[1].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[2].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[3].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].w), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].x), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].y), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_10[4].z), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[0]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[1]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[2]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[3]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[4]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[5]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[6]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[7]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[8]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[9]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[10]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[11]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[12]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[13]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[14]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[15]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[16]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[17]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[18]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[19]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_11[20]), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.unknown_12_is_zero_when_accelerating), sizeof(float));
	return stream;
}

std::stringstream& operator<<(std::stringstream& stream, Recorder::FrameInfo &info)
{
	stream.write(reinterpret_cast<char*>(&info.time), sizeof(std::chrono::high_resolution_clock::duration));
	stream << info.Vehicle;
	stream.write(reinterpret_cast<char*>(&info.Nitro), sizeof(int));
	stream.write(reinterpret_cast<char*>(&info.RPM), sizeof(float));
	stream.write(reinterpret_cast<char*>(&info.CurrentGear), sizeof(int));
	stream.write(reinterpret_cast<char*>(&info.TURBO), sizeof(int));
	for(int i = 0; i < 256; ++i)
	{
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Pressed), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Released), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Down), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Keys[i].Up), sizeof(bool));
	}
	return stream;
}

std::stringstream& operator>>(std::stringstream& stream, Recorder::FrameInfo &info)
{
	stream.read(reinterpret_cast<char*>(&info.time), sizeof(std::chrono::high_resolution_clock::duration));
	stream >> info.Vehicle;
	stream.read(reinterpret_cast<char*>(&info.Nitro), sizeof(int));
	stream.read(reinterpret_cast<char*>(&info.RPM), sizeof(float));
	stream.read(reinterpret_cast<char*>(&info.CurrentGear), sizeof(int));
	stream.read(reinterpret_cast<char*>(&info.TURBO), sizeof(int));
	for(int i = 0; i < 256; ++i)
	{
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Pressed), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Released), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Down), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Keys[i].Up), sizeof(bool));
	}
	return stream;
}