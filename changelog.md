# 1.1-alpha :white_check_mark:
**Jun 22** 
* First Alpha Release! v1.1 🎈
* Supports loading the `.asi` and `.dll` files.
* Supports the following versions of the `gta_sa.exe` : **1.0 US**, **1.01**, **3.0 (steam)**
* Supports **CLEO** 
* has `.log` file 

# 1.2.0-alpha (rc1) :white_check_mark:
**Jun 29**
* Renamed `vorbisFileHooked.dll` to `vorbisHooked.dll`
* Minor fixes
* Changed project settings

# 1.2.1-alpha (rc2) :x: **[needs testing]**
**Feb 1**
* Updated main file. ✅
* Minor fixes. ✅
* Added **resource release** feature. ✅ 
* Some project solution files have been updated ✅
* Big **refactoring** ✅
*   Added validation of **PE headers.** 
    Now the function returns `bool` to log errors correctly. ✅
* Instead of `fs::current_path().parent_path()`, now `GetModuleFileNameW()` to find the **correct folder.** ✅
* File upload errors are now correctly recorded in the log. ✅
* Added logging levels **[INFO], [ERROR]**, and **[CRITICAL].** ✅
* Logging now takes place in a file, **without interfering with the UI.** ✅
* Now, before downloading the plugin, `fs::exists(filePath)` is **checked first**, and not filtered through `std::ranges`. ✅
