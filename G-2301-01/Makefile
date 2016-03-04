CC = gcc
CFLAGS = -L$(LDIR) -I$(IDIR) -g
LDFLAGS = -lpthread -lircredes -lirctad -lredes2-G-2301-01-P1 
AR = ar

TAR_FILE= G-2301-01-P1.tar.gz
SDIR = src
SLDIR = srclib
IDIR = includes
LDIR = lib
ODIR = obj
MDIR = man
DDIR = doc
BDIR = .

_LIB = libredes2-G-2301-01-P1.a
LIB = $(patsubst %,$(LDIR)/%,$(_LIB))

_LOBJ = G-2301-01-P1-daemon.o G-2301-01-P1-tcp.o G-2301-01-P1-udp.o G-2301-01-P1-server.o G-2301-01-P1-irc.o G-2301-01-P1-tools.o
LOBJ = $(patsubst %,$(ODIR)/%,$(_LOBJ))

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = G-2301-01-P1-echo_server.o G-2301-01-P1-client.o G-2301-01-P1-irc_server.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_BIN = G-2301-01-P1-echo_server G-2301-01-P1-client G-2301-01-P1-irc_server
BIN = $(patsubst %,$(BDIR)/%,$(_BIN))

all: $(BIN)

$(LIB): $(LOBJ)
	$(AR) rcv $@ $^

$(LOBJ):$(ODIR)/%.o: $(SLDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ):$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN):%: $(ODIR)/%.o $(LIB)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

compress: clean doc
	tar -zcvf $(TAR_FILE) *

doc: Doxyfile $(ALL)
	doxygen

run: G-2301-01-P1-irc_server
	./G-2301-01-P1-irc_server 6667
log: 
	tail -F /var/log/syslog | grep irc_server
.PHONY: clean
clean:
	@rm -frv $(BIN) $(LIB) $(OBJ) $(LOBJ) 
	@rm -fv $(TAR_FILE)
	@rm -fv core vgcore*  
