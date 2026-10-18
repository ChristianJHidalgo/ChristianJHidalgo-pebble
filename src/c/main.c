#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

// bluetooth layers
static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;

// Adding the GFonts 
static GFont s_time_font;
static GFont s_date_font;

// Battery variables to store the current charge level 
static Layer *s_battery_layer;
static int s_battery_level;

// Callback that stores the new level and triggers a redraw
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;

  // Update the meter -battery-
  layer_mark_dirty(s_battery_layer);
}

//Bluetooth callback -bluetooth-
static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if (!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

  // Battery Customization Drawer
static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar (inside the border)
  int bar_width = ((s_battery_level * (bounds.size.w - 4)) / 100);

  // Draw the border
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_round_rect(ctx, bounds, 2);

  // Choose color based on battery level
  GColor bar_color;
  if (s_battery_level <= 20) {
    bar_color = PBL_IF_COLOR_ELSE(GColorRed, GColorWhite);
  } else if (s_battery_level <= 40) {
    bar_color = PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite);
  } else {
    bar_color = PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite);
  }

  // Draw the filled bar inside the border
  graphics_context_set_fill_color(ctx, bar_color);
  graphics_fill_rect(ctx, GRect(2, 2, bar_width, bounds.size.h - 4), 1, GCornerNone);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
                                                    "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);

  // Write the current date into a buffer
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %b %d", tick_time);

  // Display the date
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  
  // Load custom fonts
s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_56));
s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_JERSEY_24)); 
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
    // Center the time + date block vertically
int date_height = 30;
int block_height = 56 + date_height;
int time_y = (bounds.size.h / 2) - (block_height / 2) - 10;
int date_y = time_y + 56; 

  // Create the time TextLayer
  s_time_layer = text_layer_create(
    GRect(0, time_y, bounds.size.w, 60));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  
  //REPLACEDtext_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  
    // For the time layer (was FONT_KEY_BITHAM_42_BOLD)
text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create the date TextLayer
  s_date_layer = text_layer_create(
    GRect(0, time_y, bounds.size.w, 30));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  
  //REPLACEDtext_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  
  // For the date layer (was FONT_KEY_GOTHIC_24_BOLD)
text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add layers to the Window -date time-
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // Create battery meter Layer - visible bar near the top
int bar_width = bounds.size.w / 2;
int bar_x = (bounds.size.w - bar_width) / 2;
int bar_y = PBL_IF_ROUND_ELSE(bounds.size.h / 8, bounds.size.h / 28);
s_battery_layer = layer_create(GRect(bar_x, bar_y, bar_width, 8));
layer_set_update_proc(s_battery_layer, battery_update_proc);

// Add to Window -battery-
layer_add_child(window_layer, s_battery_layer);
  
// Bluetooth window load -bluetooth-
  
// Create the Bluetooth icon GBitmap
s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

// Create the BitmapLayer to display the GBitmap - below the battery bar, centered
int bt_y = bar_y + 12;
s_bt_icon_layer = bitmap_layer_create(GRect((bounds.size.w - 30) / 2, bt_y, 30, 30));
bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
bitmap_layer_set_compositing_mode(s_bt_icon_layer, GCompOpSet);

// Add to Window
layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

// Show the correct state of the BT connection from the start
bluetooth_callback(connection_service_peek_pebble_app_connection());
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  
  // Unload custom fonts
fonts_unload_custom_font(s_time_font);
fonts_unload_custom_font(s_date_font);
  
  // Unload Battery Layer
  layer_destroy(s_battery_layer);
  
  // Unload Bluetooth Layer
  gbitmap_destroy(s_bt_icon_bitmap);
bitmap_layer_destroy(s_bt_icon_layer);

}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorCyan);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register for battery level updates
battery_state_service_subscribe(battery_callback);

// Ensure battery level is displayed from the start -battery-
battery_callback(battery_state_service_peek());
 
  // Register for Bluetooth connection updates -bluetooth-
connection_service_subscribe((ConnectionHandlers) {
  .pebble_app_connection_handler = bluetooth_callback
});
  
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
