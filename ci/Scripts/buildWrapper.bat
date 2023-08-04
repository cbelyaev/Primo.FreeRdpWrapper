set PATH_BACKUP=%PATH%

call "%~dp0\getvars.bat"

pushd .
cd %freeRdpDir%\Primo.FreeRdpWrapper

rem restore dependencies
echo ">>>>>>>>>>>>>> restore wrapper dependencies"
%msbuild% "Primo.FreeRdpWrapper.sln" /t:restore

rem build wrapper libs
echo ">>>>>>>>>>>>>> building wrapper configuration:Debug"
%msbuild% "Primo.FreeRdpWrapper.sln" /p:Configuration=Debug /p:Platform=x64

echo ">>>>>>>>>>>>>> building wrapper configuration:Release"
%msbuild% "Primo.FreeRdpWrapper.sln" /p:Configuration=Release /p:Platform=x64

popd

set PATH=%PATH_BACKUP%
