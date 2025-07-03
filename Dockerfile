FROM ghcr.io/toltec-dev/base:v3.1

RUN opkg update && opkg install libarchive