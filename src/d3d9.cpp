/*
 * NFS: Underground 2 | 'NFSMOD' By Gamer_Z/Grasmanek94 | http://gpb.googlecode.com/
 */
#include <main.h>

extern proxyIDirect3DDevice9	*pDirect3DDevice9;
extern CD3DRender				*render;
VehicleInfo *Vehicles[NFS_MAX_VEHICLES];//14 vehicles seems to be the limit in NFS UG2 ?
VehicleInfo SAVED;
std::map<unsigned int,unsigned int> VK_DI_KEYTABLE;

std::vector <int> ValidVehicles;

auto& ScreenWidth = POINTER(int,0x007FF77C);
auto& ScreenHeight= POINTER(int,0x007FF780);

namespace Recorder
{
	short State = 0;
	std::chrono::high_resolution_clock::time_point StartTime;
	std::chrono::high_resolution_clock::duration Current;

	//std::vector<std::deque<FrameInfo>> History;
	std::deque<FrameInfo> frames;
	INPUT KEYBOARDINPUT;
	FrameInfo g_Temp;
	std::string lastreplay;

	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;

	bool ReadReplay(std::string filename,std::deque<FrameInfo> &RRframes);
	std::string SaveReplay(std::deque<FrameInfo> &frames_to_save);

	namespace Live
	{
		CActiveSocket Sender//(CActiveSocket::CSocketType::SocketTypeUdp)
			;
		CPassiveSocket Receiver//(CPassiveSocket::CSocketType::SocketTypeUdp)
			;
		CActiveSocket *pClient = NULL;		 
	};
	bool TryReplay();
	bool TryRecord();
	void Stop();
};

namespace Drawer
{
	struct TextInfo
	{
		std::string text;
		float x,y;
		DWORD start,end;
		DWORD color;
		bool textcoloring;
		TextInfo(float _x, float _y, std::string _text, DWORD _start, DWORD _end, DWORD _color, bool _tc = false)
		{
			x = _x;
			y = _y;
			text.assign(_text.c_str());
			start = _start;
			end = _end;
			color = _color;
			textcoloring = _tc;
		}
	};

	std::vector<TextInfo> data;
	TextInfo DisplayNow(0.0f,10.0f,"",0,0,0xFFFFFFFF,true);

	void Add(float x, float y, std::string text, DWORD time, DWORD delay, DWORD color, bool TextColoring)
	{
		DWORD PUT = GetTickCount()+delay;
		data.push_back(TextInfo(x,y,text,PUT,PUT+time,color,TextColoring));
	}
	void AddQueue(std::string text, DWORD time = 1000)
	{
		DisplayNow.text.assign(text);
		DWORD PUT = GetTickCount();
		DisplayNow.start = PUT;
		DisplayNow.end = PUT+time;
	}
	void Display()
	{
		DWORD now = GetTickCount();
		for(unsigned int i = 0, j = data.size(); i < j; ++i)
			if(data.at(i).end > now)
				data.erase(data.begin()+i);
		for(unsigned int i = 0, j = data.size(); i < j; ++i)
			if(data.at(i).start >= now)
				DirectXFont::Access(0)->Print(data.at(i).x,data.at(i).y,data.at(i).color,data.at(i).text.c_str(),data.at(i).textcoloring);
		if(DisplayNow.end > now)
			DirectXFont::Access(0)->Print(0.0f,10.0f,0xFFFFFFFF,DisplayNow.text.c_str(),true);
	}
};

namespace User
{
	struct Options
	{
		bool SpeedHacks;
		bool Recordings;
		bool Cheat;
		bool Jumper;
		bool Control;
		bool Saver;
		bool NoEngineHeat;
		bool MoneyHack;
		bool DriftAnywhere;
		bool ReloadNitro;
		bool RainControl;
		float RainAmount;
		bool LiveReplaySend;
		bool LiveReplayReceive;
		Options() : 
			SpeedHacks(false), Recordings(false), Cheat(true), 
			Jumper(false), Control(false), Saver(false), 
			NoEngineHeat(false), MoneyHack(false), DriftAnywhere(false),
			ReloadNitro(false), RainControl(false), RainAmount(0.5f),
			LiveReplaySend(false), LiveReplayReceive(false)
		{}
	};

	std::ofstream& operator<<(std::ofstream& stream, Options &info)
	{
		stream.write(reinterpret_cast<char*>(&info.SpeedHacks), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Recordings), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Cheat), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Jumper), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Control), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.Saver), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.NoEngineHeat), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.MoneyHack), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.DriftAnywhere), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.ReloadNitro), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.RainControl), sizeof(bool));
		stream.write(reinterpret_cast<char*>(&info.RainAmount), sizeof(float));
		return stream;
	}

	std::ifstream& operator>>(std::ifstream& stream, Options &info)
	{
		stream.read(reinterpret_cast<char*>(&info.SpeedHacks), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Recordings), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Cheat), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Jumper), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Control), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.Saver), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.NoEngineHeat), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.MoneyHack), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.DriftAnywhere), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.ReloadNitro), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.RainControl), sizeof(bool));
		stream.read(reinterpret_cast<char*>(&info.RainAmount), sizeof(float));
		return stream;
	}

	namespace Settings
	{
		Options Data;

		bool Save()
		{
			std::ofstream myfile(".\\NFSMOD\\settings.dat",std::ios::binary);
			if(myfile.good())
			{
				myfile << Data;
				myfile.close();
				return true;
			}
			return false;
		}

		bool Load()
		{
			std::ifstream myfile(".\\NFSMOD\\settings.dat",std::ios::binary);	
			if(myfile.good())
			{
				myfile >> Data;
				myfile.close();
				return true;
			}
			return false;
		}
		void CheckChange()
		{
			if(Keys(VK_APPS).Down)
			{
				if(Keys(VK_F1).ConsumeDown())
				{
					Keys(VK_F1).ConsumePressed();

					float bHeight = ScreenHeight/48;
					float maxwidth = DirectXFont::Access(0)->DrawLength("MENU + 6  - Toggle DriftAnywhere")+5.0f;//longest text

					render->D3DBoxBorder(10.0f,15.0f+bHeight*0,maxwidth+10.0f,190.0f+bHeight*16,0xFF000000,0x77777777);
					
					DirectXFont::Access(0)->Print(15.0f,20.0f+bHeight*0 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}0  {FFFFFFFF}- Toggle {FF%s00}Cheats",(Settings::Data.Cheat) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,30.0f+bHeight*1 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}1  {FFFFFFFF}- Toggle {FF%s00}Recordings",(Settings::Data.Recordings) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,40.0f+bHeight*2 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}2  {FFFFFFFF}- Toggle {FF%s00}Speedhacks",(Settings::Data.SpeedHacks) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,50.0f+bHeight*3 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}3  {FFFFFFFF}- Toggle {FF%s00}Jumper",(Settings::Data.Jumper) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,60.0f+bHeight*4 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}4  {FFFFFFFF}- Toggle {FF%s00}Control",(Settings::Data.Control) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,70.0f+bHeight*5 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}5  {FFFFFFFF}- Toggle {FF%s00}Saver",(Settings::Data.Saver) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,80.0f+bHeight*6 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}6  {FFFFFFFF}- Toggle {FF%s00}DriftAnywhere",(Settings::Data.DriftAnywhere) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,90.0f+bHeight*7 ,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}7  {FFFFFFFF}- Toggle {FF%s00}MoneyHack",(Settings::Data.MoneyHack) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,100.0f+bHeight*8,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}8  {FFFFFFFF}- Toggle {FF%s00}NoEngineHeat",(Settings::Data.NoEngineHeat) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,110.0f+bHeight*9,0xFFFFFFFF,string_format( "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}9  {FFFFFFFF}- Toggle {FF%s00}ReloadNitro",(Settings::Data.ReloadNitro) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,120.0f+bHeight*10,0xFFFFFFFF,			   "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}F1 {FFFFFFFF}- Show this help",true);
					DirectXFont::Access(0)->Print(15.0f,130.0f+bHeight*11,0xFFFFFFFF,			   "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}F2 {FFFFFFFF}- Load settings",true);
					DirectXFont::Access(0)->Print(15.0f,140.0f+bHeight*12,0xFFFFFFFF,			   "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}F3 {FFFFFFFF}- Save settings",true);
					DirectXFont::Access(0)->Print(15.0f,150.0f+bHeight*13,0xFFFFFFFF,string_format("{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}F4 {FFFFFFFF}- Toggle {FF%s00}RainControl",(Settings::Data.RainControl) ? ("00FF") : ("FF00")).c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,160.0f+bHeight*14,0xFFFFFFFF,			   "{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}F5 {FFFFFFFF}- Show Cheat Options",true);	
					DirectXFont::Access(0)->Print(15.0f,170.0f+bHeight*15,0xFFFFFFFF,string_format("{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}[  {FFFFFFFF}- Decrease Rain Amount").c_str(),true);
					DirectXFont::Access(0)->Print(15.0f,180.0f+bHeight*16,0xFFFFFFFF,string_format("{FFFF0000}MENU {FFFFFFFF}+ {FF00FF00}]  {FFFFFFFF}- Increase Rain Amount").c_str(),true);

				}
				if(Keys(VK_F2).ConsumePressed())
				{
					if(User::Settings::Load())
						Drawer::AddQueue("Settings {FFFF0000}Loaded");
					else
						Drawer::AddQueue("Settings {FF00FF00}cannot be loaded");					
				}
				if(Keys(VK_F3).ConsumePressed())
				{
					if(User::Settings::Save())
						Drawer::AddQueue("Settings {FFFF0000}Saved");
					else
						Drawer::AddQueue("Settings {FF00FF00}cannot be saved");
				}
				if(Keys(VK_F4).ConsumePressed())
				{
					Data.RainControl ^= 1;
					if(!Data.RainControl)
						Drawer::AddQueue("RainControl {FFFF0000}Disabled");
					else
						Drawer::AddQueue("RainControl {FF00FF00}Enabled");
				}
				if(Keys(VK_F5).ConsumeDown())
				{
					Keys(VK_F5).ConsumePressed();
					float bHeight = ScreenHeight/48;

					float maxwidth = DirectXFont::Access(0)->DrawLength("MENU + 6  - Toggle DriftAnywhere")+25.0f;//longest text from F1 menu
					float maxwidth2 = DirectXFont::Access(0)->DrawLength("Q         - start recording playback");

					render->D3DBoxBorder(maxwidth,15.0f+bHeight*0,maxwidth2+10.0f,140.0f+bHeight*11,0xFF000000,0x77777777);
					
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,20.0f+bHeight*0 ,0xFFFFFFFF,"{FFFF0000}BACKSPACE {FFFFFFFF}- Jump 20 units forward",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,30.0f+bHeight*1 ,0xFFFFFFFF,"{FFFF0000}SHIFT     {FFFFFFFF}- Extra speed",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,40.0f+bHeight*2 ,0xFFFFFFFF,"{FFFF0000}~         {FFFFFFFF}- Extra brake",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,50.0f+bHeight*3 ,0xFFFFFFFF,"{FFFF0000}TAB       {FFFFFFFF}- Super brake",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,60.0f+bHeight*4 ,0xFFFFFFFF,"{FFFF0000}F5        {FFFFFFFF}- Start recording playback",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,70.0f+bHeight*5 ,0xFFFFFFFF,"{FFFF0000}F6        {FFFFFFFF}- Play last replay",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,80.0f+bHeight*6 ,0xFFFFFFFF,"{FFFF0000}F9        {FFFFFFFF}- Save vehicle position",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,90.0f+bHeight*7 ,0xFFFFFFFF,"{FFFF0000}F11       {FFFFFFFF}- Load vehicle position",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,100.0f+bHeight*8,0xFFFFFFFF,"{FFFF0000}Q         {FFFFFFFF}- Teleport all vehicles",true);
					DirectXFont::Access(0)->Print(maxwidth + 5.0f,110.0f+bHeight*9,0xFFFFFFFF,"{FFFF0000}F7        {FFFFFFFF}- Change selected replay",true);
					DirectXFont::Access(0)->Print(maxwidth +5.0f,120.0f+bHeight*10,0xFFFFFFFF,"{FFFF0000}MENU {FFFFFFFF}+ {FFFF0000}-  {FFFFFFFF}- Decrease FOV",true);
					DirectXFont::Access(0)->Print(maxwidth +5.0f,130.0f+bHeight*11,0xFFFFFFFF,"{FFFF0000}MENU {FFFFFFFF}+ {FFFF0000}=  {FFFFFFFF}- Increase FOV",true);
				}
				if(Keys('0').ConsumePressed())
				{
					Data.Cheat ^= 1;
					if(!Data.Cheat)
						Drawer::AddQueue("Cheats {FFFF0000}Disabled");
					else
						Drawer::AddQueue("Cheats {FF00FF00}Enabled");
				}
				if(Keys('1').ConsumePressed())
				{
					if(!Recorder::State)
					{
						Data.Recordings ^= 1;
						if(!Data.Recordings)
							Drawer::AddQueue("Recordings {FFFF0000}Disabled");
						else
							Drawer::AddQueue("Recordings {FF00FF00}Enabled");
					}
					else
						Drawer::AddQueue("{FFFF0000}Please stop recording/replay first");
				}
				if(Keys('2').ConsumePressed())
				{
					Data.SpeedHacks ^= 1;
					if(!Data.SpeedHacks)
						Drawer::AddQueue("SpeedHacks {FFFF0000}Disabled");
					else
						Drawer::AddQueue("SpeedHacks {FF00FF00}Enabled");
				}
				if(Keys('3').ConsumePressed())
				{
					Data.Jumper ^= 1;
					if(!Data.Jumper)
						Drawer::AddQueue("Jumper {FFFF0000}Disabled");
					else
						Drawer::AddQueue("Jumper {FF00FF00}Enabled");
				}
				if(Keys('4').ConsumePressed())
				{
					Data.Control ^= 1;
					if(!Data.Control)
						Drawer::AddQueue("Control {FFFF0000}Disabled");
					else
						Drawer::AddQueue("Control {FF00FF00}Enabled");
				}
				if(Keys('5').ConsumePressed())
				{
					Data.Saver ^= 1;
					if(!Data.Saver)
						Drawer::AddQueue("Saver {FFFF0000}Disabled");
					else
						Drawer::AddQueue("Saver {FF00FF00}Enabled");
				}
				if(Keys('6').ConsumePressed())
				{
					Data.DriftAnywhere ^= 1;
					if(!Data.DriftAnywhere)
						Drawer::AddQueue("DriftAnywhere {FFFF0000}Disabled");
					else
						Drawer::AddQueue("DriftAnywhere {FF00FF00}Enabled");
				}
				if(Keys('7').ConsumePressed())
				{
					Data.MoneyHack ^= 1;
					if(!Data.MoneyHack)
						Drawer::AddQueue("MoneyHack {FFFF0000}Disabled");
					else
						Drawer::AddQueue("MoneyHack {FF00FF00}Enabled");
				}
				if(Keys('8').ConsumePressed())
				{
					Data.NoEngineHeat ^= 1;
					if(!Data.NoEngineHeat)
						Drawer::AddQueue("NoEngineHeat {FFFF0000}Disabled");
					else
						Drawer::AddQueue("NoEngineHeat {FF00FF00}Enabled");
				}
				if(Keys('9').ConsumePressed())
				{
					Data.ReloadNitro ^= 1;
					if(!Data.ReloadNitro)
						Drawer::AddQueue("ReloadNitro {FFFF0000}Disabled");
					else
						Drawer::AddQueue("ReloadNitro {FF00FF00}Enabled");
				}
				if(Keys(VK_OEM_4).ConsumeDown())//[{
				{
					Keys(VK_OEM_4).ConsumePressed();
					Data.RainAmount -= 0.005;
					if(Data.RainAmount < 0.0f) Data.RainAmount = 0.000000f;
					Drawer::AddQueue(string_format("Rain amount: {FF00FF00}%.3f",Data.RainAmount).c_str());
				}
				if(Keys(VK_OEM_6).ConsumeDown())//}]
				{
					Keys(VK_OEM_6).ConsumePressed();
					Data.RainAmount += 0.005;
					if(Data.RainAmount > 1.0f) Data.RainAmount = 1.000000f;
					Drawer::AddQueue(string_format("Rain amount: {FF00FF00}%.3f",Data.RainAmount).c_str());
				}
				if(Keys(VK_OEM_MINUS).ConsumeDown())
				{
					Keys(VK_OEM_MINUS).ConsumePressed();
					auto& FOV = POINTER(float,0x0078721C);
					FOV -= 0.025f;
				}
				if(Keys(VK_OEM_PLUS).ConsumeDown())
				{
					Keys(VK_OEM_PLUS).ConsumePressed();
					auto& FOV = POINTER(float,0x0078721C);
					FOV += 0.025f;
				}
				if(Keys(VK_HOME).ConsumePressed())
				{
					User::Settings::Data.LiveReplaySend ^= 1;
					if(User::Settings::Data.LiveReplaySend)
					{
						if(Recorder::Live::Sender.Initialize())
						{
							
							Recorder::Live::Sender.SetReceiveTimeout(0,15000);//15 ms timeout
							Recorder::Live::Sender.SetSendTimeout(0,15000);//15 ms timeout
							if(Recorder::Live::Sender.Open((const uint8 *)"BLANA",0xDEAD))
							{
								if(Recorder::TryRecord())
								{
									Drawer::AddQueue("Client {FF00FF00}Live Replay - Enabled");
								}
								else
								{
									Drawer::AddQueue("Recorder {FFFF0000}cannot be enabled");
									Recorder::Live::Sender.Close();
								}	
							}
							else
							{
								User::Settings::Data.LiveReplayReceive ^= 1;
								Drawer::AddQueue("Client {FFFF0000}Live Replay CANNOT be enabled {FFFFFFFF}- Cannot Open socket");	
							}
						}
						else
						{
							User::Settings::Data.LiveReplayReceive ^= 1;
							Drawer::AddQueue("Client {FFFF0000}Live Replay CANNOT be enabled {FFFFFFFF}- Cannot Initialize socket");	
						}
					}
					else
					{
						Recorder::Stop();
						Recorder::Live::Sender.Close();
						Drawer::AddQueue("Client {FFFF0000}Live Replay - Disabled");
					}
				}
				if(Keys(VK_END).ConsumePressed())
				{
					User::Settings::Data.LiveReplayReceive ^= 1;
					if(User::Settings::Data.LiveReplayReceive)
					{
						if(Recorder::Live::Receiver.Initialize())
						{
							Recorder::Live::Receiver.SetReceiveTimeout(0,15000);//15 ms timeout
							Recorder::Live::Receiver.SetSendTimeout(0,15000);//15 ms timeout
							if(Recorder::Live::Receiver.Listen((const uint8 *)"0.0.0.0",0xDEAD))
							{
								if(Recorder::TryReplay())
								{
									Drawer::AddQueue("Server {FF00FF00}Live Replay - Enabled");
								}
								else
								{
									Drawer::AddQueue("Replay {FFFF0000}cannot be enabled");	
									Recorder::Live::Sender.Close();
								}	
							}
							else
							{
								User::Settings::Data.LiveReplayReceive ^= 1;
								Drawer::AddQueue("Server {FFFF0000}Live Replay CANNOT be enabled {FFFFFFFF}- Cannot Listen socket");	
							}
						}
						else
						{
							User::Settings::Data.LiveReplayReceive ^= 1;
							Drawer::AddQueue("Server {FFFF0000}Live Replay CANNOT be enabled {FFFFFFFF}- Cannot Initialize socket");	
						}
					}
					else
					{
						if(Recorder::Live::pClient)
							Recorder::Live::pClient->Close();
						Recorder::Stop();
						Recorder::Live::Receiver.Close();
						Drawer::AddQueue("Server {FFFF0000}Live Replay - Disabled");
					}
				}
			}//ConsumeDown(VK_APPS)
		}
	};
};

namespace Recorder
{
	void ProcessRecord()
	{
		Current = std::chrono::high_resolution_clock::now()-StartTime;
		g_Temp.time = Current;
		PROTECT;//current shift
		auto& CurrentGear	= POINTER(int  ,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x38);
		auto& Nitro			= POINTER(int  ,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x41C);
		auto& RPM			= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x400);
		auto& TURBO			= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x20) + 0x9C);
		g_Temp.CurrentGear	= CurrentGear;
		g_Temp.Nitro = Nitro;
		g_Temp.RPM = RPM;
		g_Temp.TURBO = TURBO;
		UNPROTECT;
		PROTECT;
		g_Temp.Vehicle = *(VehicleInfo*)Vehicles[0];
		UNPROTECT;
		if(frames.empty())
		{
			for(unsigned int i = 0; i < 256; ++i)
			{
				if(Keys(i).Down)
					g_Temp.Keys[i].Pressed = true;
				else
					g_Temp.Keys[i].Released = true;
			}
		}
		else
		{
			for(unsigned int i = 0; i < 256; ++i)
				g_Temp.Keys[i] = Keys(i);
		}
		if(User::Settings::Data.LiveReplaySend)
		{
			Recorder::Live::Sender.Send((const uint8 *)&g_Temp,sizeof(Recorder::FrameInfo));
		}
		else
			frames.push_back(g_Temp);
		return;
	}

	void ResetFrames()
	{
		frames.clear();
		if(State == 2)
			State = 0;
	}

	void Stop()
	{
		if(State == 1)
		{
			//History.push_back(frames);
			lastreplay = SaveReplay(frames);
		}
		else if(State == 2)
		{
			for(unsigned int i = 0; i < 256; ++i)
			{
				if(VK_DI_KEYTABLE.find(i) == VK_DI_KEYTABLE.end())
					continue;
				KEYBOARDINPUT.type = INPUT_KEYBOARD;
				KEYBOARDINPUT.ki.wScan = VK_DI_KEYTABLE[i]; // hardware scan code for key
				KEYBOARDINPUT.ki.wVk = 0; // virtual-key code for the  key
				KEYBOARDINPUT.ki.time = 0;
				KEYBOARDINPUT.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
				SendInput(1, &KEYBOARDINPUT, sizeof(INPUT));	
			}
			frames.clear();
		}
		State = 0;
	}

	void ProcessPlay(bool Simple = true,bool SuperSimple = false,bool NoKeys = false)
	{
		if(!User::Settings::Data.LiveReplayReceive)
		{
			if(!frames.empty())
			{
				Current = std::chrono::high_resolution_clock::now()-StartTime;
				if(Current >= frames.front().time)
				{
					if(!SuperSimple)
					{
						PROTECT;//current shift
						auto& CurrentGear	= POINTER(int  ,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x38);
						auto& Nitro			= POINTER(int  ,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x41C);
						auto& RPM			= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x400);
						auto& TURBO			= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x20) + 0x9C);
						TURBO = frames.front().TURBO;
						CurrentGear = frames.front().CurrentGear;
						Nitro = frames.front().Nitro;
						RPM = frames.front().RPM;
						UNPROTECT;
					}
					PROTECT;	
					#define LRECORDASSIGN(x) Vehicles[0]->x = frames.front().Vehicle.x
					if(!NoKeys)
					{
						for(int i = 0; i < 256; ++i)
						{
							if(VK_DI_KEYTABLE.find(i) == VK_DI_KEYTABLE.end())
								continue;
							if(frames.front().Keys[i].Pressed)
							{
								KEYBOARDINPUT.type = INPUT_KEYBOARD;
								KEYBOARDINPUT.ki.wScan = VK_DI_KEYTABLE[i]; // hardware scan code for key
								KEYBOARDINPUT.ki.wVk = 0; // virtual-key code for the  key
								KEYBOARDINPUT.ki.time = 0;
								KEYBOARDINPUT.ki.dwFlags = KEYEVENTF_SCANCODE;
								SendInput(1, &KEYBOARDINPUT, sizeof(INPUT));
							}
							else if(frames.front().Keys[i].Released)
							{
								KEYBOARDINPUT.type = INPUT_KEYBOARD;
								KEYBOARDINPUT.ki.wScan = VK_DI_KEYTABLE[i]; // hardware scan code for key
								KEYBOARDINPUT.ki.wVk = 0; // virtual-key code for the  key
								KEYBOARDINPUT.ki.time = 0;
								KEYBOARDINPUT.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
								SendInput(1, &KEYBOARDINPUT, sizeof(INPUT));	
							}
						}
					}
					if(!Simple)
					{
						//LRECORDASSIGN(unknown_01[0]);LRECORDASSIGN(unknown_01[1]);LRECORDASSIGN(unknown_01[2]);LRECORDASSIGN(unknown_01[3]);
						//LRECORDASSIGN(unknown_01[4]);LRECORDASSIGN(unknown_01[5]);LRECORDASSIGN(unknown_01[6]);LRECORDASSIGN(unknown_01[7]);
						LRECORDASSIGN(unknown_02);LRECORDASSIGN(unknown_03.x);LRECORDASSIGN(unknown_03.y);LRECORDASSIGN(unknown_03.z);
						LRECORDASSIGN(unknown_04);LRECORDASSIGN(unknown_05);LRECORDASSIGN(unknown_06);
						//LRECORDASSIGN(unknown_07[0]);
						//LRECORDASSIGN(unknown_07[1]);LRECORDASSIGN(unknown_07[2]);LRECORDASSIGN(unknown_08.x);LRECORDASSIGN(unknown_08.y);
						//LRECORDASSIGN(unknown_08.z);LRECORDASSIGN(unknown_09);
						//LRECORDASSIGN(unknown_10[0].w);LRECORDASSIGN(unknown_10[0].x);
						//LRECORDASSIGN(unknown_10[0].y);LRECORDASSIGN(unknown_10[0].z);LRECORDASSIGN(unknown_10[1].w);LRECORDASSIGN(unknown_10[1].x);
						//LRECORDASSIGN(unknown_10[1].y);LRECORDASSIGN(unknown_10[1].z);LRECORDASSIGN(unknown_10[2].w);LRECORDASSIGN(unknown_10[2].x);
						//LRECORDASSIGN(unknown_10[2].y);LRECORDASSIGN(unknown_10[2].z);LRECORDASSIGN(unknown_10[3].w);LRECORDASSIGN(unknown_10[3].x);
						//LRECORDASSIGN(unknown_10[3].y);LRECORDASSIGN(unknown_10[3].z);LRECORDASSIGN(unknown_10[4].w);LRECORDASSIGN(unknown_10[4].x);
						//LRECORDASSIGN(unknown_10[4].y);LRECORDASSIGN(unknown_10[4].z);LRECORDASSIGN(unknown_12_is_zero_when_accelerating);
					}
					LRECORDASSIGN(SpeedAndBrakeAccelerator);LRECORDASSIGN(Pos.x);LRECORDASSIGN(Pos.y);LRECORDASSIGN(Pos.z);LRECORDASSIGN(Velocity.x);
					LRECORDASSIGN(Velocity.y);LRECORDASSIGN(Velocity.z);LRECORDASSIGN(TurnSpeed.x);LRECORDASSIGN(TurnSpeed.y);LRECORDASSIGN(TurnSpeed.z);
					LRECORDASSIGN(Rotation.Front.w);LRECORDASSIGN(Rotation.Front.x);LRECORDASSIGN(Rotation.Front.y);LRECORDASSIGN(Rotation.Front.z);
					LRECORDASSIGN(Rotation.Right.w);LRECORDASSIGN(Rotation.Right.x);LRECORDASSIGN(Rotation.Right.y);LRECORDASSIGN(Rotation.Right.z);
					LRECORDASSIGN(Rotation.Up.w);LRECORDASSIGN(Rotation.Up.x);LRECORDASSIGN(Rotation.Up.y);LRECORDASSIGN(Rotation.Up.z);	
					UNPROTECT;
					frames.pop_front();
				}
				else
				{
					while(frames.front().time < Current)
					{
						frames.pop_front();
						if(frames.empty())
						{
							State = 0;
							break;
						}
					}
				}
			}
			else
			{
				State = 0;
			}
		}
		else
		{
			if(Recorder::Live::pClient == NULL)
			{
				Recorder::Live::pClient = Recorder::Live::Receiver.Accept();
			}
			else
            {
				switch(Recorder::Live::pClient->Receive(sizeof(Recorder::FrameInfo)))
				{
					case 0:
					{
						Recorder::Live::pClient = NULL;
						break;
					}
					case -1:
					{
						break;
					}
					default:
					{
						//std::stringstream received;
						//received << (const char *)Recorder::Live::pClient->GetData();
						//received >> g_Temp;
						g_Temp = (*(Recorder::FrameInfo*)Recorder::Live::pClient->GetData());
						if(!SuperSimple)
						{
							PROTECT;//current shift
							auto& CurrentGear	= POINTER(int  ,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x38);
							auto& Nitro			= POINTER(int  ,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x41C);
							auto& RPM			= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x400);
							auto& TURBO			= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x20) + 0x9C);
							TURBO = g_Temp.TURBO;
							CurrentGear = g_Temp.CurrentGear;
							Nitro = g_Temp.Nitro;
							RPM = g_Temp.RPM;
							UNPROTECT;
						}
						PROTECT;	
						#define AXLRECORDASSIGN(x) Vehicles[0]->x = g_Temp.Vehicle.x
						if(!NoKeys)
						{
							for(int i = 0; i < 256; ++i)
							{
								if(VK_DI_KEYTABLE.find(i) == VK_DI_KEYTABLE.end())
									continue;
								if(g_Temp.Keys[i].Pressed)
								{
									KEYBOARDINPUT.type = INPUT_KEYBOARD;
									KEYBOARDINPUT.ki.wScan = VK_DI_KEYTABLE[i]; // hardware scan code for key
									KEYBOARDINPUT.ki.wVk = 0; // virtual-key code for the  key
									KEYBOARDINPUT.ki.time = 0;
									KEYBOARDINPUT.ki.dwFlags = KEYEVENTF_SCANCODE;
									SendInput(1, &KEYBOARDINPUT, sizeof(INPUT));
								}
								else if(g_Temp.Keys[i].Released)
								{
									KEYBOARDINPUT.type = INPUT_KEYBOARD;
									KEYBOARDINPUT.ki.wScan = VK_DI_KEYTABLE[i]; // hardware scan code for key
									KEYBOARDINPUT.ki.wVk = 0; // virtual-key code for the  key
									KEYBOARDINPUT.ki.time = 0;
									KEYBOARDINPUT.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
									SendInput(1, &KEYBOARDINPUT, sizeof(INPUT));	
								}
							}
						}
						if(!Simple)
						{
							//LRECORDASSIGN(unknown_01[0]);LRECORDASSIGN(unknown_01[1]);LRECORDASSIGN(unknown_01[2]);LRECORDASSIGN(unknown_01[3]);
							//LRECORDASSIGN(unknown_01[4]);LRECORDASSIGN(unknown_01[5]);LRECORDASSIGN(unknown_01[6]);LRECORDASSIGN(unknown_01[7]);
							AXLRECORDASSIGN(unknown_02);AXLRECORDASSIGN(unknown_03.x);AXLRECORDASSIGN(unknown_03.y);AXLRECORDASSIGN(unknown_03.z);
							AXLRECORDASSIGN(unknown_04);AXLRECORDASSIGN(unknown_05);AXLRECORDASSIGN(unknown_06);
							//LRECORDASSIGN(unknown_07[0]);
							//LRECORDASSIGN(unknown_07[1]);LRECORDASSIGN(unknown_07[2]);LRECORDASSIGN(unknown_08.x);LRECORDASSIGN(unknown_08.y);
							//LRECORDASSIGN(unknown_08.z);LRECORDASSIGN(unknown_09);
							//LRECORDASSIGN(unknown_10[0].w);LRECORDASSIGN(unknown_10[0].x);
							//LRECORDASSIGN(unknown_10[0].y);LRECORDASSIGN(unknown_10[0].z);LRECORDASSIGN(unknown_10[1].w);LRECORDASSIGN(unknown_10[1].x);
							//LRECORDASSIGN(unknown_10[1].y);LRECORDASSIGN(unknown_10[1].z);LRECORDASSIGN(unknown_10[2].w);LRECORDASSIGN(unknown_10[2].x);
							//LRECORDASSIGN(unknown_10[2].y);LRECORDASSIGN(unknown_10[2].z);LRECORDASSIGN(unknown_10[3].w);LRECORDASSIGN(unknown_10[3].x);
							//LRECORDASSIGN(unknown_10[3].y);LRECORDASSIGN(unknown_10[3].z);LRECORDASSIGN(unknown_10[4].w);LRECORDASSIGN(unknown_10[4].x);
							//LRECORDASSIGN(unknown_10[4].y);LRECORDASSIGN(unknown_10[4].z);LRECORDASSIGN(unknown_12_is_zero_when_accelerating);
						}
						AXLRECORDASSIGN(SpeedAndBrakeAccelerator);AXLRECORDASSIGN(Pos.x);AXLRECORDASSIGN(Pos.y);AXLRECORDASSIGN(Pos.z);AXLRECORDASSIGN(Velocity.x);
						AXLRECORDASSIGN(Velocity.y);AXLRECORDASSIGN(Velocity.z);AXLRECORDASSIGN(TurnSpeed.x);AXLRECORDASSIGN(TurnSpeed.y);AXLRECORDASSIGN(TurnSpeed.z);
						AXLRECORDASSIGN(Rotation.Front.w);AXLRECORDASSIGN(Rotation.Front.x);AXLRECORDASSIGN(Rotation.Front.y);AXLRECORDASSIGN(Rotation.Front.z);
						AXLRECORDASSIGN(Rotation.Right.w);AXLRECORDASSIGN(Rotation.Right.x);AXLRECORDASSIGN(Rotation.Right.y);AXLRECORDASSIGN(Rotation.Right.z);
						AXLRECORDASSIGN(Rotation.Up.w);AXLRECORDASSIGN(Rotation.Up.x);AXLRECORDASSIGN(Rotation.Up.y);AXLRECORDASSIGN(Rotation.Up.z);	
						UNPROTECT;
						break;
					}
				}
			}
		}
	}

	bool TryRecord()
	{
		if(State != 0)
			return false;
		ResetFrames();
		StartTime = std::chrono::high_resolution_clock::now();
		State = 1;
		return true;
	}

	bool TryReplay()
	{	
		if(State != 0)
			return false;
		if(!User::Settings::Data.LiveReplayReceive)
			if(lastreplay.length() > 13)
				if(!ReadReplay(lastreplay,frames))
					return false;
		StartTime = std::chrono::high_resolution_clock::now();
		State = 2;
		return true;
	}

	bool SelectNextReplay()
	{
		if(hFind == INVALID_HANDLE_VALUE)
		{
			hFind = FindFirstFile(".\\NFSMOD\\Replays\\*.rec", &ffd);
			if(hFind == INVALID_HANDLE_VALUE)
			{
				lastreplay.assign("");
				return false;
			}
			lastreplay.assign(string_format(".\\NFSMOD\\Replays\\%s",ffd.cFileName));
			return true;
		}
		else
		{
			if(FindNextFile(hFind, &ffd) != 0)
			{
				lastreplay.assign(string_format(".\\NFSMOD\\Replays\\%s",ffd.cFileName));
				return true;
			}
			FindClose(hFind);
			hFind = INVALID_HANDLE_VALUE;
			return false;
		}
	}

	std::string SaveReplay(std::deque<FrameInfo> &frames_to_save)
	{
		std::ofstream myfile;
		time_t t = time(0);
		struct tm * now = localtime( & t );
		srand(abs((long)GetTickCount()));
		std::string filename(string_format(".\\NFSMOD\\Replays\\NFS-%02d-%02d-%02d-%02d-%02d-%02d-%09d.rec",now->tm_year-100,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec,rand()).c_str());
		myfile.open (filename.c_str(),std::ios::binary);
		if(myfile.good())
		{
			for(unsigned int i = 0,j = frames_to_save.size(); i < j; ++i)
			{
				myfile << frames_to_save.at(i);
			}
			myfile.close();
			return filename;
		}
		return std::string("");
	}

	bool ReadReplay(std::string filename,std::deque<FrameInfo> &RRframes)
	{
		std::ifstream myfile;
		myfile.open (filename.c_str(),std::ios::binary);	
		if(myfile.good())
		{
			if(!RRframes.empty())
				RRframes.clear();
			while (true) 
			{
				if( myfile.eof() ) break;
				myfile >> g_Temp;
				RRframes.push_back(g_Temp);
			}
			myfile.close();
			return true;
		}
		return false;
	}
};

//crash prevention
bool ValidVehicle(VehicleInfo *&vehicle)
{
	if(vehicle)
	{
		if(!IsBadWritePtr(&vehicle->Pos.x,12))//make sure XYZ pos is WRITEABLE
		{
			if(!IsBadReadPtr(&vehicle->Pos.x,12))//make sure XYZ pos is READABLE
			{
				if(vehicle->Pos.x != NULL)//make sure the XYZ pos actually does exist in the world
				{
					return true;
				}
			}
		}
	}
	return false;
}

//true if some vehicles found, false if pool cannot be populated
bool PopulateVehiclePool()
{
	ValidVehicles.clear();
	for(int i = 0; i < NFS_MAX_VEHICLES; ++i)//populate vehicle pool
	{
		PROTECT;
		if(!POINTER(unsigned int,0x0089CDA8 + (i*4)))
			continue;
		if(!POINTER(unsigned int,(POINTER(unsigned int,0x0089CDA8 + (i*4)) + 0x20)))
			continue;
		if(!&POINTER(VehicleInfo,POINTER(unsigned int,(POINTER(unsigned int,0x0089CDA8 + (i*4)) + 0x20))))
			continue;
		Vehicles[i] = &POINTER(VehicleInfo,POINTER(unsigned int,(POINTER(unsigned int,0x0089CDA8 + (i*4)) + 0x20)));
		if(ValidVehicle(Vehicles[i]))
		{
			ValidVehicles.push_back(i);
		}
		UNPROTECT;
	}
	return !ValidVehicles.empty();
}

void FrameTick(IDirect3DDevice9 * device, HWND wnd)
{
	KeyManagerRun();//key press/release detection
	
	User::Settings::CheckChange();
	Drawer::Display();

	if(!PopulateVehiclePool())//try to find valid vehicles
	{
		DirectXFont::Access(0)->Print(0.0,0.0,0xFFFFFFFF,"{FFFF0000}NFS {FFFFFF00}MOD {FF00FFFF}1.0 {FFFFFFFF}By Gamer_Z a.k.a Grasmanek94 | {FFFFFF00}Website: {FF00FFFF}http://gpb.googlecode.com/",true);
		return;//no valid vehicles found so end the execution here
	}

	if(User::Settings::Data.Recordings)
	{
		switch(Recorder::State)
		{
			case 0:
			{
				if(Keys(VK_F5).ConsumePressed())
				{
					if(Recorder::TryRecord())
						Drawer::AddQueue("Recorder {FF00FF00}Enabled");	
					else
						Drawer::AddQueue("Recorder {FFFF0000}cannot be enabled");
				}
				if(Keys(VK_F6).ConsumePressed())
				{
					if(Recorder::TryReplay())
						Drawer::AddQueue("Replay {FF00FF00}Enabled");	
					else
						Drawer::AddQueue("Replay {FFFF0000}cannot be enabled");				
				}
				if(Keys(VK_F7).ConsumePressed())
				{
					if(Recorder::SelectNextReplay())
						Drawer::AddQueue(string_format("Selected Replay: {FF00FF00}%s",Recorder::lastreplay.c_str()),3000);	
					else
						Drawer::AddQueue("{FFFF0000}Last Recorded Replay Selected");				
				}
				break;
			}
			case 1://recording
			{
				Recorder::ProcessRecord();
				if(User::Settings::Data.LiveReplaySend)
					DirectXFont::Access(0)->Print(0.0,30.0,0xFFFF0000,"RECORDING (LIVE)");
				else
				{
					DirectXFont::Access(0)->Print(0.0,30.0,0xFFFF0000,string_format("RECORDING (%d frames recorded)",Recorder::frames.size()).c_str(),true);
					if(Keys(VK_F5).ConsumePressed())
					{
						Recorder::Stop();
						Drawer::AddQueue("Recorder {FFFF0000}Disabled");
					}
				}
				break;
			}
			case 2://playing
			{
				Recorder::ProcessPlay();
				if(User::Settings::Data.LiveReplayReceive)
					DirectXFont::Access(0)->Print(0.0,30.0,0xFF00FF00,"REPLAY (LIVE)");
				else		
				{
					DirectXFont::Access(0)->Print(0.0,30.0,0xFF00FF00,string_format("REPLAY (%d frames left)",Recorder::frames.size()).c_str(),true);
					if(Keys(VK_F6).ConsumePressed())
					{
						Recorder::Stop();
						Drawer::AddQueue("Replay {FFFF0000}Disabled");
					}
				}
				return;
			}
		}
	}

	if(!User::Settings::Data.Cheat)
		return;

	if(User::Settings::Data.SpeedHacks)
	{
		if(Keys(VK_SHIFT).Down)//extra speed/hack
		{
			Vehicles[PLAYER_VEHICLE]->Velocity.x *= 1.010f;
			Vehicles[PLAYER_VEHICLE]->Velocity.y *= 1.010f;
			Vehicles[PLAYER_VEHICLE]->Velocity.z *= 1.010f;
		}
		if(Keys(VK_OEM_3 /*backtick*/).Down)//slowdown '`'
		{
			Vehicles[PLAYER_VEHICLE]->Velocity.x /= 1.010f;
			Vehicles[PLAYER_VEHICLE]->Velocity.y /= 1.010f;
			Vehicles[PLAYER_VEHICLE]->Velocity.z /= 1.010f;
		}
		if(Keys(VK_TAB).Down)//super brake
		{
			Vehicles[PLAYER_VEHICLE]->Velocity.x = 0.0f;
			Vehicles[PLAYER_VEHICLE]->Velocity.y = 0.0f;
			Vehicles[PLAYER_VEHICLE]->Velocity.z = 0.0f;
		}
	}
	if(User::Settings::Data.Jumper)
	{
		if(Keys(VK_BACK).Pressed)//jump forward 20 units
		{
			float offX = 20.0f;float offY = 0.0f;float offZ = 0.0f;

			float x = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.x + offY * Vehicles[PLAYER_VEHICLE]->Rotation.Right.x + offZ * Vehicles[PLAYER_VEHICLE]->Rotation.Up.x + Vehicles[PLAYER_VEHICLE]->Pos.x;
			float y = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.y + offY * Vehicles[PLAYER_VEHICLE]->Rotation.Right.y + offZ * Vehicles[PLAYER_VEHICLE]->Rotation.Up.y + Vehicles[PLAYER_VEHICLE]->Pos.y;
			float z = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.z + offY * Vehicles[PLAYER_VEHICLE]->Rotation.Right.z + offZ * Vehicles[PLAYER_VEHICLE]->Rotation.Up.z + Vehicles[PLAYER_VEHICLE]->Pos.z;

			Vehicles[PLAYER_VEHICLE]->Pos.x = x;Vehicles[PLAYER_VEHICLE]->Pos.y = y;Vehicles[PLAYER_VEHICLE]->Pos.z = z;
		}
	}
	if(User::Settings::Data.Saver)
	{
		if(Keys(VK_F9).Pressed)
		{
			SAVEPlayerVehicleInfo();
			Drawer::AddQueue("Information {FF00FF00}saved");
		}
		if(Keys(VK_F11).Pressed)
		{
			LOADPlayerVehicleInfo();
			Drawer::AddQueue("Information {FF00FF00}loaded");
		}
	}
	if(User::Settings::Data.Control)
	{
		if(Keys('Q').Down)//teleport all here LOL
		{
			for(unsigned int i = 1, j = ValidVehicles.size(); i < j; ++i)
			{
				int vehicle = ValidVehicles.at(i);
				float offX = 7*i;float offY = 0.0f;float offZ = 0.0f;

				VehicleInfoAssign(Vehicles[vehicle],Vehicles[PLAYER_VEHICLE]);

				Vehicles[vehicle]->Pos.x = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.x + offY * Vehicles[PLAYER_VEHICLE]->Rotation.Right.x + offZ * Vehicles[PLAYER_VEHICLE]->Rotation.Up.x + Vehicles[PLAYER_VEHICLE]->Pos.x;
				Vehicles[vehicle]->Pos.y = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.y + offY * Vehicles[PLAYER_VEHICLE]->Rotation.Right.y + offZ * Vehicles[PLAYER_VEHICLE]->Rotation.Up.y + Vehicles[PLAYER_VEHICLE]->Pos.y;
				Vehicles[vehicle]->Pos.z = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.z + offY * Vehicles[PLAYER_VEHICLE]->Rotation.Right.z + offZ * Vehicles[PLAYER_VEHICLE]->Rotation.Up.z + Vehicles[PLAYER_VEHICLE]->Pos.z;
			}
		}
	}

	PROTECT;//nitro & RPM
	if(User::Settings::Data.ReloadNitro)
	{
		auto& Nitro			= POINTER(int,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x41C);
		Nitro = 50000;
	}
	auto& RPM			= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x34) + 0x400);
	UNPROTECT;	

	PROTECT;//RAIN
	if(User::Settings::Data.RainControl)
	{
		auto& RainLevel		= POINTER(float,POINTER(unsigned int,0x00832EB4) + 0x1F0);
		RainLevel = User::Settings::Data.RainAmount;
	}
	UNPROTECT;	
	
	PROTECT;//camera stuff
	auto& CamRot		= POINTER(Point4D,POINTER(unsigned int,POINTER(unsigned int,0x0086E6FC) + 0x40));
	auto& CamPos		= POINTER(Point3D,POINTER(unsigned int,0x0089CCF8) + 0x50);
	auto& CamOther1		= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,0x0086E6FC) + 0x38) + 0x00);
	auto& CamOther2		= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,0x0086E6FC) + 0x38) + 0x08);
	auto& CamOther3		= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,0x0086E6FC) + 0x38) + 0xB8);
	UNPROTECT;	

	PROTECT;//current shift
	auto& CurrentShift = POINTER(int,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x4C) + 0x38);
	UNPROTECT;

	PROTECT;//distance in run
	auto& DistBtwnCars	= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,0x0089CD04) + 0x1C) + 0x24);
	DistBtwnCars = 301.0f;
	UNPROTECT;

	PROTECT;//#drag race engine no blow hack
	if(User::Settings::Data.NoEngineHeat)
	{
		auto& EngineHeat	= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,0x0089CDA8) + 0x48) + 0x8C);
		EngineHeat = 0.0f;
	}
	UNPROTECT;

	PROTECT;//money hack
	if(User::Settings::Data.MoneyHack)
	{
		auto& money = POINTER(int,0x00861E74);
		money = 2000000000;
	}
	UNPROTECT;

	PROTECT;//#drifting vars
	if(User::Settings::Data.DriftAnywhere)
	{
		auto& curr_percent_traveled		= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,0x00890118) + 0xBD0) + 0x14);
		auto& curr_percent_traveled2	= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,0x00890118) + 0xBD0) + 0x20);
		curr_percent_traveled = 0.001f;
		curr_percent_traveled2 = 0.001f;
		auto& percent_traveled_to		= POINTER(float,POINTER(unsigned int,POINTER(unsigned int,POINTER(unsigned int,0x00890118) + 0xBD0) + 0x1D8) + 0x30);
		percent_traveled_to = 0.000f;
	}
	UNPROTECT;
	
	//draw
	//float bHeight = ScreenHeight/48;
	//float bWidth = ScreenWidth/10;
}


void InitializeDX(IDirect3DDevice9 * device)
{
	float SCR = (float)ScreenHeight;
	DirectXFont::Add("Lucida Console",(int)((SCR/*-12.0f*/)/48.0f),FW_BOLD);
	DirectXFont::InitializeAll();
}

void UninitializeDX(IDirect3DDevice9 * device)
{
	DirectXFont::InvalidateAll();
}

IDirect3D9 * WINAPI sys_Direct3DCreate9 ( UINT SDKVersion )
{
	pDirect3D9 = NULL;
	if ( process_init() )
		pDirect3D9 = new proxyIDirect3D9( orig_Direct3DCreate9(SDKVersion) );

	return pDirect3D9;
}

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

bool GameInternsInited = false;
void OnGameLaunch()
{
	if(GameInternsInited)
		return;

	GameInternsInited = true;

	if(User::Settings::Load())
		Drawer::AddQueue("Settings {FF00FF00}loaded");
	else
		Drawer::AddQueue("Settings {FFFF0000}cannot be loaded");

	VK_DI_KEYTABLE[VK_ESCAPE] = DIKEYBOARD_ESCAPE; //0x0401 //
	VK_DI_KEYTABLE['1'] = DIKEYBOARD_1; //0x0402 //
	VK_DI_KEYTABLE['2'] = DIKEYBOARD_2; //0x0403 //
	VK_DI_KEYTABLE['3'] = DIKEYBOARD_3; //0x0404 //
	VK_DI_KEYTABLE['4'] = DIKEYBOARD_4; //0x0405 //
	VK_DI_KEYTABLE['5'] = DIKEYBOARD_5; //0x0406 //
	VK_DI_KEYTABLE['6'] = DIKEYBOARD_6; //0x0407 //
	VK_DI_KEYTABLE['7'] = DIKEYBOARD_7; //0x0408 //
	VK_DI_KEYTABLE['8'] = DIKEYBOARD_8; //0x0409 //
	VK_DI_KEYTABLE['9'] = DIKEYBOARD_9; //0x040A //
	VK_DI_KEYTABLE['0'] = DIKEYBOARD_0; //0x040B //
	VK_DI_KEYTABLE[VK_OEM_MINUS] = DIKEYBOARD_MINUS; //0x040C // (* - on main keyboard *)
	VK_DI_KEYTABLE[VK_OEM_PLUS] = DIKEYBOARD_EQUALS; //0x040D //
	VK_DI_KEYTABLE[VK_BACK] = DIKEYBOARD_BACK; //0x040E // (* backspace *)
	VK_DI_KEYTABLE[VK_TAB] = DIKEYBOARD_TAB; //0x040F //
	VK_DI_KEYTABLE['Q'] = DIKEYBOARD_Q; //0x0410 //
	VK_DI_KEYTABLE['W'] = DIKEYBOARD_W; //0x0411 //
	VK_DI_KEYTABLE['E'] = DIKEYBOARD_E; //0x0412 //
	VK_DI_KEYTABLE['R'] = DIKEYBOARD_R; //0x0413 //
	VK_DI_KEYTABLE['T'] = DIKEYBOARD_T; //0x0414 //
	VK_DI_KEYTABLE['Y'] = DIKEYBOARD_Y; //0x0415 //
	VK_DI_KEYTABLE['U'] = DIKEYBOARD_U; //0x0416 //
	VK_DI_KEYTABLE['I'] = DIKEYBOARD_I; //0x0417 //
	VK_DI_KEYTABLE['O'] = DIKEYBOARD_O; //0x0418 //
	VK_DI_KEYTABLE['P'] = DIKEYBOARD_P; //0x0419 //
	VK_DI_KEYTABLE[VK_OEM_4] = DIKEYBOARD_LBRACKET; //0x041A //
	VK_DI_KEYTABLE[VK_OEM_6] = DIKEYBOARD_RBRACKET; //0x041B //
	VK_DI_KEYTABLE[VK_RETURN] = DIKEYBOARD_RETURN; //0x041C // (* Enter on main keyboard *)
	VK_DI_KEYTABLE[VK_LCONTROL] = DIKEYBOARD_LCONTROL; //0x041D //
	VK_DI_KEYTABLE['A'] = DIKEYBOARD_A; //0x041E //
	VK_DI_KEYTABLE['S'] = DIKEYBOARD_S; //0x041F //
	VK_DI_KEYTABLE['D'] = DIKEYBOARD_D; //0x0420 //
	VK_DI_KEYTABLE['F'] = DIKEYBOARD_F; //0x0421 //
	VK_DI_KEYTABLE['G'] = DIKEYBOARD_G; //0x0422 //
	VK_DI_KEYTABLE['H'] = DIKEYBOARD_H; //0x0423 //
	VK_DI_KEYTABLE['J'] = DIKEYBOARD_J; //0x0424 //
	VK_DI_KEYTABLE['K'] = DIKEYBOARD_K; //0x0425 //
	VK_DI_KEYTABLE['L'] = DIKEYBOARD_L; //0x0426 //
	VK_DI_KEYTABLE[VK_OEM_1] = DIKEYBOARD_SEMICOLON; //0x0427 //
	VK_DI_KEYTABLE[VK_OEM_7] = DIKEYBOARD_APOSTROPHE; //0x0428 //
	VK_DI_KEYTABLE[VK_OEM_3] = DIKEYBOARD_GRAVE; //0x0429 // (* accent grave *)
	VK_DI_KEYTABLE[VK_LSHIFT] = DIKEYBOARD_LSHIFT; //0x042A //
	VK_DI_KEYTABLE[VK_OEM_5] = DIKEYBOARD_BACKSLASH; //0x042B //
	VK_DI_KEYTABLE['Z'] = DIKEYBOARD_Z; //0x042C //
	VK_DI_KEYTABLE['X'] = DIKEYBOARD_X; //0x042D //
	VK_DI_KEYTABLE['C'] = DIKEYBOARD_C; //0x042E //
	VK_DI_KEYTABLE['V'] = DIKEYBOARD_V; //0x042F //
	VK_DI_KEYTABLE['B'] = DIKEYBOARD_B; //0x0430 //
	VK_DI_KEYTABLE['N'] = DIKEYBOARD_N; //0x0431 //
	VK_DI_KEYTABLE['M'] = DIKEYBOARD_M; //0x0432 //
	VK_DI_KEYTABLE[VK_OEM_COMMA] = DIKEYBOARD_COMMA; //0x0433 //
	VK_DI_KEYTABLE[VK_OEM_PERIOD] = DIKEYBOARD_PERIOD; //0x0434 // (* . on main keyboard *)
	VK_DI_KEYTABLE[VK_OEM_2] = DIKEYBOARD_SLASH; //0x0435 // (* / on main keyboard *)
	VK_DI_KEYTABLE[VK_RSHIFT] = DIKEYBOARD_RSHIFT; //0x0436 //
	VK_DI_KEYTABLE[VK_MULTIPLY] = DIKEYBOARD_MULTIPLY; //0x0437 // (* * on numeric keypad *)
	VK_DI_KEYTABLE[VK_LMENU] = DIKEYBOARD_LMENU; //0x0438 // (* left Alt *)
	VK_DI_KEYTABLE[VK_SPACE] = DIKEYBOARD_SPACE; //0x0439 //
	VK_DI_KEYTABLE[VK_CAPITAL] = DIKEYBOARD_CAPITAL; //0x043A //
	VK_DI_KEYTABLE[VK_F1] = DIKEYBOARD_F1; //0x043B //
	VK_DI_KEYTABLE[VK_F2] = DIKEYBOARD_F2; //0x043C //
	VK_DI_KEYTABLE[VK_F3] = DIKEYBOARD_F3; //0x043D //
	VK_DI_KEYTABLE[VK_F4] = DIKEYBOARD_F4; //0x043E //
	VK_DI_KEYTABLE[VK_F5] = DIKEYBOARD_F5; //0x043F //
	VK_DI_KEYTABLE[VK_F6] = DIKEYBOARD_F6; //0x0440 //
	VK_DI_KEYTABLE[VK_F7] = DIKEYBOARD_F7; //0x0441 //
	VK_DI_KEYTABLE[VK_F8] = DIKEYBOARD_F8; //0x0442 //
	VK_DI_KEYTABLE[VK_F9] = DIKEYBOARD_F9; //0x0443 //
	VK_DI_KEYTABLE[VK_F10] = DIKEYBOARD_F10; //0x0444 //
	VK_DI_KEYTABLE[VK_NUMLOCK] = DIKEYBOARD_NUMLOCK; //0x0445 //
	VK_DI_KEYTABLE[VK_SCROLL] = DIKEYBOARD_SCROLL; //0x0446 // (* Scroll Lock *)
	VK_DI_KEYTABLE[VK_NUMPAD7] = DIKEYBOARD_NUMPAD7; //0x0447 //
	VK_DI_KEYTABLE[VK_NUMPAD8] = DIKEYBOARD_NUMPAD8; //0x0448 //
	VK_DI_KEYTABLE[VK_NUMPAD9] = DIKEYBOARD_NUMPAD9; //0x0449 //
	VK_DI_KEYTABLE[VK_SUBTRACT] = DIKEYBOARD_SUBTRACT; //0x044A // (* - on numeric keypad *)
	VK_DI_KEYTABLE[VK_NUMPAD4] = DIKEYBOARD_NUMPAD4; //0x044B //
	VK_DI_KEYTABLE[VK_NUMPAD5] = DIKEYBOARD_NUMPAD5; //0x044C //
	VK_DI_KEYTABLE[VK_NUMPAD6] = DIKEYBOARD_NUMPAD6; //0x044D //
	VK_DI_KEYTABLE[VK_ADD] = DIKEYBOARD_ADD; //0x044E // (* + on numeric keypad *)
	VK_DI_KEYTABLE[VK_NUMPAD1] = DIKEYBOARD_NUMPAD1; //0x044F //
	VK_DI_KEYTABLE[VK_NUMPAD2] = DIKEYBOARD_NUMPAD2; //0x0450 //
	VK_DI_KEYTABLE[VK_NUMPAD3] = DIKEYBOARD_NUMPAD3; //0x0451 //
	VK_DI_KEYTABLE[VK_NUMPAD0] = DIKEYBOARD_NUMPAD0; //0x0452 //
	VK_DI_KEYTABLE[VK_OEM_COMMA] = DIKEYBOARD_DECIMAL; //0x0453 // (* . on numeric keypad *)
	VK_DI_KEYTABLE[VK_OEM_102] = DIKEYBOARD_OEM_102; //0x0456 // (* < > | on UK/Germany keyboards *)
	VK_DI_KEYTABLE[VK_F11] = DIKEYBOARD_F11; //0x0457 //
	VK_DI_KEYTABLE[VK_F12] = DIKEYBOARD_F12; //0x0458 //
	VK_DI_KEYTABLE[VK_F13] = DIKEYBOARD_F13; //0x0464 // (* (NEC PC98) *)
	VK_DI_KEYTABLE[VK_F14] = DIKEYBOARD_F14; //0x0465 // (* (NEC PC98) *)
	VK_DI_KEYTABLE[VK_F15] = DIKEYBOARD_F15; //0x0466 // (* (NEC PC98) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_KANA; //0x0470 // (* (Japanese keyboard) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_ABNT_C1; //0x0473 // (* / ? on Portugese (Brazilian) keyboards *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_CONVERT; //0x0479 // (* (Japanese keyboard) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_NOCONVERT; //0x047B // (* (Japanese keyboard) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_YEN; //0x047D // (* (Japanese keyboard) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_ABNT_C2; //0x047E // (* Numpad . on Portugese (Brazilian) keyboards *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_NUMPADEQUALS; //0x048D // (* = on numeric keypad (NEC PC98) *)
	VK_DI_KEYTABLE[VK_MEDIA_PREV_TRACK] = DIKEYBOARD_PREVTRACK; //0x0490 // (* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_AT; //0x0491 // (* (NEC PC98) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_COLON; //0x0492 // (* (NEC PC98) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_UNDERLINE; //0x0493 // (* (NEC PC98) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_KANJI; //0x0494 // (* (Japanese keyboard) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_STOP; //0x0495 // (* (NEC PC98) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_AX; //0x0496 // (* (Japan AX) *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_UNLABELED; //0x0497 // (* (J3100) *)
	VK_DI_KEYTABLE[VK_MEDIA_NEXT_TRACK] = DIKEYBOARD_NEXTTRACK; //0x0499 // (* Next Track *)
	VK_DI_KEYTABLE[VK_RETURN] = DIKEYBOARD_NUMPADENTER; //0x049C // (* Enter on numeric keypad *)
	VK_DI_KEYTABLE[VK_RCONTROL] = DIKEYBOARD_RCONTROL; //0x049D //
	VK_DI_KEYTABLE[VK_VOLUME_MUTE] = DIKEYBOARD_MUTE; //0x04A0 // (* Mute *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_CALCULATOR; //0x04A1 // (* Calculator *)
	VK_DI_KEYTABLE[VK_MEDIA_PLAY_PAUSE] = DIKEYBOARD_PLAYPAUSE; //0x04A2 // (* Play / Pause *)
	VK_DI_KEYTABLE[VK_MEDIA_STOP] = DIKEYBOARD_MEDIASTOP; //0x04A4 // (* Media Stop *)
	VK_DI_KEYTABLE[VK_VOLUME_DOWN] = DIKEYBOARD_VOLUMEDOWN; //0x04AE // (* Volume - *)
	VK_DI_KEYTABLE[VK_VOLUME_UP] = DIKEYBOARD_VOLUMEUP; //0x04B0 // (* Volume + *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WEBHOME; //0x04B2 // (* Web home *)
	VK_DI_KEYTABLE[VK_DECIMAL] = DIKEYBOARD_NUMPADCOMMA; //0x04B3 // (* , on numeric keypad (NEC PC98) *)
	VK_DI_KEYTABLE[VK_DIVIDE] = DIKEYBOARD_DIVIDE; //0x04B5 // (* / on numeric keypad *)
	VK_DI_KEYTABLE[VK_SNAPSHOT] = DIKEYBOARD_SYSRQ; //0x04B7 //
	VK_DI_KEYTABLE[VK_RMENU] = DIKEYBOARD_RMENU; //0x04B8 // (* right Alt *)
	VK_DI_KEYTABLE[VK_PAUSE] = DIKEYBOARD_PAUSE; //0x04C5 // (* Pause *)
	VK_DI_KEYTABLE[VK_HOME] = DIKEYBOARD_HOME; //0x04C7 // (* Home on arrow keypad *)
	VK_DI_KEYTABLE[VK_UP] = DIKEYBOARD_UP; //0x04C8 // (* UpArrow on arrow keypad *)
	VK_DI_KEYTABLE[VK_PRIOR] = DIKEYBOARD_PRIOR; //0x04C9 // (* PgUp on arrow keypad *)
	VK_DI_KEYTABLE[VK_LEFT] = DIKEYBOARD_LEFT; //0x04CB // (* LeftArrow on arrow keypad *)
	VK_DI_KEYTABLE[VK_RIGHT] = DIKEYBOARD_RIGHT; //0x04CD // (* RightArrow on arrow keypad *)
	VK_DI_KEYTABLE[VK_END] = DIKEYBOARD_END; //0x04CF // (* End on arrow keypad *)
	VK_DI_KEYTABLE[VK_DOWN] = DIKEYBOARD_DOWN; //0x04D0 // (* DownArrow on arrow keypad *)
	VK_DI_KEYTABLE[VK_NEXT] = DIKEYBOARD_NEXT; //0x04D1 // (* PgDn on arrow keypad *)
	VK_DI_KEYTABLE[VK_INSERT] = DIKEYBOARD_INSERT; //0x04D2 // (* Insert on arrow keypad *)
	VK_DI_KEYTABLE[VK_DELETE] = DIKEYBOARD_DELETE; //0x04D3 // (* Delete on arrow keypad *)
	VK_DI_KEYTABLE[VK_LWIN] = DIKEYBOARD_LWIN; //0x04DB // (* Left Windows key *)
	VK_DI_KEYTABLE[VK_RWIN] = DIKEYBOARD_RWIN; //0x04DC // (* Right Windows key *)
	VK_DI_KEYTABLE[VK_APPS] = DIKEYBOARD_APPS; //0x04DD // (* AppMenu key *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_POWER; //0x04DE // (* System Power *)
	VK_DI_KEYTABLE[VK_SLEEP] = DIKEYBOARD_SLEEP; //0x04DF // (* System Sleep *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WAKE; //0x04E3 // (* System Wake *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WEBSEARCH; //0x04E5 // (* Web Search *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WEBFAVORITES; //0x04E6 // (* Web Favorites *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WEBREFRESH; //0x04E7 // (* Web Refresh *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WEBSTOP; //0x04E8 // (* Web Stop *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WEBFORWARD; //0x04E9 // (* Web Forward *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_WEBBACK; //0x04EA // (* Web Back *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_MYCOMPUTER; //0x04EB // (* My Computer *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_MAIL; //0x04EC // (* Mail *)
	//VK_DI_KEYTABLE[] = DIKEYBOARD_MEDIASELECT; //0x04ED // (* Media Select *)
	Drawer::AddQueue(" ",0);
}


	//float Angle = 0.0f;
	//{//calculate the cars' ZAngle
	//	float offX = 20.0f;
	//	float x = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.x + Vehicles[PLAYER_VEHICLE]->Pos.x;
	//	float y = offX * Vehicles[PLAYER_VEHICLE]->Rotation.Front.y + Vehicles[PLAYER_VEHICLE]->Pos.y;
	//	Angle = (atan2(x-Vehicles[PLAYER_VEHICLE]->Pos.x, y-Vehicles[PLAYER_VEHICLE]->Pos.y) * 180.0f / PI);
	//}

	/*
	static bool airbreak = false;
	if(Keys(VK_F4).Pressed)
	{
		if(airbreak)
			airbreak = false;
		else
			airbreak = true;
	}

	if(airbreak)
	{
		if ( Keys('W').Down )d[0] += 1.0f;
		if ( Keys('S').Down )d[0] -= 1.0f;
		if ( Keys('A').Down )d[1] += 1.0f;
		if ( Keys('D').Down )d[1] -= 1.0f;
		if ( Keys('Q').Down )d[2] += 1.0f;
		if ( Keys('Z').Down )d[2] -= 1.0f;
		// pitch (x-axis)
		if ( Keys('I').Down )xyvect[0] += 1.0f;
		if ( Keys('K').Down )xyvect[0] -= 1.0f;
		// roll (y-axis)
		if ( Keys('J').Down )xyvect[1] += 1.0f;
		if ( Keys('L').Down )xyvect[1] -= 1.0f;
		// yaw (z-axis)
		if ( Keys('U').Down )zvect[2] -= 1.0f;
		if ( Keys('O').Down )zvect[2] += 1.0f;
		//TODO: SET ROT AND POS
	}
	*/