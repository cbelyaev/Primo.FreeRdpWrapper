## Вариант FreeRDP для Primo RPA

### Инструкция по сборке
* Необходима Visual Studio 2022 установленная `C:\Program Files` с поддержкой разработки на C++, включая C++ CMake tools и Git.  

#### 
* Установите [StrawberryPerl](http://strawberryperl.com).  Убедитесь что команда `perl` доступна из переменной окружения PATH.
  На Windows 10/11 можно использовать:
```
winget install -e --id StrawberryPerl.StrawberryPerl
```

#### Сборка FreeRDP и сборка OpenSSL (необходима для FreeRDP)

* Шаги
- Клонируйте [OpenSSL](https://github.com/openssl/openssl) в `..\Primo.FreeRdpWrapper.OpenSSL` и перейдите на тэг `OpenSSL_1_0_2u` 
(сценарий `ci\Scripts\getOpenSsl.bat`)
- Соберите OpenSSL в `..\Primo.FreeRdpWrapper.OpenSSL-VC-64`.  (сценарий `ci\Scripts\buildOpenSsl`)
- С помощью CMake сгенерируйте и соберите решения Visual Studio 2022.  (сценарий `ci\Scripts\BuildFreeRDP`)
- Файл решения `FreeRDP.sln` будет находиться в каталоге `Build\x64\`.

* Сценарии
- Простой
```
cd ci/Scripts
.\PrepareFreeRdpDev
```
-- или подробный
```
cd ci/Scripts
.\getOpenSsl
.\buildOpenSsl
.\buildFreeRDP Debug
```

### Работа с Primo.FreeRdpWrapper
* Откройте `Primo.FreeRdpWrapper/Primo.FreeRdpWrapper.sln`
