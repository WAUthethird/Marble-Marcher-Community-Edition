#I wanted the compliation to be more "You just launch the script and it works" so, it's just a improved version of the manual method.

#The script installs glm manually into the working directory, because for some weird reason cmake can just never find the file. If it's against the license agreements ( the GLM this script downloads is licensed under a modified version of MIT https://github.com/g-truc/glm/blob/master/copying.txt so it SHOULD be ok. ) please let me know.

echo This script will install brew and various libraries required for the build. Wait 5 seconds if you allow us to install files into your computer.
sleep 5

which -s brew
if [[ $? != 0 ]] ; then
	ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
fi

brew install cmake eigen sfml anttweakbar glm glew

cd "$(dirname "$0")"
wget https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
unzip glm-0.9.9.8.zip

mkdir build && cd build
cmake ..
cmake -DCMAKE_CXX_FLAGS="-I/usr/local/include" ..
cd ..
cmake --build build

cd build
echo cd "$(dirname "$0")" > MarbleMarcher.sh
echo ./MarbleMarcher >> MarbleMarcher.sh
./MarbleMarcher
