###########################################################
#
# Makefile: hfsh2
#
###########################################################
.SUFFIXES: .h .c .cpp .l .o

CC = gcc
CXX = g++
CFLAGS = -g 
LEX = flex
LIBS = -lfl -lpthread
RM = /bin/rm
RMFLAGS = -rf

EXE = hfsh2
OBJS = hfsh2.o csapp.o lex.yy.o


$(EXE): $(OBJS)
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CXX) $(CFLAGS) -c $<

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c $<

lex.yy.c: scan.l
	$(LEX) $<

clean:
	$(RM) $(RMFLAGS) *.o *~ hfsh2 lex.yy.c tests-out *.txt

test:
	./test-hfsh2.sh

test-all:
	./test-hfsh2.sh -c

