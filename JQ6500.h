#ifndef JQ6500_H
#define JQ6500_H

#include <Arduino.h>

// Khai báo chân UART
#define RXD2 20  // Chân RX của ESP32-C3
#define TXD2 21  // Chân TX của ESP32-C3

class JQ6500 {
public:
    // Khởi tạo UART
    void begin();

    // Điều khiển nhạc
    void play();                     // Phát nhạc
    void pauseMP3();                    // Tạm dừng
    void resetMP3();                    // Tạm dừng
    void nextTrack();                // Bài tiếp theo
    void prevTrack();                // Bài trước
    void playSpecificFile(uint8_t folder, uint8_t file); // Phát file cụ thể
    void playFileByIndex(int index); // Phát file theo index
    
    // Cấu hình
    void setVolume(uint8_t volume);  // Đặt âm lượng (0-30)
    void setLoopMode(uint8_t mode);  // Đặt chế độ lặp (0-4)

    // Truy vấn trạng thái
    uint8_t getStatus();             // Lấy trạng thái
    String readResponse();           // Đọc phản hồi

private:
    void sendCommand(uint8_t cmd, uint8_t arg1 = 0, uint8_t arg2 = 0); // Gửi lệnh UART
};

#endif
