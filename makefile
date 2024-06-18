FLAGS := -lws2_32 -lole32 -lgdi32
INCLUDE := C:\Users\Admin\Desktop\boost_1_85_0
LIBRARY := C:\Users\Admin\Desktop\boost_1_85_0\stage\lib

all:
	g++ serialtest.cpp -o serialtest -I$(INCLUDE) -L$(LIBRARY) $(LIBRARY)\libboost_serialization-mgw13-mt-x64-1_85.a $(LIBRARY)\libboost_system-mgw13-mt-x64-1_85.a $(FLAGS)