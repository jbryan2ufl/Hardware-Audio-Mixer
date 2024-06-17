FLAGS := -lws2_32 -lole32 -lgdi32

all:
#	g++ serialtest.cpp -o serialtest -IC:\Users\Administrator\Downloads\boost_1_85_0 -LC:\Users\Administrator\Downloads\boost_1_85_0\stage\lib -llibboost_serialization-mgw13-mt-x64-1_85.a -llibboost_system-mgw13-mt-x64-1_85.a
	g++ serialtest.cpp -o serialtest -IC:\Users\Administrator\Downloads\boost_1_85_0 -LC:\Users\Administrator\Downloads\boost_1_85_0\stage\lib C:\Users\Administrator\Downloads\boost_1_85_0\stage\lib\libboost_serialization-mgw13-mt-x64-1_85.a C:\Users\Administrator\Downloads\boost_1_85_0\stage\lib\libboost_system-mgw13-mt-x64-1_85.a $(FLAGS)