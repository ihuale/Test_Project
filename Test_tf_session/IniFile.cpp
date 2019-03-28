#include "IniFile.h"

#include <Windows.h>

IniFile::IniFile()
{

}

IniFile::~IniFile(void)
{
}

void IniFile::setFileName(std::string fileName)
{
	/*int nPos;
	char cFileName[MAX_PATH];
	nPos = GetCurrentDirectoryA(MAX_PATH, cFileName);
	m_filePath = cFileName;
	m_fileName = m_filePath + "\\" + fileName;*/
	m_fileName = fileName;
}

double IniFile::getValueDouble(std::string strGoup, std::string strName)
{
	std::string var;
	GetPrivateProfileStringA(strGoup.c_str(), strName.c_str(), "default", const_cast<char *>(var.c_str()), MAX_PATH, m_fileName.c_str());
	double resVar = atof(var.c_str());
	return resVar;
}

int IniFile::getValueInt(std::string strGroup, std::string strName)
{
	int resVar = GetPrivateProfileIntA(strGroup.c_str(), strName.c_str(), 0, m_fileName.c_str());
	return resVar;
}

std::string IniFile::getValueStr(std::string strGroup, std::string strName)
{
	std::string var;
	char c[MAX_PATH];
	GetPrivateProfileStringA(strGroup.c_str(), strName.c_str(), "default", c, MAX_PATH, m_fileName.c_str());
	var = c;
	return var;
}

bool IniFile::setValue(std::string strGroup, std::string strName, double doubleValue)
{
	char s[12];
	_gcvt(doubleValue, 6, s);
	bool res = WritePrivateProfileStringA(strGroup.c_str(), strName.c_str(), s, m_fileName.c_str());
	return res;
}

bool IniFile::setValue(std::string strGroup, std::string strName, int intValue)
{
	char s[12];
	_itoa(intValue, s, 10);
	bool res = WritePrivateProfileStringA(strGroup.c_str(), strName.c_str(), s, m_fileName.c_str());
	return res;
}

bool IniFile::setValue(std::string strGroup, std::string strName, std::string strValue)
{
	bool res = WritePrivateProfileStringA(strGroup.c_str(), strName.c_str(), strValue.c_str(), m_fileName.c_str());
	return res;
}