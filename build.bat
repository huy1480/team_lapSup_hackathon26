@echo off
set /p api_key="API Key: "

echo Checking python dependencies...

:: Batch doesn't have a direct equivalent to pacman, 
:: so we default to pip for Windows environments.
pip install python-dotenv google-generativeai

if not exist "build" (
    mkdir build
)
cd build

:: Create the .env file and write the key
echo GEMINI_API_KEY='%api_key%' > .env

:: Run CMake build process
cmake ..
cmake --build .

:: Copy assets folder (using xcopy for directories)
xcopy /E /I /Y "..\assets" ".\assets"

:: Move the specific python file
move ".\assets\gemini_dialog.py" "."

echo Setup complete.
pause
