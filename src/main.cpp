#include <Arduino.h>
//---------------------------------------- Including Libraries.
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define T_CS_PIN  22 //--> T_CS

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Variables for x, y and z values ​​on the touchscreen.
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];


// Declaring the "XPT2046_Touchscreen" object as "touchscreen" and its settings.
XPT2046_Touchscreen touchscreen(T_CS_PIN);


//________________________________________________________________________________ log_print()
// If logging is enabled, it will inform the user about what is happening in the library.
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}


//________________________________________________________________________________ touchscreen_read()
// Get the Touchscreen data.
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    
    // Calibrate Touchscreen points with map function to the correct width and height.
    x = map(p.x, 230, 3920, 1, SCREEN_WIDTH);
    y = map(p.y, 400, 3905, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates.
    data->point.x = x;
    data->point.y = y;

    // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor.
    // Serial.print("X = ");
    // Serial.print(x);
    // Serial.print(" | Y = ");
    // Serial.print(y);
    // Serial.print(" | Pressure = ");
    // Serial.print(z);
    // Serial.println();
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}


static void add_data(lv_timer_t * t)
{
    lv_obj_t * chart = (lv_obj_t *)lv_timer_get_user_data(t);
    lv_chart_series_t * ser = lv_chart_get_series_next(chart, NULL);

    lv_chart_set_next_value(chart, ser, analogRead(14));

    uint16_t p = lv_chart_get_point_count(chart);
    uint16_t s = lv_chart_get_x_start_point(chart, ser);
    int32_t * a = lv_chart_get_y_array(chart, ser);

    a[(s + 1) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;

    lv_chart_refresh(chart);
}

void lv_draw_screen() {
  /*Create a stacked_area_chart.obj*/
  lv_obj_t * chart = lv_chart_create(lv_screen_active());
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 1600, 2000);
  lv_obj_set_style_size(chart, 0, 0, LV_PART_INDICATOR);
  lv_obj_set_size(chart, 280, 150);
  lv_obj_center(chart);

  lv_chart_set_point_count(chart, 80);
  lv_chart_series_t * ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  /*Prefill with data*/
  uint32_t i;
  for(i = 0; i < 80; i++) {
      lv_chart_set_next_value(chart, ser, lv_rand(10, 11));
  }

  lv_timer_create(add_data, 1, chart);
}


void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Serial.println("ESP32 + TFT LCD Touchscreen ILI9341 + LVGL + EEZ Studio");
  Serial.println();
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  Serial.println();
  Serial.println("Start LVGL Setup.");

  // Start the SPI for the touchscreen and init the touchscreen.
  touchscreen.begin();

  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0. "touchscreen.setRotation(0);"
  touchscreen.setRotation(0);

  // Start LVGL.
  lv_init();

  // Register print function for debugging.
  lv_log_register_print_cb(log_print);

  // Create a display object.
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library.
  disp = lv_tft_espi_create( SCREEN_WIDTH,SCREEN_HEIGHT, draw_buf, DRAW_BUF_SIZE);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

    // Initialize an LVGL input device object (Touchscreen).
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    
    // Set the callback function to read Touchscreen input.
    lv_indev_set_read_cb(indev, touchscreen_read);

    Serial.println();
    Serial.println("LVGL Setup Completed.");


    lv_draw_screen();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);
}
