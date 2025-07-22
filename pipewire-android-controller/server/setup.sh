#!/bin/bash

# PipeWire Controller Server Setup Script

set -e

echo "Setting up PipeWire Controller Server..."

# Check if running as root
if [[ $EUID -eq 0 ]]; then
   echo "This script should not be run as root. Please run as your user account."
   exit 1
fi

# Check for required dependencies
echo "Checking dependencies..."
if ! command -v python3 &> /dev/null; then
    echo "Python 3 is required but not installed."
    echo "Install with: sudo apt install python3 python3-pip python3-venv"
    exit 1
fi

if ! command -v pipewire &> /dev/null; then
    echo "PipeWire is required but not installed."
    echo "Install with: sudo apt install pipewire pipewire-pulse"
    exit 1
fi

# Create installation directory
INSTALL_DIR="/opt/pipewire-controller"
sudo mkdir -p "$INSTALL_DIR"
sudo chown $USER:$USER "$INSTALL_DIR"

# Copy files
echo "Installing server files..."
cp -r * "$INSTALL_DIR/"
cd "$INSTALL_DIR"

# Create virtual environment
echo "Creating virtual environment..."
python3 -m venv venv
source venv/bin/activate

# Install Python dependencies
echo "Installing Python dependencies..."
pip install --upgrade pip
pip install -r requirements.txt

# Make server executable
chmod +x pipewire_server.py

# Install systemd service
echo "Installing systemd service..."
sudo cp pipewire-controller.service /etc/systemd/system/pipewire-controller@.service
sudo systemctl daemon-reload

echo "Setup complete!"
echo ""
echo "To start the server:"
echo "sudo systemctl enable pipewire-controller@$USER"
echo "sudo systemctl start pipewire-controller@$USER"
echo ""
echo "To run manually:"
echo "cd $INSTALL_DIR && source venv/bin/activate && python pipewire_server.py"
echo ""
echo "The server will be available at: http://$(hostname -I | awk '{print $1}'):5000"
echo "Configure this URL in your Android app."