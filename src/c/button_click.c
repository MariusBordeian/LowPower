#include <pebble.h>

static Window *window;
static TextLayer *time_layer;
static Layer *battery_layer;
static int s_battery_level;
static bool first = true;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  text_layer_set_text(time_layer, s_buffer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  int width = (s_battery_level * bounds.size.w) / 100;

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void click_handler(ClickRecognizerRef recognizer, void *context) {
  BatteryChargeState state = battery_state_service_peek();
  s_battery_level = state.charge_percent;
  layer_mark_dirty(battery_layer);
  
  if (first) {
    text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
    first = false;
  }
  update_time();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, click_handler);
  window_single_click_subscribe(BUTTON_ID_UP,     click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,   click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  battery_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h / 2));
  layer_set_update_proc(battery_layer, battery_update_proc);
  
  time_layer = text_layer_create(GRect(0, bounds.size.h / 2, bounds.size.w, bounds.size.h / 2));
  text_layer_set_text(time_layer, "gotta press a button");
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(time_layer, GColorFromRGB(255, 255, 255)); // full white
  text_layer_set_background_color(time_layer, GColorFromRGB(0, 0, 0)); // full black
  
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, battery_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(time_layer);
  layer_destroy(battery_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = false; //true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}