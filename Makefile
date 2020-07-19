PROGRAM   = main.out
CXX       = g++
CXXFLAGS  = -g -std=c++20 -Wall -Wextra -Iinclude
LFLAGS    = -pthread -lboost_system -lhttp_parser -lfmt -lsoci_sqlite3 -lsoci_core

$(PROGRAM): main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

examples: soci_example.out json_example.out
	@echo Making examples

soci_example.out: soci_example.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

soci_example.o: examples/soci_example.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

json_example.out: json_example.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

json_example.o: examples/json_example.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main.o: main.cpp include/handler.hpp include/actions.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean dist

clean:
	-rm *.o $(PROGRAM) *.out

dist: clean
	-tar -czvf $(PROGRAM).tar.bz2 *
