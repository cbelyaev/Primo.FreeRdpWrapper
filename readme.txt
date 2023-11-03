git clone https://github.com/cbelyaev/Primo.FreeRdpWrapper.git
cd Primo.FreeRdpWrapper

git clone  --depth 1 --branch 2023.10.19 https://github.com/Microsoft/vcpkg.git
git clone  --depth 1 --branch 2023.08.09 https://github.com/Microsoft/vcpkg.git




.\vcpkg\bootstrap-vcpkg.bat -disableMetrics
./vcpkg/bootstrap-vcpkg.sh -disableMetrics

cp x64-linux-static-release.cmake ./vcpkg/triplets/community

xcopy .\freerdp .\vcpkg\ports\freerdp /R /Y /Q
cp -r freerdp ./vcpkg/ports

.\vcpkg\vcpkg.exe install freerdp:x64-windows-static-release
./vcpkg/vcpkg install freerdp:x64-linux-static-release

msbuild "Primo.FreeRdpWrapper.sln" /p:Configuration=Debug /p:Platform=x64
msbuild "Primo.FreeRdpWrapper.sln" /p:Configuration=Release /p:Platform=x64





Centos Stream 8
yum install git gcc gcc-c++ python3

Redos
dnf install git gcc gcc-c++ perl-IPC-Cmd perl-FindBin perl-File-Compare perl-File-Copy

Astra
apt install git curl zip unzip tar




2.11.2 722d95d7591b5ce6a7e8a3b6ac8999df278dbcfc286a532f56bcbc4a3881e75b02c7e3cd4b296e67bc19d1165020acdcca198bf4bcc92aea5611760037fcc57f

2.9.0
8b43ff28c5afaf6dc12f73fe9cab3969049f40725dedc873af5af1caabefec4bdd1bedad7df121c1440493e5441296db076ba7726242bfb32f76dad0b21564ed