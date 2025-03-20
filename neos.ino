#include <lvgl.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD.h>
#include "touch.h"

// Define rotation constants if not defined in touch.h
#ifndef ROTATION_NORMAL
  #define ROTATION_NORMAL   0
  #define ROTATION_INVERTED 1
  #define ROTATION_LEFT     2
  #define ROTATION_RIGHT    3
#endif

// Create TFT instance
TFT_eSPI tft = TFT_eSPI();

#define SD_CS 5  // SD Card Chip Select

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

// LVGL Touchpad Read Callback
void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
  if (touch_touched()) {
    data->state = LV_INDEV_STATE_PR;
    data->point.x = touch_last_x;
    data->point.y = touch_last_y;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// Button event callback
static void btn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    // Generate a random color for the button background
    lv_color_t new_color = lv_color_make(random(0, 256), random(0, 256), random(0, 256));
    lv_obj_set_style_bg_color(btn, new_color, LV_PART_MAIN);
  }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}

    // Set chip select pins
    pinMode(10, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(10, HIGH);
    digitalWrite(5, HIGH);

    // Initialize TFT display
    tft.init();
    tft.setRotation(1);

    // Initialize LVGL
    lv_init();

    // Initialize touch with updated parameters
    touch_init(480, 320, ROTATION_NORMAL);

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

    // Initialize LVGL input device driver for touch
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Seed the random number generator
    randomSeed(analogRead(0));

    // Create a Screen & Set Background to Black
    lv_obj_t *scr = lv_scr_act(); 
    lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);

    // Create the touch-sensitive button in the middle
    lv_obj_t *btn = lv_btn_create(scr);
    lv_obj_set_size(btn, 50, 100); // Change size to 50 wide and 100 tall
    lv_obj_center(btn); // Center the button
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    // Create a Rounded Bar Outline on the Right (Wider Bar)
    lv_obj_t *rounded_bar = lv_obj_create(scr);
    lv_obj_set_size(rounded_bar, 80, 300); // Make the bar 80 wide and 300 tall
    lv_obj_align(rounded_bar, LV_ALIGN_RIGHT_MID, -30, 0); // Slightly move top bar to the right
    lv_obj_set_style_radius(rounded_bar, 20, LV_PART_MAIN); // Rounded corners
    lv_obj_set_style_bg_color(rounded_bar, lv_color_hex(0x1F1F1F), LV_PART_MAIN); // Darker shade
    lv_obj_set_style_bg_opa(rounded_bar, LV_OPA_COVER, LV_PART_MAIN); // Fully opaque background
    lv_obj_set_style_border_width(rounded_bar, 0, LV_PART_MAIN);  // No border

    // Ensure no padding
    lv_obj_set_style_pad_all(rounded_bar, 0, LV_PART_MAIN);  // Remove all padding

    // Add three rounded boxes inside the bottom/right bar
    // First rounded box
    lv_obj_t *round_box1 = lv_obj_create(rounded_bar);
    lv_obj_set_size(round_box1, 50, 50); // 50x50 pixel box
    lv_obj_align(round_box1, LV_ALIGN_TOP_MID, 0, 30); // Position near top of bar
    lv_obj_set_style_radius(round_box1, 10, LV_PART_MAIN); // Slightly rounded corners
    lv_obj_set_style_border_color(round_box1, lv_color_white(), LV_PART_MAIN); // White border
    lv_obj_set_style_border_width(round_box1, 2, LV_PART_MAIN); // 2px border width
    lv_obj_set_style_border_opa(round_box1, LV_OPA_COVER, LV_PART_MAIN); // Fully opaque border
    lv_obj_set_style_bg_opa(round_box1, LV_OPA_TRANSP, LV_PART_MAIN); // Transparent background

    // Second rounded box
    lv_obj_t *round_box2 = lv_obj_create(rounded_bar);
    lv_obj_set_size(round_box2, 50, 50); // 50x50 pixel box
    lv_obj_align(round_box2, LV_ALIGN_CENTER, 0, 0); // Position in center of bar
    lv_obj_set_style_radius(round_box2, 10, LV_PART_MAIN); // Slightly rounded corners
    lv_obj_set_style_border_color(round_box2, lv_color_white(), LV_PART_MAIN); // White border
    lv_obj_set_style_border_width(round_box2, 2, LV_PART_MAIN); // 2px border width
    lv_obj_set_style_border_opa(round_box2, LV_OPA_COVER, LV_PART_MAIN); // Fully opaque border
    lv_obj_set_style_bg_opa(round_box2, LV_OPA_TRANSP, LV_PART_MAIN); // Transparent background

    // Third rounded box
    lv_obj_t *round_box3 = lv_obj_create(rounded_bar);
    lv_obj_set_size(round_box3, 50, 50); // 50x50 pixel box
    lv_obj_align(round_box3, LV_ALIGN_BOTTOM_MID, 0, -30); // Position near bottom of bar
    lv_obj_set_style_radius(round_box3, 10, LV_PART_MAIN); // Slightly rounded corners
    lv_obj_set_style_border_color(round_box3, lv_color_white(), LV_PART_MAIN); // White border
    lv_obj_set_style_border_width(round_box3, 2, LV_PART_MAIN); // 2px border width
    lv_obj_set_style_border_opa(round_box3, LV_OPA_COVER, LV_PART_MAIN); // Fully opaque border
    lv_obj_set_style_bg_opa(round_box3, LV_OPA_TRANSP, LV_PART_MAIN); // Transparent background

    // Initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(SD_CS, tft.getSPIinstance())) {
        Serial.println("Card Mount Failed");
        return;
    }
    if (SD.cardType() == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }
    Serial.println("SD Card Initialized Successfully.");
}

void loop() {
    lv_task_handler();  // LVGL tasks
    delay(5);
}