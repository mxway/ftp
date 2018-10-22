#include <stdio.h>
#include "FtpClient.h"
#include <vld.h>

int main()
{
	CFtpClient	client;
	client.connectToHost("127.0.0.1");
	client.login("mengxl","mxl123");
	client.list("/log");
	//client.put("/cryptopp-CRYPTOPP_7_0_0.zip", "F:\\cryptopp-CRYPTOPP_7_0_0.zip");
	client.quit();
	return 0;
}