CC = g++
CFLAGS = -Wall -O3 -Ofast -std=c++11 
TARGET = dym
INCLUDES = src/CorpusParser.hpp src/ProbabilityCorpusParser.hpp src/SimpleCorpusParser.hpp src/Suggest.hpp src/Tri.hpp src/Tri.tpp.hpp src/Searcher.hpp src/CorrectionSearcher.hpp src/CorrectionSearcher.tpp.hpp src/IO.hpp src/IO_.hpp

all: $(TARGET)

$(TARGET): src/main.o src/IO.o
	$(CC) $(CFLAGS) -o bin/$(TARGET) src/main.o src/IO.o

src/main.o: src/main.cpp $(INCLUDES)
	$(CC) $(CFLAGS) -c -o src/main.o src/main.cpp

src/IO.o: src/IO.cpp $(INCLUDES)
	$(CC) $(CFLAGS) -c -o src/IO.o src/IO.cpp

documentation: src/main.cpp src/IO.cpp $(INCLUDES)
	doxygen Doxyfile
clean: 
	rm src/main.o src/IO.o bin/$(TARGET)
