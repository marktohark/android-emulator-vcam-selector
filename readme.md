# Introduction  
This is a tool to help x86 Android emulators such as NOX, LDPlayer, etc., select a camera.  
You may find that NOX Player, LDPlayer, or similar Android emulators do not provide a camera selection feature, which means that when using DeepFaceLive or other virtual cameras, you need to disable the hardware camera from Device Manager first.  
This project hooks into the DirectShow camera enumeration function, allowing the Android emulator to only select the camera you want to use.  
P.S. So far, it has only been tested on NoxPlayer (7.0.6.1), LDPlayer (9.1.23.0), and Memu Player (9.1.8.0).  
P.S. OBS Virtual Camera can be find, but I dont know why that cant use.

# Contidion
OS: Windows 10  
NOX Player: 7.0.6.1  
LDPLayer: 9.1.23.0  
MEmu Player: 9.1.8.0

# How to use
The fastest way is to download the Release ([Download](https://github.com/marktohark/android-emulator-vcam-selector/releases/download/v1.0.0/AN9Picker.zip)), and you'll be ready to use it.  
The following are the targets for DLL injection. Except for NOX, other emulators are handled through separate programs.  
<table>
  <tr>
    <td>Emulator</td>
    <td>DLL Inject Target</td>
  </tr>
  <tr>
    <td>NOX Player 7.0.6.1</td>
    <td>Nox/bin/Nox.exe</td>
  </tr>
<tr>
    <td>LDPLayer 9.1.23.0</td>
    <td>LDPlayer/LDPlayer9/ldcam.exe</td>
  </tr>
<tr>
    <td>MEmu Player 9.1.8.0</td>
    <td>Microvirt/MEmu/screenrecord.exe</td>
  </tr>
</table>

Demo and use：[go to youtube](https://youtu.be/QQiNvyk9_lk)

# Compile
If you want to compile everything manually, you can refer to this.  
### Prepare Env
Visual Studio 2022 ([download](https://visualstudio.microsoft.com/zh-hant/downloads/))  
Go 1.23.0 ([download](https://go.dev/dl/))  
Wails 2.9.2 ([install](https://wails.io/docs/gettingstarted/installation))  
Detours ([github](https://github.com/microsoft/Detours))

### Project Introduction
An9VCamPicker => A DLL project that implements hooking of the DirectShow API. (C++20)    
An9VCamPickerUI => A GUI for communicating with the DLL. (Go 1.23.0)  
LsCamera => Enable Golang to enumerate the current system's camera list. (c++11+)    

### Step.1
Go to the An9VCamPicker project and use git clone to clone the Detours project.  
Then, open the 'x86 Native Tools Command Prompt for VS 2022' command prompt.
```shell
cd An9VCamPicker/Detours
nmake -f makefile
```
Wait for compile.
Check An9VCamPicker/Detours/bin.X86/setdll.exe exist.
Check An9VCamPicker/Detours/lib.X86/detours.lib exist.

### Step.2
Open An9VCamPicker.sln  
Set compile setting "Release", "X86".
Compile it.
P.S. If you encounter issues such as 'detours.lib' or missing include or lib files, you need to set the project's include path and lib path to the Detours directory you just compiled.
Check An9VCamPicker/Release/An9VCamPicker.dll exist.

### Step.3 
Go to the LsCamera project and open LsCamera.sln  
Set compile setting "Release", "X64".
Compile it.
Check LsCamera\x64\Release\LsCamera.exe exist.

### Step.4
Go to the An9VCamPickerUI project  
```shell
cd An9VCamPickerUI
go install github.com/wailsapp/wails/v2/cmd/wails@v2.9.2
wails build
```
Check An9VCamPickerUI/build/bin/An9VCamPickerUI.exe exist.

### Step.5
Now copy the following files into the same folder.
* An9VCamPicker/Detours/bin.X86/setdll.exe  
* An9VCamPicker/Release/An9VCamPicker.dll
* LsCamera\x64\Release\LsCamera.exe
* An9VCamPickerUI/build/bin/An9VCamPickerUI.exe

### Step.6
Enjoy
