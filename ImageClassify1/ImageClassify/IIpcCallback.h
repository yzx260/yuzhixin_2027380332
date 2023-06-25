#pragma once
#include <string>

using namespace  std;

//功能: 函数回调接口类
class IIpcCallback
{
public:

virtual void OnDataCallback(char* pData, DWORD dwLength) = 0;

};

