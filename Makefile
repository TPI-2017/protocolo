CXXFLAGS = -std=c++11 -O0 -g -Wall -DPOSIX

Message.o:

exec_test: Message.o test.o
	$(CXX) $(CXXFLAGS) *.o -o exec_test

test.o:

clean:
	rm -rvf *.o test exec_test