#include <pebble.h>

#define M_LAYER_W 33
#define M_LAYER_H 20
#define STATUS_LINE_TIMEOUT 5000
static Window *s_main_window;
static TextLayer *h_layer;
static TextLayer *m_layer;
static TextLayer *x_date_layer;
static uint8_t m_x=144, m_y=168;

static AppTimer *shake_timeout = NULL;


static GFont h_font;
static GFont m_font;
static GFont x_font;

void show_extra (void *isShow) {
  //if hiding
  if (!isShow) {
    isShow=false;
    shake_timeout=NULL;
  }

  layer_set_hidden(text_layer_get_layer(h_layer), isShow);
  layer_set_hidden(text_layer_get_layer(m_layer), isShow);
  layer_set_hidden(text_layer_get_layer(x_date_layer), !isShow);
  
}
static void wrist_flick_handler(AccelAxisType axis, int32_t direction) {
  if (axis == 1 && !shake_timeout) {
    show_extra((void *)true);
    shake_timeout = app_timer_register (STATUS_LINE_TIMEOUT, show_extra, (AppTimerCallback)false);
  }
  /*
  else if (axis == 1 && shake_timeout && !weather_force_update && bt_connect_toggle) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "SHAKE it like a Polaroid picture");
    app_message_outbox_send();
    update_time("loading...");
    weather_force_update = true;
    app_timer_reschedule(shake_timeout, STATUS_LINE_TIMEOUT);
  }
  */
}

static void update_time(char *load_status) {
  time_t temp = time(NULL); 
  struct tm *current_time = localtime(&temp);

  static char hour[] = "00";
  static char minute[] = "00";
  static char date[] = "wednesday 31";
  static char stat[] = "¦: 100%%         §: ERR\nwednesday 31\n\n\n\n\n100oC\nisolated thundershowers";
  static char weat[] = "100oC\nisolated thundershowers";
  //static char weat[] = "100oC\ncloudy";
  static uint8_t m;
  BatteryChargeState charge_state = battery_state_service_peek();
  
  // HOUR
  if(clock_is_24h_style() == true) { 
    strftime(hour, sizeof(hour), "%H", current_time);
  } else {
    strftime(hour, sizeof(hour), "%I", current_time);
  }
  if(hour[0] == '0'){
    for(int i = 1; i < 3; i++){ //get rid of that leading 0
      hour[i - 1] = hour[i];      
    }
  }
  text_layer_set_text(h_layer, hour);
  
  //MINUTE
  strftime(minute, sizeof(minute), "%M", current_time);
  m = atoi(minute);

  text_layer_set_text_alignment(m_layer, GTextAlignmentLeft);
  
  if      (m>=59 && m<2 ) { m_x =  57; m_y =   3;}
  else if (m>=2  && m<4 ) { m_x =  82; m_y =   3;}
  else if (m>=4  && m<7 ) { m_x = 107; m_y =   3;}
  else if (m>=7  && m<9 ) { m_x = 107; m_y =  21;}
  else if (m>=9  && m<12) { m_x = 107; m_y =  39; text_layer_set_text_alignment(m_layer, GTextAlignmentRight);}
  else if (m>=12 && m<14) { m_x = 116; m_y =  57;}
  else if (m>=14 && m<17) { m_x = 116; m_y =  73;}
  else if (m>=17 && m<19) { m_x = 116; m_y =  93;}
  else if (m>=19 && m<22) { m_x = 107; m_y = 111; text_layer_set_text_alignment(m_layer, GTextAlignmentRight);}
  else if (m>=22 && m<24) { m_x = 107; m_y = 129;}
  else if (m>=24 && m<27) { m_x = 107; m_y = 145;}
  else if (m>=27 && m<29) { m_x =  82; m_y = 145;}
  else if (m>=29 && m<32) { m_x =  57; m_y = 145;}
  else if (m>=32 && m<34) { m_x =  28; m_y = 145;}
  else if (m>=34 && m<37) { m_x =   4; m_y = 145;}
  else if (m>=37 && m<39) { m_x =   4; m_y = 129;}
  else if (m>=39 && m<42) { m_x =   4; m_y = 111;}
  else if (m>=42 && m<44) { m_x =   4; m_y =  93;}
  else if (m>=44 && m<47) { m_x =   4; m_y =  73;}
  else if (m>=47 && m<49) { m_x =   4; m_y =  57;}
  else if (m>=49 && m<52) { m_x =   4; m_y =  39;}
  else if (m>=52 && m<54) { m_x =   4; m_y =  21;}
  else if (m>=54 && m<57) { m_x =   4; m_y =   3;}
  else if (m>=57 && m<59) { m_x =  28; m_y =   4;}
  
  text_layer_set_text(m_layer, minute);
  layer_set_frame(text_layer_get_layer(m_layer), GRect(m_x, m_y, M_LAYER_W, M_LAYER_H));
  layer_mark_dirty(text_layer_get_layer(m_layer));

  
  //X_DATE
  strftime(date, sizeof(date), "%A %e", current_time);
  snprintf(stat, sizeof(stat), "¦: %d%%  §: ERR\n%s\n\n%s", charge_state.charge_percent, date, weat);
  text_layer_set_text(x_date_layer, stat);
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed){
  update_time("");
}

static void main_window_load(Window *window) {
  h_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BOLD_32));
  m_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SLIM_16));
  x_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_NORMAL_16));

  h_layer = text_layer_create(GRect(7, 59, 144, 45));
  text_layer_set_background_color(h_layer, GColorClear);
  text_layer_set_text_color(h_layer, GColorWhite);
  text_layer_set_font(h_layer, h_font);
  text_layer_set_text_alignment(h_layer, GTextAlignmentCenter);

  m_layer = text_layer_create(GRect(m_x, m_y, M_LAYER_W, M_LAYER_H));
  text_layer_set_background_color(m_layer, GColorClear);
  text_layer_set_text_color(m_layer, GColorWhite);
  text_layer_set_font(m_layer, m_font);
  text_layer_set_text_alignment(m_layer, GTextAlignmentCenter);

  x_date_layer = text_layer_create(GRect(0, 42, 144, 168));
  text_layer_set_background_color(x_date_layer, GColorClear);
  text_layer_set_text_color(x_date_layer, GColorWhite);
  text_layer_set_font(x_date_layer, x_font);
  text_layer_set_text_alignment(x_date_layer, GTextAlignmentCenter);
  text_layer_set_overflow_mode(x_date_layer, GTextOverflowModeWordWrap);

  // Add them as child layers to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(h_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(m_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(x_date_layer));
  layer_set_hidden(text_layer_get_layer(x_date_layer), true);
}


static void main_window_unload(Window *window) {
  text_layer_destroy(h_layer);
  text_layer_destroy(m_layer);
  text_layer_destroy(x_date_layer);
  fonts_unload_custom_font(h_font);
  fonts_unload_custom_font(m_font);
  fonts_unload_custom_font(x_font);
}

void handle_init(void) {
  
  s_main_window = window_create();
  window_set_fullscreen(s_main_window, true);
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
          .load = main_window_load,
          .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, time_handler);
  accel_tap_service_subscribe(wrist_flick_handler);

  update_time("");
}

void handle_deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}