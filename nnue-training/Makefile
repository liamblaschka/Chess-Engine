macos:
	clang++ -std=c++17 -dynamiclib \
	cpp/DataLoader.cpp cpp/Dataset.cpp cpp/SparseBatch.cpp \
	-o build/libdataloader.dylib \

linux:
	g++ -std=c++17 -fPIC -shared -pthread \
	cpp/DataLoader.cpp cpp/Dataset.cpp cpp/SparseBatch.cpp cpp/SparseFeatures.cpp \
	-o build/libdataloader.so