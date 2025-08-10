#!/bin/bash
#mostly stolen from rmkit
#copies your app to the tablet, runs it, then waits for interrupt
#before closing it and restarting the remarkable interface

RM_PORT=${RM_PORT:="22"}
REMARKABLE_HOST=${REMARKABLE_HOST:="remarkable"}
APP_PATH=${1}
APP=$(basename "${APP_PATH}")
BASE_DIR="/home/root/${APP}"
RM_USER="root"

function kill_remote_app() {
  ssh ${RM_USER}@${REMARKABLE_HOST} -p${RM_PORT}  killall ${APP} 2> /dev/null
}

function cleanup() {
  kill_remote_app
  #ssh ${RM_USER}@${REMARKABLE_HOST} rm ${BASE_DIR}/${APP}
  #ssh ${RM_USER}@${REMARKABLE_HOST} -p${RM_PORT} "source ~/.bashrc; launcherctl start-launcher"
  rsync -e "ssh -p ${RM_PORT}" -azP ${RM_USER}@${REMARKABLE_HOST}:~/.cache/${APP}/screens ~/git/${APP}/screens
  echo "FINISHED"
  trap - EXIT
  exit 0
}

trap cleanup EXIT
trap cleanup SIGINT
#!/bin/bash

# Function to show usage
usage() {
    echo "Usage: $0 SOURCE_PATH [-p PORT] [-h HOST] [-o OUTPATH]"
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

kill_remote_app
#this is probably brittle, I'm sure it's fine
ssh ${RM_USER}@${REMARKABLE_HOST} -p${RM_PORT} "mkdir -p ${BASE_DIR}"
rsync -rzP --port ${RM_PORT} ${APP_PATH} ${RM_USER}@${REMARKABLE_HOST}:${BASE_DIR}/${APP}
echo "RUNNING ${APP}"
#remove rm2fb-client if you're running on a RM1
ssh -p${RM_PORT}  ${RM_USER}@${REMARKABLE_HOST} "source ~/.bashrc; killall gdbserver; launcherctl stop-launcher"
ssh -p${RM_PORT}  ${RM_USER}@${REMARKABLE_HOST} "LD_PRELOAD=/opt/lib/librm2fb_client.so ${BASE_DIR}/${APP}"
