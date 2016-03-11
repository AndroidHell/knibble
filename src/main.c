#include <pebble.h>

static Window *counter_window;
TextLayer *top_counter;
TextLayer *top_counter_label;
TextLayer *bot_counter;
TextLayer *bot_counter_label;
static ActionBarLayer *init_action_bar;

void menu_handler(ClickRecognizerRef recognizer, void *context) {
  
}

void click_config_provider(void *context) {
   ButtonId menu_button = BUTTON_ID_SELECT;  // The Select button
   window_single_click_subscribe(menu_button, menu_handler);
}

void handle_init(void) {
  
  counter_window = window_create(); // Create the entry window
  Layer *root_layer = window_get_root_layer(counter_window);
  
  window_set_background_color(counter_window, GColorRajah);
  
  init_action_bar = action_bar_layer_create(); // Create the ActionBarLayer for entry window
  action_bar_layer_set_background_color(init_action_bar, GColorOrange);
  action_bar_layer_set_click_config_provider(init_action_bar, click_config_provider); // Attach click provider to action bar


  // Construct the top counter
  top_counter = text_layer_create(GRect(10, 30, 90, 50));
  text_layer_set_font(top_counter, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
 // text_layer_set_background_color(top_counter, GColorClear);
  text_layer_set_text_alignment(top_counter, GTextAlignmentRight);
  
  text_layer_set_text(top_counter, "10");
  
  
  // Construct the bottom counter
  bot_counter = text_layer_create(GRect(10, 110, 90, 50));
  text_layer_set_font(bot_counter, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
// text_layer_set_background_color(bot_counter, GColorClear);
  text_layer_set_text_alignment(bot_counter, GTextAlignmentRight);
  
  text_layer_set_text(bot_counter, "15");
  
  layer_add_child(root_layer, text_layer_get_layer(top_counter));
  layer_add_child(root_layer, text_layer_get_layer(bot_counter));
  
  
  
  action_bar_layer_add_to_window(init_action_bar, counter_window);
  window_stack_push(counter_window, true); // Push entry window to stack, animate
}

void handle_deinit(void) {
  text_layer_destroy(top_counter);
  text_layer_destroy(bot_counter);
  window_destroy(counter_window);
  action_bar_layer_destroy(init_action_bar);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
