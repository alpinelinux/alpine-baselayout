PACKAGE=alpine-baselayout
VERSION=2.0_alpha3

PV 		=$(PACKAGE)-$(VERSION)
TARBALL 	=$(PV).tar.bz2
SUBDIRS 	=src init.d

GENERATED_FILES =TZ hosts profile
ETC_FILES 	=$(GENERATED_FILES) group fstab inittab nsswitch.conf \
		passwd protocols services shadow shells issue mdev.conf \
		crontab sysctl.conf 
CONFD_FILES = $(addprefix conf.d/, cron hwclock localinit rdate syslog tuntap vlan watchdog)
SBIN_FILES	=runscript-alpine.sh functions.sh rc_add rc_delete rc_status
RC_SH_FILES 	=rc-services.sh
UDHCPC_FILES 	=default.script 
LIB_MDEV_FILES 	=ide_links sd_links subdir_dev usbdev dvbdev
MODPROBED_FILES	=aliases blacklist i386
SENDBUG_FILES	=sendbug.conf
CRONTABS 	=crontab
DISTFILES 	=$(ETC_FILES) $(SBIN_FILES) $(UDHCPC_FILES) $(RC_SH_FILES)\
		$(LIB_MDEV_FILES) $(SENDBUG_FILES) $(MODPROBED_FILES) Makefile

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
	echo "127.0.0.1	localhost" > hosts

profile:
	echo "export PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin" > $@
	echo "export PAGER=less" >> $@
	echo "export PS1='\\h:\\w\\$$ '" >>$@
	echo "umask 022" >> $@

shadow:	passwd
	@lastchange=$$(( `date +%s` / ( 24 * 3600 ) ));\
	awk -F: ' { \
		pw = ":!:";\
		if ($$1 == "root") { pw = "::" };\
		print $$1 pw "'"$$lastchange"':0:::::"  \
	}' passwd > $@

#		":" $$pw ":"'"$$lastchange"'":0:::::"
install:
	install -m 0755 -d $(addprefix $(DESTDIR)/, \
		dev \
		dev/pts \
		dev/shm \
		etc \
		etc/apk \
		etc/conf.d \
		etc/crontabs \
		etc/modprobe.d \
		etc/network/if-down.d \
		etc/network/if-post-down.d \
		etc/network/if-pre-up.d \
		etc/network/if-up.d \
		etc/periodic/15min \
		etc/periodic/daily \
		etc/periodic/hourly \
		etc/periodic/monthly \
		etc/periodic/weekly \
		etc/rcK.d \
		etc/rcL.d \
		etc/sendbug \
		lib/firmware \
		lib/mdev \
		lib/rcscripts/sh \
		media/cdrom \
		media/floppy \
		media/usb \
		proc \
		sbin \
		sys \
		usr/bin \
		usr/local/bin \
		usr/local/lib \
		usr/local/share \
		usr/share/udhcpc \
		var/lib/misc \
		var/lock/subsys \
		var/log \
		var/run \
		var/spool/cron \
		var/tmp \
		)
	install -d -m 0770 $(DESTDIR)/root
	install -d -m 1777 $(DESTDIR)/tmp
	for i in $(SUBDIRS) ; do \
		cd $$i && make install && cd .. ;\
	done
	install -m 0644 $(ETC_FILES) $(DESTDIR)/etc
	install -m 0644 $(SENDBUG_FILES) $(DESTDIR)/etc/sendbug
	chmod 600 $(DESTDIR)/etc/shadow
	install -m 0644 $(CONFD_FILES) $(DESTDIR)/etc/conf.d
	install -m 0755 $(SBIN_FILES) $(DESTDIR)/sbin
	install -m 0755 $(UDHCPC_FILES) $(DESTDIR)/usr/share/udhcpc
	install -m 0755 $(RC_SH_FILES) $(DESTDIR)/lib/rcscripts/sh
	install -m 0755 $(LIB_MDEV_FILES) $(DESTDIR)/lib/mdev
	install -m 0755 $(MODPROBED_FILES) $(DESTDIR)/etc/modprobe.d
	mv $(DESTDIR)/etc/crontab $(DESTDIR)/etc/crontabs/root
	ln -s /etc/crontabs $(DESTDIR)/var/spool/cron/crontabs
	echo "af_packet" >$(DESTDIR)/etc/modules



$(TARBALL): $(DISTFILES) $(SUBDIRS)
	rm -rf $(PACKAGE)
	mkdir $(PACKAGE)
	for i in $(SUBDIRS) ; do \
		cd $$i && make clean && cd .. ; \
	done
	cp $(DISTFILES) $(PACKAGE)
	mkdir $(PACKAGE)/conf.d
	cp $(CONFD_FILES) $(PACKAGE)/conf.d/
	rsync -Cr $(SUBDIRS) $(PACKAGE)
	tar -cjf $@ $(PACKAGE)
	rm -r $(PACKAGE)

dist:	$(TARBALL)


.PHONY:	install clean all
