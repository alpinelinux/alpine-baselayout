
CC = gcc
LD = gcc

SBIN_TARGETS = runscript readahead cttyhack
TARGET = $(BIN_TARGETS) $(SBIN_TARGETS)

.PHONY: all clean
all: $(TARGET)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.o core *~

install: $(TARGET)
	install -m 0755 -d $(DESTDIR)/bin
	install -m 0755 -d $(DESTDIR)/sbin
#	install -m 0755 $(BIN_TARGETS) $(DESTDIR)/bin
	install -m 0755 $(SBIN_TARGETS) $(DESTDIR)/sbin

