#pragma once
#include <string>

using namespace  std;

//����: �����ص��ӿ���
class IIpcCallback
{
public:

virtual void OnDataCallback(char* pData, DWORD dwLength) = 0;

};

