# Makefile for TX3 Mini VFD Controller
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
TARGET = vfd_controller
SOURCE = vfd_controller.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)

service: install
	sudo cp vfd-controller.service /etc/systemd/system/
	sudo systemctl daemon-reload
	@echo "Service installed! To enable and start:"
	@echo "  sudo systemctl enable vfd-controller.service"
	@echo "  sudo systemctl start vfd-controller.service"

enable-service: service
	sudo systemctl enable vfd-controller.service
	sudo systemctl start vfd-controller.service
	@echo "VFD Controller service enabled and started!"
	@echo "Check status with: sudo systemctl status vfd-controller.service"

status:
	sudo systemctl status vfd-controller.service

stop:
	sudo systemctl stop vfd-controller.service

restart:
	sudo systemctl restart vfd-controller.service

logs:
	sudo journalctl -u vfd-controller.service -f

uninstall:
	sudo systemctl stop vfd-controller.service 2>/dev/null || true
	sudo systemctl disable vfd-controller.service 2>/dev/null || true
	sudo rm -f /etc/systemd/system/vfd-controller.service
	sudo rm -f /usr/local/bin/$(TARGET)
	sudo systemctl daemon-reload

clean:
	rm -f $(TARGET)

.PHONY: all install service enable-service status stop restart logs uninstall clean
