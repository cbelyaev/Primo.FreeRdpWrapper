## FreeRDP Wrapper для Primo RPA

### Сборка для Windows- простой сценарий

* Необходима Visual Studio 2022 установленная `C:\Program Files` с поддержкой разработки на C++, включая Git.

```
git clone https://github.com/cbelyaev/Primo.FreeRdpWrapper.git
cd Primo.FreeRdpWrapper
rebuild.bat
```

#### Подробный сценарий

* Необходима Visual Studio 2022 установленная `C:\Program Files` с поддержкой разработки на C++, включая Git.

```
git clone https://github.com/cbelyaev/Primo.FreeRdpWrapper.git
cd Primo.FreeRdpWrapper

git clone  --depth 1 --branch 2023.10.19 https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat -disableMetrics

.\vcpkg\vcpkg.exe install freerdp:x64-windows-static-release

msbuild "Primo.FreeRdpWrapper.sln" /p:Configuration=Debug /p:Platform=x64
msbuild "Primo.FreeRdpWrapper.sln" /p:Configuration=Release /p:Platform=x64
```

#### Primo.FreeRdpWrapper.dll
Целевые сборки находятся в каталогах `x64/Release` и `x64/Debug`.

### Сборка для Linux - простой сценарий

```
git clone https://github.com/cbelyaev/Primo.FreeRdpWrapper.git
cd Primo.FreeRdpWrapper
./rebuild.sh
```

#### Подробный сценарий

```
git clone https://github.com/cbelyaev/Primo.FreeRdpWrapper.git
cd Primo.FreeRdpWrapper

git clone  --depth 1 --branch 2023.10.19 https://github.com/Microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh -disableMetrics

cp x64-linux-static-release.cmake ./vcpkg/triplets/community
./vcpkg/vcpkg install freerdp:x64-linux-static-release

mkdir -p ./x64/Debug ./x64/Release
gcc \
    -DDEBUG \
    -shared \
    -o ./x64/Debug/Primo.FreeRdpWrapper.so \
    -fPIC \
    -I./vcpkg/installed/x64-linux-static-release/include \
    -Wall \
    Logging.c \
    FreeRdpWrapper.c \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp2.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp-client2.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libwinpr2.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libssl.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libcrypto.a
gcc \
    -shared \
    -o ./x64/Release/Primo.FreeRdpWrapper.so \
    -fPIC \
    -I./vcpkg/installed/x64-linux-static-release/include \
    -Wall \
    Logging.c \
    FreeRdpWrapper.c \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp2.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libfreerdp-client2.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libwinpr2.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libssl.a \
    ./vcpkg/installed/x64-linux-static-release/lib/libcrypto.a
strip ./x64/Release/Primo.FreeRdpWrapper.so
```

#### Primo.FreeRdpWrapper.so
Целевые сборки находятся в каталогах `x64/Release` и `x64/Debug`.
