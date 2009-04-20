
CC = gcc
LD = gcc

SBIN_TARGETS = mkmntdirs 
TARGET = $(BIN_TARGETS) $(SBIN_TARGETS) $(USR_BIN_TARGETS)

.PHONY: all clean
all: $(TARGET)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.o core *~

install: $(TARGET)
	install -m 0755 -d $(DESTDIR)/sbin
	install -m 0755 $(SBIN_TARGETS) $(DESTDIR)/sbin

