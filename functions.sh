# Copyright 2007-2008 Natanael Copa <natanael.copa@gmail.com>
# Copyright 2007-2008 Roy Marples <roy@marples.name>
# All rights reserved. Released under the 2-clause BSD license.

RC_GOT_FUNCTIONS="yes"

# load user settings
[ -r /etc/conf.d/rc ] && . /etc/conf.d/rc



svclib="/lib/rcscripts"
svcdir="${svcdir:-/var/lib/init.d}"

# void import_addon(char *Addon)
import_addon() {
	local addon="$svclib/addons/$1"
	[ -r "$addon" ] && . "$addon"
}

echon () {
	if [ -z "$ECHON" ]; then
		# Determine how to "echo" without newline: "echo -n"
		# or "echo ...\c"
		if [ "X`echo -n`" = "X-n" ]; then
			ECHON=echo
			NNL="\c"
			# "
		else
			ECHON="echo -n"
			NNL=""
		fi
	fi
	$ECHON "$*$NNL"
}


eerror() {
	echo $* >&2
}

einfo() {
	echo $* >&2
}

einfon() {
	echon $* >&2
}

ewarn() {
	echo $* >&2
}

ebegin() {
	echon " * $*: "
}

eend() {
	local msg
	if [ "$1" = 0 ] || [ $# -lt 1 ] ; then
		msg="ok."
	else
		shift
		msg=" failed. $*"
	fi
	echo "$msg"
}

eindent() {
	true
}

eoutdent() {
	true
}

start_addon() {
	(import_addon "$1-start.sh")
	return 0
}

stop_addon() {
	(import_addon "$1-stop.sh")
	return 0
}

save_options() {
	local myopts="$1"
	mkdir -p -m 0755 "$svcdir/options/$SVCNAME"
	shift
	echo "$*" > "$svcdir/options/$SVCNAME/$myopts"
}

get_options() {
	local svc="$SVCNAME"
	[ "$2" ] && svc="$2"
	cat "$svcdir/options/$svc/$1" 2>/dev/null
}

die() {
	eerror "$1"
	exit 1
}

yesno() {
	[ -z "$1" ] && return 1

	case "$1" in
		[Yy][Ee][Ss]|[Tt][Rr][Uu][Ee]|[Oo][Nn]|1) return 0;;
		[Nn][Oo]|[Ff][Aa][Ll][Ss][Ee]|[Oo][Ff][Ff]|0) return 1;;
	esac

	local value=
	eval value=\$${1}
	case "${value}" in
		[Yy][Ee][Ss]|[Tt][Rr][Uu][Ee]|[Oo][Nn]|1) return 0;;
		[Nn][Oo]|[Ff][Aa][Ll][Ss][Ee]|[Oo][Ff][Ff]|0) return 1;;
		*) ewarn "\$${1} is not set properly"; return 1;;
	esac
}

