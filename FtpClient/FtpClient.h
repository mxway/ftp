#ifndef FTP_CLIENT_INCLUDE_H
#define FTP_CLIENT_INCLUDE_H
#include <windows.h>
#include <WinSock.h>
#include <string>
#include <vector>

using namespace std;

typedef struct  
{
	BOOL	m_isDir;//为目录该变量为TRUE，否则为FALSE
	char	m_fileName[MAX_PATH + 2];//文件名
	__int64	m_fileSize;//文件大小
}Ftp_File_s;

class CFtpClient
{
public:
	CFtpClient();
	~CFtpClient();

	/*************************************
	*
	*	函数名：	connectToHost
	*	函数说明：	连接到FTP服务器
	*	参数描述：	@param serverIP[输入参数]远程ftp服务器IP地址
	*	参数描述：	@param serverPort[输入参数]远程FTP服务监听端口
	*	返回值：	连接成功返回true，否则返回false
	*
	**************************************/
	bool		connectToHost(const string &serverIP,int serverPort=21);

	/*************************************
	*
	*	函数名：	login
	*	函数说明：	使用用户名密码认证ftp访问权限
	*	参数描述：	@param user[输入参数]用户名
	*	参数描述：	@param password[输入参数]密码
	*	返回值：	认证成功返回true，否则返回false
	*
	**************************************/
	bool		login(const string &user,const string &password);

	/*************************************
	*
	*	函数名：	cd
	*	函数说明：	切换ftp服务端当前工作目录
	*	参数描述：	@param path[输入参数]该参数以unix路径格式描述，可以是
					绝对路径；或者是相对路径，相对路径是相对ftp当前工作目录。
					如path值为:/log/log1，则无论ftp当前工作目录是什么，执行cd成功后
					ftp当前工作目录为/log/log1。
					若当前工作目录为/log/log1，path取值为:log2/log3，则cd执行成功后
					ftp当前工作目录为/log/log1/log2/log3
	*	返回值：	成功改变当前工作目录返回true，否则返回false
	*
	**************************************/
	bool		cd(const string &path);

	/*************************************
	*
	*	函数名：	deleteFile
	*	函数说明：	删除远程ftp中的文件
	*	参数描述：	@param path[输入参数]远程文件名称，可以是绝对路径或是相对当前
					工作目录的相对路径。参考cd函数中的解释
	*	返回值：	操作成功返回true，否则返回false
	*
	**************************************/
	bool		deleteFile(const string &path);

	/*************************************
	*
	*	函数名：	rmdir
	*	函数说明：	删除远程ftp的指定目录
	*	参数描述：	@param path[输入参数]远程待删除目录名，可以是相对路径或绝对路径,参考cd函数说明
	*	返回值：	操作成功返回true，否则返回false
	*	补充说明：  若待删除的目录含有子目录或子文件则删除失败
	*
	**************************************/
	bool		rmdir(const string &path);

	/*************************************
	*
	*	函数名：	mkdir
	*	函数说明：	创建远程目录
	*	参数描述：	@param path[输入参数]创建目录的名称，可以是相对路径或绝对路径
	*	返回值：	成功返回true，否则返回false
	*	补充说明:	假设远程目录"/log"下没有子目录和子文件，若path值为/log/log1/log2
	*				则目录创建有可能失败，这是因为远程FTP服务器可能不支持递归创建目录。
	*				若想创建/log/log1/log2这样的目录可以分两次调用mkdir：mkdir("/log/log1");mkdir("/log/log1/log2")
	*
	**************************************/
	bool		mkdir(const string &path);

	/*************************************
	*
	*	函数名：	pwd
	*	函数说明：	获取远程ftp当前工作目录
	*	返回值：	当前工作目录字符串
	*
	**************************************/
	string		pwd();

	/*************************************
	*
	*	函数名：	list
	*	函数说明：	列出指定目录下的所有子目录及子文件信息
	*	参数描述：	@param path[输入参数]待列出的目录路径，可以是相对路径或绝对路径
	*	参数描述：	@param fileList[输出参数]ftp返回的文件列表放入该变量中。
	*	返回值：	void
	*
	**************************************/
	void		list(const string &path,vector<Ftp_File_s*> &fileList);

	/*************************************
	*
	*	函数名：	put
	*	函数说明：	将本地文件上传至远程服务器
	*	参数描述：	@param remoteFile[输入参数]保存到远程ftp服务器后的文件名，
					可以是相对路径或绝对路径
	*	参数描述：	@param localFile[输入参数]本地待上传到远程ftp服务器的文件
	*	返回值：	void
	*
	**************************************/
	void		put(const string &remoteFile, const string &localFile);

	/*************************************
	*
	*	函数名：	get
	*	函数说明：	将远程文件下载至本地磁盘保存
	*	参数描述：	@param remoteFile[输入参数]远程待下载文件名，可以是相对路径名或绝对路径名
	*	参数描述：	@param localFile[输入参数]远程文件下载到本文件磁盘后的文件名。
	*	返回值：	void
	*
	**************************************/
	void		get(const string &remoteFile, const string &localFile);

	/*************************************
	*
	*	函数名：	quit
	*	函数说明：	断开ftp控制连接
	*	返回值：	bool
	*
	**************************************/
	bool		quit();
	
	/*************************************
	*
	*	函数名：	sendRawCommand
	*	函数说明：	发送除了接口提供的常用ftp命令以外的命令
	*	参数描述：	@param cmd[输入参数]命令字符串
	*	参数描述：	@param data[输入参数]命令参数值
	*	参数描述：	char expectCode[输入参数]如果命令成功执行则返回的状态码第一个字符
						应与该字符相等
	*	返回值：	命令执行成功返回true，否则返回false
	*
	**************************************/
	bool		sendRawCommand(const char *cmd,const char *data,char expectCode);

	/*************************************
	*
	*	函数名：	getStatusCode
	*	函数说明：	获取ftp命令执行后服务器返回的状态码
	*	返回值：	int
	*
	**************************************/
	int			getStatusCode(){return m_statusCode;}

	/*************************************
	*
	*	函数名：	getDesc
	*	函数说明：	获取ftp命令执行后，服务器返回的描述信息文本
	*	返回值：	std::string
	*
	**************************************/
	string		getDesc(){return m_desc;}
private:
	/*************************************
	*
	*	函数名：	createSocket
	*	函数说明：	创建tcp连接，建立本地与server的链路
	*	参数描述：	@param serverIP[输入参数]远程IP地址
	*	参数描述：	@param port[输入参数]远程端口号
	*	返回值：	SOCKET
	*
	**************************************/
	SOCKET	createSocket(ULONG serverIP,USHORT port);

	/*************************************
	*
	*	函数名：	readResponse
	*	函数说明：	ftp客户端每发送完一条指令后，服务端都会
					响应一状态码及相关文字描述。该函数用于读取
					响应状态码及相关文字描述信息
	*	参数描述：	@param resCode[输出参数]ftp服务端响应的状态码存入该参数
	*	参数描述：	@param str[输出参数]ftp返回的结果描述文字
	*	返回值：	成功读取返回true，否则返回false
	*
	**************************************/
	bool	readResponse(char	*resCode,string &str);

	/*************************************
	*
	*	函数名：	initSocket
	*	函数说明：	初始化ftp控制流socket
	*	返回值：	void
	*
	**************************************/
	void	initSocket();

	/*************************************
	*
	*	函数名：	sendCommand
	*	函数说明：	向ftp服务器发送命令，该函数组成:cmd+" "+data+"\r\n"的字符串
					将组成的字符串发送至ftp服务器
	*	参数描述：	@param cmd[输入参数]向FTP发送的命令字符串
	*	参数描述：	@param data[输入参数]发送命令带的参数数据信息。
	*	返回值：	发送成功返回true，否则返回false
	*
	**************************************/
	bool	sendCommand(const char *cmd,const char *data);

	/*************************************
	*
	*	函数名：	getResponseResult
	*	函数说明：	该函数调用readResponse读取命令发送后ftp服务器返回的状态码
					及描述文字信息。通过将状态码的第一个字符与参数code进行比较
					判断命令是否正确执行。
	*	参数描述：	@param code[输入参数]待判断状态码
	*	参数描述：	@param respStr[输出参数]结果描述信息
	*	返回值：	若ftp返回的状态码第一个字符与code相同返回true，
					若不同、或发生网络错误返回false
	*
	**************************************/
	bool	getResponseResult(char code,string &respStr);

	/*************************************
	*
	*	函数名：	parsePasvData
	*	函数说明：	执行pasv命令，并解析pasv命令执行成功后返回
					的用于数据传输的IP地址和端口信息。
	*	参数描述：	@param prmServerIP[输出参数]pasv执行成功后返回的ip地址信息
	*	参数描述：	@param prmPort[输出参数]pasv执行成功后返回的端口信息
	*	返回值：	pasv执行成功，并且ip及端口解析成功返回true；否则返回false
	*
	**************************************/
	bool	parsePasvData(ULONG  *prmServerIP, USHORT *prmPort);

	/*************************************
	*
	*	函数名：	openDataChannel
	*	函数说明：	打开用于数据传输的通道
	*	参数描述：	@param serverIP[输入参数]由pasv返回的ip地址
	*	参数描述：	@param port[输入参数]由pasv返回的port地址
	*	返回值：	数据通道打开成功返回true，否则返回false
	*
	**************************************/
	bool	openDataChannel(ULONG serverIP,USHORT port);

	/*************************************
	*
	*	函数名：	parseLines
	*	函数说明：	一行一行的解析list返回的数据。
	*	参数描述：	@param msg[输入参数]收到的数据缓存区
	*	参数描述：	@param bytes[输入参数]收到数据字节数
	*	参数描述：	@param fileList[输出参数]把解析出的文件信息存入到该变量中
	*	返回值：	返回从msg中已经解析出的字节数
	*
	**************************************/
	int	parseLines(char *msg, int bytes, vector<Ftp_File_s*> &fileList);

	/*************************************
	*
	*	函数名：	parseFtpFileInfo
	*	函数说明：	解析一条文件信息，放入到fileList中
	*	参数描述：	char * msg
	*	参数描述：	vector<Ftp_File_s * > & fileList
	*	返回值：	void
	*
	**************************************/
	void parseFtpFileInfo(char *msg, vector<Ftp_File_s*> &fileList);
private:
	SOCKET	m_socket;//用于ftp控制连接的socket
	SOCKET	m_dataSocket;//用于ftp数据传输的socket
	int 	m_statusCode;//ftp命令执行完成后的状态码
	string  m_desc;//ftp命令执行完成后的字符描述信息
};

#endif