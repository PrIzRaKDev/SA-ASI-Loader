#include "dllmain.h"
#include "pch.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

#define MAX_PATH_A 512

using namespace std;

vector<HMODULE> loadedASIModules;
vector<HMODULE> loadedDLLModules;
wofstream logFile;

void GetASIFiles(const wstring& directory, vector<wstring>& out) {
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW((directory + L"\\*.asi").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                out.push_back(directory + L"\\" + findFileData.cFileName);
            }
        } while (FindNextFileW(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
}

void GetDLLFiles(const wstring& directory, vector<wstring>& out) {
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW((directory + L"\\*.dll").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                out.push_back(directory + L"\\" + findFileData.cFileName);
            }
        } while (FindNextFileW(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
}

static void WriteToLogFile(const wstring& message) {
    if (!logFile.is_open()) {
        logFile.open(L"ASI_Loader.log", ios_base::app);
        if (!logFile.is_open()) {
            wcerr << L"Не удалось открыть лог файл." << endl;
            return;
        }
    }
    logFile << message << endl;
    logFile.flush();
}

static void exeUnprotect() {
    auto hExecutableInstance = (size_t)GetModuleHandle(NULL);
    IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(hExecutableInstance + ((IMAGE_DOS_HEADER*)hExecutableInstance)->e_lfanew);
    SIZE_T size = ntHeader->OptionalHeader.SizeOfImage;
    DWORD oldProtect;
    if (!VirtualProtect((VOID*)hExecutableInstance, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        MessageBoxW(nullptr, L"Не удалось сделать unprotect!", L"Ошибка!", MB_OK);
    } else {
        MessageBoxW(nullptr, L"DebugMsg: exeUnprotect успешно выполнен!", L"Успешно! Debug", MB_OK);
        WriteToLogFile(L"Exe unprotected successfully!");
    }
}

static void InitializeASILoader() {
    if (logFile.is_open()) {
        logFile.clear();
    }

    wstring MessageINIT = L"ASI Loader v.1.2.0 by Pr!zRaK initialized! \n";
    WriteToLogFile(MessageINIT);

    wchar_t modulePath[MAX_PATH_A];
    GetModuleFileNameW(nullptr, modulePath, MAX_PATH_A);

    wstring ASIDirectory = filesystem::path(modulePath).parent_path().wstring();
    wstring DLLDirectory = filesystem::path(modulePath).parent_path().wstring();

    vector<wstring> asiFiles;
    vector<wstring> dllFiles;
    GetASIFiles(ASIDirectory, asiFiles);
    GetDLLFiles(DLLDirectory, dllFiles);

    for (const auto& ASIFile : asiFiles) {
        HMODULE ASIHandle = LoadLibraryExW(ASIFile.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (ASIHandle != nullptr) {
            loadedASIModules.push_back(ASIHandle);
            wstring messageTrue = L"Plugin " + ASIFile + L" loaded! \n";
            WriteToLogFile(messageTrue);
        } else {
            DWORD errorCode = GetLastError();
            wstring errorMessage = L"Error loading: " + ASIFile + L" Error code: " + to_wstring(errorCode) + L"\n";
            WriteToLogFile(errorMessage);
        }
    }

    for (const auto& DLLFile : dllFiles) {
        HMODULE DLLHandle = LoadLibraryExW(DLLFile.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (DLLHandle != nullptr) {
            loadedDLLModules.push_back(DLLHandle);
            wstring messageTrue = L"Plugin " + DLLFile + L" loaded! \n";
            WriteToLogFile(messageTrue);
        } else {
            DWORD errorCode = GetLastError();
            wstring errorMessage = L"Error loading: " + DLLFile + L" Error code: " + to_wstring(errorCode) + L"\n";
            WriteToLogFile(errorMessage);
        }
    }
}

extern "C" __declspec(dllexport) void __cdecl ASILoader() {
    InitializeASILoader();
    exeUnprotect();
}
