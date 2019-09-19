INCLUDE=-Ispeech/
LIB=-ljsoncpp -lcurl -lcrypto -lpthread
jarvis:jarvis.cc
	g++ -o $@ $^ $(LIB) -std=c++11 $(INCLUDE) #-static
.PHONY:clean 
clean:
	rm -f jarvis
