#pragma once

#include <string>

class IniFile
{
public:
	IniFile();
	~IniFile();

	void setFileName(std::string fileName);
	int getValueInt(std::string strGroup, std::string strName);
	double getValueDouble(std::string strGroup, std::string strName);
	std::string getValueStr(std::string strGroup, std::string strName);

	bool setValue(std::string strGroup, std::string strName, std::string strValue);
	bool setValue(std::string strGroup, std::string strName, int intValue);
	bool setValue(std::string strGroup, std::string strName, double doubleValue);

private:
	std::string m_filePath;
	std::string m_fileName;
};
