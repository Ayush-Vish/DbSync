#!/bin/bash
set -e

echo "======================================"
echo " DbSync Installation & Setup Script "
echo "======================================"

# 1. Install required packages
echo "[1/4] Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y build-essential liburing-dev pkg-config cmake libabsl-dev

# 2. Build DbSync
echo "[2/4] Compiling DbSync..."
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

# Ensure we have the required abseil libs for linking
ABSL_LIBS=$(pkg-config --libs absl_flat_hash_map absl_hash absl_city absl_raw_hash_set 2>/dev/null || echo "-labsl_hash -labsl_city -labsl_raw_hash_set -labsl_low_level_hash")

g++ -std=c++20 -O3 src/main.cpp src/reactor.cpp -Iinclude -o DbSync -luring -pthread $ABSL_LIBS

# 3. Create systemd service file
echo "[3/4] Creating systemd service file..."
SERVICE_FILE="/etc/systemd/system/dbsync.service"

sudo bash -c "cat > $SERVICE_FILE" <<EOF
[Unit]
Description=DbSync - Shared-Nothing Key-Value Engine
After=network.target

[Service]
Type=simple
User=$USER
WorkingDirectory=$PROJECT_DIR
ExecStart=$PROJECT_DIR/DbSync
Restart=on-failure
RestartSec=3
LimitNOFILE=65536

[Install]
WantedBy=multi-user.target
EOF

# 4. Enable and start the daemon
echo "[4/4] Starting DbSync systemd daemon..."
sudo systemctl daemon-reload
sudo systemctl enable dbsync.service
sudo systemctl restart dbsync.service

echo "======================================"
echo "✅ DbSync is successfully installed and running!"
echo "   Check daemon status with: sudo systemctl status dbsync.service"
echo "   Stop the daemon with:     sudo systemctl stop dbsync.service"
echo "   View live logs with:      journalctl -u dbsync.service -f"
echo "======================================"
