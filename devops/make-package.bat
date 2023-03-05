@echo off

setlocal

if "%~1" == "" (
  echo Package revision must be provided as the first argument
  goto :EOF
)

set PKG_VER=1.1.0
set PKG_REV=%~1

set RAPIDJSON_FNAME=rapidjson-%PKG_VER%.tar.gz
set RAPIDJSON_DNAME=rapidjson-%PKG_VER%
set RAPIDJSON_SHA256=bf7ced29704a1e696fbccf2a2b4ea068e7774fa37f6d7dd4039d0787f8bed98e

set PATCH=%PROGRAMFILES%\Git\usr\bin\patch.exe
set SEVENZIP_EXE=%PROGRAMFILES%\7-Zip\7z.exe
set VCVARSALL=%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall

curl --location --output %RAPIDJSON_FNAME% https://github.com/Tencent/rapidjson/archive/refs/tags/v%PKG_VER%.tar.gz

if ERRORLEVEL 1 (
  echo Failed to download %RAPIDJSON_FNAME%
  goto :EOF
)

"%SEVENZIP_EXE%" h -scrcSHA256 %RAPIDJSON_FNAME% | findstr /C:"SHA256 for data" | call devops\check-sha256 "%RAPIDJSON_SHA256%"

if ERRORLEVEL 1 (
  echo SHA-256 signature for %RAPIDJSON_FNAME% does not match
  goto :EOF
)

tar xzf %RAPIDJSON_FNAME%

cd %RAPIDJSON_DNAME%

rem
rem Patch the source to work around build problems described for
rem each patch.
rem

rem Replace auto_ptr with unique_ptr to allow compiling in C++17
"%PATCH%" -p1 --unified --input ..\patches\01-c++17.patch
"%PATCH%" -p1 --unified --input ..\patches\02-cmake-install-prefix.patch

call "%VCVARSALL%" x64

rem
rem Generate build projects
rem
cmake -S . -B build -A x64 ^
    -DRAPIDJSON_BUILD_DOC=OFF ^
    -DRAPIDJSON_BUILD_EXAMPLES=OFF ^
    -DRAPIDJSON_BUILD_TESTS=OFF ^
    -DRAPIDJSON_BUILD_THIRDPARTY_GTEST=OFF ^
    -DRAPIDJSON_HAS_STDSTRING=ON

rem
rem Build x64 Debug/Release
rem 
rem Without examples and/or tests nothing is actually built
rem because it's a header-only package.
rem 
cmake --build build --config Debug
cmake --build build --config Release

rem
rem Collect artifacts
rem
mkdir ..\nuget\licenses
copy license.txt ..\nuget\licenses\

mkdir ..\nuget\build\native\include\rapidjson
xcopy /Y /S include\rapidjson\* ..\nuget\build\native\include\rapidjson\

cd ..

rem
rem Create a package
rem
nuget pack nuget\StoneSteps.RapidJSON.VS2022.Static.nuspec -Version %PKG_VER%.%PKG_REV%
