#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <chrono>
#include <vector>
#include <string>
#include <mysql/mysql.h>
#include <nlohmann/json.hpp> // For JSON parsing

// Define error codes if they are not already defined
#ifndef CR_SERVER_GONE_ERROR
#define CR_SERVER_GONE_ERROR 2006
#endif
#ifndef CR_SERVER_LOST
#define CR_SERVER_LOST 2013
#endif

// MySQL connection details
const char* MYSQL_HOST = std::getenv("MYSQL_HOST");
const char* MYSQL_USER = std::getenv("MYSQL_USER");
const char* MYSQL_PASSWORD = std::getenv("MYSQL_PASSWORD");
const char* MYSQL_DB = std::getenv("MYSQL_DB");

void configure_sensor(int serial_port) {
    // Get the current time_point from the system clock
    auto now = std::chrono::system_clock::now();

    // Convert the time_point to a duration since the epoch
    auto duration = now.time_since_epoch();

    // Convert the duration to seconds
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    // List of configuration commands
    std::vector<std::string> config_commands = {
        "UM",                  // US for MPH , UK for Km/H, UM for m/s
        "C=" + std::to_string(seconds),
        "S2",                  // 10Ksps
        "PX",                  // Max power
        "M>8\n",               // Magnitude must be > this
        "m>8\n"                // Low range magnitude filter
        "F3",                  // F-zero for no decimal reporting
        "BV",                  // Blanks pref: silence
        "OS",                  // Turns on Speed report
        "Od",                  // OD turns on Range report
        "OM",                  // Turns on Magnitude report
        "OU",                  // Turns on Units report
        "OV",                  // Ordering of signals (MAX first)
        "OT",                  // Turn on time report
        "OJ",                  // JSON output format
        "O1",                  // Number of reports (magnitude and speed)
        "WC",                  // 100ms delay between reports
        "R<200\n",             // Report only < than this speed
        "R>1\n",               // Report only > this speed
        "R+",                  // Inbound only. Bidirectional reporting:|
	"IR",                  // If USB is connected, send data still via UART
	"A!",                  // Save configuration to memory
    };

    for (const auto& command : config_commands) {
        std::string cmd = command + "\n"; // Ensure each command is followed by a newline
        write(serial_port, cmd.c_str(), cmd.size());
        usleep(100000); // Wait for 100ms to ensure the command is processed
    }
}

bool insert_data(MYSQL* conn, const std::string& time, const std::string& unit, const std::string& magnitude, const std::string& speed) {
    std::string query = "INSERT INTO hail_speed_data (time, unit, magnitude, speed) VALUES ('" + time + "', '" + unit + "', '" + magnitude + "', '" + speed + "')";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "INSERT failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    return true;
}

void attempt_reconnect(MYSQL* &conn) {
    std::cerr << "Attempting to reconnect..." << std::endl;
    while (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, 0, NULL, 0) == NULL) {
        std::cerr << "Reconnection failed: " << mysql_error(conn) << std::endl;
        sleep(5); // Wait 5 seconds before trying again
    }
    std::cerr << "Reconnected successfully" << std::endl;
}

int main() {
    const char* port = "/dev/serial0"; // or /dev/ttyAMA0 depending on your setup
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

    // Set Baud Rate
    cfsetospeed(&tty, B19200);
    cfsetispeed(&tty, B19200);

    // 8N1 Mode
    tty.c_cflag &= ~PARENB; // No parity bit
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8 data bits

    // No flow control
    tty.c_cflag &= ~CRTSCTS;

    // Raw mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(INLCR | ICRNL); // Disable special handling of received bytes
    tty.c_oflag &= ~OPOST; // Raw output mode

    // Set blocking mode
    tty.c_cc[VMIN] = 1;  // Minimum number of characters to read
    tty.c_cc[VTIME] = 10; // Timeout in deciseconds for read

    // Apply the configuration
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        std::cerr << "Error from tcsetattr: " << strerror(errno) << std::endl;
        close(serial_port);
        return 1;
    }

    // Configure the sensor
    configure_sensor(serial_port);

    // Connect to MySQL database
    MYSQL* conn;
    conn = mysql_init(NULL);
    if (conn == NULL) {
        std::cerr << "mysql_init() failed" << std::endl;
        return 1;
    }

    if (mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DB, 0, NULL, 0) == NULL) {
        std::cerr << "mysql_real_connect() failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return 1;
    }

    // Buffer to store failed inserts
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> failed_inserts;

    // Read data from the OPS243 sensor
    std::string buffer;
    char buf[256];
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

                    if (!time.empty() && !unit.empty() && !magnitude.empty() && !speed.empty()) {
                        if (!insert_data(conn, time, unit, magnitude, speed)) {
                            failed_inserts.emplace_back(time, unit, magnitude, speed);
                            if (mysql_errno(conn) == CR_SERVER_GONE_ERROR || mysql_errno(conn) == CR_SERVER_LOST) {
                                attempt_reconnect(conn);
                                for (auto it = failed_inserts.begin(); it != failed_inserts.end();) {
                                    if (insert_data(conn, std::get<0>(*it), std::get<1>(*it), std::get<2>(*it), std::get<3>(*it))) {
                                        it = failed_inserts.erase(it);
                                    } else {
                                        ++it;
                                    }
                                }
                            }
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

    mysql_close(conn);
    close(serial_port);
    return 0;
}

