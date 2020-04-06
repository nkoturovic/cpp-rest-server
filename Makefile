PROGRAM   = main.out
CXX       = g++
CXXFLAGS  = -g -std=c++2a -Wall -Wextra -Wpedantic -Iinclude -fconcepts
LFLAGS    = -pthread -lboost_system -lhttp_parser -lfmt -lsoci_sqlite3 -lsoci_core

$(PROGRAM): main.o api_handlers.o route.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

examples: soci_example.out
	@echo Making examples

soci_example.out: soci_example.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

soci_example.o: examples/soci_example.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main.o: src/main.cpp include/api_routes.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

api_handlers.o: src/api_handlers.cpp include/route.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

route.o: src/route.cpp include/route.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean dist

clean:
	-rm *.o $(PROGRAM) *.out

dist: clean
	-tar -czvf $(PROGRAM).tar.bz2 *
