macos:
	clang++ -std=c++17 -dynamiclib \
	cpp/DataLoader.cpp cpp/Dataset.cpp cpp/SparseBatch.cpp \
	-o build/libdataloader.dylib \

linux:
	g++ -std=c++17 -shared \
	cpp/DataLoader.cpp cpp/Dataset.cpp cpp/SparseBatch.cpp \
	-o build/libdataloader.so \