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

//生产者
IMAGE_TYPE CTaskManager::RunTask(std::wstring strFilePath)
{
	//创建一个任务
	//构造一个JSON对象
	//JSON对象转字符串
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
	//把这个任务发给机器学习(python)
	CIpcClient::GetInstance()->SendPacket(pBuffer,strRoot.length());

	//等待python执行完成的消息

	//返回分类结果
// 	{
// 		"type": "classify",
// 		"index" : "0",
// 		"class" : 1;
// 	}


	return (IMAGE_TYPE)0;
}

void CTaskManager::RunDirTask(std::wstring strDirPath)
{
	//1.for循环来枚举目录下的所有文件


	//2.每个文件都调用AddTask
}

//消费者
void CTaskManager::ProcessTask()
{
	for (auto it = m_vecTasks.begin(); it != m_vecTasks.end(); ++it)
	{
		//处理任务

		//删除任务
		m_vecTasks.erase(it);
	}
}

void CTaskManager::OnDataCallback(char* pData, DWORD dwLength)
{
	//处理数据
	//字符串-->json对象-->解出来
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

