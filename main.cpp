#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#define READ_BUF_SIZE		1
#define READ_TIMEOUT		500      // millisecond

HANDLE hComm;

DWORD WINAPI UartRead(LPVOID lpParameter)
{
	DWORD dwRead;
	BOOL fWaitingOnRead = FALSE;
	OVERLAPPED osReader = { 0 };
	DWORD dwRes;
	CHAR lpBuf[READ_BUF_SIZE];
	int read = 1;

	osReader.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (osReader.hEvent == NULL)
	{
		return 0; // Error creating overlapped event; abort.
	}

	while (read)
	{
		if (!fWaitingOnRead)
		{
			// Issue read operation.
			if (!ReadFile(hComm, lpBuf, READ_BUF_SIZE, &dwRead, &osReader))
			{
				if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
				{
					printf("hiba: ReadFile %d", GetLastError());
					// Error in communications; report it.
				}
				else
				{
					fWaitingOnRead = TRUE;
				}
			}
			else
			{
				printf("%c", lpBuf[0]);
			}
		}

		DWORD dwRes;

		if (fWaitingOnRead)
		{
			dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
			switch (dwRes)
			{
				// Read completed.
			case WAIT_OBJECT_0:
				if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE))
				{
					printf("hiba: GetOverlappedResult %d", GetLastError());
					// Error in communications; report it.
				}
				else
				{
					printf("%c", lpBuf[0]);
				}

				//  Reset flag so that another opertion can be issued.
				fWaitingOnRead = FALSE;
				break;

			case WAIT_TIMEOUT:
				//printf("Wait_Timeout");
				break;

			default:
				printf("default");
				break;
			}
		}
	}

	return 0;
}

int main(void)
{
	LPCWSTR gszPort = L"COM3";
	
	hComm = CreateFile(gszPort,
		GENERIC_READ | GENERIC_WRITE,
		0,
		0,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		0);

	if (hComm == INVALID_HANDLE_VALUE)
	{
		return 0;
	}


	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(hComm, &dcb))
	{
		return 0;
	}

	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (!SetCommState(hComm, &dcb))
	{
		return 0;
	}

	HANDLE threadhandle;

	threadhandle = CreateThread(NULL, NULL, UartRead, NULL, NULL, NULL);

	WaitForSingleObject(threadhandle, INFINITE);

	return 0;
}