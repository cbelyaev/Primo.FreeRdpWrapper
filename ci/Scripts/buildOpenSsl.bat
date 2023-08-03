call %~dp0\getvars.bat

pushd .
cd %freeRdpDir%\..\Primo.FreeRdpWrapper.OpenSSL
git clean -xdff

pushd .
call "%vsDir%\VC\Auxiliary\Build\vcvars64.bat"
popd

perl Configure VC-WIN64A no-asm --prefix=..\Primo.FreeRdpWrapper.OpenSSL-VC-64
call ms\do_win64a
nmake -f ms\nt.mak
nmake -f ms\nt.mak install
popd
