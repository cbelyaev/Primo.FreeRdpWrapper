## Вариант FreeRDP для Primo RPA

### Инструкция по сборке
* Необходима Visual Studio 2022 установленная `C:\Program Files` с поддержкой разработки на C++, включая C++ CMake tools и Git.  

#### 
* Установите [StrawberryPerl](http://strawberryperl.com).  Убедитесь что команда `perl` доступна из переменной окружения PATH.
  На Windows 10/11 можно использовать:
```
winget install -e --id StrawberryPerl.StrawberryPerl
```

#### Сборка FreeRDP, OpenSSL (необходима для FreeRDP) и FreeRdpWrapper

* Шаги
- Клонируйте [OpenSSL](https://github.com/openssl/openssl) в `..\Primo.FreeRdpWrapper.OpenSSL` и перейдите на тэг `OpenSSL_1_0_2u` 
(сценарий `ci\Scripts\getOpenSsl.bat`)
- Соберите OpenSSL в `..\Primo.FreeRdpWrapper.OpenSSL-VC-64`.  (сценарий `ci\Scripts\buildOpenSsl`)
- С помощью CMake сгенерируйте и соберите решения Visual Studio 2022.  (сценарий `ci\Scripts\BuildFreeRDP`)
- Файл решения `FreeRDP.sln` будет находиться в каталоге `Build\x64\`.
- Откройте `FreeRDP.sln` в Visual Studio 2022 и скомпилируйте библиотеку для конфигураций Release и Debug.
- Откройте `Primo.FreeRdpWrapper/Primo.FreeRdpWrapper.sln` в Visual Studio 2022 и скомпилируйте библиотеку для конфигураций Release и Debug.

* Простой сценарий
- 
```
cd ci/Scripts
.\BuildAll
```

* Подробный сценарий
```
cd ci/Scripts
.\getOpenSsl
.\buildOpenSsl
.\buildFreeRDP
.\buildWrapper
```

* Primo.FreeRdpWrapper.dll
Целевые сборки находятся в каталогах `Output/bin/Release/x64` и `Output/bin/Debug/x64`.
