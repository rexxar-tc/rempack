#!/bin/bash

RM_PORT="22"
REMARKABLE_HOST="remarkable"
APP_PATH=${1}
APP=$(basename "${APP_PATH}")
BASE_DIR="/tmp/CLion/debug"
RM_USER="root"

usage() {
    echo "Usage: $0 SOURCE_PATH [-p PORT] [-h HOST]"
    exit 1
}

# Ensure at least one argument (SOURCE_PATH)
if [[ $# -lt 1 ]]; then
    usage
fi

shift

# Parse options
while [[ $# -gt 0 ]]; do
    case "$1" in
        -p)
            shift
            RM_PORT="$1"
            ;;
        -p*)
            RM_PORT="${1#-p}"
            ;;
        -h)
            shift
            REMARKABLE_HOST="$1"
            ;;
        -h*)
            REMARKABLE_HOST="${1#-h}"
            ;;
        -o)
            shift
            BASE_DIR="$1"
            ;;
        -o*)
            BASE_DIR="${1#-o}"
            ;;
        -*)
            echo "Unknown option: $1"
            usage
            ;;
        *)
            echo "Unexpected argument: $1"
            usage
            ;;
    esac
    shift
done

ssh -p${RM_PORT} ${RM_USER}@${REMARKABLE_HOST} "/opt/bin/launcherctl stop-launcher; killall gdbserver 2>/dev/null; killall ${APP} 2>/dev/null; mkdir -p ${BASE_DIR}"
#ssh ${RM_USER}@${REMARKABLE_HOST} "/opt/bin/launcherctl stop-launcher; killall ${APP} 2>/dev/null" &
#ssh ${RM_USER}@${REMARKABLE_HOST} "killall ${APP} 2>/dev/null"
#rsync -zP --port "${RM_PORT}" "${APP_PATH}" "${RM_USER}@${REMARKABLE_HOST}:${BASE_DIR}/"
exit 0
#this is probably brittle, I'm sure it's fine
#scp ${APP_PATH} "${RM_USER}@${REMARKABLE_HOST}:/tmp/CLion/debug/${APP}"
#echo "RUNNING ${APP}"
#remove rm2fb-client if you're running on a RM1
#ssh ${RM_USER}@${REMARKABLE_HOST} "cd ${BASE_DIR}; gdbserver --wrapper /opt/bin/rm2fb-client --once :1243" &