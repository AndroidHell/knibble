#include <pebble.h>

#define COUNTER_LIMIT 3

static Window *counter_window;
TextLayer *top_counter;
TextLayer *top_counter_label;
TextLayer *bot_counter;
TextLayer *bot_counter_label;
static ActionBarLayer *init_action_bar;
static StatusBarLayer *s_status_bar;
static GBitmap *icon_plus, *icon_menu;
static ActionMenu *init_action_menu;
static ActionMenuLevel *action_menu_root;

// Storage versioning
const uint32_t storage_version_key = 0;
const int current_storage_version = 1;

const uint32_t last_counter_key = 1;

static int current_counter;

// Storage structure
struct Counter {
  int row_count;
  int row_repeat;
  int row_overflow;
  bool init;
};



struct Counter counters_container[COUNTER_LIMIT]; // Artificially limit these counters at 3

const uint32_t counters_container_key = 2;

int read_saved_data() {
  int counter_position = 0;
  // Load in the last counter
  if(persist_exists(1)) {
    persist_read_data(last_counter_key, &counter_position, sizeof(counter_position));
  }
  if(persist_exists(2)) {
    for(int i = 0; i < COUNTER_LIMIT; i++) {
      persist_read_data(counters_container_key, &counters_container[i], sizeof(counters_container[i]));
    }
  }
  else {
    for(int i = 0; i < COUNTER_LIMIT; i++) {
      counters_container[i].row_count = 0;
      counters_container[i].row_repeat = 0;
      counters_container[i].row_overflow = -1;
      counters_container[i].init = false;
    }
  }
  
  return counter_position;
}

void write_saved_data() {
  persist_write_data(counters_container_key, &counters_container, sizeof(counters_container));
}

void update_counter_text() {
  static char row_text[50];
  snprintf(row_text, sizeof(row_text), "%d", counters_container[current_counter].row_count);
  text_layer_set_text(top_counter, row_text);
  
  static char repeat_text[50];
  snprintf(repeat_text, sizeof(repeat_text), "%d", counters_container[current_counter].row_repeat);
  text_layer_set_text(bot_counter, repeat_text);
}

static void reset_project_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  counters_container[current_counter].row_count = 0;
  counters_container[current_counter].row_repeat = 0;
  update_counter_text();
}

void menu_handler(ClickRecognizerRef recognizer, void *context) {
  action_menu_root = action_menu_level_create(3);
  action_menu_level_add_action(action_menu_root, "Reset Project", reset_project_callback, NULL);
  action_menu_level_add_action(action_menu_root, "Switch Project", reset_project_callback, NULL);
  
  ActionMenuConfig config = (ActionMenuConfig) {
    .root_level = action_menu_root,
    .colors = {
      .background = GColorBlack,
      .foreground = GColorWhite,
    },
    .align = ActionMenuAlignCenter
  };
  
  init_action_menu = action_menu_open(&config);
}

void row_increment_handler(ClickRecognizerRef recognizer, void *context) {
  if(counters_container[current_counter].row_count < 998) {
    counters_container[current_counter].row_count++;
  }
  if(counters_container[current_counter].row_count == counters_container[current_counter].row_overflow) {
    counters_container[current_counter].row_count = 0;
    if(counters_container[current_counter].row_repeat < 998) {
      counters_container[current_counter].row_repeat++;
    }
  }
  update_counter_text();
}

void row_decrement_handler(ClickRecognizerRef recognizer, void *context) {
  if(counters_container[current_counter].row_count > 0) {
    counters_container[current_counter].row_count--;
  }
  update_counter_text();
}
void repeat_increment_handler(ClickRecognizerRef recognizer, void *context) {
  if(counters_container[current_counter].row_repeat < 998) {
    counters_container[current_counter].row_repeat++;
  }
  update_counter_text();
}

void repeat_decrement_handler(ClickRecognizerRef recognizer, void *context) {
  if(counters_container[current_counter].row_repeat > 0) {
    counters_container[current_counter].row_repeat--;
  }
  update_counter_text();
}

void click_config_provider(void *context) {
   window_single_click_subscribe(BUTTON_ID_SELECT, menu_handler);
   window_single_click_subscribe(BUTTON_ID_UP, row_increment_handler);
   window_single_click_subscribe(BUTTON_ID_DOWN, repeat_increment_handler);
   window_long_click_subscribe(BUTTON_ID_UP, 500, row_decrement_handler, NULL);
   window_long_click_subscribe(BUTTON_ID_DOWN, 500, repeat_decrement_handler, NULL);
}

void handle_init(void) {
  
  current_counter = read_saved_data();
  
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
  
  // Action bar and icon setup
  icon_plus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_PLUS);
  icon_menu = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_MENU);
  init_action_bar = action_bar_layer_create(); // Create the ActionBarLayer for entry window
  action_bar_layer_set_background_color(init_action_bar, GColorOrange);
  action_bar_layer_set_click_config_provider(init_action_bar, click_config_provider); // Attach click provider to action bar
  action_bar_layer_set_icon(init_action_bar, BUTTON_ID_UP, icon_plus);
  action_bar_layer_set_icon(init_action_bar, BUTTON_ID_DOWN, icon_plus);
  action_bar_layer_set_icon(init_action_bar, BUTTON_ID_SELECT, icon_menu);

  // Construct the top counter
  top_counter = text_layer_create(GRect(10, 35, 90, 50));
  text_layer_set_font(top_counter, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_background_color(top_counter, GColorClear);
  text_layer_set_text_alignment(top_counter, GTextAlignmentRight);
  layer_add_child(root_layer, text_layer_get_layer(top_counter));
  
  static char rows_text[50];
  snprintf(rows_text, sizeof(rows_text), "%d", counters_container[current_counter].row_count);
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
  snprintf(repeat_text, sizeof(repeat_text), "%d", counters_container[current_counter].row_repeat);
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
  gbitmap_destroy(icon_plus);
  gbitmap_destroy(icon_menu);
  write_saved_data();
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}