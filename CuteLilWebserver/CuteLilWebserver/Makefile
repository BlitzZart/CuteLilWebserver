CC = /usr/bin/g++
CCC = /usr/bin/gcc
LDFLAGS = -lrt -lcrypt -pthread
BUILD = debug
BUILDVER = unknown
BLDDIR = .
INCDIR = 	$(BLDDIR)/asio/asio/include
SRCDIR = 	$(BLDDIR)
OBJDIR = $(BLDDIR)/bin-$(BUILD)
INC_PARAMS = $(INCDIR:%=-I%)
CFLAGS = -g -c -Wall -std=c++11 -DASIO_STANDALONE -lstdc++fs $(INC_PARAMS)
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ1 = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
OBJ = $(patsubst $(SRCDIR)/%.hpp, $(OBJDIR)/%.o, $(OBJ1))

EXE = $(OBJDIR)/clws

CXXFLAGS.debug = -Og -fstack-protector-all
CXXFLAGS.release = -O3 -march=native -DNDEBUG
CXXFLAGS = ${CFLAGS} ${CXXFLAGS.${BUILD}}

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJDIR)/*.o -o $@ $(LDFLAGS) 

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o : $(SRCDIR)/%.hpp
	@mkdir -p $(@D)
	$(CC) $(CXXFLAGS) $< -o $@

clean:
	-rm -f $(OBJDIR)/*.o $(EXE)