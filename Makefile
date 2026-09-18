make:
	g++ -Isrc -Isrc/board -Isrc/movegeneration src/main.cpp src/board/Board.cpp src/movegeneration/MoveGenerator.cpp -o build/chess -std=c++17

test:
	g++ -Iinclude src/Board.cpp src/Piece.cpp src/MoveGenerator.cpp tests/MoveGeneratorTests.cpp -o build/tests -std=c++17

engine:
	g++ -std=c++17 -Iinclude \
	-O3 -march=native \
	src/main.cpp \
	src/UCI.cpp \
	src/MoveGenerator.cpp \
	src/Piece.cpp \
	src/Board.cpp \
	src/Game.cpp \
	src/Search.cpp \
	src/Accumulator.cpp \
	src/LinearLayer.cpp \
	src/NNUE.cpp \
	-o build/chess \
