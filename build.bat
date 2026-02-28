@echo off

pip install genai python-dotenv
mkdir build
cd build
cmake ..
cmake --build .
xcopy /E /I ..\assets .\assets
