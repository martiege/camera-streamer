#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR/.."

set -xeo pipefail
make -j$(nproc)
$GDB ./camera-streamer -camera-path=$(echo /dev/v4l/by-id/usb-*-video-index0) "$@"
