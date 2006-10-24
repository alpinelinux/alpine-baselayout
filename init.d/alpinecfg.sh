#!/bin/busybox sh

# the purpose of this script is to find and import the alpine config.
# load it or set default values.

#depracated
echo "remeber to remove this $0 file..."
exit 

get_boot_var () {
	# Look for CFG_MEDIA in cmdline
	for i in `cat /proc/cmdline` ; do
#		if echo $i | grep $ > /dev/null ; then
#			echo $i | sed 's/'$1'=//'
#		fi
		case $i in
			$1=*) echo $i | sed 's|'$1'=||' ;;
		esac
	done
}


import_config() {
	if mount $1 ; then
		echo "Using Alpine config from $1"
		cp $1/$2 /etc/alpine.conf
		sleep 1
		umount $1
	fi
}

CFG="alpine.conf"
CFG_MEDIA=`get_boot_var cfg_media`
#if [ -z "$CFG_MEDIA" ] ; then
	# cfg_media was not set in cmdline. We use the defaults...
	echo "looking for alpine.conf on default locations"
	for i in /media/* ; do
		import_config $i $CFG
	done
#else
	# Only import for specified location
#	import_config "/media/$CFG_MEDIA" $CFG 
#fi

