#include "dllmain.h"
#include "pch.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#define MAX_PATH 512

using namespace std;


// Global variables for storing loaded modules and log file
std::vector<HMODULE> loadedASIModules;
std::vector<HMODULE> loadedDLLModules;
std::wofstream logFile;

// Function to get list of .asi files
void GetASIFiles(const std::wstring& directory, std::vector<std::wstring>& out) {
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

void GetDLLFiles(const std::wstring& directory, std::vector<std::wstring>& out) {
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
        logFile.open(L"ASI_Loader.log", std::ios_base::app);
        logFile << message << std::endl;
    }
    else if (logFile.is_open())
    {
        logFile << message << std::endl;
    }
    else 
    {
        std::wcerr << L"Failed to open log file." << std::endl;
        return;
    }
}

static void exeUnprotect()
{
    /* This "tool" using for unprotecting any code regions in game executable file, for correct writing and/or reading memory sections. */
            auto hExecutableInstance = (size_t)GetModuleHandle(NULL);
            IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(hExecutableInstance + ((IMAGE_DOS_HEADER*)hExecutableInstance)->e_lfanew);
            SIZE_T size = ntHeader->OptionalHeader.SizeOfImage;
            DWORD oldProtect;
	    VirtualProtect((VOID*)hExecutableInstance, size, PAGE_EXECUTE_READWRITE, &oldProtect);
            if (!VirtualProtect((VOID*)hExecutableInstance, size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
		   MessageBoxW(nullptr, L"Не удалось сделать unprotect!", L"Ошибка!", MB_OK);
		} 
		else 
		{
		   MessageBoxW(nullptr, L"DebugMsg: exeUnprotect успешно выполнен!", L"Успешно! Debug", MB_OK);
		   WriteToLogFile(L"Exe unprotected successfully!");
		}	

}



// Function to initialize ASI loader
static void InitializeASILoader() {

    if (logFile.is_open())
    {
        logFile.clear();
    }
    
    std::wstring MessageINIT = L"ASI Loader v.1.2.0 by Pr!zRaK initialized! \n";
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
        HMODULE ASIHandle = LoadLibraryExW(ASIFile.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
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
static void FreeResources() {
    for (HMODULE asiModule : loadedASIModules) {
	FreeLibrary(asiModule);
    }
    loadedASIModules.clear();

    for (HMODULE dllModule : loadedDLLModules) {
	FreeLibrary(dllModule);
    }
    loadedDLLModules.clear();

    if (logFile.is_open()) {
        logFile.close();
    }
}

// DLL entry point
static BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule); // Disable DLL_THREAD_ATTACH and DLL_THREAD_DETACH calls
            InitializeASILoader();
	    exeUnprotect();
	        MessageBoxW(nullptr, L"ASI Loader 1.2.0 Успешно иниацилизирован!", L" ", MB_OK);
		
            break;
        case DLL_PROCESS_DETACH:
            FreeResources();
            break;
    }
    return TRUE;
}
