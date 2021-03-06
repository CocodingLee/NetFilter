#include "stdafx.h"
#include "AuxiliaryFuncs.h"


AuxiliaryFuncs::AuxiliaryFuncs() {}

AuxiliaryFuncs::~AuxiliaryFuncs() {}

bool AuxiliaryFuncs::win2DosPath(const std::string& winPath, std::string& dosPath) {
	TCHAR tDosDrive[MAX_PATH];

	if (GetLogicalDriveStrings(MAX_PATH, tDosDrive)) {
		LPTSTR lpDosDrive = tDosDrive;

		while (tDosDrive[0] != 0) {
			std::string sDosDrive = std::string(lpDosDrive).substr(0, 2);

			TCHAR tWinDrive[MAX_PATH];
			if (QueryDosDevice(sDosDrive.c_str(), tWinDrive, MAX_PATH)) {
				std::string winDrive = tWinDrive;
				size_t pos = winPath.find(winDrive);
				if (pos != winPath.npos) {
					std::string leftPathPart = winPath.substr(pos, pos + winDrive.size());
					std::string rightPathPart = winPath.substr(pos + winDrive.size());
					dosPath = sDosDrive + rightPathPart;
					return true;
				}
			}
			else {
				return false;
			}

			lpDosDrive = strchr(lpDosDrive, 0) + 1;
		}
	}

	return true;
}

DWORD AuxiliaryFuncs::getProcessName(DWORD pid, std::string& processName, bool fullName) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (hProcess != NULL) {
		TCHAR processFullWinName[MAX_PATH];
		if (GetProcessImageFileName(hProcess, processFullWinName, MAX_PATH)) {
			std::string processFullDosName;
			if (win2DosPath(std::string(processFullWinName), processFullDosName)) {
				if (!fullName) {
					TCHAR name[MAX_PATH];
					TCHAR ext[MAX_PATH];
					_splitpath_s(processFullDosName.c_str(), nullptr, 0, nullptr, 0, name, MAX_PATH, ext, MAX_PATH);
					processName += name;
					processName += ext;
				}
				else {
					processName = processFullDosName;
				}
			}
		}
		else {
			CloseHandle(hProcess);
			return GetLastError();
		}
	}
	else {
		return GetLastError();
	}

	return ERROR_SUCCESS;
}

std::string AuxiliaryFuncs::getTimeStamp(const std::string& format) {

	SYSTEMTIME time;
	memset(&time, 0, sizeof(time));
	GetLocalTime(&time);

	TCHAR timeString[64];
	sprintf_s(timeString, 64, format.c_str(), time.wYear, time.wMonth, time.wDay,
			time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	return timeString;
}

bool AuxiliaryFuncs::forceDirectories(const std::string& aPath) {
	TCHAR* temp = _tcsdup(aPath.c_str());
	bool done = false;

	for (TCHAR* p = temp; *p != 0; ++p) {
		if (*p != '\\')
			continue;

		*p = 0;
		DWORD attrs = GetFileAttributes(temp);
		if (attrs == INVALID_FILE_ATTRIBUTES) {
			if (::CreateDirectory(temp, NULL) == 0 && GetLastError() != ERROR_ALREADY_EXISTS) {
				goto out;
			}
		}
		else if ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			goto out;
		}
		*p = '\\';
	}

	done = ::CreateDirectory(temp, NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;

out:
	free(temp);
	return done;
}

void AuxiliaryFuncs::toLower(std::string & str) {
	for (auto& sym : str) {
		sym = tolower(sym);
	}
}

BOOL AuxiliaryFuncs::IsWow64() {
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process;

	BOOL bIsWow64 = FALSE;

	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.

	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

	if (NULL != fnIsWow64Process) {
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
			//handle error
		}
	}
	return bIsWow64;
}

bool AuxiliaryFuncs::getCurrentFile(std::string& disk, std::string& folder, std::string& file, std::string& ext) {
	HMODULE hModule = GetModuleHandle(NULL);
	TCHAR currentPath[MAX_PATH];

	if (!GetModuleFileName(hModule, currentPath, MAX_PATH)) {
		return false;
	}

	TCHAR currentDisk[MAX_PATH];
	TCHAR currentFolder[MAX_PATH];
	TCHAR currentFileName[MAX_PATH];
	TCHAR currentFileExt[MAX_PATH];
	_splitpath_s(currentPath, currentDisk, MAX_PATH,
		currentFolder, MAX_PATH, currentFileName, MAX_PATH, currentFileExt, MAX_PATH);

	disk = currentDisk;
	folder = currentFolder;
	file = currentFileName;
	ext = currentFileExt;

	return true;
}

