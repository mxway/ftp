#ifndef FTP_CLIENT_INCLUDE_H
#define FTP_CLIENT_INCLUDE_H
#include <windows.h>
#include <WinSock.h>
#include <string>

using namespace std;

class CFtpClient
{
public:
	CFtpClient();
	~CFtpClient();

	/*************************************
	*
	*	��������	connectToHost
	*	����˵����	���ӵ�FTP������
	*	����������	@param serverIP[�������]Զ��ftp������IP��ַ
	*	����������	@param serverPort[�������]Զ��FTP��������˿�
	*	����ֵ��	���ӳɹ�����true�����򷵻�false
	*
	**************************************/
	bool		connectToHost(const string &serverIP,int serverPort=21);

	/*************************************
	*
	*	��������	login
	*	����˵����	ʹ���û���������֤ftp����Ȩ��
	*	����������	@param user[�������]�û���
	*	����������	@param password[�������]����
	*	����ֵ��	��֤�ɹ�����true�����򷵻�false
	*
	**************************************/
	bool		login(const string &user,const string &password);

	/*************************************
	*
	*	��������	cd
	*	����˵����	�л�ftp����˵�ǰ����Ŀ¼
	*	����������	@param path[�������]�ò�����unix·����ʽ������������
					����·�������������·�������·�������ftp��ǰ����Ŀ¼��
					��pathֵΪ:/log/log1��������ftp��ǰ����Ŀ¼��ʲô��ִ��cd�ɹ���
					ftp��ǰ����Ŀ¼Ϊ/log/log1��
					����ǰ����Ŀ¼Ϊ/log/log1��pathȡֵΪ:log2/log3����cdִ�гɹ���
					ftp��ǰ����Ŀ¼Ϊ/log/log1/log2/log3
	*	����ֵ��	�ɹ��ı䵱ǰ����Ŀ¼����true�����򷵻�false
	*
	**************************************/
	bool		cd(const string &path);

	/*************************************
	*
	*	��������	deleteFile
	*	����˵����	ɾ��Զ��ftp�е��ļ�
	*	����������	@param path[�������]Զ���ļ����ƣ������Ǿ���·��������Ե�ǰ
					����Ŀ¼�����·�����ο�cd�����еĽ���
	*	����ֵ��	�����ɹ�����true�����򷵻�false
	*
	**************************************/
	bool		deleteFile(const string &path);

	/*************************************
	*
	*	��������	rmdir
	*	����˵����	ɾ��Զ��ftp��ָ��Ŀ¼
	*	����������	@param path[�������]Զ�̴�ɾ��Ŀ¼�������������·�������·��,�ο�cd����˵��
	*	����ֵ��	�����ɹ�����true�����򷵻�false
	*	����˵����  ����ɾ����Ŀ¼������Ŀ¼�����ļ���ɾ��ʧ��
	*
	**************************************/
	bool		rmdir(const string &path);

	/*************************************
	*
	*	��������	mkdir
	*	����˵����	����Զ��Ŀ¼
	*	����������	@param path[�������]����Ŀ¼�����ƣ����������·�������·��
	*	����ֵ��	�ɹ�����true�����򷵻�false
	*	����˵��:	����Զ��Ŀ¼"/log"��û����Ŀ¼�����ļ�����pathֵΪ/log/log1/log2
	*				��Ŀ¼�����п���ʧ�ܣ�������ΪԶ��FTP���������ܲ�֧�ֵݹ鴴��Ŀ¼��
	*				���봴��/log/log1/log2������Ŀ¼���Է����ε���mkdir��mkdir("/log/log1");mkdir("/log/log1/log2")
	*
	**************************************/
	bool		mkdir(const string &path);

	/*************************************
	*
	*	��������	pwd
	*	����˵����	��ȡԶ��ftp��ǰ����Ŀ¼
	*	����ֵ��	��ǰ����Ŀ¼�ַ���
	*
	**************************************/
	string		pwd();

	/*************************************
	*
	*	��������	list
	*	����˵����	�г�ָ��Ŀ¼�µ�������Ŀ¼�����ļ���Ϣ
	*	����������	@param path[�������]���г���Ŀ¼·�������������·�������·��
	*	����ֵ��	void
	*
	**************************************/
	void		list(const string &path);

	/*************************************
	*
	*	��������	put
	*	����˵����	�������ļ��ϴ���Զ�̷�����
	*	����������	@param remoteFile[�������]���浽Զ��ftp����������ļ�����
					���������·�������·��
	*	����������	@param localFile[�������]���ش��ϴ���Զ��ftp���������ļ�
	*	����ֵ��	void
	*
	**************************************/
	void		put(const string &remoteFile, const string &localFile);

	/*************************************
	*
	*	��������	get
	*	����˵����	��Զ���ļ����������ش��̱���
	*	����������	@param remoteFile[�������]Զ�̴������ļ��������������·���������·����
	*	����������	@param localFile[�������]Զ���ļ����ص����ļ����̺���ļ�����
	*	����ֵ��	void
	*
	**************************************/
	void		get(const string &remoteFile, const string &localFile);

	/*************************************
	*
	*	��������	quit
	*	����˵����	�Ͽ�ftp��������
	*	����ֵ��	bool
	*
	**************************************/
	bool		quit();
	
	/*************************************
	*
	*	��������	sendRawCommand
	*	����˵����	���ͳ��˽ӿ��ṩ�ĳ���ftp�������������
	*	����������	@param cmd[�������]�����ַ���
	*	����������	@param data[�������]�������ֵ
	*	����������	char expectCode[�������]�������ɹ�ִ���򷵻ص�״̬���һ���ַ�
						Ӧ����ַ����
	*	����ֵ��	����ִ�гɹ�����true�����򷵻�false
	*
	**************************************/
	bool		sendRawCommand(const char *cmd,const char *data,char expectCode);

	/*************************************
	*
	*	��������	getStatusCode
	*	����˵����	��ȡftp����ִ�к���������ص�״̬��
	*	����ֵ��	int
	*
	**************************************/
	int			getStatusCode(){return m_statusCode;}

	/*************************************
	*
	*	��������	getDesc
	*	����˵����	��ȡftp����ִ�к󣬷��������ص�������Ϣ�ı�
	*	����ֵ��	std::string
	*
	**************************************/
	string		getDesc(){return m_desc;}
private:
	/*************************************
	*
	*	��������	createSocket
	*	����˵����	����tcp���ӣ�����������server����·
	*	����������	@param serverIP[�������]Զ��IP��ַ
	*	����������	@param port[�������]Զ�̶˿ں�
	*	����ֵ��	SOCKET
	*
	**************************************/
	SOCKET	createSocket(ULONG serverIP,USHORT port);

	/*************************************
	*
	*	��������	readResponse
	*	����˵����	ftp�ͻ���ÿ������һ��ָ��󣬷���˶���
					��Ӧһ״̬�뼰��������������ú������ڶ�ȡ
					��Ӧ״̬�뼰�������������Ϣ
	*	����������	@param resCode[�������]ftp�������Ӧ��״̬�����ò���
	*	����������	@param str[�������]ftp���صĽ����������
	*	����ֵ��	�ɹ���ȡ����true�����򷵻�false
	*
	**************************************/
	bool	readResponse(char	*resCode,string &str);

	/*************************************
	*
	*	��������	initSocket
	*	����˵����	��ʼ��ftp������socket
	*	����ֵ��	void
	*
	**************************************/
	void	initSocket();

	/*************************************
	*
	*	��������	sendCommand
	*	����˵����	��ftp��������������ú������:cmd+" "+data+"\r\n"���ַ���
					����ɵ��ַ���������ftp������
	*	����������	@param cmd[�������]��FTP���͵������ַ���
	*	����������	@param data[�������]����������Ĳ���������Ϣ��
	*	����ֵ��	���ͳɹ�����true�����򷵻�false
	*
	**************************************/
	bool	sendCommand(const char *cmd,const char *data);

	/*************************************
	*
	*	��������	getResponseResult
	*	����˵����	�ú�������readResponse��ȡ����ͺ�ftp���������ص�״̬��
					������������Ϣ��ͨ����״̬��ĵ�һ���ַ������code���бȽ�
					�ж������Ƿ���ȷִ�С�
	*	����������	@param code[�������]���ж�״̬��
	*	����������	@param respStr[�������]���������Ϣ
	*	����ֵ��	��ftp���ص�״̬���һ���ַ���code��ͬ����true��
					����ͬ������������󷵻�false
	*
	**************************************/
	bool	getResponseResult(char code,string &respStr);

	/*************************************
	*
	*	��������	parsePasvData
	*	����˵����	ִ��pasv���������pasv����ִ�гɹ��󷵻�
					���������ݴ����IP��ַ�Ͷ˿���Ϣ��
	*	����������	@param prmServerIP[�������]pasvִ�гɹ��󷵻ص�ip��ַ��Ϣ
	*	����������	@param prmPort[�������]pasvִ�гɹ��󷵻صĶ˿���Ϣ
	*	����ֵ��	pasvִ�гɹ�������ip���˿ڽ����ɹ�����true�����򷵻�false
	*
	**************************************/
	bool	parsePasvData(ULONG  *prmServerIP, USHORT *prmPort);

	/*************************************
	*
	*	��������	openDataChannel
	*	����˵����	���������ݴ����ͨ��
	*	����������	@param serverIP[�������]��pasv���ص�ip��ַ
	*	����������	@param port[�������]��pasv���ص�port��ַ
	*	����ֵ��	����ͨ���򿪳ɹ�����true�����򷵻�false
	*
	**************************************/
	bool	openDataChannel(ULONG serverIP,USHORT port);

	int	parseLines(char *msg,int bytes);

	void parseFtpFileInfo(char *msg,int len);
private:
	SOCKET	m_socket;//����ftp�������ӵ�socket
	SOCKET	m_dataSocket;//����ftp���ݴ����socket
	int 	m_statusCode;//ftp����ִ����ɺ��״̬��
	string  m_desc;//ftp����ִ����ɺ���ַ�������Ϣ
};

#endif