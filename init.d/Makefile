SCRIPTS = cron \
	dnsd \
	httpd \
	hwdrivers \
	inetd \
	mdev \
	syslog \
	tuntap \
	vlan \
	watchdog

all:

install: $(SCRIPTS)
	install -m 0755 -d $(DESTDIR)/etc
	install -m 0755 -d $(DESTDIR)/etc/init.d
	
	for i in $(SCRIPTS) ; do \
		install -m 755 -D $$i $(DESTDIR)/etc/init.d/$$i ;\
	done
	
clean:
	rm -f *~

.PHONY: all clean
