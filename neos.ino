#include <lvgl.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>

// Create a TFT instance
TFT_eSPI tft = TFT_eSPI();

#define SD_CS 5  // SD Card Chip Select Pin

// LVGL Display Flush Callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint16_t w = (area->x2 - area->x1 + 1);
  uint16_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}  // Wait for Serial Monitor

  // Set chip select pins as OUTPUT and ensure they’re high during initialization
  pinMode(10, OUTPUT); 
  pinMode(5, OUTPUT);
  digitalWrite(10, HIGH); // TFT CS
  digitalWrite(5, HIGH);  // SD CS

  // Initialize TFT display
  tft.init();
  tft.setRotation(1); // Adjust as needed

  // Initialize LVGL
  lv_init();

  // Create Display Buffer
  static lv_color_t buf[480 * 10];  
  static lv_disp_draw_buf_t draw_buf;
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 480 * 10);

  // Register Display Driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 480;
  disp_drv.ver_res = 320;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Create a Screen & Set Background to Black
  lv_obj_t *scr = lv_scr_act(); 
  lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);

  // Create a White Rounded Bar Outline on the Right
  lv_obj_t *rounded_bar = lv_obj_create(scr);
  lv_obj_set_size(rounded_bar, 60, 300); // Make the bar 60 wide and 300 tall
  lv_obj_align(rounded_bar, LV_ALIGN_RIGHT_MID, -10, 0); // Position at the right with slight margin
  lv_obj_set_style_radius(rounded_bar, 20, LV_PART_MAIN); // Rounded corners
  lv_obj_set_style_border_color(rounded_bar, lv_color_white(), LV_PART_MAIN); // White border
  lv_obj_set_style_border_width(rounded_bar, 2, LV_PART_MAIN); // Border thickness
  lv_obj_set_style_bg_opa(rounded_bar, LV_OPA_0, LV_PART_MAIN); // Transparent background

  // Initialize SD card using the TFT SPI instance
  Serial.println("Initializing SD card...");
  
  if (!SD.begin(SD_CS, tft.getSPIinstance())) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.println("SD Card Initialized Successfully.");
}

void loop() {
  lv_task_handler();  // LVGL task handler
  delay(5);
}
