
CC = gcc
LD = gcc

SBIN_TARGETS = runscript readahead cttyhack
USR_BIN_TARGETS = sendbug
TARGET = $(BIN_TARGETS) $(SBIN_TARGETS) $(USR_BIN_TARGETS)

.PHONY: all clean
all: $(TARGET)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

sendbug: atomicio.o sendbug.o lib.o
	$(CC) $(LDFLAGS) -o $@ $^ 

clean:
	rm -f $(TARGET) *.o core *~

install: $(TARGET)
	install -m 0755 -d $(DESTDIR)/bin
	install -m 0755 -d $(DESTDIR)/sbin
#	install -m 0755 -d $(DESTDIR)/usr
#	install -m 0755 -d $(DESTDIR)/usr/bin
#	install -m 0755 $(USR_BIN_TARGETS) $(DESTDIR)/usr/bin
	install -m 0755 $(SBIN_TARGETS) $(DESTDIR)/sbin

