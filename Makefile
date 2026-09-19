make:
	g++ -Isrc -Isrc/board -Isrc/movegeneration src/main.cpp src/board/Board.cpp src/movegeneration/MoveGenerator.cpp -o build/chess -std=c++17

test:
	g++ -Iinclude src/Board.cpp src/Piece.cpp src/MoveGenerator.cpp tests/MoveGeneratorTests.cpp -o build/tests -std=c++17

engine:
	g++ -std=c++17 \
	-O3 -march=native \
	-Iinclude \
	src/main.cpp \
	src/UCI.cpp \
	src/MoveGenerator.cpp \
	src/Piece.cpp \
	src/Board.cpp \
	src/Game.cpp \
	src/Search.cpp \
	src/NNUE.cpp \
	-o build/chess \

# engine:
# 	g++ -std=c++17 \
# 	-O3 -march=native \
# 	-fopt-info-vec-optimized=vec.txt \
# 	-fopt-info-vec-missed=vec-missed.txt \
# 	-Iinclude \
# 	src/main.cpp \
# 	src/UCI.cpp \
# 	src/MoveGenerator.cpp \
# 	src/Piece.cpp \
# 	src/Board.cpp \
# 	src/Game.cpp \
# 	src/Search.cpp \
# 	src/NNUE.cpp \
# 	-o build/chess

datagenerator:
	g++ -std=c++17 \
	-O3 -march=native \
	-Iinclude \
	src/DataGeneratorMain.cpp \
	src/DataGenerator.cpp \
	src/MoveGenerator.cpp \
	src/Piece.cpp \
	src/Board.cpp \
	src/Game.cpp \
	src/Search.cpp \
	src/NNUE.cpp \
	-o build/datagenerator \