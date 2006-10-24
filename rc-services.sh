
RC_GOT_SERVICES="yes"

[ "$RC_GOT_FUNCTIONS" ] || . /sbin/functions.sh

# void save_options(char *option, char *optstring)
save_options() {
        local myopts="$1"

        shift
        if [ ! -d "${svcdir}/options/${SVCNAME}" ] ; then
                mkdir -p -m 0755 "${svcdir}/options/${SVCNAME}"
        fi

        echo "$*" > "${svcdir}/options/${SVCNAME}/${myopts}"
}

# char *get_options(char *option)
get_options() {
        local svc="${SVCNAME}"
        [ -n $2 ] && svc="$2"

        if [ -f "${svcdir}/options/${svc}/$1" ] ; then
                echo "$(< ${svcdir}/options/${svc}/$1)"
        fi
}
