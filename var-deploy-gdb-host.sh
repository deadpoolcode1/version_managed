#!/bin/bash
readonly TARGET_IP="$1"
readonly PROGRAM="$2"
readonly TARGET_DIR="/home/root"

echo "Debug over Host PC"

# kill gdbserver on target and delete old binary
/usr/bin/killall -q gdbserver

# Must match endsPattern in tasks.json
echo "Starting GDB Server on Host PC"

# start gdbserver on target
gdbserver localhost:3000 ${PROGRAM}
