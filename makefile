DFLAG = -g
WFLAG = -Wall
C11FLAG = -std=c++0x
THREADFLAG = -pthread

SRCC = birdmain.cpp\
			 birdmsg.cpp\
			 PracticalSocket.cpp
INCC = birdinc.h\
       birdprot.h\
       comconst.h\
			 PracticalSocket.h
OBJC = $(SRCC:.cpp=.o)
SRCDIR = src
OBJDIR = obj
INCDIR = inc
MVOBJ = mv -f *.o obj/

CSRC = $(patsubst %,$(SRCDIR)/%,$(SRCC))
COBJ = $(patsubst %,$(OBJDIR)/%,$(OBJC))
CINC = $(patsubst %,$(INCDIR)/%,$(INCC))


SRCP = pigmain.cpp\
			 pigmsg.cpp\
			 pigelect.cpp\
			 PracticalSocket.cpp
INCP = piginc.h\
			 pigds.h\
			 comconst.h\
			 PracticalSocket.h
OBJP = $(SRCP:.cpp=.o)

PSRC = $(patsubst %,$(SRCDIR)/%,$(SRCP))
POBJ = $(patsubst %,$(OBJDIR)/%,$(OBJP))
PINC = $(patsubst %,$(INCDIR)/%,$(INCP))

CREATEDIR = mkdir -p obj bin

all: bird pig

bird: $(COBJ)
	$(CREATEDIR)
	g++ -o bin/bird $(WFLAG) $(COBJ) -lm $(THREADFLAG)

obj/PracticalSocket.o: src/PracticalSocket.cpp $(CINC)
	$(CREATEDIR)
	g++ -c src/PracticalSocket.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/PracticalSocket.o

obj/birdmain.o: src/birdmain.cpp $(CINC)
	$(CREATEDIR)
	g++ -c src/birdmain.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/birdmain.o

obj/birdmsg.o: src/birdmsg.cpp $(CINC)
	$(CREATEDIR)
	g++ -c src/birdmsg.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/birdmsg.o

pig: $(POBJ)
	$(CREATEDIR)
	g++ -o bin/pig $(WFLAG) $(POBJ) -lm $(THREADFLAG)

obj/pigmain.o: src/pigmain.cpp $(PINC)
	$(CREATEDIR)
	g++ -c src/pigmain.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/pigmain.o

obj/pigelect.o: src/pigelect.cpp $(PINC)
	$(CREATEDIR)
	g++ -c src/pigelect.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/pigelect.o

obj/pigmsg.o: src/pigmsg.cpp $(PINC)
	$(CREATEDIR)
	g++ -c src/pigmsg.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/pigmsg.o

clean:
	rm -rf bin/*
	rm -rf obj/*
