
CC = gcc
LD = gcc

#BIN_TARGETS = splashbard
#SBIN_TARGETS = runscript start-stop-daemon
SBIN_TARGETS = runscript
TARGET = $(BIN_TARGETS) $(SBIN_TARGETS)

.PHONY: all clean
all: $(TARGET)

tsplashd: splashbard.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

runscript: runscript.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

start-stop-daemon: start-stop-daemon.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.o core *~

install: $(TARGET)
	install -m 0755 -d $(DESTDIR)/bin
	install -m 0755 -d $(DESTDIR)/sbin
#	install -m 0755 $(BIN_TARGETS) $(DESTDIR)/bin
	install -m 0755 $(SBIN_TARGETS) $(DESTDIR)/sbin

