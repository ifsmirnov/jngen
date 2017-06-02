CXXFLAGS = -O2 -Wall -Wextra -Werror -std=c++11
CXXFLAGS += -DJNGEN_DECLARE_ONLY
LDFLAGS += -lboost_system -lboost_unit_test_framework

CXX = g++-4.9

.PHONY: clean run

run: main
	./main

../lib.o: ../jngen.h ../lib.cpp
	cd .. && $(CXX) $(CXXFLAGS) -UJNGEN_DECLARE_ONLY lib.cpp -c

%.o: %.cpp  ../jngen.h
	$(CXX) $(CXXFLAGS) $< -c

main: $(subst .cpp,.o,$(wildcard *.cpp)) ../lib.o
	$(info $^)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -rf -- main *.o
