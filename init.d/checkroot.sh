#!/bin/sh

rootmode=rw
rootopts=rw
rootcheck=yes
udevfs=

# read the fstab
sed 's/#.*//' /etc/fstab | while read fs mnt type opts dump pass junk
do
	[ "$type" = udevfs ] && udefs="$fs"
	[ "$mnt" != / ] && continue
	rootopts="$opts"
	[ "$pass" = 0 -o "$pass" = "" ] && rootcheck=no
	case "$opts" in
		ro|ro,*|*,ro|*,ro,*)
			rootmode=ro
			;;
	esac
done

if [ "$rootcheck" = yes ] ; then
	if grep "^$fs" /proc/mounts ; then
		echo "$fs is mounted. Something is wrong. Please fix and reboot."
		echo "sorry newbies..."
		echo
		echo "CONTROL-D will exit from this shell and reboot the system."
		echo
		/sbin/sulogin $CONSOLE
		reboot -f
	elif ! fsck -C "$fs" ; then
		echo "fsck failed.  Please repair manually and reboot.  Please note"
		echo "that the root file system is currently mounted read-only.  To"
		echo "remount it read-write:"
		echo
		echo "   # mount -n -o remount,rw /"
		echo
		echo "CONTROL-D will exit from this shell and REBOOT the system."
		echo
		/sbin/sulogin $CONSOLE
		reboot -f
	fi
fi

mount -o remount,$rootmode /

