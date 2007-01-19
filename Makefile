VERSION=1.2
PV = alpine-baselayout-$(VERSION)
TARBALL = $(PV).tar.gz

SUBDIRS = src init.d

GENERATED_FILES = TZ hosts profile
ETC_FILES = $(GENERATED_FILES) group fstab inittab nsswitch.conf passwd protocols services shadow shells issue mdev.conf crontab sysctl.conf
CONFD_FILES = $(addprefix conf.d/, cron localinit tuntap watchdog)
SBIN_FILES = runscript-alpine.sh functions.sh rc_add rc_delete rc_status
RC_SH_FILES = rc-services.sh
UDHCPC_FILES = default.script 
LIB_MDEV_FILES = ide_links sd_links subdir_dev usbdev
CRONTABS = crontab
DISTFILES = $(ETC_FILES) $(SBIN_FILES) $(UDHCPC_FILES) $(RC_SH_FILES) $(LIB_MDEV_FILES) Makefile

all:	$(GENERATED_FILES)
	for i in $(SUBDIRS) ; do \
		cd $$i && make && cd ..  ; \
	done

clean:
	for i in $(SUBDIRS) ; do \
		cd $$i && make clean && cd .. ; \
	done
	rm -f $(TARBALL) $(GENERATED_FILES) *~

TZ:
	echo "UTC" > TZ

hostname:
	echo localhost > hostname

hosts:
	echo -e "127.0.0.1\tlocalhost" > hosts

profile:
	echo "export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin" > profile
	echo "umask 022" >> profile

shadow:	passwd
	@lastchange=$$(( `date +%s` / ( 24 * 3600 ) ));\
	awk -F: ' { \
		pw = ":!:";\
		if ($$1 == "root") { pw = "::" };\
		print $$1 pw "'"$$lastchange"':0:::::"  \
	}' passwd > $@

#		":" $$pw ":"'"$$lastchange"'":0:::::"
install:
	install -m 0755 -d $(addprefix $(DESTDIR)/, etc sbin) \
		$(DESTDIR)/etc/conf.d \
		$(DESTDIR)/etc/crontabs \
		$(DESTDIR)/lib/rcscripts/sh \
		$(DESTDIR)/usr/share/udhcpc \
		$(DESTDIR)/lib/mdev \
		$(DESTDIR)/var/spool/cron \
		$(DESTDIR)/etc/periodic/15min \
		$(DESTDIR)/etc/periodic/hourly \
		$(DESTDIR)/etc/periodic/daily \
		$(DESTDIR)/etc/periodic/weekly \
		$(DESTDIR)/etc/periodic/monthly 
	for i in $(SUBDIRS) ; do \
		cd $$i && make install && cd .. ;\
	done
	install -m 0644 $(ETC_FILES) $(DESTDIR)/etc
	chmod 600 $(DESTDIR)/etc/shadow
	install -m 0644 $(CONFD_FILES) $(DESTDIR)/etc/conf.d
	install -m 0755 $(SBIN_FILES) $(DESTDIR)/sbin
	install -m 0755 $(UDHCPC_FILES) $(DESTDIR)/usr/share/udhcpc
	install -m 0755 $(RC_SH_FILES) $(DESTDIR)/lib/rcscripts/sh
	install -m 0755 $(LIB_MDEV_FILES) $(DESTDIR)/lib/mdev
	mv $(DESTDIR)/etc/crontab $(DESTDIR)/etc/crontabs/root
	ln -s /etc/crontabs $(DESTDIR)/var/spool/cron/crontabs

$(TARBALL): $(DISTFILES) $(SUBDIRS)
	rm -rf $(PV)
	mkdir $(PV)
	for i in $(SUBDIRS) ; do \
		cd $$i && make clean && cd .. ; \
	done
	cp $(DISTFILES) $(PV)
	mkdir $(PV)/conf.d
	cp $(CONFD_FILES) $(PV)/conf.d/
	rsync -Cr $(SUBDIRS) $(PV)
	tar --exclude=.svn -czf $@ $(PV)
	rm -r $(PV)

dist:	$(TARBALL)


.PHONY:	install clean all
