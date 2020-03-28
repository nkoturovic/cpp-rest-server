PROGRAM   = main.out
CXX       = clang++
CXXFLAGS  = -g -std=c++2a -Wall -Wextra -Wpedantic -Iinclude
LFLAGS    = -pthread -lboost_system -lhttp_parser -lfmt

$(PROGRAM): main.o api_handlers.o route.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

main.o: src/main.cpp include/api_routes.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

api_handlers.o: src/api_handlers.cpp include/route.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

route.o: src/route.cpp include/route.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean dist

clean:
	-rm *.o $(PROGRAM)

dist: clean
	-tar -czvf $(PROGRAM).tar.bz2 *
