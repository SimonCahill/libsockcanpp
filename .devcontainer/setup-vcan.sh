#!/usr/bin/env sh
set -eu

IFACE="${1:-vcan0}"

sudo modprobe vcan 2>/dev/null || true

if ! ip link show "${IFACE}" >/dev/null 2>&1; then
    sudo ip link add dev "${IFACE}" type vcan
fi

# CAN FD frames use the larger CANFD_MTU. Some kernels already set this for
# vcan, so keep going if the MTU is not adjustable in the current environment.
sudo ip link set dev "${IFACE}" mtu 72 2>/dev/null || true
sudo ip link set up "${IFACE}"

ip -details link show "${IFACE}"
