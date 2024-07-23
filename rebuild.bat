@echo off

if exist .\vcpkg rmdir /Q /S .\vcpkg
if exist .\x64 rmdir /Q /S .\x64

set vswhere="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`%vswhere% -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set msbuild="%%i"
)
for /f "usebackq delims=" %%i in (`%vswhere% -prerelease -latest -property installationPath`) do (
  set vsDir=%%i
)
call "%vsdir%\Common7\Tools\vsdevcmd.bat"

git clone  --depth 1 --branch 2024.07.12 https://github.com/Microsoft/vcpkg.git
call .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg\vcpkg.exe install freerdp:x64-windows-static-release

msbuild "Primo.FreeRdpWrapper.sln" /p:Configuration=Debug /p:Platform=x64
msbuild "Primo.FreeRdpWrapper.sln" /p:Configuration=Release /p:Platform=x64
