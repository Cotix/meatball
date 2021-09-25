fast: src/* 
	g++ -march=native -O3 -o fast src/*.cpp

fastest: src/*
	g++ -DNDEBUG -march=native -O3 -o fast src/*.cpp

debug: src/* 
	g++ -march=native -o debug src/*.cpp
