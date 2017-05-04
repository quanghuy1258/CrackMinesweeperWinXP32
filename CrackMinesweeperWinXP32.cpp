#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

using namespace std;

#define target L"winmine.exe"

DWORD FindProcessId(LPCWSTR processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);
	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;
	Process32First(processesSnapshot, &processInfo);
	if (wcscmp(processName,processInfo.szExeFile) == 0)
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}
	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (wcscmp(processName, processInfo.szExeFile) == 0)
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}
	CloseHandle(processesSnapshot);
	return 0;
}

unsigned long numCol = 0, numRow = 0, prevNumCol = 0, prevNumRow = 0;
unsigned char tableMine[768] = { 0 }, prevTableMine[768] = { 0 };
int tst = 0, prevTst = 0;

int checkDiff() {
	int res = 0;
	if (tst != prevTst || numCol != prevNumCol || numRow != prevNumRow) res = 1;
	if (numCol == prevNumCol && numRow == prevNumRow)
		for (unsigned long i = 0; i < numRow; ++i)
			for (unsigned long j = 0; j < numCol; ++j)
				if (tableMine[32 * i + j] != prevTableMine[32 * i + j]) res = 1;
	if (res) {
		prevTst = tst;
		prevNumCol = numCol;
		prevNumRow = numRow;
		for (unsigned long i = 0; i < numRow; ++i)
			for (unsigned long j = 0; j < numCol; ++j)
				prevTableMine[32 * i + j] = tableMine[32 * i + j];
	}
	return res;
}

int main()
{
	cout << "Not found ..." << endl;
	while (true) {
		DWORD processId = FindProcessId(target);
		if (processId) {
			tst = 1;
			HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
			ReadProcessMemory(processHandle, (unsigned char*)0x01005334, &numCol, sizeof(unsigned long), NULL);
			ReadProcessMemory(processHandle, (unsigned char*)0x01005338, &numRow, sizeof(unsigned long), NULL);
			ReadProcessMemory(processHandle, (unsigned char*)0x01005361, tableMine, 768, NULL);
			CloseHandle(processHandle);
		}
		else tst = 0;
		if (checkDiff()) {
			system("cls");
			if (tst) {
				cout << "PID: " << processId << endl;
				for (unsigned long i = 0; i < numRow; ++i) {
					for (unsigned long j = 0; j < numCol; ++j) {
						switch (tableMine[32 * i + j])
						{
						case 0x0F: cout << '.'; break;
						case 0x8F: case 0x8A: cout << '*'; break;
						case 0xCC: cout << '!'; break;
						case 0x0E: case 0x8E: cout << 'F'; break;
						case 0x0B: cout << 'W'; break;
						case 0x0D: case 0x8D: cout << '?'; break;
						default:
							if (tableMine[32 * i + j] >= 65 && tableMine[32 * i + j] <= 72) cout << char(tableMine[32 * i + j] - 64 + '0');
							else cout << ' ';
							break;
						}
					}
					cout << endl;
				}
			}
			else cout << "Not found ..." << endl;
		}
	}
	return 0;
}