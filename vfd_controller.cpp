/*
 * TX3 Mini VFD Controller - C++ Version
 * Optimized for 4-digit 7-segment display
 * Author: AI Assistant
 * Date: September 2025
 */

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <csignal>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>
#include <vector>
#include <array>

// Display and LED paths
const std::string DISPLAY_TEXT = "/sys/devices/platform/spi/spi_master/spi0/spi0.0/display_text";
const std::string LED_BASE = "/sys/class/leds";

struct LEDPaths {
    std::string alarm = LED_BASE + "/:alarm/brightness";
    std::string colon = LED_BASE + "/:colon/brightness";
    std::string lan = LED_BASE + "/:lan/brightness";
    std::string pause = LED_BASE + "/:pause/brightness";
    std::string play = LED_BASE + "/:play/brightness";
    std::string usb = LED_BASE + "/:usb/brightness";
    std::string wlan = LED_BASE + "/:wlan/brightness";
} leds;

// Global flag for graceful shutdown
volatile sig_atomic_t keep_running = 1;

// Signal handler for clean exit
void signal_handler(int signal) {
    std::cout << "\nExiting VFD controller..." << std::endl;
    keep_running = 0;
}

// Write text to display
bool write_display_text(const std::string& text) {
    std::string display_text = text.substr(0, 4);
    display_text.resize(4, ' '); // Pad with spaces to 4 characters
    
    std::ofstream file(DISPLAY_TEXT);
    if (file.is_open()) {
        file << display_text;
        file.close();
        return true;
    }
    return false;
}

// Control LED
bool write_led(const std::string& led_path, int value) {
    std::ofstream file(led_path);
    if (file.is_open()) {
        file << value;
        file.close();
        return true;
    }
    return false;
}

// Get CPU temperature
float get_cpu_temperature() {
    // Try vcgencmd first
    FILE* pipe = popen("vcgencmd measure_temp 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            std::string output(buffer);
            if (output.find("temp=") != std::string::npos) {
                size_t pos = output.find("temp=") + 5;
                size_t end = output.find("'C");
                if (end != std::string::npos) {
                    pclose(pipe);
                    return std::stof(output.substr(pos, end - pos));
                }
            }
        }
        pclose(pipe);
    }
    
    // Try hwmon sensors
    std::vector<std::string> temp_paths = {
        "/sys/class/hwmon/hwmon0/temp1_input",
        "/sys/class/hwmon/hwmon1/temp1_input",
        "/sys/class/hwmon/hwmon2/temp1_input"
    };
    
    for (const auto& path : temp_paths) {
        std::ifstream file(path);
        if (file.is_open()) {
            int temp_raw;
            file >> temp_raw;
            file.close();
            if (temp_raw > 1000) {
                return temp_raw / 1000.0f; // millidegrees to degrees
            } else {
                return static_cast<float>(temp_raw);
            }
        }
    }
    
    // Try lm-sensors
    pipe = popen("sensors -u 2>/dev/null | grep 'temp1_input\\|Core 0' | head -1", "r");
    if (pipe) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            std::string output(buffer);
            size_t pos = output.find(':');
            if (pos != std::string::npos) {
                std::string temp_str = output.substr(pos + 1);
                float temp = std::stof(temp_str);
                if (temp > 10 && temp < 100) {
                    pclose(pipe);
                    return temp;
                }
            }
        }
        pclose(pipe);
    }
    
    return 50.0f; // Default fallback temperature
}

// Get memory usage percentage
float get_memory_usage() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return 0.0f;
    
    long mem_total = 0, mem_available = 0;
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string label, value;
            iss >> label >> value;
            mem_total = std::stol(value);
        } else if (line.find("MemAvailable:") == 0) {
            std::istringstream iss(line);
            std::string label, value;
            iss >> label >> value;
            mem_available = std::stol(value);
        }
    }
    
    if (mem_total > 0) {
        return 100.0f * (mem_total - mem_available) / mem_total;
    }
    return 0.0f;
}

// Get disk usage percentage
int get_disk_usage() {
    FILE* pipe = popen("df / | tail -1 | awk '{print $5}' | sed 's/%//'", "r");
    if (pipe) {
        char buffer[16];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            pclose(pipe);
            return std::stoi(std::string(buffer));
        }
        pclose(pipe);
    }
    return 0;
}

// Check network status
std::pair<bool, bool> get_network_status() {
    bool wifi_connected = false;
    bool ethernet_connected = false;
    
    // Check ethernet interfaces
    FILE* pipe = popen("ip addr show eth0 2>/dev/null | grep 'state UP' && ip addr show eth0 | grep 'inet '", "r");
    if (pipe) {
        char buffer[512];
        std::string output;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            output += buffer;
        }
        pclose(pipe);
        if (output.find("state UP") != std::string::npos && output.find("inet ") != std::string::npos) {
            ethernet_connected = true;
        }
    }
    
    // Check wifi interfaces
    pipe = popen("ls /sys/class/net/ | grep -E '^(wlan|wlp|wifi)' | head -1", "r");
    if (pipe) {
        char interface[32];
        if (fgets(interface, sizeof(interface), pipe)) {
            pclose(pipe);
            // Remove newline
            std::string iface(interface);
            iface.erase(iface.find_last_not_of(" \n\r\t") + 1);
            
            // Check if interface is up and has IP
            std::string cmd = "ip addr show " + iface + " 2>/dev/null | grep 'state UP' && ip addr show " + iface + " | grep 'inet '";
            pipe = popen(cmd.c_str(), "r");
            if (pipe) {
                char buffer[512];
                std::string output;
                while (fgets(buffer, sizeof(buffer), pipe)) {
                    output += buffer;
                }
                pclose(pipe);
                if (output.find("state UP") != std::string::npos && output.find("inet ") != std::string::npos) {
                    wifi_connected = true;
                }
            }
        } else {
            pclose(pipe);
        }
    }
    
    return {wifi_connected, ethernet_connected};
}

// Update status LEDs
void update_status_leds() {
    float temp = get_cpu_temperature();
    float mem_usage = get_memory_usage();
    int disk_usage = get_disk_usage();
    auto [wifi_connected, ethernet_connected] = get_network_status();
    
    write_led(leds.alarm, (temp > 70) ? 1 : 0);
    write_led(leds.usb, (disk_usage > 80) ? 1 : 0);
    write_led(leds.wlan, wifi_connected ? 1 : 0);
    write_led(leds.lan, ethernet_connected ? 1 : 0);
    write_led(leds.pause, (mem_usage > 80) ? 1 : 0);
}

// Display clock
void display_clock() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto local_tm = *std::localtime(&time_t);
    
    int hour = local_tm.tm_hour;
    int minute = local_tm.tm_min;
    int second = local_tm.tm_sec;
    
    // Convert to 12-hour format
    int display_hour = hour % 12;
    if (display_hour == 0) display_hour = 12;
    bool is_pm = hour >= 12;
    
    // Blink colon every second
    write_led(leds.colon, (second % 2 == 0) ? 1 : 0);
    
    // Update status LEDs
    update_status_leds();
    write_led(leds.play, is_pm ? 1 : 0);
    
    // Format time as HHMM
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << display_hour 
        << std::setw(2) << minute;
    write_display_text(oss.str());
}

// Display system stats
void display_system_stats(const std::string& stat_type) {
    float temp = get_cpu_temperature();
    float mem_usage = get_memory_usage();
    int disk_usage = get_disk_usage();
    
    std::string display_text;
    if (stat_type == "cpu") {
        display_text = "C " + std::to_string(static_cast<int>(temp));
    } else if (stat_type == "mem") {
        display_text = "r " + std::to_string(static_cast<int>(mem_usage));
    } else if (stat_type == "storage") {
        display_text = "S " + std::to_string(disk_usage);
    }
    
    update_status_leds();
    write_led(leds.colon, 1); // Turn on colon for stats
    write_display_text(display_text);
}

// Check if file exists
bool file_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// Clear all LEDs
void clear_all_leds() {
    write_led(leds.alarm, 0);
    write_led(leds.colon, 0);
    write_led(leds.lan, 0);
    write_led(leds.pause, 0);
    write_led(leds.play, 0);
    write_led(leds.usb, 0);
    write_led(leds.wlan, 0);
}

int main() {
    // Check if running as root
    if (geteuid() != 0) {
        std::cerr << "This program requires root privileges. Please run with sudo." << std::endl;
        return 1;
    }
    
    // Set up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    std::cout << "Starting 4-Digit VFD Controller for TX3 Mini (C++ Version)" << std::endl;
    std::cout << "Display text path: " << DISPLAY_TEXT << std::endl;
    
    // Check if display is available
    bool display_available = file_exists(DISPLAY_TEXT);
    std::cout << "SPI display available: " << (display_available ? "Yes" : "No") << std::endl;
    
    // Initial display
    if (display_available) {
        write_display_text("TX3M");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Main loop timing:
    // Clock: 10 seconds, CPU temp: 4 seconds, Memory: 3 seconds, Storage: 3 seconds
    // Total cycle: 20 seconds
    auto cycle_start = std::chrono::steady_clock::now();
    
    while (keep_running) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - cycle_start);
        int cycle_position = elapsed.count() % 20;
        
        if (cycle_position < 10) {
            // Clock for 10 seconds
            display_clock();
        } else if (cycle_position < 14) {
            // CPU temperature for 4 seconds
            display_system_stats("cpu");
        } else if (cycle_position < 17) {
            // Memory usage for 3 seconds
            display_system_stats("mem");
        } else {
            // Storage usage for 3 seconds
            display_system_stats("storage");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // Cleanup on exit
    write_display_text("    ");
    clear_all_leds();
    
    return 0;
}
