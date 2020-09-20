echo This script will install brew and various libraries required for the build. Wait 5 seconds if you allow us to install files into your computer.
sleep 5

which -s brew
if [[ $? != 0 ]] ; then
	brewinstalled=0
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
