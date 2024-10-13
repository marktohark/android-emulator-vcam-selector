#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Communicate.h"
#include <fstream>
#include <format>

#pragma comment(lib, "ws2_32.lib")

DWORD WINAPI run(LPVOID l) {
	Communicate* _this = (Communicate*)l;
	
	while (true) {
		auto vcamName = _this->Recv();
		_this->SetVCamName(vcamName);
	}
	_this->Close();
	return 0;
}

void Communicate::Start() {
	CreateThread(0, 0, run, this, 0, 0);
}

std::wstring Communicate::GetVCamWName() {
	m_rw_vcamName_mutex.lock_shared();
	std::wstring r(m_vcamName.begin(), m_vcamName.end());
	m_rw_vcamName_mutex.unlock_shared();
	return r;
}

std::string Communicate::GetVCamName() {
	m_rw_vcamName_mutex.lock_shared();
	std::string r(m_vcamName);
	m_rw_vcamName_mutex.unlock_shared();
	return r;
}

void Communicate::SetVCamName(std::string name) {
	m_rw_vcamName_mutex.lock();
	m_vcamName = name;
	m_rw_vcamName_mutex.unlock();
}

void Communicate::Send(std::string bs) {
	SOCKET s = INVALID_SOCKET;
	m_rw_socket_mutex.lock_shared();
	s = m_socket;
	m_rw_socket_mutex.unlock_shared();
	if (s == INVALID_SOCKET) return;

	m_send_mutex.lock();
	send(s, bs.c_str(), bs.size(), 0);
	m_send_mutex.unlock();
}

std::string Communicate::Connect() {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		return "WSAStartup init failed(" + std::to_string(WSAGetLastError()) + ")";
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		WSACleanup();
		return "socket init failed(" + std::to_string(WSAGetLastError()) + ")";
	}
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8693);
	inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

	result = connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
	if (result == SOCKET_ERROR) {
		closesocket(clientSocket);
		WSACleanup();
		return "connect init failed(" + std::to_string(WSAGetLastError()) + ")";
	}
	m_rw_socket_mutex.lock();
	this->m_socket = clientSocket;
	m_rw_socket_mutex.unlock();

	// tell server who am I
	auto pid = GetCurrentProcessId();
	char pName[MAX_PATH] = { 0 };
	GetModuleBaseNameA(GetCurrentProcess(), 0, pName, MAX_PATH);
	if (strlen(pName) == 0) {
		this->Close();
		return "GetModuleBaseNameA Failed(" + std::to_string(GetLastError()) + ")";
	}
	this->Send(std::format("init;{};{};", pid, pName));
	return "";
}

void Communicate::Close() {
	if (this->m_socket != INVALID_SOCKET) {
		closesocket(this->m_socket);
		WSACleanup();
	}
}

std::string Communicate::Recv() {
	char bufr[512] = { 0 };
	while (true) {
		SOCKET s = INVALID_SOCKET;
		m_rw_socket_mutex.lock_shared();
		s = m_socket;
		m_rw_socket_mutex.unlock_shared();
		if (s == INVALID_SOCKET) {
			auto err = this->Connect();
			if (err != "") {
				Sleep(1000);
				continue;
			}
			m_rw_socket_mutex.lock_shared();
			s = m_socket;
			m_rw_socket_mutex.unlock_shared();
		}

		auto r = recv(s, bufr, 511, 0);
		if (r <= 0) {
			this->Close();
			m_rw_socket_mutex.lock();
			m_socket = INVALID_SOCKET;
			m_rw_socket_mutex.unlock();
			continue;
		}
		bufr[r] = '\0';
		m_packages += bufr;
		auto endPos = m_packages.find(";");
		if (endPos == std::string::npos) {
			continue;
		}

		std::string res = m_packages.substr(0, endPos);
		m_packages = m_packages.substr(endPos + 1, std::string::npos);
		


		return res;
	}
}