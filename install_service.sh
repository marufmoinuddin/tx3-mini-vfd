#!/bin/bash
# install_service.sh - Easy installer for TX3 Mini VFD Controller

set -e

echo "🚀 TX3 Mini VFD Controller - Service Installer"
echo "=============================================="
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo "❌ Please don't run this script as root. It will ask for sudo when needed."
    exit 1
fi

# Check if we're in the right directory
if [ ! -f "vfd_controller.cpp" ] || [ ! -f "Makefile" ]; then
    echo "❌ Error: Please run this script from the project directory containing vfd_controller.cpp"
    exit 1
fi

echo "📋 Installation Steps:"
echo "1. Compile the VFD controller"
echo "2. Install binary to /usr/local/bin/"
echo "3. Install systemd service"
echo "4. Enable and start the service"
echo ""

read -p "Do you want to proceed? [y/N]: " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Installation cancelled."
    exit 0
fi

echo ""
echo "🔨 Step 1: Compiling..."
make clean
make

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

echo "✅ Compilation successful!"
echo ""

echo "📦 Step 2: Installing binary..."
sudo make install

if [ $? -ne 0 ]; then
    echo "❌ Binary installation failed!"
    exit 1
fi

echo "✅ Binary installed to /usr/local/bin/vfd_controller"
echo ""

echo "🔧 Step 3: Installing systemd service..."
sudo make service

if [ $? -ne 0 ]; then
    echo "❌ Service installation failed!"
    exit 1
fi

echo "✅ Service file installed!"
echo ""

echo "🚀 Step 4: Enabling and starting service..."
sudo systemctl enable vfd-controller.service
sudo systemctl start vfd-controller.service

if [ $? -eq 0 ]; then
    echo "✅ Service enabled and started successfully!"
    echo ""
    echo "📊 Service Status:"
    sudo systemctl status vfd-controller.service --no-pager -l
    echo ""
    echo "🎉 Installation Complete!"
    echo ""
    echo "📖 Useful Commands:"
    echo "  Check status:    sudo systemctl status vfd-controller.service"
    echo "  View logs:       sudo journalctl -u vfd-controller.service -f"
    echo "  Stop service:    sudo systemctl stop vfd-controller.service"
    echo "  Restart service: sudo systemctl restart vfd-controller.service"
    echo "  Disable service: sudo systemctl disable vfd-controller.service"
    echo "  Uninstall:       make uninstall"
    echo ""
    echo "The VFD should now be displaying the time and system stats!"
else
    echo "❌ Failed to start service. Check the logs:"
    echo "sudo journalctl -u vfd-controller.service -n 20"
    exit 1
fi
