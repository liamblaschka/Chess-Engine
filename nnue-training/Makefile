macos:
	clang++ -std=c++17 -dynamiclib \
	cpp/DataLoader.cpp cpp/Dataset.cpp cpp/SparseBatch.cpp \
	-o build/libdataloader.dylib \

windows:
	g++ -std=c++17 -shared \
	cpp/DataLoader.cpp cpp/Dataset.cpp cpp/SparseBatch.cpp \
	-o build/libdataloader.dll \