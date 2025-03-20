#include <FT6336.h>

#define TOUCH_FT6336
#define TOUCH_FT6336_SCL 35
#define TOUCH_FT6336_SDA 37
#define TOUCH_FT6336_INT -1
#define TOUCH_FT6336_RST 36
#define TOUCH_MAP_X1 0
#define TOUCH_MAP_X2 320
#define TOUCH_MAP_Y1 0
#define TOUCH_MAP_Y2 480

// Rotation constants if they're not defined elsewhere
#ifndef ROTATION_NORMAL
  #define ROTATION_NORMAL   0
  #define ROTATION_INVERTED 1
  #define ROTATION_LEFT     2
  #define ROTATION_RIGHT    3
#endif

// For portrait mode orientation, we generally don't want to swap X/Y
// Comment this out since we're in portrait mode
// #define TOUCH_SWAP_XY

int touch_last_x = 0, touch_last_y = 0;
unsigned short int width = 0, height = 0, rotation, min_x = 0, max_x = 0, min_y = 0, max_y = 0;

// Create the FT6336 instance.
// Note: using max(TOUCH_MAP_X1, TOUCH_MAP_X2) ensures a positive value for the touch sensor dimensions.
FT6336 ts = FT6336(TOUCH_FT6336_SDA, TOUCH_FT6336_SCL, TOUCH_FT6336_INT, TOUCH_FT6336_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));

void touch_init(unsigned short int w, unsigned short int h, unsigned char r) {
  width = w;
  height = h;
  rotation = r;
  
  switch (r) {
    case ROTATION_NORMAL:
    case ROTATION_INVERTED:
      min_x = TOUCH_MAP_X1;
      max_x = TOUCH_MAP_X2;
      min_y = TOUCH_MAP_Y1;
      max_y = TOUCH_MAP_Y2;
      break;
    case ROTATION_LEFT:
    case ROTATION_RIGHT:
      min_x = TOUCH_MAP_Y1;
      max_x = TOUCH_MAP_Y2;
      min_y = TOUCH_MAP_X1;
      max_y = TOUCH_MAP_X2;
      break;
    default:
      break;
  }
  ts.begin();
  ts.setRotation(r);
}

bool touch_touched(void) {
  ts.read();
  if (ts.isTouched) {
    #if defined(TOUCH_SWAP_XY)
      // Swap the axes: use the touch sensor's Y reading for display X and vice versa.
      touch_last_x = map(ts.points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, width - 1);
      touch_last_y = map(ts.points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, height - 1);
    #else
      touch_last_x = map(ts.points[0].x, min_x, max_x, 0, width - 1);
      touch_last_y = map(ts.points[0].y, min_y, max_y, 0, height - 1);
    #endif
    return true;
  } else {
    return false;
  }
}

bool touch_has_signal(void) {
  return true;
}

bool touch_released(void) {
  return true;
}