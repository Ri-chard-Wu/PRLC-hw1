CXX = g++
CXXFLAGS = -ltbb -std=c++17 -O3 -pthread -fopenmp
TARGETS = hw1

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS)
