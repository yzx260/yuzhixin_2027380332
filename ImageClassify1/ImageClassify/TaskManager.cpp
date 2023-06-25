#include "pch.h"
#include "TaskManager.h"
#include "IpcClient.h"
#include "include/json/json.h"

CTaskManager* CTaskManager::m_pInstance = nullptr;

CTaskManager::CTaskManager()
{

}

CTaskManager::~CTaskManager()
{

}

CTaskManager* CTaskManager::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new CTaskManager;
	}

	return m_pInstance;
}

void CTaskManager::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	CIpcClient::GetInstance()->Start("127.0.0.1", 35000);
	CIpcClient::GetInstance()->SetPacketReceiver(this);
}

string CTaskManager::UnicodeToUTF8(const wchar_t* src)
{
	if (src == NULL || src[0] == '\0')
	{
		return "";
	}

	wstring strSrc(src);
	int length = WideCharToMultiByte(CP_UTF8, 0, strSrc.c_str(),
		-1, NULL, 0, NULL, FALSE);
	char* buf = new char[length + 1];
	WideCharToMultiByte(CP_UTF8, 0, strSrc.c_str(), -1, buf, length, NULL, FALSE);
	buf[length] = '\0';
	string dest = buf;
	delete[] buf;
	return dest;
}

//������
IMAGE_TYPE CTaskManager::RunTask(std::wstring strFilePath)
{
	//����һ������
	//����һ��JSON����
	//JSON����ת�ַ���
	Json::Value vRoot;
	vRoot["type"] = "classify";

// 	Json::Value vParams;
// 	vParams["path"] = "";
// 	vParams["index"] = 0;
// 
// 	vRoot["params"] = vParams;
	string strPath = UnicodeToUTF8(strFilePath.c_str());
	vRoot["params"]["path"] = strPath;
	vRoot["params"]["index"] = 0;

	string strRoot = vRoot.toStyledString();
	char* pBuffer = new char[strRoot.length() + 1];
	strcpy(pBuffer, strRoot.c_str());
	//��������񷢸�����ѧϰ(python)
	CIpcClient::GetInstance()->SendPacket(pBuffer,strRoot.length());

	//�ȴ�pythonִ����ɵ���Ϣ

	//���ط�����
// 	{
// 		"type": "classify",
// 		"index" : "0",
// 		"class" : 1;
// 	}


	return (IMAGE_TYPE)0;
}

void CTaskManager::RunDirTask(std::wstring strDirPath)
{
	//1.forѭ����ö��Ŀ¼�µ������ļ�


	//2.ÿ���ļ�������AddTask
}

//������
void CTaskManager::ProcessTask()
{
	for (auto it = m_vecTasks.begin(); it != m_vecTasks.end(); ++it)
	{
		//��������

		//ɾ������
		m_vecTasks.erase(it);
	}
}

void CTaskManager::OnDataCallback(char* pData, DWORD dwLength)
{
	//��������
	//�ַ���-->json����-->�����
	string strRoot(pData, dwLength);
	Json::Reader reader;
	Json::Value vRoot;
	if (!reader.parse(strRoot, vRoot))
	{
		return;
	}
	string strType = vRoot["type"].asString();
	//int nIndex = vRoot["index"].asInt();
	string strClass = vRoot["class"].asString();
	
	//::PostMessage
	string* pnClass = new string(strClass);

	::PostMessage(m_hWnd, EVENT_IMAGE_CLASSIFY_FINISHED, 0, (LPARAM)pnClass);
	
}

