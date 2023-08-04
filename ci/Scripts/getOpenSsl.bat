set PATH_BACKUP=%PATH%

call %~dp0\getvars.bat

pushd .
cd %freeRdpDir%\..

rem checkout openssl
mkdir Primo.FreeRdpWrapper.OpenSSL
cd Primo.FreeRdpWrapper.OpenSSL
git clone https://github.com/openssl/openssl .
git branch buildOpenSSl %openSSLTag%
git checkout buildOpenSSl
popd

set PATH=%PATH_BACKUP%
