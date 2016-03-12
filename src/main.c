#include <pebble.h>

#define COUNTER_LIMIT 3

static Window *counter_window;
TextLayer *top_counter;
TextLayer *top_counter_label;
TextLayer *bot_counter;
TextLayer *bot_counter_label;
TextLayer *project_indicator;
static Layer *indicator_box_canvas;
static ActionBarLayer *init_action_bar;
static StatusBarLayer *s_status_bar;
static GBitmap *icon_plus, *icon_menu;
static ActionMenu *init_action_menu;
static ActionMenuLevel *action_menu_root;
static ActionMenuLevel *action_menu_project_select_layer;
static NumberWindow *input_number_window;

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
  // Load in the last project
  if(persist_exists(1)) {
    persist_read_data(last_counter_key, &counter_position, sizeof(counter_position));
  }
  if(persist_exists(2)) {
    persist_read_data(counters_container_key, &counters_container, sizeof(counters_container));
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
  persist_write_data(last_counter_key, &current_counter, sizeof(current_counter));
}

void update_counter_text() {
  static char row_text[50];
  snprintf(row_text, sizeof(row_text), "%d", counters_container[current_counter].row_count);
  text_layer_set_text(top_counter, row_text);
  
  static char repeat_text[50];
  snprintf(repeat_text, sizeof(repeat_text), "%d", counters_container[current_counter].row_repeat);
  text_layer_set_text(bot_counter, repeat_text);
}

void update_indicator_text(){
  static char indicator_text[2];
  switch(current_counter) {
    case 0:
      strcpy(indicator_text, "A");
      break;
    case 1:
      strcpy(indicator_text, "B");
      break;
    case 2:
      strcpy(indicator_text, "C");
      break;
  }
  text_layer_set_text(project_indicator, indicator_text);
}

void initialize_action_menu(); // Forward declaration

static void set_linked_value(NumberWindow *number_window, void *context) {
  counters_container[current_counter].row_overflow = number_window_get_value(number_window);
  // Fix if counter has been initialized
  if(counters_container[current_counter].row_count > counters_container[current_counter].row_overflow) {
    counters_container[current_counter].row_repeat += counters_container[current_counter].row_count / counters_container[current_counter].row_overflow;
    counters_container[current_counter].row_count = counters_container[current_counter].row_count % counters_container[current_counter].row_overflow;
  }
  else if(counters_container[current_counter].row_count == counters_container[current_counter].row_overflow) {
    counters_container[current_counter].row_count = 0;
    counters_container[current_counter].row_repeat++;
  }
  action_menu_hierarchy_destroy(action_menu_root, NULL, NULL);
  initialize_action_menu();
  update_counter_text();
  window_stack_pop(number_window_get_window(input_number_window));
};

static void link_project_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  window_stack_push(number_window_get_window(input_number_window), true);
}

static void unlink_project_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  counters_container[current_counter].row_overflow = -1;
  action_menu_hierarchy_destroy(action_menu_root, NULL, NULL);
  initialize_action_menu();
}

static void reset_project_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  counters_container[current_counter].row_count = 0;
  counters_container[current_counter].row_repeat = 0;
  counters_container[current_counter].row_overflow = -1;
  action_menu_hierarchy_destroy(action_menu_root, NULL, NULL);
  initialize_action_menu();
  update_counter_text();
}

static void switch_project_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
  current_counter = (int)action_menu_item_get_action_data(action);
  action_menu_hierarchy_destroy(action_menu_root, NULL, NULL);
  initialize_action_menu();
  update_counter_text();
  update_indicator_text();
}


void initialize_action_menu() {
  action_menu_root = action_menu_level_create(4);
  action_menu_project_select_layer = action_menu_level_create(COUNTER_LIMIT);
  if(counters_container[current_counter].row_overflow != -1) {
    action_menu_level_add_action(action_menu_root, "Unlink counters", unlink_project_callback, NULL);
  }
  else {
    action_menu_level_add_action(action_menu_root, "Link counters...", link_project_callback, NULL);
  }
  action_menu_level_add_child(action_menu_root, action_menu_project_select_layer, "Switch Project");
  action_menu_level_add_action(action_menu_project_select_layer, "Project A", switch_project_callback, (int*)0);
  action_menu_level_add_action(action_menu_project_select_layer, "Project B", switch_project_callback, (int*)1);
  action_menu_level_add_action(action_menu_project_select_layer, "Project C", switch_project_callback, (int*)2);
  
  action_menu_level_add_action(action_menu_root, "Reset Project", reset_project_callback, NULL);
}

void menu_handler(ClickRecognizerRef recognizer, void *context) {
  
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

static void indicator_update_proc(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 3, GCornersAll);
  //graphics_draw_round_rect(ctx, layer_get_bounds(layer), 3);
}


void handle_init(void) {
  
  current_counter = read_saved_data();
  
  counter_window = window_create(); // Create the entry window
  Layer *root_layer = window_get_root_layer(counter_window);
  
  // Initialize action menu
  initialize_action_menu();
  
  // Create the number picker window
  
  NumberWindowCallbacks number_window_config = {
    .incremented = NULL,
    .decremented = NULL,
    .selected = set_linked_value
  };
  
  static char number_window_label[] = "Rows per repeat:";
  input_number_window = number_window_create(number_window_label, number_window_config, NULL);
  number_window_set_min(input_number_window, 1);
  number_window_set_max(input_number_window, 50);

  
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

  // Construct the indicator
  // set up the box
  GRect indicator_bounds = GRect(4, 147, 17, 17);
  indicator_box_canvas = layer_create(indicator_bounds);
  layer_set_update_proc(indicator_box_canvas, indicator_update_proc);
  
  project_indicator = text_layer_create(GRect(5, 146, 15, 17));
  text_layer_set_background_color(project_indicator, GColorClear);
  
  
  text_layer_set_font(project_indicator, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(project_indicator, GTextAlignmentCenter);
  text_layer_set_text_color(project_indicator, GColorWhite);
  layer_add_child(root_layer, text_layer_get_layer(project_indicator));
  
  layer_add_child(root_layer, indicator_box_canvas);
  layer_add_child(root_layer, text_layer_get_layer(project_indicator));
  update_indicator_text();
  
  // Construct the top counter
  top_counter = text_layer_create(GRect(10, 35, 90, 50));
  text_layer_set_font(top_counter, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_background_color(top_counter, GColorClear);
  text_layer_set_text_alignment(top_counter, GTextAlignmentRight);
  layer_add_child(root_layer, text_layer_get_layer(top_counter));
  
  // Set the counter label
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
  
  // Set the counter label
  bot_counter_label = text_layer_create(GRect(10, 90, 90, 30));
  text_layer_set_font(bot_counter_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text(bot_counter_label, "REPEAT");
  text_layer_set_text_alignment(bot_counter_label, GTextAlignmentRight);
  text_layer_set_background_color(bot_counter_label, GColorClear);
  layer_add_child(root_layer, text_layer_get_layer(bot_counter_label));
  
  update_counter_text();
  action_bar_layer_add_to_window(init_action_bar, counter_window);
  window_stack_push(counter_window, true); // Push entry window to stack, animate
}

void handle_deinit(void) {
  status_bar_layer_destroy(s_status_bar);
  text_layer_destroy(top_counter);
  text_layer_destroy(top_counter_label);
  text_layer_destroy(bot_counter);
  text_layer_destroy(bot_counter_label);
  text_layer_destroy(project_indicator);
  layer_destroy(indicator_box_canvas);
  window_destroy(counter_window);
  action_bar_layer_destroy(init_action_bar);
  gbitmap_destroy(icon_plus);
  gbitmap_destroy(icon_menu);
  number_window_destroy(input_number_window);
  write_saved_data();
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}