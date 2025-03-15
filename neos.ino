#wefsgsdg

#include <lvgl.h>
#include <TFT_eSPI.h>
#include "touch.h"

// Define rotation constants if not defined in touch.h
#ifndef ROTATION_NORMAL
  #define ROTATION_NORMAL   0
  #define ROTATION_INVERTED 1
  #define ROTATION_LEFT     2
  #define ROTATION_RIGHT    3
#endif

// Create TFT instance (using TFT_eSPI)
TFT_eSPI tft = TFT_eSPI();

//-------------------------------------------------------------
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

//-------------------------------------------------------------
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

//-------------------------------------------------------------
// Button Event Callback
static void btn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target(e);
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    // Generate a random color for the button background.
    lv_color_t new_color = lv_color_make(random(0, 256), random(0, 256), random(0, 256));
    lv_obj_set_style_bg_color(btn, new_color, LV_PART_MAIN);
  }
}

//-------------------------------------------------------------
// Setup Function
void setup() {
  Serial.begin(115200);

  // Initialize TFT display
  tft.init();
  tft.setRotation(1);  // Adjust rotation as needed

  // Initialize LVGL
  lv_init();

  // Initialize touch with updated parameters.
  // The display resolution is now assumed to be 480 (width) x 320 (height)
  touch_init(480, 320, ROTATION_NORMAL);

  // Create an LVGL draw buffer
  static lv_color_t buf[480 * 10];  // Buffer for 10 lines (adjust as needed)
  static lv_disp_draw_buf_t draw_buf;
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 480 * 10);

  // Initialize the display driver for LVGL.
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 480; // New horizontal resolution
  disp_drv.ver_res = 320; // New vertical resolution
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize LVGL input device driver for touch.
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  // Seed the random number generator
  randomSeed(analogRead(0));

  // Create a button on the active screen.
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn, 100, 50); // Set button size
  lv_obj_center(btn);           // Center the button
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

  // Add a label on the button.
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Click me!");
  lv_obj_center(label);
}

//-------------------------------------------------------------
// Main loop: let LVGL handle tasks.
void loop() {
  lv_task_handler();
  delay(5);
}
