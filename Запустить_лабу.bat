@echo off
cd /d "%~dp0"

if not exist "build\Debug\sdl2-demo.exe" (
  echo Program file was not found.
  echo Build the project first or ask to rebuild it.
  pause
  exit /b 1
)

start "" "build\Debug\sdl2-demo.exe"
