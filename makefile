CC=g++
CCOPTS=-g -w

OBJS = $(BINDIR)/manager.o $(BINDIR)/tcpcon.o
TARGETS = $(BINDIR)/manager
BINDIR = build

all: $(TARGETS) $(OBJS)

clean:
	rm -f $(TARGETS) $(OBJS)

.PHONY: all clean

$(TARGETS): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/%.o: %.cpp
	$(CC) -c $(CCOPTS) -o $@ $<