#include "pch.h"
#include "IpcClient.h"
#include <future>
#include <iostream>

#define BUFFSIZE			512*1024

#pragma comment(lib, "ws2_32.lib")

using namespace std;

CIpcClient::CIpcClient()
{
	InitializeCriticalSection(&m_CriticalSection);
}

CIpcClient::~CIpcClient()
{
	DeleteCriticalSection(&m_CriticalSection);

}

CIpcClient* CIpcClient::GetInstance()
{
	static CIpcClient instance;
	return &instance;
}

#pragma region 接口部分

DWORD WINAPI ThreadClientListen(LPVOID lpParam)
{
	CIpcClient::GetInstance()->ClientListenThread();
	return 0;
}

DWORD WINAPI ThreadProcessPakcet(LPVOID lpParam)
{
	CIpcClient::GetInstance()->OnTcpPacket();
	return 0;
}


bool CIpcClient::Start(string strIp, int nPort)
{
	m_strIp = strIp;
	m_nPort = nPort;
	CreateThread(NULL, 0, ThreadClientListen, NULL, 0, NULL);//(LPTHREAD_START_ROUTINE)
	CreateThread(NULL, 0, ThreadProcessPakcet, NULL, 0, NULL);
	return true;
}

void CIpcClient::Stop()
{

	cout << "Stop Ipc client" << endl;

	m_bExit = true;

	if (m_vecTcpPacket.size())
	{
		::EnterCriticalSection(&m_CriticalSection);

		for (auto packet : m_vecTcpPacket)
		{
			if (packet.pData)
			{
				delete packet.pData;
				packet.pData = nullptr;
			}
		}

		m_vecTcpPacket.clear();
		::LeaveCriticalSection(&m_CriticalSection);
	}
	

	if (m_clientSocket != INVALID_SOCKET)
	{
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
		WSACleanup();
	}
}

void CIpcClient::SetPacketReceiver(IIpcCallback* pReceiver)
{
	m_pPacketpReceiver = pReceiver;

}
#pragma endregion

#pragma region 数据接收处理

void CIpcClient::ClientListenThread()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(1, 1), &wsa) != NO_ERROR)
	{
		cout << "WSAStartup failed" << endl;
		return ;
	}

	m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_clientSocket == INVALID_SOCKET)
	{
		cout << "create socket failed,errorCode" << WSAGetLastError() << endl;
		return ;
	}
	SOCKADDR_IN clientAddr;
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(m_nPort);
	clientAddr.sin_addr.S_un.S_addr = inet_addr(m_strIp.c_str());

	int nRet = 1;
	while (nRet != NO_ERROR)
	{
		nRet = connect(m_clientSocket, (sockaddr*)&clientAddr, sizeof(SOCKADDR));//客户机向服务器发出连接请求 
		Sleep(500);
	}
	cout << "connect server success" << endl;

	//收包
	char receiveBuf[BUFFSIZE + 1];
	while (!m_bExit)
	{
		memset(receiveBuf, 0, sizeof(receiveBuf));
		nRet = recv(m_clientSocket, receiveBuf, BUFFSIZE + 1, 0);

		if (nRet > 0)
		{
			cout << "recv data length:" << nRet << ", " << receiveBuf << endl;
			CIpcClient::GetInstance()->AddTcpPakcet(receiveBuf, nRet);
		}
		else {
			cout << "client disconnect,close client socket" << endl;
			closesocket(m_clientSocket);//关闭
			m_bExit = true;
			break;
		}
	}
	cout << "Thread Exit" << endl;

}


void CIpcClient::AddTcpPakcet(char* pData, DWORD dwLength)
{
	::EnterCriticalSection(&m_CriticalSection);

	TCP_PACKET packet;

	packet.pData = (char*)malloc(dwLength);

	memcpy(packet.pData, pData, dwLength);

	packet.dwDataLength = dwLength;
	m_vecTcpPacket.push_back(packet);

	::LeaveCriticalSection(&m_CriticalSection);

}


bool CIpcClient::OnTcpPacket()
{
	while (!m_bExit)
	{
		while (m_vecTcpPacket.size())
		{
			::EnterCriticalSection(&m_CriticalSection);
			if (m_vecTcpPacket.size() == 0)
			{
				::LeaveCriticalSection(&m_CriticalSection);
				break;
			}
			vector<TCP_PACKET>::iterator iter = m_vecTcpPacket.begin();
			TCP_PACKET packet = *iter;

			iter = m_vecTcpPacket.erase(iter);
			if (iter != m_vecTcpPacket.end())
			{
				iter++;
			}
			::LeaveCriticalSection(&m_CriticalSection);

			if (m_pPacketpReceiver)
			{
				m_pPacketpReceiver->OnDataCallback(packet.pData, packet.dwDataLength);
			}
		}
		//Sleep(5);
	}
	return true;
}


bool CIpcClient::SendPacket(char* pData, DWORD nDataLen)
{
	unique_ptr<char[]> smartData(pData);

	if (m_clientSocket == INVALID_SOCKET)
	{
		cout << "no connections" << endl;
		return false;
	}

	int ret = send(m_clientSocket, pData, nDataLen, 0);

	if (ret < 0)
	{
		cout << "SendPacket failed!errorCode: " << WSAGetLastError() << endl;
		return false;
	}
	else {
		cout << "SendPacket success!ret: " << ret << endl;
		return true;
	}

	return true;
}

#pragma endregion
