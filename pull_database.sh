#!/bin/bash
#mostly stolen from rmkit
#copies your app to the tablet, runs it, then waits for interrupt
#before closing it and restarting the remarkable interface

RM_PORT=${RM_PORT:="22"}
REMARKABLE_HOST=${REMARKABLE_HOST:="remarkable"}
BASE_DIR="./data/opkg/"
RM_USER="root"

# Function to show usage
usage() {
    echo "Usage: $0 [-p PORT] [-h HOST] [-o OUTPATH]"
    exit 1
}

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

mkdir -p "${BASE_DIR}/opt/var"
mkdir -p "${BASE_DIR}/opt/lib"
rsync -rzP --port ${RM_PORT} "${RM_USER}@${REMARKABLE_HOST}:/opt/lib/opkg" "${BASE_DIR}/opt/lib/"
rsync -rzP --port ${RM_PORT} "${RM_USER}@${REMARKABLE_HOST}:/opt/var/opkg-lists" "${BASE_DIR}/opt/var/"
