#include "dllmain.h"
#include "pch.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;

// Global variables for storing loaded modules and log file
std::vector<HMODULE> loadedASIModules;
std::vector<HMODULE> loadedDLLModules;
std::wofstream logFile;

// Function to get list of .asi files
static void __fastcall GetASIFiles(const std::wstring& directory, std::vector<std::wstring>& out) {
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

static void __fastcall GetDLLFiles(const std::wstring& directory, std::vector<std::wstring>& out) {
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

// Function to write messages to log file
static void WriteToLogFile(const std::wstring& message) {
    if (!logFile.is_open()) {
        logFile.open(L"asi_loader.log", std::ios_base::app);
    }
    if (logFile.is_open()) {
        logFile << message << std::endl;
    } else {
        std::wcerr << L"Failed to open log file." << std::endl;
    }
}

// Function to initialize ASI loader
void __fastcall InitializeASILoader() {
        

    std::wstring MessageINIT = L"ASI Loader v.1.1 by Pr!zRaK initialized! \n";
    WriteToLogFile(MessageINIT);

    wchar_t modulePath[MAX_PATH];
    GetModuleFileNameW(nullptr, modulePath, MAX_PATH);

    std::wstring ASIDirectory = std::filesystem::path(modulePath).parent_path().wstring();
    std::wstring DLLDirectory = std::filesystem::path(modulePath).parent_path().wstring();

    std::vector<std::wstring> asiFiles;
    std::vector<std::wstring> dllFiles;
    GetASIFiles(ASIDirectory, asiFiles);
    GetDLLFiles(DLLDirectory, dllFiles);

    for (const auto& ASIFile : asiFiles) {
        HMODULE ASIHandle = LoadLibraryExW(ASIFile.c_str(), nullptr, 0x00000008);
        if (ASIHandle != nullptr) {
            loadedASIModules.push_back(ASIHandle);
            std::wstring Messagetrue = L"Plugin " + ASIFile + L" loaded! \n";
            WriteToLogFile(Messagetrue);
        } else {
            DWORD errorCode = GetLastError();
            std::wstring errorMessage = L"Error loading: " + ASIFile + L" Error code: " + std::to_wstring(errorCode) + L"\n";
            WriteToLogFile(errorMessage);
        }
    }

     for (const auto& DLLFile : dllFiles) {
    HMODULE DLLHandle = LoadLibraryExW(DLLFile.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (DLLHandle != nullptr) { 
        loadedDLLModules.push_back(DLLHandle);
        std::wstring messageTrue = L"Dynamic Link Library " + DLLFile + L" loaded successfully!\n";
        WriteToLogFile(messageTrue);
    } else {
        DWORD errorCode = GetLastError();
        std::wstring errorMessage = L"Error loading: " + DLLFile + L" Error code: " + std::to_wstring(errorCode) + L"\n";
        WriteToLogFile(errorMessage);

        }
    }
}

// Function to free resources
void FreeResources() {
    for (HMODULE &asiModule : loadedASIModules) {
	delete[] asiModule;
	FreeLibrary(asiModule);
    }
    loadedASIModules.clear();

    for (HMODULE &dllModule : loadedDLLModules) {
	delete[] dllModule;
	FreeLibrary(dllModule);
    }
    loadedDLLModules.clear();

    if (logFile.is_open()) {
        logFile.close();
    }
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule); // Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH calls
            InitializeASILoader();
	    MessageBoxW(nullptr, L"ASI Loader 1.1 Успешно иниацилизирован", L" ", MB_OK);
            break;
        case DLL_PROCESS_DETACH:
            FreeResources();
            break;
    }
    return TRUE;
}
