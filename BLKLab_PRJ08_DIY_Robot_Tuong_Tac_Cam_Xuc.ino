/*
  Banlinhkien.com 
  PRJ09: DIY Robot Tương Tác Cảm Xúc
  Tính năng:
    - Hiển Thị Cảm Xúc Qua Màn Hình LCD Tròn: Màn hình LCD tròn hiển thị các biểu cảm khuôn mặt sinh động như: vui, buồn, tức giận, ngạc nhiên, buồn ngủ...
    - Phát Âm Thanh Phù Hợp Với Từng Cảm Xúc: Sử dụng module âm thanh kết hợp loa mini để phát âm thanh tương ứng với từng biểu cảm.
    - Cảm Biến Chạm Điều Khiển Cảm Xúc: Tích hợp cảm biến chạm để người dùng dễ dàng chuyển đổi giữa các trạng thái cảm xúc chỉ bằng thao tác đơn giản.
    - Lưu Trữ Hình Ảnh và Âm Thanh Trên Thẻ SD: Tất cả dữ liệu hình ảnh và âm thanh được lưu trong thẻ microSD giúp dễ dàng cập nhật và mở rộng nội dung.
  Đấu nối:
  --------------------------------
  ESP32-C3        |           MÀN HÌNH GC9A01
  VCC             |           3.3V  
  GND             |           GND  
  6               |           SCL 
  7               |           SDA  
  8               |           DC  
  9               |           CS  
  10              |           RST  
  ---------------------------------
  ESP32-C3        |            Cảm biến chạm
  VCC             |            5V
  GND             |            GND
  2             |            I/O
  ---------------------------------
  ESP32-C3        |         Module thẻ nhớ micorSD
  VCC             |           5V
  GND             |           GND
  1               |           CS
  3               |           SCL
  4               |           MOSI
  5               |           MISO 
  --------------------------------
  Lưu ý khi nạp code:
   + Chọn Board: ESP32C3 Dev Module
   + Chọn Port : Port tương ứng
   + USB CDC On BOOT : Enable
   + Patition Scheme: Huge App (3MB No OTA/ 1Mb SPIFFS)
  Chọn đường dẫn thư viện:
    Vào File -> Preferences -> Sketchbook location -> Các bạn trỏ đến thư mục chứa thư mục BLKLab_Thu_Vien trong tài liệu
    Chú ý đường dẫn không được có dấu tiếng việt, không có kí đặc biệt.
*/
#include <Arduino_GFX_Library.h>                // Thư viện điều khiển màn hình TFT
#include <SD.h>                                 // Thư viện làm việc với thẻ nhớ SD
#include "GifClass.h"                           // Thư viện xử lý ảnh GIF
#include "JQ6500.h"

// Cấu hình SPI cho màn hình GC9A01A
#define GFX_BL 18
#define TFT_MISO 19
#define TFT_SCLK 6
#define TFT_MOSI 7
#define TFT_CS 9
#define TFT_DC 8
#define TFT_RST 10

// Cấu hình SPI cho thẻ SD
#define SD_MOSI 4
#define SD_MISO 5
#define SD_SCLK 3
#define SD_CS   1
#define btn 2

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST /* RST */, 0 /* rotation */, true /* IPS */);

JQ6500 jq6500;  // Đối tượng JQ6500

int currentSongNumber = 1;                    // Chỉ mục file âm thanh, bắt đầu từ 1
int volumeJQ6500 = 20;                        // Mức âm lượng (0-30, tùy module JQ6500)

static GifClass gifClass;                     // Đối tượng xử lý GIF
File root;                                    // Biến lưu thư mục gốc chứa ảnh GIF
File file;                                    // Biến đại diện cho file ảnh GIF
String filegif;                               // Tên file GIF
bool isNext = false;                          // Biến cờ kiểm tra trạng thái chuyển ảnh GIF

void play(String namafile) {                  // Hàm hiển thị GIF
  isNext = false;                             // Đặt lại biến cờ khi bắt đầu hiển thị GIF
  while (!isNext) {                           // Lặp lại cho đến khi nút nhấn được bấm (isNext chuyển thành true)
    File gifFile = SD.open(namafile, "r");
    gd_GIF *gif = gifClass.gd_open_gif(&gifFile);
    if (!gif) {
      Serial.println(F("gd_open_gif() failed!"));
    } else {
      uint8_t *buf = (uint8_t *)malloc(gif->width * gif->height);
      if (!buf) {
        Serial.println(F("buf malloc failed!"));
      } else {
        int16_t x = (gfx->width() - gif->width) / 2;
        int16_t y = (gfx->height() - gif->height) / 2;

        Serial.println(F("GIF video start"));
        int32_t t_fstart, t_delay = 0, t_real_delay, delay_until;
        int32_t res = 1;
        int32_t duration = 0, remain = 0;
        while (res > 0) {                     // Lặp qua từng frame của ảnh GIF và hiển thị lên màn hình
          t_fstart = millis();
          t_delay = gif->gce.delay * 10;
          res = gifClass.gd_get_frame(gif, buf);
          if (res < 0) {
            Serial.println(F("ERROR: gd_get_frame() failed!"));
            break;
          } else if (res > 0) {               // Vẽ frame GIF lên màn hình
            gfx->drawIndexedBitmap(x, y, buf, gif->palette->colors, gif->width, gif->height);

            t_real_delay = t_delay - (millis() - t_fstart);
            duration += t_delay;
            remain += t_real_delay;
            delay_until = millis() + t_real_delay;
            while (millis() < delay_until) {
              delay(1);
            }
            if (digitalRead(btn) == HIGH) {   // Kiểm tra nút nhấn để chuyển GIF
              isNext = true;
              res = 0;
              delay(500);                     // Chống dội nút
              // Đợi cho đến khi nút được thả ra (giúp chống dội và đảm bảo chỉ nhận 1 lần nhấn)
              while(digitalRead(btn) == HIGH) {
                delay(10);
              }
            }
          }
        }
        Serial.println(F("GIF video end"));
        Serial.print(F("duration: "));
        Serial.print(duration);
        Serial.print(F(", remain: "));
        Serial.print(remain);
        Serial.print(F(" ("));
        Serial.print(100.0 * remain / duration);
        Serial.println(F("%)"));

        gifClass.gd_close_gif(gif);           // Đóng file GIF
        free(buf);                            // Giải phóng bộ nhớ
      }
    }
  }
}

void next() {
  file = root.openNextFile();  // Mở file GIF tiếp theo
  if (!file) {
    Serial.println("Hết file GIF, quay lại từ đầu");
    root.rewindDirectory();  // Quay lại đầu danh sách file GIF
    file = root.openNextFile();  // Lấy file GIF đầu tiên
    jq6500.playFileByIndex(1);  // Phát bài hát đầu tiên để đồng bộ
    jq6500.play();
  }
  if (!file.isDirectory()) {
    filegif = file.name();
    if (filegif.endsWith(".gif")) {
      filegif = "/gif/" + filegif;  // Đường dẫn tới file GIF
      jq6500.nextTrack();  // Chuyển sang bài hát tiếp theo
      play(filegif);  // Hiển thị file GIF
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("🚀 Khởi động hệ thống...");

  // Khởi tạo JQ6500 (điều chỉnh chân RX, TX nếu cần)
  jq6500.begin();                    // Khởi tạo JQ6500 với Serial1
  Serial.println("🔄 JQ6500 UART Initialized");

  gfx->begin();                             // Khởi động màn hình TFT
  gfx->fillScreen(BLACK);                   // Bật đèn nền
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
  pinMode(btn, INPUT);                      // Cấu hình nút nhấn

  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, -1); // Khởi tạo giao tiếp với thẻ SD
  if (!SD.begin(SD_CS, SPI, 2000000)) {
    Serial.println(F("ERROR: ❌ Không tìm thấy thẻ SD!"));
    gfx->println(F("ERROR: KHONG TIM THAY THE SD!!!"));
    delay(500);
    exit(0);
  }
  Serial.println("✅ SD đã sẵn sàng!");

  root = SD.open("/gif");
  if (!root) {
    Serial.println("❌ Không tìm thấy thư mục GIF!");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("❌ Không tìm thấy File GIF!");
    return;
  }
  next();                                   // Bắt đầu với GIF đầu tiên
}

void loop() {
  if (isNext) {
    isNext = false;
    next();                                 // Chuyển sang GIF tiếp theo khi isNext = true
  }
}
