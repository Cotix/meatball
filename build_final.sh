cat src/*.hpp src/*.cpp > tmp.cpp
sed '/#include "\w\.hpp"/d' ./tmp.cpp > result.cpp
rm tmp.cpp