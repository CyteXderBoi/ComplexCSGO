#pragma region Ids registrados

UCHAR szFileSys[255], szVolNameBuff[255];

DWORD dwMFL, dwSysFlags, dwSerial;

#define HWID_0 (-362683040) // minha id

#define HWID_1 (1392352309) //

#define HWID_2 (-994941395) //

#define HWID_3 (-362683040) //

#define HWID_4 (-765437037)//

#pragma endregion



bool __stdcall DllMain(HMODULE, DWORD Reason, LPVOID)
{
	if (Reason == DLL_PROCESS_ATTACH)
	{
		GetVolumeInformation("C:\\", (LPTSTR)szVolNameBuff, 255, &dwSerial, &dwMFL, &dwSysFlags, (LPTSTR)szFileSys, 255);
		if (dwSerial == HWID_0 || dwSerial == HWID_1 || dwSerial == HWID_2 || dwSerial == HWID_3 || dwSerial == HWID_4)
		{
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)NameVoid, NULL, 0, 0);

		}
		else
			ExitProcess(1);
	}

	return 1;
}
