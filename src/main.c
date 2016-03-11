#include <pebble.h>

static Window *counter_window;
TextLayer *top_counter;
TextLayer *top_counter_label;
TextLayer *bot_counter;
TextLayer *bot_counter_label;
static ActionBarLayer *init_action_bar;
static StatusBarLayer *s_status_bar;

// Storage versioning
const uint32_t storage_version_key = 0;
const int current_storage_version = 1;

struct Counter {
  int row_count;
  int row_repeat;
  int row_overflow;
};

struct Counter counter_a_container;
struct Counter counter_b_container;
struct Counter counter_c_container;

const uint32_t counter_a_key = 1;
const uint32_t counter_b_key = 2;
const uint32_t counter_c_key = 3;

void menu_handler(ClickRecognizerRef recognizer, void *context) {
  
}

void click_config_provider(void *context) {
   ButtonId menu_button = BUTTON_ID_SELECT;  // The Select button
   window_single_click_subscribe(menu_button, menu_handler);
}

void handle_init(void) {
  
  if(persist_exists(1)) {
  persist_read_data(counter_a_key, &counter_a_container, sizeof(counter_a_container));
  }
  else {
    counter_a_container.row_count = 0;
    counter_a_container.row_repeat = 0;
    counter_a_container.row_overflow = -1;
  }
  
  
  if(persist_exists(2)) {
    persist_read_data(counter_b_key, &counter_b_container, sizeof(counter_b_container));
  }
  else {
    counter_b_container.row_count = 0;
    counter_b_container.row_repeat = 0;
    counter_b_container.row_overflow = -1;
  }
  
  
  if(persist_exists(3)) {
    persist_read_data(counter_c_key, &counter_c_container, sizeof(counter_c_container));
  }
  else {
    counter_c_container.row_count = 0;
    counter_c_container.row_repeat = 0;
    counter_c_container.row_overflow = -1;
  }
  
  counter_window = window_create(); // Create the entry window
  Layer *root_layer = window_get_root_layer(counter_window);
  
  
  // Create the StatusBarLayer
  s_status_bar = status_bar_layer_create();
  
  // Set properties
  status_bar_layer_set_colors(s_status_bar, GColorRajah, GColorBlack);
  status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);
  
  int16_t width = layer_get_bounds(root_layer).size.w - ACTION_BAR_WIDTH;
  GRect frame = GRect(0, 0, width, STATUS_BAR_LAYER_HEIGHT);
  layer_set_frame(status_bar_layer_get_layer(s_status_bar), frame);
  layer_add_child(root_layer, status_bar_layer_get_layer(s_status_bar));
  
  // Add to Window
  layer_add_child(root_layer, status_bar_layer_get_layer(s_status_bar));

  
  window_set_background_color(counter_window, GColorRajah);
  
  init_action_bar = action_bar_layer_create(); // Create the ActionBarLayer for entry window
  action_bar_layer_set_background_color(init_action_bar, GColorOrange);
  action_bar_layer_set_click_config_provider(init_action_bar, click_config_provider); // Attach click provider to action bar


  // Construct the top counter
  top_counter = text_layer_create(GRect(10, 35, 90, 50));
  text_layer_set_font(top_counter, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_background_color(top_counter, GColorClear);
  text_layer_set_text_alignment(top_counter, GTextAlignmentRight);
  layer_add_child(root_layer, text_layer_get_layer(top_counter));
  
  static char rows_text[50];
  snprintf(rows_text, sizeof(rows_text), "%d", counter_a_container.row_count);
  text_layer_set_text(top_counter, rows_text);
  
  top_counter_label = text_layer_create(GRect(10, 15, 90, 30));
  text_layer_set_font(top_counter_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(top_counter_label, "ROW COUNT");
  text_layer_set_text_alignment(top_counter_label, GTextAlignmentRight);
  text_layer_set_background_color(top_counter_label, GColorClear);
  layer_add_child(root_layer, text_layer_get_layer(top_counter_label));
  
  
  // Construct the bottom counter
  bot_counter = text_layer_create(GRect(10, 110, 90, 50));
  text_layer_set_font(bot_counter, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_background_color(bot_counter, GColorClear);
  text_layer_set_text_alignment(bot_counter, GTextAlignmentRight);
  layer_add_child(root_layer, text_layer_get_layer(bot_counter));
    
  static char repeat_text[50];
  snprintf(repeat_text, sizeof(repeat_text), "%d", counter_a_container.row_repeat);
  text_layer_set_text(bot_counter, repeat_text);
  
  bot_counter_label = text_layer_create(GRect(10, 90, 90, 30));
  text_layer_set_font(bot_counter_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(bot_counter_label, "REPEAT");
  text_layer_set_text_alignment(bot_counter_label, GTextAlignmentRight);
  text_layer_set_background_color(bot_counter_label, GColorClear);
  layer_add_child(root_layer, text_layer_get_layer(bot_counter_label));
  
  action_bar_layer_add_to_window(init_action_bar, counter_window);
  window_stack_push(counter_window, true); // Push entry window to stack, animate
}

void handle_deinit(void) {
  status_bar_layer_destroy(s_status_bar);
  text_layer_destroy(top_counter);
  text_layer_destroy(top_counter_label);
  text_layer_destroy(bot_counter);
  text_layer_destroy(bot_counter_label);
  window_destroy(counter_window);
  action_bar_layer_destroy(init_action_bar);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
