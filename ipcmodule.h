#pragma once
#include "calculator.h"
#include <WinSock2.h>
#include <regex>
#include <vector>
#include <map>

const std::string DEFAULT_PORT = "6789";
const std::string LOCAL_HOST = "127.0.0.1";
const std::string EXPR_ATTR = "expr";
const std::string MODE_ATTR = "mode";
const std::string CONV_TO_ATTR = "to";
const std::string CONV_FROM_ATTR = "from";
const std::string EXIT1 = "q";
const std::string EXIT2 = "quit";
const std::string EXIT3 = "exit";
const std::string HCOUNT = "hcount";
const char END_MSG = '\0'; // character that signals end of message
const unsigned short BUFFER_SIZE = 2048;
const DWORD MAX_THREAD = 100;
const DWORD TIMEOUT = 10000; // set timeout to 10 seconds
const DWORD SPIN_COUNT = 1024;
const std::vector<std::string> attributes{ EXPR_ATTR, MODE_ATTR, CONV_TO_ATTR , CONV_FROM_ATTR };

class CalcIPC;
class MyMutex {
public:
	MyMutex(CalcIPC* obj);
	~MyMutex();

private:
	CRITICAL_SECTION* m_Mutex;
};

class CalcIPC {
	friend MyMutex;

public:
	CalcIPC();
	~CalcIPC();
	void exec();

private:
	static unsigned int __stdcall acceptConnections(LPVOID parent);
	static unsigned int __stdcall communicationThread(LPVOID args);
	static std::string evaluate(std::string message, calc* calculator, CalcIPC* parent);
	static std::map<std::string, std::regex> populateSettingsMap();

	void waitForThreadsToExit() const;
	DWORD getActiveHandleCount() const;
	void updateThreadList(long index, const HANDLE &handle);
	void createSocket();
	long selectHandle();

	WSAData m_WSAData;
	SOCKET m_Socket;
	HANDLE m_acceptThread;
	CRITICAL_SECTION m_Mutex;
	HANDLE m_threadList[MAX_THREAD];
	volatile bool m_Stop;

	static std::map<std::string, std::regex> settingsMap;
};

struct workerThreadArgs {
	SOCKET sock;
	CalcIPC* obj;
};