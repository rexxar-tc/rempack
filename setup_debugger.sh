#!/bin/bash

REMARKABLE_HOST="remarkable"
APP_PATH=${1}
APP=$(basename "${APP_PATH}")
BASE_DIR="/home/root/${APP}"
RM_USER="root"

ssh ${RM_USER}@${REMARKABLE_HOST} "/opt/bin/launcherctl stop-launcher; killall gdbserver 2>/dev/null; killall ${APP} 2>/dev/null" &
#this is probably brittle, I'm sure it's fine
#scp ${APP_PATH} "${RM_USER}@${REMARKABLE_HOST}:${BASE_DIR}/${APP}"
#scp ${APP_PATH} "${RM_USER}@${REMARKABLE_HOST}:/tmp/CLion/debug/${APP}"
#echo "RUNNING ${APP}"
#remove rm2fb-client if you're running on a RM1
#ssh ${RM_USER}@${REMARKABLE_HOST} "cd ${BASE_DIR}; gdbserver --wrapper /opt/bin/rm2fb-client --once :1243" &