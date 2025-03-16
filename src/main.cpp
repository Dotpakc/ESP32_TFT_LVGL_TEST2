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

void lv_draw_screen() {
  lv_obj_t * scr = lv_disp_get_scr_act(NULL);

  lv_obj_t * label = lv_label_create(scr);
  lv_label_set_text(label, "Hello World!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t * btn = lv_btn_create(scr);
  lv_obj_align(btn, LV_ALIGN_CENTER, 0, 40);
  lv_obj_add_event_cb(btn, [](lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
      Serial.println("Button Clicked");
      static int count = 0;
      char buf[32];
      snprintf(buf, sizeof(buf), "Button Clicked %d", count++);
      lv_label_set_text(lv_obj_get_child(obj, 0), buf);
    }
  }, LV_EVENT_ALL, NULL);

  lv_obj_t * label_btn = lv_label_create(btn);
  lv_label_set_text(label_btn, "Click Me!");
  lv_obj_center(label_btn);
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
