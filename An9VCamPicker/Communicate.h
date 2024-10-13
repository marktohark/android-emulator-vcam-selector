#pragma once

#include <shared_mutex>
#include <string>


#ifndef COMMUNICATE_H
#define COMMUNICATE_H



class Communicate {
private:
	std::string m_vcamName{""};
	std::shared_mutex m_rw_vcamName_mutex;

	std::mutex m_send_mutex;

	SOCKET m_socket{ INVALID_SOCKET };
	std::shared_mutex m_rw_socket_mutex;

	std::string m_packages{ "" };

public: 
	void Start();
	std::wstring GetVCamWName();
	std::string GetVCamName();
	void SetVCamName(std::string);
	void Send(std::string);
	std::string Recv();
	std::string Connect();
	void Close();

private:

};

#endif