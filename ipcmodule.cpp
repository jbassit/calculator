#include "stdafx.h"
#include "ipcmodule.h"
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <Windows.h>
#include <process.h>
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")
using namespace std;

const string WSA_START_ERROR = "Unable to initialize WSADATA object:  ";
const string GETADDRINFO_ERROR = "getaddinfo() failed: ";
const string SOCK_CREATION_ERROR = "Unable to create socket: ";
const string BIND_ERROR = "Unable to bind socket: ";
const string LISTEN_ERROR = "listen() failed: ";
const string SET_RECV_TO_ERROR = "Failed to set timeout for recv: ";
const string SET_SEND_TO_ERROR = "Failed to set timeout for send: ";
const string TIMEOUT_ERROR = "Error: connection timed out";
const string CONNECTION_LIMIT_REACHED = "Error: connection limit reached";
const string MAIN_THREAD_ERROR = "Failed to start main thread";
const string MUTEX_INIT_ERROR = "Failed to initalize mutex object";
const string RECV_TIMEDOUT = "recv() timedout";
const string RECV_ERROR = "Failed to recieve message";
const string WAIT_FOR_THREADS = "Waiting for threads to exit";
const string CLOSED_HANDLES = "Closed all handles";

map<string, regex> CalcIPC::settingsMap = CalcIPC::populateSettingsMap();

CalcIPC::CalcIPC()
	: m_Socket(INVALID_SOCKET), m_acceptThread(0), m_Stop(false)
{
	int errVal;

	for (DWORD i = 0; i < MAX_THREAD; i++) {
		m_threadList[i] = 0;
	}

	errVal = WSAStartup(MAKEWORD(2, 2), &m_WSAData);
	if (errVal != 0) {
		throw WSA_START_ERROR + to_string(errVal);
	}
	if (!InitializeCriticalSectionAndSpinCount(&m_Mutex, SPIN_COUNT)) {
		throw MUTEX_INIT_ERROR;
	}
}

CalcIPC::~CalcIPC() {
	for (DWORD i = 0; i < MAX_THREAD; i++) {
		if (m_threadList[i] != 0) {
			CloseHandle(m_threadList[i]);
		}
	}
	cout << CLOSED_HANDLES << endl;

	WSACleanup();
	CloseHandle(m_acceptThread);
	DeleteCriticalSection(&m_Mutex);
}

void CalcIPC::createSocket() {
	struct addrinfo *result = NULL, hints;
	int errVal;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	errVal = getaddrinfo(LOCAL_HOST.c_str(), DEFAULT_PORT.c_str(), &hints, &result);
	if (errVal != 0) {
		throw GETADDRINFO_ERROR + to_string(errVal);
	}

	m_Socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_Socket == INVALID_SOCKET) {
		freeaddrinfo(result);
		throw SOCK_CREATION_ERROR + to_string(WSAGetLastError());
	}

	errVal = bind(m_Socket, result->ai_addr, (int)result->ai_addrlen);
	if (errVal == SOCKET_ERROR) {
		closesocket(m_Socket);
		freeaddrinfo(result);
		throw BIND_ERROR + to_string(WSAGetLastError());
	}
	freeaddrinfo(result);

	if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char*)(&TIMEOUT), sizeof(DWORD)) == SOCKET_ERROR) {
		closesocket(m_Socket);
		throw SET_RECV_TO_ERROR + to_string(WSAGetLastError());
	}

	if (setsockopt(m_Socket, SOL_SOCKET, SO_SNDTIMEO, (char*)(&TIMEOUT), sizeof(DWORD)) == SOCKET_ERROR) {
		closesocket(m_Socket);
		throw SET_SEND_TO_ERROR + to_string(WSAGetLastError());
	}
}

unsigned int __stdcall CalcIPC::acceptConnections(LPVOID parent) {
	CalcIPC* obj = (CalcIPC*)parent;
	SOCKET clientSocket;
	HANDLE handleID;
	workerThreadArgs* args;
	long handleIndex;

	while (!obj->m_Stop) {
		clientSocket = INVALID_SOCKET;
		clientSocket = accept(obj->m_Socket, NULL, NULL);
		if (clientSocket != INVALID_SOCKET) {
			handleIndex = obj->selectHandle();
			if (handleIndex != -1) {
				args = new workerThreadArgs;
				args->obj = obj;
				args->sock = clientSocket;
				handleID = (HANDLE)_beginthreadex(NULL, 0, communicationThread, (LPVOID)args, 0, NULL);
				obj->updateThreadList(handleIndex, handleID);
				if (handleID == 0) {
					delete args;
				}
			}
			else {
				send(clientSocket, CONNECTION_LIMIT_REACHED.c_str(), CONNECTION_LIMIT_REACHED.size(), 0);
				shutdown(clientSocket, SD_SEND);
				closesocket(clientSocket);
			}
		}
	}

	cout << WAIT_FOR_THREADS << endl;
	obj->waitForThreadsToExit();

	return EXIT_SUCCESS;
}

void CalcIPC::exec() {
	string input;

	createSocket();
	if (listen(m_Socket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(m_Socket);
		throw LISTEN_ERROR + to_string(WSAGetLastError());
	}
	m_acceptThread = (HANDLE)_beginthreadex(NULL, 0, acceptConnections, (LPVOID)(this), 0, NULL);
	if (m_acceptThread == 0)
		throw MAIN_THREAD_ERROR;

	cout << "Enter \"" + EXIT1 + "\" or \"" + EXIT2 + "\" or \"" + EXIT3 + "\" to stop server" << endl;
	cout << "Enter \"" + HCOUNT + "\" for number of active handles" << endl;
	do {
		cout << "<< ";
		cin >> input;
		if (input == EXIT1 || input == EXIT2 || input == EXIT3)
			m_Stop = true;
		if (input == HCOUNT) {
			cout << ">> " << getActiveHandleCount() << endl;;
		}
	} while (!m_Stop);

	closesocket(m_Socket); // close socket to interrupt listening in main thread
	WaitForSingleObject(m_acceptThread, INFINITE);
}

unsigned int __stdcall CalcIPC::communicationThread(LPVOID args) {
	workerThreadArgs* temp = (workerThreadArgs*)args;
	SOCKET clientSocket = temp->sock;
	CalcIPC *parent = temp->obj;
	delete args;
	char inBuffer[BUFFER_SIZE + 1];
	string buffer, fullMessage;
	int bytesRecieved, sentBytes;
	calc *calculator = calc::access(); // thread safe in c11

	do {
		bytesRecieved = recv(clientSocket, inBuffer, BUFFER_SIZE, 0);

		if (bytesRecieved > 0) {
			inBuffer[bytesRecieved] = '\0';
			buffer = inBuffer;
			fullMessage += buffer;
		}
		else if (bytesRecieved < 0) {
			if (WSAGetLastError() == WSAETIMEDOUT) {
				send(clientSocket, TIMEOUT_ERROR.c_str(), TIMEOUT_ERROR.size(), 0);
				cout << RECV_TIMEDOUT << endl;
				shutdown(clientSocket, SD_SEND);
				closesocket(clientSocket);
			}
			else
				cout << RECV_ERROR << WSAGetLastError() << endl;

			closesocket(clientSocket);
			return EXIT_FAILURE;
		}
	} while (bytesRecieved > 0 && buffer[bytesRecieved - 1] != END_MSG);

	fullMessage = evaluate(fullMessage, calculator, parent); // evaluates request

	sentBytes = send(clientSocket, fullMessage.c_str(), fullMessage.size(), 0);

	if (sentBytes == SOCKET_ERROR) {
		closesocket(clientSocket);
		return EXIT_FAILURE;
	}

	shutdown(clientSocket, SD_SEND);
	closesocket(clientSocket);
	return EXIT_SUCCESS;
}

string CalcIPC::evaluate(string message, calc* calculator, CalcIPC* parent) {
	string expr;
	calc::format toVal, fromVal;
	calc::mode mode;
	smatch parsedResult;

	for (map<string, regex>::iterator it = settingsMap.begin(); it != settingsMap.end(); it++) {
		if (regex_search(message, parsedResult, it->second)) {
			if (it->first == EXPR_ATTR) {
				expr = parsedResult[1];
			}
			else if (it->first == MODE_ATTR) {
				if (parsedResult[1] == "Prefix")
					mode = calc::prefix;
				else if (parsedResult[1] == "Infix")
					mode = calc::infix;
				else if (parsedResult[1] == "Postfix")
					mode = calc::postfix;
				else if (parsedResult[1] == "Converter")
					mode = calc::converter;
			}
			else if (it->first == CONV_TO_ATTR || it->first == CONV_FROM_ATTR) {
				if (parsedResult[1] == "Bin")
					(it->first == CONV_TO_ATTR) ? toVal = calc::bin : fromVal = calc::bin;
				else if (parsedResult[1] == "Oct")
					(it->first == CONV_TO_ATTR) ? toVal = calc::oct : fromVal = calc::oct;
				else if (parsedResult[1] == "Dec")
					(it->first == CONV_TO_ATTR) ? toVal = calc::dec : fromVal = calc::dec;
				else if (parsedResult[1] == "Hex")
					(it->first == CONV_TO_ATTR) ? toVal = calc::hex : fromVal = calc::hex;
			}
		}
	}
	MyMutex lock(parent); // safe synchronization
	cerr << "Message received: " << message << endl;
	calculator->setMode(mode);
	calculator->setConvertTo(toVal);
	calculator->setConvertFrom(fromVal);
	expr = calculator->solve(expr);
	cerr << "Message to be sent: " << expr << endl;
	return expr;
}


void CalcIPC::updateThreadList(long index, const HANDLE &handle) {
	if (index < MAX_THREAD)
		m_threadList[index] = handle;
}

map<string, regex> CalcIPC::populateSettingsMap() {
	map<string, regex> temp;
	for (size_t i = 0; i < attributes.size(); i++) {
		temp[attributes[i]] = "\"" + attributes[i] + "\":\"([^\"]+)";
	}

	return temp;
}

long CalcIPC::selectHandle() {
	for (DWORD i = 0; i < MAX_THREAD; i++) {
		if (!m_threadList[i] || WaitForSingleObject(m_threadList[i], 0) == WAIT_OBJECT_0) { // checks to see if either handler is unsued or the thread it belongs to is in signaled state, ie free
			if (m_threadList[i]) { // close handle if thread is in signaled state
				CloseHandle(m_threadList[i]);
			}
			return i;
		}
	}
	return -1;
}

void CalcIPC::waitForThreadsToExit() const {
	for (DWORD i = 0; i < MAX_THREAD; i++) {
		if (m_threadList[i] != 0) {
			WaitForSingleObject(m_threadList[i], INFINITE);
		}
	}
}

DWORD CalcIPC::getActiveHandleCount() const {
	DWORD count = 0;
	for (DWORD i = 0; i < MAX_THREAD; i++) {
		if (m_threadList[i] != 0) {
			count++;
		}
	}
	return count;
}

MyMutex::MyMutex(CalcIPC* obj)
	: m_Mutex(&(obj->m_Mutex))
{
	EnterCriticalSection(m_Mutex);
}
MyMutex::~MyMutex() {
	LeaveCriticalSection(m_Mutex);
}