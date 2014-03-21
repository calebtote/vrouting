CC=g++
CCOPTS=-g -w

OBJS = $(BINDIR)/manager.o $(BINDIR)/rtngnode.o $(BINDIR)/linkstate.o $(BINDIR)/distvec.o $(BINDIR)/rtngmsg.o
TARGETS = $(BINDIR)/manager $(BINDIR)/linkstate $(BINDIR)/distvec
BINDIR = build

all: $(TARGETS) $(OBJS)

clean:
	rm -f $(TARGETS) $(OBJS)

.PHONY: all clean

$(BINDIR)/manager : $(BINDIR)/rtngmsg.o  $(BINDIR)/manager.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/distvec : $(BINDIR)/rtngmsg.o $(BINDIR)/rtngnode.o $(BINDIR)/distvec.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/linkstate : $(BINDIR)/rtngmsg.o $(BINDIR)/rtngnode.o $(BINDIR)/linkstate.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
 
$(BINDIR)/%.o: %.cpp
	$(CC) -c $(CCOPTS) -o $@ $<
