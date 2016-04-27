CC = gcc
CFLAGS = -L$(LDIR) -I$(IDIR) -g 
LDFLAGS = -lpthread -lircredes -lirctad -lssl -lredes2-G-2301-01-P1 -lcrypto
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

_LOBJ = G-2301-01-P1-daemon.o G-2301-01-P1-tcp.o G-2301-01-P1-udp.o G-2301-01-P1-server.o G-2301-01-P1-irc.o G-2301-01-P1-tools.o G-2301-01-P3-ssl.o G-2301-01-P3-ssl-server.o
LOBJ = $(patsubst %,$(ODIR)/%,$(_LOBJ))

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = G-2301-01-P1-echo_server.o G-2301-01-P1-client.o G-2301-01-P1-irc_server.o G-2301-01-P3-ssl_echo_server.o G-2301-01-P3-ssl_client.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_BIN = G-2301-01-P1-echo_server G-2301-01-P1-client G-2301-01-P1-irc_server G-2301-01-P3-ssl_echo_server G-2301-01-P3-ssl_client
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
	rm -rf $(TAR_FILE)
	rm -rf G-2301-01-P1
	tar -zcvf ../$(TAR_FILE) ../G-2301-01-P1
	mv ../$(TAR_FILE) $(TAR_FILE)

doc: 
	doxygen

kill:
	pkill G-2301-01-P1-ir

run: G-2301-01-P1-irc_server
	./G-2301-01-P1-irc_server 6667

log: 
	tail -F /var/log/syslog | grep irc

.PHONY: clean
clean:
	@rm -frv $(BIN) $(LIB) $(OBJ) $(LOBJ) 
	@mkdir -p obj lib
	@rm -fv $(TAR_FILE)
	@rm -fv core vgcore*  
