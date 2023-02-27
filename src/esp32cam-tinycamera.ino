#include "esp_camera.h"
#include "SPI.h"
#include <TFT_eSPI.h>              // Hardware-specific library
#include <TJpg_Decoder.h>
#include "FS.h"
#include "SD_MMC.h"

/*
 * Important Note:
 * 1. Remember to flash firmware first, because boot-select pins and serial port are used
 * 
 * Esp32-cam Exposed Pin Mapping:
 * ESP32  CAMERA  PSRAM  TFT-LCD  SD-MMC    BUTTON
 *   1                      CS                      (UART TX)
 *   3                     SDA                      (UART RX)
 *   0     MCLK                                     (BOOT SELECT)
 *  16              CS
 *   4                      DC
 *   2                            DATA
 *  14                             CLK
 *  15                             CMD              (BOOT HIGH)
 *  13                     SCL
 *  12                                       INT    (BOOT LOW)

 * Settings in TFT_eSPI/User_Setup.h:
 
 * Pin definition for SPI displays to ESP32-CAM:
 *    #define TFT_MOSI  3
 *    #define TFT_SCLK 13
 *    #define TFT_CS    1
 *    #define TFT_DC    4
 *    #define TFT_RST  -1

 * For 1.8 inch ST7735 screen:
 *    #define ST7735_DRIVER
 *    #define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
 *    #define TFT_WIDTH  128
 *    #define TFT_HEIGHT 160
 *    #define ST7735_GREENTAB
 * For 3.5 inch RPi-TFT screen:
 *    #define RPI_ILI9486_DRIVER // 20MHz maximum SPI

*/

// Copy configs from official example. This is for AI-thinker esp32-cam
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

int BUTTON = 12; // BUTTON on GPIO 12
int RED_LED = 33; // onboard led on GPIO 33
boolean buttonPressed = false;

boolean hasSD = false;
int fileCount = 0;

int countFilesInDir(fs::FS &fs, const char* path){
  int count = 0;
  File root = fs.open(path);
  while(true){
    File entry =  root.openNextFile();
    if (!entry) break;// no more files
    if (!entry.isDirectory()) count++;
    entry.close();
  }
  root.close();
  return count;
}

void writeFile(fs::FS &fs, const char * path, const uint8_t* data, const size_t len){
    File file = fs.open(path, FILE_WRITE);
    if(!file) return;
    file.write(data, len);
    file.close();
}

// Test if SD card present. If so, set to 1-bit mode.
void init_sd(){
  if (!SD_MMC.begin("/sdcard", true)) return;
  else {
    hasSD = true;
    fileCount = countFilesInDir(SD_MMC, "/");
  }
}

// High resolution config for SD card
void init_camera_highres(){
  esp_camera_deinit();
  delay(10);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_UXGA; // 1600x1200
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10; //< Quality of JPEG output. 0-63 lower means higher quality
  config.fb_count = 2; //Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed)

  esp_err_t err = esp_camera_init(&config);

  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
}

// Low resolution config for TFT view
void init_camera_lowres(){
  esp_camera_deinit();
  delay(10);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_DRAM;
  config.jpeg_quality = 10; //< Quality of JPEG output. 0-63 lower means higher quality
  config.fb_count = 2; //Number of frame buffers to be allocated. If more than one, then each frame will be acquired (double speed)

  esp_err_t err = esp_camera_init(&config);

  sensor_t * s = esp_camera_sensor_get();
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
}

void init_tft(){

  // Initialise the TFT
  tft.begin();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);//1:landscape 3:inv. landscape
  tft.setSwapBytes(true); // Swap byte order if display weird color

  // The jpeg image can be scaled down by a factor of 1, 2, 4, or 8
  // FRAMESIZE_QQVGA(160*120), FRAMESIZE_QVGA(320*240), FRAMESIZE_VGA(640*480), and FRAMESIZE_SXGA(1280*1024)
  // FRAMESIZE_HVGA(480*320)
  TJpgDec.setJpgScale(2); // QVGA with 160x128 screen
  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);
}

// This next function will be called during decoding of the jpeg file to render each
// 16x16 or 8x8 image tile (Minimum Coding Unit) to the TFT.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  // Return 1 to decode next block.
  return 1;
}

// When pressed, set flag only
void buttonHandler(){
  buttonPressed = true;
}

void setup() {
  init_sd(); // For 1-bit mode, needs to be first one to set GPIOs
  init_tft();

  pinMode(BUTTON, INPUT_PULLDOWN); // GPIO 12 needs to be low on boot
  attachInterrupt(BUTTON, buttonHandler, FALLING); // this means just release the button

  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);

  init_camera_lowres(); // default to low-res

}

void loop() {
  // change to higher resolution, take a shot and then save to SD. lastly change resolution back.
  if (buttonPressed){ // not processed before
    while(digitalRead(BUTTON)) delay(10);
    buttonPressed = false;
    if (hasSD){
      init_camera_highres(); // capture at high-res
      camera_fb_t *fb = NULL;
      digitalWrite(RED_LED, LOW); // ON
      for(int i=0;i<10;i++) { // Drop first few frames for image quality to stablize
        fb = esp_camera_fb_get();
        delay(10);
        esp_camera_fb_return(fb);
        delay(10);
      }
      fb = esp_camera_fb_get();
      digitalWrite(RED_LED, HIGH); // OFF
      char filename[15];
      sprintf(filename, "/img%05d.jpg", ++fileCount);
      writeFile(SD_MMC, filename, fb->buf, fb->len);
      esp_camera_fb_return(fb);
      init_camera_lowres(); // return to low-res for display
    }
  }
  else { // show image on screen
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    fb = esp_camera_fb_get();
    if (!fb) {
      esp_camera_fb_return(fb);
      delay(100);
      return;
    }
    if (fb->format != PIXFORMAT_JPEG) {
      delay(100);
      return;
    }
    TJpgDec.drawJpg(0, 0,  fb->buf, fb->len);
    esp_camera_fb_return(fb);
  }
}
