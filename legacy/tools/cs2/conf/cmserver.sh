#!/bin/sh

### BEGIN INIT INFO
# Provides:          cmserver
# Required-Start:    $local_fs $remote_fs $network $syslog
# Required-Stop:     $local_fs $remote_fs $network $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: starts the cmserver fast-cgi
# Description:       starts cmserver using start-stop-daemon
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/bin/spawn-fcgi
FCGI=/srv/src/PlatformSDK/sandbox/sunjac/xirang2/trunk/binary/linux/bin/linux/gnu_4.9.1/x64/release/webapp
#FCGI=/home/finger/github/xr/trunk/build/build_dir/gcc_debug/tools/webapp
WORKDIR=/opt/cmserver/tools/webapp
NAME=cmserver
DESC=cmserver
DAEMON_OPTS="-s /var/run/cm.sock -P /var/run/$NAME.pid -U www-data  -- $FCGI $WORKDIR"

test -x $DAEMON || exit 0
test -x $FCGI || exit 0

touch /var/run/$NAME.pid

set -e

. /lib/lsb/init-functions

case "$1" in
	start)
		echo -n "Starting $DESC: "
		start-stop-daemon --start --quiet \
		    --exec $DAEMON  -- $DAEMON_OPTS || true
		echo "$NAME."
		;;

	stop)
		echo -n "Stopping $DESC: "
		start-stop-daemon --stop --quiet --pidfile /var/run/$NAME.pid || true
		echo "$NAME."
		;;

	restart)
		echo -n "Restarting $DESC: "
		start-stop-daemon --stop --quiet --pidfile \
		    /var/run/$NAME.pid || true
		sleep 1
		start-stop-daemon --start --quiet \
		    --exec $DAEMON -- $DAEMON_OPTS || true
		echo "$NAME."
		;;

	status)
		status_of_proc -p /var/run/$NAME.pid "$DAEMON" cmserver && exit 0 || exit $?
		;;
	*)
		echo "Usage: $NAME {start|stop|restart|status}" >&2
		exit 1
		;;
esac

exit 0
