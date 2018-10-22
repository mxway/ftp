#include <stdio.h>
#include "FtpClient.h"
#include <vld.h>

int main()
{
	vector<Ftp_File_s*>	fileList;
	CFtpClient	client;
	client.connectToHost("127.0.0.1");
	client.login("mengxl","mxl123");
	client.list("/",fileList);
	client.quit();
	for (size_t i = 0; i < fileList.size(); i++)
	{
		delete fileList[i];
	}
	return 0;
}