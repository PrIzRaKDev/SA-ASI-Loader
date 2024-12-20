#include "dllmain.h"
#include "pch.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <mutex>
#include <memory>

using namespace std;

namespace fs = std::filesystem;

class ASILoader {
public:
    ASILoader() = default;
    ~ASILoader() { FreeResources(); }

    void Initialize() {
        OpenLogFile();
        WriteToLog(L"ASI Loader v1.2.0 initialized.\n");
        exeUnprotect();
        LoadPlugins();
    }

private:
    vector<unique_ptr<HMODULE, decltype(&FreeLibrary)>> loadedASIModules{ &FreeLibrary };
    vector<unique_ptr<HMODULE, decltype(&FreeLibrary)>> loadedDLLModules{ &FreeLibrary };
    wofstream logFile;
    mutex logMutex;

    void OpenLogFile() {
        logFile.open(L"ASI_Loader.log", ios_base::app);
        if (!logFile.is_open()) {
            wcerr << L"Failed to open log file." << endl;
            throw runtime_error("Log file could not be opened.");
        }
    }

    void WriteToLog(const wstring& message) {
        lock_guard<mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << message << endl;
            logFile.flush();
        }
    }

    void exeUnprotect() {
        auto hExecutableInstance = reinterpret_cast<size_t>(GetModuleHandle(nullptr));
        auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(hExecutableInstance + reinterpret_cast<IMAGE_DOS_HEADER*>(hExecutableInstance)->e_lfanew);
        SIZE_T size = ntHeader->OptionalHeader.SizeOfImage;
        DWORD oldProtect;

        if (!VirtualProtect(reinterpret_cast<void*>(hExecutableInstance), size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            MessageBoxW(nullptr, L"Failed to unprotect executable!", L"Error", MB_OK);
        } else {
            MessageBoxW(nullptr, L"Debug: exeUnprotect succeeded!", L"Success", MB_OK);
            WriteToLog(L"Exe unprotected successfully!");
        }
    }

    vector<fs::path> GetPluginFiles(const fs::path& directory, const wstring& extension) {
        vector<fs::path> files;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == extension) {
                files.push_back(entry.path());
            }
        }
        return files;
    }

    void LoadPlugins() {
        fs::path modulePath = fs::current_path();
        fs::path pluginDirectory = modulePath.parent_path();

        auto asiFiles = GetPluginFiles(pluginDirectory, L".asi");
        auto dllFiles = GetPluginFiles(pluginDirectory, L".dll");

        for (const auto& file : asiFiles | std::ranges::views::filter(fs::exists)) {
            LoadPlugin(file, loadedASIModules);
        }

        for (const auto& file : dllFiles | std::ranges::views::filter(fs::exists)) {
            LoadPlugin(file, loadedDLLModules);
        }
    }

    void LoadPlugin(const fs::path& filePath, vector<unique_ptr<HMODULE, decltype(&FreeLibrary)>>& modules) {
        HMODULE handle = LoadLibraryExW(filePath.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (handle) {
            modules.emplace_back(handle, &FreeLibrary);
            WriteToLog(L"Plugin loaded: " + filePath.wstring());
        } else {
            DWORD errorCode = GetLastError();
            WriteToLog(L"Failed to load plugin: " + filePath.wstring() + L" Error Code: " + to_wstring(errorCode));
        }
    }

    void FreeResources() {
        loadedASIModules.clear();
        loadedDLLModules.clear();
    }
};


// TODO: BUG. There is an endless loading of models, the collision breaks and the lodes appear.
extern "C" __declspec(dllexport) void __cdecl ASILoader() {
    try {
        ASILoader loader;
        loader.Initialize();
    } catch (const exception& e) {
        wcerr << L"ASI Loader encountered an error: " << e.what() << endl;
    }
}
