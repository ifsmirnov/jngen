CXXFLAGS = -O2 -Wall -Wextra -Werror -std=c++11
CXXFLAGS += -DJNGEN_DECLARE_ONLY
LDFLAGS += -lboost_system -lboost_unit_test_framework
CXXFLAGS += -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC
CXXFLAGS += -fsanitize=undefined

CXX = g++
# CXX = clang++

.PHONY: clean run

run: main
	./main --random=1

lib.o: ../jngen.h ../lib.cpp
	$(CXX) $(CXXFLAGS) -UJNGEN_DECLARE_ONLY ../lib.cpp -c -o lib.o

%.o: %.cpp  ../jngen.h
	$(CXX) $(CXXFLAGS) $< -c

main: $(subst .cpp,.o,$(wildcard *.cpp)) lib.o
	$(info $^)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf -- main *.o
