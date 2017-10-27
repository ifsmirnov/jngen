CXXFLAGS = -O2 -Wall -Wextra -Werror -std=c++11
CXXFLAGS += -DJNGEN_DECLARE_ONLY
LDFLAGS += -lboost_system -lboost_unit_test_framework
CXXFLAGS += -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC

CXX = g++-5
# CXX = clang++-3.5

.PHONY: clean run

run: main
	./main

lib.o: ../jngen.h ../lib.cpp
	$(CXX) $(CXXFLAGS) -UJNGEN_DECLARE_ONLY ../lib.cpp -c -o lib.o

%.o: %.cpp  ../jngen.h
	$(CXX) $(CXXFLAGS) $< -c

main: $(subst .cpp,.o,$(wildcard *.cpp)) lib.o
	$(info $^)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf -- main *.o
