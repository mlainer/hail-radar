#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <chrono>
#include <vector>
#include <string>
#include <nlohmann/json.hpp> // For JSON parsing

void configure_sensor(int serial_port) {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    std::vector<std::string> config_commands = {    
        "UM",                  
        "C=" + std::to_string(seconds),
        "S2",                  
        "PX",                  
        "M>4\n",               
        "m>4\n",
        "F3",                  
        "BV",                  
        "OS",                  
        "OD",                  
        "OM",                  
        "OU",                  
        "OV",                  
        "OT",                  
        "OJ",
        "OY",	
        "O1",                  
        "WC",                  
        "R>0",
        "r>0",	
        "R|",                  
        "IR",                  
        "?R"                   
    };

    for (const auto& command : config_commands) {
        std::string cmd = command + "\n";
        write(serial_port, cmd.c_str(), cmd.size());
        usleep(100000);
    }
}


bool log_data_to_csv(const std::string& filename, const std::string& time, const std::string& unit, const std::string& magnitude, const std::string& speed) {
    std::ofstream csv_file;
    csv_file.open(filename, std::ios_base::app);
    if (!csv_file.is_open()) {
        std::cerr << "Failed to open CSV file!" << std::endl;
        return false;
    }
    csv_file << time << "," << unit << "," << magnitude << "," << speed << "\n";
    csv_file.close();
    return true;
}

bool log_range_to_csv(const std::string& filename, const std::string& time, const std::string& unit, const std::string& range) {
    std::ofstream csv_file;
    csv_file.open(filename, std::ios_base::app);
    if (!csv_file.is_open()) {
        std::cerr << "Failed to open CSV file for range!" << std::endl;
        return false;
    }
    csv_file << time << "," << unit << "," << range << "\n";
    csv_file.close();
    return true;
}

int main() {
    const char* port = "/dev/serial0";
    int serial_port = open(port, O_RDWR | O_NOCTTY | O_SYNC);

    if (serial_port == -1) {
        std::cerr << "Failed to open the serial port!" << std::endl;
        return 1;
    }

    termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(serial_port, &tty) != 0) {
        std::cerr << "Error from tcgetattr: " << strerror(errno) << std::endl;
        close(serial_port);
        return 1;
    }

    cfsetospeed(&tty, B19200);
    cfsetispeed(&tty, B19200);

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(INLCR | ICRNL);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
        close(serial_port);
        return 1;
    }

    configure_sensor(serial_port);

    std::string buffer;
    char buf[256];
    const std::string csv_filename = "./data/hail_speed_data.csv";
    const std::string range_csv_filename = "./data/hail_range_data.csv";

    std::ofstream csv_file;
    csv_file.open(csv_filename, std::ios_base::out);
    if (csv_file.is_open()) {
        csv_file << "Time,Unit,Magnitude,Speed\n";
        csv_file.close();
    } else {
        std::cerr << "Failed to create CSV file for speed data!" << std::endl;
        return 1;
    }

    std::ofstream range_csv_file;
    range_csv_file.open(range_csv_filename, std::ios_base::out);
    if (range_csv_file.is_open()) {
        range_csv_file << "Time,Unit,Range\n";
        range_csv_file.close();
    } else {
        std::cerr << "Failed to create CSV file for range data!" << std::endl;
        return 1;
    }

    while (true) {
        int n = read(serial_port, &buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            buffer.append(buf, n);

            size_t pos = 0;
            while ((pos = buffer.find('\n')) != std::string::npos) {
                std::string message = buffer.substr(0, pos);
                std::cout << "Received: " << message << std::endl;

                try {
                    auto json_message = nlohmann::json::parse(message);
                    std::string time = json_message.value("time", "");
                    std::string unit = json_message.value("unit", "");
                    std::string magnitude = json_message.value("magnitude", "");
                    std::string speed = json_message.value("speed", "");
                    std::string range = json_message.value("range", "");

                    if (!time.empty() && !unit.empty()) {
                        if (!magnitude.empty() && !speed.empty()) {
                            log_data_to_csv(csv_filename, time, unit, magnitude, speed);
                        } else if (!range.empty()) {
                            log_range_to_csv(range_csv_filename, time, unit, range);
                        }
                    }
                } catch (nlohmann::json::parse_error& e) {
                    std::cerr << "JSON parse error: " << e.what() << std::endl;
                }

                buffer.erase(0, pos + 1);
            }
        } else if (n < 0) {
            std::cerr << "Error reading from serial port: " << strerror(errno) << std::endl;
            break;
        }
    }

    close(serial_port);
    return 0;
}
