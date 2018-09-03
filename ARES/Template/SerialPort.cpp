//////////////////////////////////////////////////////////////////////////  
/// COPYRIGHT NOTICE  
/// Copyright (c) 2009, ���пƼ���ѧtickTick Group  ����Ȩ������  
/// All rights reserved.  
///   
/// @file    SerialPort.cpp    
/// @brief   ����ͨ�����ʵ���ļ�  
///  
/// ���ļ�Ϊ����ͨ�����ʵ�ִ���  
///  
/// @version 1.0     
/// @author  ¬��    
/// @E-mail��lujun.hust@gmail.com  
/// @date    2010/03/19  
///   ���ʹ�õ���ԭ��ĳ�ʼ����
///  
///  �޶�˵����  
//////////////////////////////////////////////////////////////////////////  

#include "Stdafx.h"  
#include "SerialPort.h"  
#include <process.h>  
#include <iostream>  
//�ⲿ����
//extern char recieve_data[COM_BUF_LEN];
//extern char send_data[COM_BUF_LEN];

/** �߳��˳���־ */
bool CSerialPort::s_bExit = false;
/** ������������ʱ,sleep���´β�ѯ�����ʱ��,��λ:���� */
const UINT SLEEP_TIME_INTERVAL = 3;

CSerialPort::CSerialPort(void)
	: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;

	InitializeCriticalSection(&m_csCommunicationSync);

}

CSerialPort::~CSerialPort(void)
{
	CloseListenTread();
	ClosePort();
	DeleteCriticalSection(&m_csCommunicationSync);
}

//���ڳ�ʼ�����򿪲����ô��ڣ�
//bool CSerialPort::InitPort(UINT portNo)
//{
//	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
//	if (!openPort(portNo))
//	{
//		return false;
//	}
//	/** �����ٽ�� */
//	EnterCriticalSection(&m_csCommunicationSync);
//
//	//���ô��ڻ�������С,Ҫע�������Դ�һЩ,���⻺�������
//	if (!SetupComm(m_hComm, 1024, 1024))
//	{
//		printf("���û�����ʧ�ܣ�\n");
//		ClosePort();
//		return false;
//	}
//	/** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */
//	COMMTIMEOUTS  CommTimeouts;
//	CommTimeouts.ReadIntervalTimeout = 0;
//	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
//	CommTimeouts.ReadTotalTimeoutConstant = 0;
//	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
//	CommTimeouts.WriteTotalTimeoutConstant = 0;
//	if (!SetCommTimeouts(m_hComm, &CommTimeouts))
//	{
//		printf("���ó�ʱʧ�ܣ�\n");
//		ClosePort();
//		return false;
//	}
//	//���ô��ڲ���
//	DCB dcb = { 0 };
//	if (!GetCommState(m_hComm, &dcb))
//	{
//		printf("GetCommState fail\n");
//		ClosePort();
//		return false;
//	}
//	dcb.DCBlength = sizeof(dcb);
//	if (!BuildCommDCB("115200,n,8,1", &dcb))//���ģãµ����ݴ����ʡ���żУ�����͡�����λ��ֹͣλ
//	{
//		printf("BuileCOmmDCB fail\n");
//		ClosePort();
//		return false;
//	}
//	if (!SetCommState(m_hComm, &dcb))
//	{
//		printf("SetCommState fail!\n");
//		ClosePort();
//		return false;
//	}
//	/**  ��մ��ڻ����� */
//	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
//	/** �뿪�ٽ�� */
//	LeaveCriticalSection(&m_csCommunicationSync);
//
//	return TRUE;
//}

bool CSerialPort::InitPort(UINT portNo /*= 1*/, UINT baud /*= CBR_9600*/, char parity /*= 'N'*/,
	UINT databits /*= 8*/, UINT stopsbits /*= 1*/, DWORD dwCommEvents /*= EV_RXCHAR*/)
{

	/** ��ʱ����,���ƶ�����ת��Ϊ�ַ�����ʽ,�Թ���DCB�ṹ */
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
	if (!openPort(portNo))
	{
		return false;
	}

	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ƿ��д����� */
	BOOL bIsSuccess = TRUE;

	/** �ڴ˿���������������Ļ�������С,���������,��ϵͳ������Ĭ��ֵ.
	*  �Լ����û�������Сʱ,Ҫע�������Դ�һЩ,���⻺�������
	*/
	/*if (bIsSuccess )
	{
	bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** ���ô��ڵĳ�ʱʱ��,����Ϊ0,��ʾ��ʹ�ó�ʱ���� */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 0;
	if (bIsSuccess)
	{
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	DCB  dcb;
	if (bIsSuccess)
	{
		// ��ANSI�ַ���ת��ΪUNICODE�ַ���    
		DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
		wchar_t *pwText = new wchar_t[dwNum];
		if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		{
			bIsSuccess = TRUE;
		}

		/** ��ȡ��ǰ�������ò���,���ҹ��촮��DCB���� */
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(pwText, &dcb);
		/** ����RTS flow���� */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		/** �ͷ��ڴ�ռ� */
		delete[] pwText;
	}

	if (bIsSuccess)
	{
		/** ʹ��DCB�������ô���״̬ */
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}

	/**  ��մ��ڻ����� */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** �뿪�ٽ�� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return bIsSuccess == TRUE;
}

//���ڳ�ʼ������
void CSerialPort::my_InitPort(UINT portNo)
{
	if (InitPort(portNo))
	{
		printf("���ڳ�ʼ���ɹ���\n");
	}
	else
	{
		printf("���ڳ�ʼ��ʧ�ܣ�\n");
	}
}

//���ô��ڲ���
bool CSerialPort::InitPort(UINT portNo, const LPDCB& plDCB)
{
	/** ��ָ������,�ú����ڲ��Ѿ����ٽ�������,�����벻Ҫ�ӱ��� */
	if (!openPort(portNo))
	{
		return false;
	}
	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** ���ô��ڲ��� */
	if (!SetCommState(m_hComm, plDCB))
	{
		return false;
	}
	/**  ��մ��ڻ����� */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** �뿪�ٽ�� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

//�رմ���
void CSerialPort::ClosePort()
{
	/** ����д��ڱ��򿪣��ر��� */
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

//�رմ���
void CSerialPort::my_Close_Port()
{
	ClosePort();
}

//�򿪴���
bool CSerialPort::openPort(UINT portNo)
{
	/** �����ٽ�� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �Ѵ��ڵı��ת��Ϊ�豸�� */
	char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);

	/** ��ָ���Ĵ��� */
	m_hComm = CreateFileA(szPort,  /** �豸��,COM1,COM2�� */
		GENERIC_READ | GENERIC_WRITE, /** ����ģʽ,��ͬʱ��д */
		0,                            /** ����ģʽ,0��ʾ������ */
		NULL,                         /** ��ȫ������,һ��ʹ��NULL */
		OPEN_EXISTING,                /** �ò�����ʾ�豸�������,���򴴽�ʧ�� */
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		0);

	/** �����ʧ�ܣ��ͷ���Դ������ */
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** �˳��ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

bool CSerialPort::OpenListenThread()
{
	/** ����߳��Ƿ��Ѿ������� */
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** �߳��Ѿ����� */
		return false;
	}

	s_bExit = false;
	/** �߳�ID */
	UINT threadId;
	/** �����������ݼ����߳� */
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread)
	{
		return false;
	}
	/** �����̵߳����ȼ�,������ͨ�߳� */
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		return false;
	}

	return true;
}

void CSerialPort::my_OpenListenThread()
{
	if (OpenListenThread())
	{
		printf("��ʼ�������ڣ�\n");
	}
	else
	{
		printf("���Լ�������ʧ�� ...\n");
	}
}

bool CSerialPort::CloseListenTread()
{
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** ֪ͨ�߳��˳� */
		s_bExit = true;

		/** �ȴ��߳��˳� */
		Sleep(10);

		/** ���߳̾����Ч */
		CloseHandle(m_hListenThread);
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}

DWORD CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;  /** ������ */
	COMSTAT  comstat;   /** COMSTAT�ṹ��,��¼ͨ���豸��״̬��Ϣ */
	memset(&comstat, 0, sizeof(COMSTAT));

	UINT BytesInQue = 0;
	/** �ڵ���ReadFile��WriteFile֮ǰ,ͨ�������������ǰ�����Ĵ����־ */
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue = comstat.cbInQue; /** ��ȡ�����뻺�����е��ֽ��� */
	}

	return BytesInQue;
}

UINT WINAPI CSerialPort::ListenThread(void* pParam)
{

	/** �õ������ָ�� */
	CSerialPort *pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// �߳�ѭ��,��ѯ��ʽ��ȡ��������    
	while (!pSerialPort->s_bExit)
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();
		/** ����������뻺������������,����Ϣһ���ٲ�ѯ */
		if (BytesInQue == 0)
		{
			Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		/** ��ȡ���뻺�����е����ݲ������ʾ */
		/*char cRecved = 0x00;
		do
		{
		cRecved = 0x00;
		if (pSerialPort->ReadChar(cRecved) == true)
		{
		std::cout << cRecved;
		continue;
		}
		} while (--BytesInQue);*/
		DWORD RecieveLen = pSerialPort->ReadBlock(pSerialPort->my_RecieveBuff, BytesInQue);
		if (RecieveLen != 0)
		{
			if (pSerialPort->MyRecieveData(pSerialPort->my_RecieveBuff, pSerialPort->my_recieve_data))
			{
				//printf("=========����У��ɹ���\n");
				//printf("\n");
				/*char cmd = recieve_data[1];
				int a = *(INT16*)&recieve_data[4];
				int b = *(INT16*)&recieve_data[6];
				int c = *(INT16*)&recieve_data[8];
				printf("�ɹ�����===%c===%d===%d===%d \n", cmd, a, b, c);*/
			}
			else
			{
				//printf("=========����У��ʧ�ܣ�\n");
				//printf("\n");


			}
		}

	}

	return 0;
}

DWORD CSerialPort::ReadBlock(char* R_buff, DWORD len)
{
	BOOL  bResult;
	DWORD ReadBytesWant = COM_BUF_LEN;
	DWORD ReadBytesReal = 0;
	OVERLAPPED MyOsRead;
	memset(&MyOsRead, 0, sizeof(OVERLAPPED));
	MyOsRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ReadBytesWant = min(ReadBytesWant, len);
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �ӻ�������ȡ���� */
	bResult = ReadFile(m_hComm, R_buff, ReadBytesWant, &ReadBytesReal, &MyOsRead);
	if ((!bResult))
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			GetOverlappedResult(m_hComm,
				&MyOsRead, &ReadBytesReal, TRUE);
			// GetOverlappedResult���������һ��������ΪTRUE��
			//������һֱ�ȴ���ֱ����������ɻ����ڴ�������ء�
			/** ��մ��ڻ����� */
			PurgeComm(m_hComm, PURGE_RXABORT | PURGE_RXCLEAR);
			LeaveCriticalSection(&m_csCommunicationSync);
			return ReadBytesReal;
		}
		/** ��մ��ڻ����� */
		PurgeComm(m_hComm, PURGE_RXABORT | PURGE_RXCLEAR);
		LeaveCriticalSection(&m_csCommunicationSync);
		return 0;
	}
	/** ��մ��ڻ����� */
	PurgeComm(m_hComm, PURGE_RXABORT | PURGE_RXCLEAR);
	LeaveCriticalSection(&m_csCommunicationSync);
	return ReadBytesReal;
}

bool CSerialPort::MyRecieveData(char* srcBuff, char* dstBuff)
{
	bool flag = true;
	/** �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);
	for (int i = 0; i < DATA_LEN; i++)
	{
		dstBuff[i] = srcBuff[i];
	}
	if (UART_Receive_Buff(srcBuff))//CRCУ��
	{
		flag = true;
	}
	else
	{
		flag = false;
	}
	//�뿪�ٽ籣����
	LeaveCriticalSection(&m_csCommunicationSync);
	return flag;
}

bool CSerialPort::WriteData(char* pData, UINT16 length)
{
	BOOL   bResult;
	DWORD dwError;
	DWORD  WriteBytesReal;
	OVERLAPPED MyOsWrite;
	memset(&MyOsWrite, 0, sizeof(OVERLAPPED));
	MyOsWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	/** �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);
	if (ClearCommError(m_hComm, &dwError, NULL))
	{
		PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR);
	}
	else
	{
		return false;
	}
	if (!WriteFile(m_hComm, pData, length, &WriteBytesReal, &MyOsWrite))
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			while (!GetOverlappedResult(m_hComm, &MyOsWrite, &WriteBytesReal, FALSE))
				//�ɹ����ط�0��ʧ�ܷ���0
			{
				if (GetLastError() == ERROR_IO_INCOMPLETE)
				{
					continue;
				}
				else
				{
					ClearCommError(m_hComm, &dwError, NULL);
					/** �뿪�ٽ��� */
					LeaveCriticalSection(&m_csCommunicationSync);
					return false;
				}
			}
		}
	}
	/** �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);
	return true;
}



//=======================================================//
//CRC16���
void CSerialPort::ISO14443AAppendCRCA(void* Buffer, UINT16 ByteCount)
{
	UINT16 Checksum = 0x6363;
	BYTE* DataPtr = (BYTE*)Buffer;

	while (ByteCount--) {
		BYTE Byte = *DataPtr++;

		Byte ^= (BYTE)(Checksum & 0x00FF);
		Byte ^= Byte << 4;

		Checksum = (Checksum >> 8) ^ ((UINT16)Byte << 8) ^
			((UINT16)Byte << 3) ^ ((UINT16)Byte >> 4);
	}

	*DataPtr++ = (Checksum >> 0) & 0x00FF;
	*DataPtr = (Checksum >> 8) & 0x00FF;
}

//CRC16���
BYTE CSerialPort::ISO14443ACheckCRCA(void* Buffer, UINT16 ByteCount)
{
	UINT16 Checksum = 0x6363;
	BYTE* DataPtr = (BYTE*)Buffer;

	while (ByteCount--) {
		BYTE Byte = *DataPtr++;

		Byte ^= (BYTE)(Checksum & 0x00FF);
		Byte ^= Byte << 4;

		Checksum = (Checksum >> 8) ^ ((UINT16)Byte << 8) ^
			((UINT16)Byte << 3) ^ ((UINT16)Byte >> 4);
	}

	return (DataPtr[0] == ((Checksum >> 0) & 0xFF)) && (DataPtr[1] == ((Checksum >> 8) & 0xFF));
}

//������ݳ����Ƿ�ϸ�
BYTE CSerialPort::ISO14443ACheckLen(BYTE* Buffer)
{
	if ((Buffer[0] + Buffer[1]) == 0xff && Buffer[0]<COMMAND_BUF_LEN - 2)
		return 1;
	else
		return 0;
}

//���ݰ�����
bool CSerialPort::UART_Send_Buff(char command, char *data_input, UINT16 data_len)
{
	BYTE  Buffer[COMMAND_BUF_LEN];
	Buffer[0] = 0x55;//֡ͷ
	Buffer[1] = command;//���� 
	Buffer[2] = data_len;//���ݳ���
	Buffer[3] = 0xff - data_len;//ȡ��
	memcpy(Buffer + HEAD_LEN, data_input, data_len);//��������
	ISO14443AAppendCRCA(Buffer, data_len + HEAD_LEN);//���CRC
	if (WriteData((char*)Buffer, data_len + HEAD_LEN + 2))
		return true;
	else
	{
		return false;
	}
}

//���ݽ���
bool CSerialPort::UART_Receive_Buff(char* arrRC_Buf)
{
	if (arrRC_Buf[0] == 0x55)
	{
		if (ISO14443ACheckLen((BYTE*)(arrRC_Buf + HEAD_LEN - 2)))
		{
			if (ISO14443ACheckCRCA(arrRC_Buf, (UINT16)(arrRC_Buf[2] + HEAD_LEN)))
			{

			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

bool CSerialPort::ReadChar(char &cRecved)
{
	BOOL  bResult = TRUE;
	DWORD BytesRead = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** �ٽ������� */
	EnterCriticalSection(&m_csCommunicationSync);

	/** �ӻ�������ȡһ���ֽڵ����� */
	bResult = ReadFile(m_hComm, &cRecved, 1, &BytesRead, NULL);
	if ((!bResult))
	{
		/** ��ȡ������,���Ը��ݸô�����������ԭ�� */
		DWORD dwError = GetLastError();

		/** ��մ��ڻ����� */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** �뿪�ٽ��� */
	LeaveCriticalSection(&m_csCommunicationSync);

	return (BytesRead == 1);

}