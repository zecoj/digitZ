#include <pebble.h>

#define M_LAYER_W 33
#define M_LAYER_H 20
#define STATUS_LINE_TIMEOUT 5000
  
#define  CONF_BLUETOOTH             1
#define  CONF_WEATHER               2
#define  WEATHER_ICON_KEY           3
#define  WEATHER_TEMPERATURE_C_KEY  4

static Window *s_main_window;
static TextLayer *h_layer;
static TextLayer *m_layer;
static TextLayer *x_date_layer;
static uint8_t m_x=144, m_y=168;

static AppTimer *shake_timeout = NULL;

static char bluetooth_str[]= "  §: OK ";
static bool bluetooth = true;
static bool bluetooth_old = true;
static uint8_t weather = 15;
static bool weather_force_update = false;

bool bt_connect_toggle;

static AppSync sync;
static uint8_t sync_buffer[128];
#define MyTupletCString(_key, _cstring) ((const Tuplet) { .type = TUPLE_CSTRING, .key = _key, .cstring = { .data = _cstring, .length = strlen(_cstring) + 1 }})

static char weather_str[] = "no data, updating...   ";
static char temp_c_str[] = "     ";

static GFont h_font;
static GFont m_font;
static GFont x_font;

static void update_time(char *load_status, bool update_main, bool update_x);


void update_bluetooth_str () {
  if (bluetooth) {
    if (bt_connect_toggle) { snprintf(bluetooth_str, sizeof(bluetooth_str), "  §: OK "); }
    else { snprintf(bluetooth_str, sizeof(bluetooth_str), "  §: ERR"); }
  }
  else { strcpy(bluetooth_str,""); }
}
void bluetooth_connection_handler(bool connected) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "bluetooth_connection_handler called");
  if(bluetooth){
    if (!bt_connect_toggle && connected) {
      bt_connect_toggle = true;
      vibes_short_pulse();
    }
    if (bt_connect_toggle && !connected) {
      bt_connect_toggle = false;
      vibes_short_pulse();
    }
    //layer_set_hidden(text_layer_get_layer(x_blue_layer), bt_connect_toggle);
  }
  //else {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "I'm still here, just ignoring bluetooth events");
    //layer_set_hidden(text_layer_get_layer(x_blue_layer), true);
  //}
  update_bluetooth_str();
}

void show_extra (void *isShow) {
  //if hiding
  if (!isShow) {
    isShow=false;
    shake_timeout=NULL;
    update_time("", true, false);
    weather_force_update=false;
  }
  else {
    update_time("", false, true);
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
  //disabled due to potential hit on battery life
  /*
  else if (axis == 1 && shake_timeout && weather && !weather_force_update && bt_connect_toggle) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "SHAKE it like a Polaroid picture");
    app_message_outbox_send();
    update_time("loading...", false, true);
    weather_force_update = true;
    app_timer_reschedule(shake_timeout, STATUS_LINE_TIMEOUT);
  }
  */
}

static void update_time(char *load_status, bool update_main, bool update_x) {
  time_t temp = time(NULL);
  struct tm *current_time = localtime(&temp);

  static char hour[] = "00";
  static char minute[] = "00";
  static char date[] = "wednesday 31";
  static char stat[80];
  static char weat[] = "100oC\nisolated thundershowers";
  static uint8_t m;
  BatteryChargeState charge_state;
  
  if (update_main) {
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

    if      (m>=59 || m<2 ) { m_x =  57; m_y =   3;}
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
  }
  if (update_x) {
    //X_DATE 
    if (weather && bt_connect_toggle && strcmp(weather_str, "no data") ) {
      if(strcmp(load_status, "")){
        snprintf(weat, sizeof(weat), "%s", load_status);
      }
      else {
        snprintf(weat, sizeof(weat), "%s\n%s", temp_c_str, weather_str);
      }
    }
    else {
      strcpy(weat, "");
    }
    charge_state = battery_state_service_peek();
    update_bluetooth_str();
    strftime(date, sizeof(date), "%A %e", current_time);
    snprintf(stat, sizeof(stat), "¦: %d%%%s\n%s\n\n%s", charge_state.charge_percent, bluetooth_str, date, weat);
    text_layer_set_text(x_date_layer, stat);
  }
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time("", true, false);
  if(weather && tick_time->tm_min % weather == 0) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "MINUTE_TICK I'm supposed to check the weather now");
    app_message_outbox_send();
  }
}

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %i - %s", app_message_error, translate_error(app_message_error));
    strcpy(weather_str, "no data");
    strcpy(temp_c_str, "01234");
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  // process the first and subsequent update
  switch (key) {
    case CONF_BLUETOOTH:
      bluetooth_old = bluetooth;
      bluetooth = new_tuple->value->uint8 == 1;
      persist_write_bool(CONF_BLUETOOTH, bluetooth);
      if (bluetooth && !bluetooth_old) {
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "re-subscribing");
        bluetooth_connection_service_subscribe(bluetooth_connection_handler);
        bt_connect_toggle = bluetooth_connection_service_peek();
      }
      if (!bluetooth && bluetooth_old) {
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "unsubscribing");
        bluetooth_connection_service_unsubscribe();
      }
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Set bluetooth: %u", bluetooth ? 1 : 0);
      break;

    case CONF_WEATHER:
      weather = new_tuple->value->uint8;
      persist_write_int(CONF_WEATHER, weather);
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Set weather: %u", weather);
      break;

    case WEATHER_ICON_KEY:
      strcpy(weather_str, new_tuple->value->cstring);
      persist_write_string(WEATHER_ICON_KEY, weather_str);
      break;

    case WEATHER_TEMPERATURE_C_KEY:
      strcpy(temp_c_str, new_tuple->value->cstring);
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Set temp_c_str: %s", temp_c_str);
      persist_write_string(WEATHER_TEMPERATURE_C_KEY, temp_c_str);
      if(weather_force_update) {
        update_time("", false, true);
      }
      break;
  }
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

  // prepare the initial values of your data
  Tuplet initial_values[] = {
      TupletInteger(CONF_BLUETOOTH, (uint8_t) bluetooth ? 1 : 0),
      TupletInteger(CONF_WEATHER,   (uint8_t) weather),
      MyTupletCString(WEATHER_ICON_KEY, weather_str),
      MyTupletCString(WEATHER_TEMPERATURE_C_KEY, temp_c_str)
  };
  // initialize the syncronization
  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);
  //send_cmd();
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
  // Load settings from persistent storage
  if (persist_exists(CONF_BLUETOOTH))
  {
    bluetooth = persist_read_bool(CONF_BLUETOOTH);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Read CONF_BLUETOOTH from store: %u", bluetooth ? 1 : 0);
  }
  if (persist_exists(CONF_WEATHER))
  {
    weather = persist_read_int(CONF_WEATHER);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Read CONF_WEATHER from store: %u", weather);
  }
  if (persist_exists(WEATHER_ICON_KEY))
  {
    persist_read_string(WEATHER_ICON_KEY, weather_str, sizeof(weather_str));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Read WEATHER_ICON_KEY from store: %s", weather_str);
  }
  if (persist_exists(WEATHER_TEMPERATURE_C_KEY))
  {
    persist_read_string(WEATHER_TEMPERATURE_C_KEY, temp_c_str, sizeof(temp_c_str));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Read WEATHER_TEMPERATURE_C_KEY from store: %s", temp_c_str);
  }

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
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);

  if (bluetooth) {
    bluetooth_connection_service_subscribe(bluetooth_connection_handler);
    bt_connect_toggle = bluetooth_connection_service_peek();
  }
  update_bluetooth_str();
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  update_time("", true, false);
}

void handle_deinit(void) {
  window_destroy(s_main_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}