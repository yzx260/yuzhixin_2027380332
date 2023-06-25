#pragma once
#include <winsock.h>
#include "IIpcCallback.h"
#include <vector>

namespace IPC_SOCKET
{
	static const int	nPort = 8869;
	static const char*  pIp = "127.0.0.1";
};


//�������TCP��
typedef struct _TCP_PACKET_
{
	char*	pData;		//������ָ��
	DWORD	dwDataLength;	//����
}TCP_PACKET, * PTCP_PACKET;



class CIpcClient
{
public:
	static CIpcClient* GetInstance();

	~CIpcClient();
public:
	bool Start(string strIp, int nPort);

	void Stop();

	void SetPacketReceiver(IIpcCallback* pReceiver);

	void ClientListenThread();//����˰������߳�,����client�˷�����Ϣ

	bool OnTcpPacket();

	bool SendPacket(char* pData, DWORD nDataLen);
private:
	CIpcClient();
	void AddTcpPakcet(char* pData, DWORD dwLength); //��m_vecTcpPacket�����һ���������

private:
	IIpcCallback* m_pPacketpReceiver = nullptr; //���ս��մ�����

	SOCKET m_clientSocket = INVALID_SOCKET;		//�ͻ���socket
	std::vector<TCP_PACKET>	m_vecTcpPacket;	//�������TCP������
	CRITICAL_SECTION m_CriticalSection;
	bool m_bExit = false;

	string m_strIp = IPC_SOCKET::pIp;
	int m_nPort = IPC_SOCKET::nPort;
};

