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
#include <unordered_set>

using namespace std;
namespace fs = std::filesystem;

class ASILoader {
public:
    ASILoader() = default;
    ~ASILoader() { FreeResources(); }

    void Initialize() {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        OpenLogFile();
        WriteToLog(L"ASI Loader v1.2.1 rc2 initialized.\n");

        if (!exeUnprotect()) {
            WriteToLog(L"[ERROR] Failed to unprotect executable!");
            return;
        }

        LoadPlugins();
        LogLoadedModules();
    }

private:
    vector<unique_ptr<HMODULE, decltype(&FreeLibrary)>> loadedASIModules{ &FreeLibrary };
    vector<unique_ptr<HMODULE, decltype(&FreeLibrary)>> loadedDLLModules{ &FreeLibrary };
    unordered_set<wstring> loadedModuleNames;
    wofstream logFile;
    mutex logMutex;

    void OpenLogFile() {
        logFile.open(L"ASI_Loader.log", ios_base::app);
        if (!logFile.is_open()) {
            wcerr << L"[ERROR] Failed to open log file." << endl;
            throw runtime_error("Log file could not be opened.");
        }
    }

    void WriteToLog(const wstring& message) {
        lock_guard<mutex> lock(logMutex);
        if (logFile.is_open()) {
            logFile << message << endl;
            logFile.flush();
        }
        wcout << message << endl;
    }

    bool exeUnprotect() {
        auto hExecutableInstance = reinterpret_cast<size_t>(GetModuleHandle(nullptr));
        if (!hExecutableInstance) return false;

        auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(hExecutableInstance);
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return false;

        auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(hExecutableInstance + dosHeader->e_lfanew);
        if (ntHeader->Signature != IMAGE_NT_SIGNATURE) return false;

        SIZE_T size = ntHeader->OptionalHeader.SizeOfImage;
        DWORD oldProtect;

        return VirtualProtect(reinterpret_cast<void*>(hExecutableInstance), size, PAGE_EXECUTE_READWRITE, &oldProtect);
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
        wchar_t modulePath[MAX_PATH];
        GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
        fs::path pluginDirectory = fs::path(modulePath).parent_path();

        auto asiFiles = GetPluginFiles(pluginDirectory, L".asi");
        auto dllFiles = GetPluginFiles(pluginDirectory, L".dll");

        for (const auto& file : asiFiles) {
            LoadPlugin(file, loadedASIModules);
        }

        for (const auto& file : dllFiles) {
            LoadPlugin(file, loadedDLLModules);
        }
    }

    void LoadPlugin(const fs::path& filePath, vector<unique_ptr<HMODULE, decltype(&FreeLibrary)>>& modules) {
        if (!fs::exists(filePath)) {
            WriteToLog(L"[ERROR] Plugin not found: " + filePath.wstring());
            return;
        }

        if (loadedModuleNames.count(filePath.filename().wstring())) {
            WriteToLog(L"[WARNING] Plugin already loaded, skipping: " + filePath.wstring());
            return;
        }

        HMODULE handle = LoadLibraryExW(filePath.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (handle) {
            modules.emplace_back(handle, &FreeLibrary);
            loadedModuleNames.insert(filePath.filename().wstring());
            WriteToLog(L"[INFO] Plugin loaded: " + filePath.wstring());

            // Проверяем конфликт загрузки (если ASI-плагин использует DllMain)
            if (GetProcAddress(handle, "DllMain")) {
                WriteToLog(L"[WARNING] Plugin " + filePath.wstring() + L" содержит DllMain, возможен конфликт с другими ASI-плагинами.");
            }
        } else {
            DWORD errorCode = GetLastError();
            WriteToLog(L"[ERROR] Failed to load plugin: " + filePath.wstring() + L" Error Code: " + to_wstring(errorCode));
        }
    }

    void LogLoadedModules() {
        WriteToLog(L"\n=== ASI Loader Summary ===");
        WriteToLog(L"Total ASI Plugins Loaded: " + to_wstring(loadedASIModules.size()));
        WriteToLog(L"Total DLL Plugins Loaded: " + to_wstring(loadedDLLModules.size()));
    }

    void FreeResources() {
        loadedASIModules.clear();
        loadedDLLModules.clear();
    }
};

// Экспортируемая функция
extern "C" __declspec(dllexport) void __cdecl ASILoader() {
    try {
        ASILoader loader;
        loader.Initialize();
    } catch (const exception& e) {
        wcerr << L"[CRITICAL] ASI Loader encountered an error: " << e.what() << endl;
    }
}
