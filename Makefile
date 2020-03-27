PROGRAM   = main.out
CXX       = clang++
CXXFLAGS  = -g -std=c++2a -Wall -Wextra -Wpedantic -Iinclude
LFLAGS    = -pthread -lboost_system -lhttp_parser -lfmt

$(PROGRAM): main.o api_handlers.o routes.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LFLAGS)

main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

api_handlers.o: src/api_handlers.cpp include/routes.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

routes.o: src/routes.cpp include/routes.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean dist

clean:
	-rm *.o $(PROGRAM)

dist: clean
	-tar -czvf $(PROGRAM).tar.bz2 *
