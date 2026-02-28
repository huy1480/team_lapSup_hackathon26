#!/bin/sh

read -p "API Key: " api_key
read -p "ElevenLabs Key: " elevenlabs_key

echo "Checking python dependencies"

if [ -x "$(command -v pacman)" ]; then
	sudo pacman -S python-dotenv python-google-api-core
else
	pip install python-dotenv genai

fi

mkdir build
cd build

touch .env
echo -e "GEMINI_API_KEY='${api_key}'\nELEVENLABS_API_KEY='${elevenlabs_key}'" > .env

cmake ..
cmake --build .
cp -r ../assets .

mv ./assets/gemini_dialog.py .


