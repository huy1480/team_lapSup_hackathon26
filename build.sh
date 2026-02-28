#!/bin/sh

read -p "API Key: " api_key

echo "Checking python dependencies"

if [ -x "$(command -v pacman)" ]; then
	sudo pacman -S python-dotenv python-google-api-core
else
	pip3 install python-dotenv
	pip3 install -q -U google-genai
fi

mkdir build
cd build

touch .env
echo "GEMINI_API_KEY='${api_key}'" > .env

cmake ..
cmake --build .
cp -r ../assets .

mv ./assets/gemini_dialog.py .
