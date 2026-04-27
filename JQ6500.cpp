#include "JQ6500.h"

// Khởi tạo UART
HardwareSerial jqSerial(1);

void JQ6500::begin() {
    jqSerial.begin(9600, SERIAL_8N1, RXD2, TXD2); // UART1
    delay(500);
}

// Gửi lệnh UART
void JQ6500::sendCommand(uint8_t cmd, uint8_t arg1, uint8_t arg2) {
    uint8_t length = 2; // CMD + Stop byte
    if (arg1 != 0 || arg2 != 0) length += (arg1 != 0) + (arg2 != 0);

    jqSerial.write(0x7E);       // Start Byte
    jqSerial.write(length);     // Byte count
    jqSerial.write(cmd);        // Command
    if (arg1 != 0) jqSerial.write(arg1);
    if (arg2 != 0) jqSerial.write(arg2);
    jqSerial.write(0xEF);       // Stop Byte

    delay(100);
}

// Đọc phản hồi
String JQ6500::readResponse() {
    String response = "";
    unsigned long timeout = millis() + 1000; // Timeout 1 giây
    while (millis() < timeout) {
        if (jqSerial.available()) {
            char c = jqSerial.read();
            response += c;
        }
    }
    return response;
}

// Các lệnh điều khiển
void JQ6500::play() {
    sendCommand(0x0D); // Play
}

// Các lệnh điều khiển
void JQ6500::resetMP3() {
    sendCommand(0x0C); // Play
}

void JQ6500::pauseMP3() {
    sendCommand(0x0E); // Pause
}

void JQ6500::nextTrack() {
    sendCommand(0x01); // Next
}

void JQ6500::prevTrack() {
    sendCommand(0x02); // Prev
}

void JQ6500::playSpecificFile(uint8_t folder, uint8_t file) {
    sendCommand(0x12, folder, file); // Play specific file
}

void JQ6500::playFileByIndex(int index) {
    jqSerial.write(0x7E);
    jqSerial.write(0x04);
    jqSerial.write(0x03);
    jqSerial.write(0x00);
    jqSerial.write(index);
    jqSerial.write(0xEF);
}

// Cấu hình
void JQ6500::setVolume(uint8_t volume) {
    if (volume > 30) volume = 30;
    jqSerial.write(0x7E);       // Start Byte
    jqSerial.write(0x03);     // Byte count
    jqSerial.write(0x06);        // Command
    jqSerial.write(volume);
    jqSerial.write(0xEF);       // Stop Byte
}

void JQ6500::setLoopMode(uint8_t mode) {
    if (mode > 4) mode = 0;
    sendCommand(0x11, mode); // Set loop mode
}

// Lấy trạng thái
uint8_t JQ6500::getStatus() {
    sendCommand(0x42); // Get Status
    String response = readResponse();
    return (uint8_t)strtol(response.c_str(), NULL, 16);
}
