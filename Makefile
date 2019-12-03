APPNAME=$(shell basename `pwd`)

CC=cc
CXX=g++
FLAGS=-g -Wall -Wextra
CFLAGS=$(FLAGS)
CXXFLAGS=$(FLAGS)
LIBS = -pthread

all: $(APPNAME)

%: %.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	
%: %.c
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

.PHONY: clean
clean:
	rm -f $(APPNAME) *.o

.PHONY: gitignore
gitignore:
	echo $(APPNAME) > .gitignore
