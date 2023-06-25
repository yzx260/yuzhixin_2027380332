#pragma once
#include <string>
#include <map>
#include <vector>
#include "IIpcCallback.h"

enum IMAGE_TYPE
{
	NORMAL_IMAGE = 6,
	FG_IMAGE =10,
	NG_IMAGE ,
	AG_IMAGE 
};

class CTaskManager : public IIpcCallback
{
public:
	CTaskManager();
	~CTaskManager();
	static CTaskManager* GetInstance();

	void Init(HWND hWnd);
	string UnicodeToUTF8(const wchar_t* src);
	IMAGE_TYPE RunTask(std::wstring strFilePath);
	void RunDirTask(std::wstring strDirPath);

	void ProcessTask();

	virtual void OnDataCallback(char* pData, DWORD dwLength) override;
private:
	int m_nNums = 0;

	static CTaskManager* m_pInstance;

	std::map<std::wstring, int>  m_mapResult;
	std::vector<std::wstring>  m_vecTasks;

	HWND m_hWnd;
};

