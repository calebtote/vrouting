CC=g++
CCOPTS=-g -w

OBJS = $(BINDIR)/manager.o $(BINDIR)/rtngnode.o
TARGETS = $(BINDIR)/manager $(BINDIR)/rtngnode
BINDIR = build

all: $(TARGETS) $(OBJS)

clean:
	rm -f $(TARGETS) $(OBJS)

.PHONY: all clean

$(BINDIR)/manager : $(BINDIR)/manager.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/rtngnode : $(BINDIR)/rtngnode.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
 
$(BINDIR)/%.o: %.cpp
	$(CC) -c $(CCOPTS) -o $@ $<
