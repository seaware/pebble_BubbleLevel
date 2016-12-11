/// This implements a Bubble Level WatchApp
/// 
/// @copyright  Copyright (c) 2013 Frank Canova. All rights reserved.
///

// These keys match the JS array in settings for communicating with the phone
#define BUBBLE_KEY_HIGHLIGHT 1
#define BUBBLE_KEY_ORIENTATION 2
	
#include <pebble.h>

Window *window;
Layer *bubble_layer;
int bubble_x = 0;
int bubble_y = 0;
int bubble_z = 0;
int bubble_x_target;
int bubble_y_target;
int bubble_z_target;
int mode = 0;

#define LINESPACING 20
#define COLSPACING 14
#define TARGET_RADIUS 50
#define BUBBLE_RADIUS 20
#define ACCEL_PIXEL_RATIO 8
#define MAX_MODES 2

static void bubble_layer_update_callback(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    GRect frame = GRect(COLSPACING, bounds.size.h-LINESPACING, bounds.size.w-COLSPACING*2, LINESPACING);
	char cbuf[20];
    graphics_context_set_text_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorWhite);
	
	if (bubble_x_target != bubble_x) bubble_x += (bubble_x_target - bubble_x)/2;
	if (bubble_y_target != bubble_y) bubble_y += (bubble_y_target - bubble_y)/2;
	if (bubble_z_target != bubble_z) bubble_z += bubble_z_target - bubble_z;
	
	if (mode == 0 || mode == 1)
	{
		int x = (bubble_x*90/100)%10;
		if (x<0) x = -x;
		snprintf(cbuf, ARRAY_LENGTH(cbuf), "X %d.%d°", (bubble_x*90/1000), x);
		graphics_draw_text(ctx, cbuf,
			fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
			frame,
			GTextOverflowModeTrailingEllipsis,
			GTextAlignmentLeft,
			NULL);
	}
	if (mode == 0 || mode == 2)
	{
		int y = (bubble_y*90/100)%10;
		if (y<0) y = -y;
		snprintf(cbuf, ARRAY_LENGTH(cbuf), "%d.%d° Y",(bubble_y*90/1000), y);
		graphics_draw_text(ctx, cbuf,
			fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
			frame,
			GTextOverflowModeTrailingEllipsis,
			GTextAlignmentRight,
			NULL);
	}

	graphics_context_set_stroke_color(ctx, GColorBlack);
	switch (mode)
	{
	case 0:					// Draw bullseye
		graphics_draw_circle(ctx, GPoint(bounds.size.w/2, bounds.size.h/2), TARGET_RADIUS);
		graphics_draw_circle(ctx, GPoint(bounds.size.w/2, bounds.size.h/2), TARGET_RADIUS/2);
		graphics_draw_line(ctx, GPoint(bounds.size.w/2, bounds.size.h/2-TARGET_RADIUS-COLSPACING),
								GPoint(bounds.size.w/2, bounds.size.h/2+TARGET_RADIUS+COLSPACING));
		graphics_draw_line(ctx, GPoint(bounds.size.w/2-TARGET_RADIUS-COLSPACING, bounds.size.h/2),
								GPoint(bounds.size.w/2+TARGET_RADIUS+COLSPACING, bounds.size.h/2));
		break;
		
	case 1:					// Draw Hz level
		graphics_draw_rect(ctx, GRect(COLSPACING, bounds.size.h/2-TARGET_RADIUS/2, bounds.size.w-COLSPACING*2, TARGET_RADIUS));
		graphics_draw_line(ctx, GPoint(bounds.size.w/2-TARGET_RADIUS/2, bounds.size.h/2+BUBBLE_RADIUS/2),
								GPoint(bounds.size.w/2-TARGET_RADIUS/2, bounds.size.h/2-BUBBLE_RADIUS/2));
		graphics_draw_line(ctx, GPoint(bounds.size.w/2+TARGET_RADIUS/2, bounds.size.h/2+BUBBLE_RADIUS/2),
								GPoint(bounds.size.w/2+TARGET_RADIUS/2, bounds.size.h/2-BUBBLE_RADIUS/2));
		bubble_y = 0;		
		break;
	case 2:					// Draw Vt level
		graphics_draw_rect(ctx, GRect(bounds.size.w/2-TARGET_RADIUS/2, COLSPACING, TARGET_RADIUS, bounds.size.h-COLSPACING*2));
		graphics_draw_line(ctx, GPoint(bounds.size.w/2-BUBBLE_RADIUS/2, bounds.size.h/2+TARGET_RADIUS/2),
								GPoint(bounds.size.w/2+BUBBLE_RADIUS/2, bounds.size.h/2+TARGET_RADIUS/2));
		graphics_draw_line(ctx, GPoint(bounds.size.w/2-BUBBLE_RADIUS/2, bounds.size.h/2-TARGET_RADIUS/2),
								GPoint(bounds.size.w/2+BUBBLE_RADIUS/2, bounds.size.h/2-TARGET_RADIUS/2));		
		bubble_x = 0;
		break;
	}
	
	// Draw bubble
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "BUBBLE_KEY_HIGHLIGHT=%d", (int) persist_read_int(BUBBLE_KEY_HIGHLIGHT));
	if (bubble_x == 0 && bubble_y == 0 && (int) persist_read_int(BUBBLE_KEY_HIGHLIGHT) > 0)
	{
		graphics_context_set_stroke_color(ctx, GColorWhite);
		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_circle(ctx, GPoint((bounds.size.w/2) + (bubble_x/ACCEL_PIXEL_RATIO), 
								 (bounds.size.h/2) + (bubble_y/ACCEL_PIXEL_RATIO)), BUBBLE_RADIUS);
	}
	else
		graphics_draw_circle(ctx, GPoint((bounds.size.w/2) + (bubble_x/ACCEL_PIXEL_RATIO), 
									 (bounds.size.h/2) + (bubble_y/ACCEL_PIXEL_RATIO)), BUBBLE_RADIUS);
	graphics_draw_line(ctx, GPoint((bounds.size.w/2) - BUBBLE_RADIUS + (bubble_x/ACCEL_PIXEL_RATIO), (bounds.size.h/2) + (bubble_y/ACCEL_PIXEL_RATIO)),
					   		GPoint((bounds.size.w/2) + BUBBLE_RADIUS + (bubble_x/ACCEL_PIXEL_RATIO), (bounds.size.h/2) + (bubble_y/ACCEL_PIXEL_RATIO)));
	graphics_draw_line(ctx, GPoint((bounds.size.w/2) + (bubble_x/ACCEL_PIXEL_RATIO), (bounds.size.h/2) - BUBBLE_RADIUS + (bubble_y/ACCEL_PIXEL_RATIO)),
					   		GPoint((bounds.size.w/2) + (bubble_x/ACCEL_PIXEL_RATIO), (bounds.size.h/2) + BUBBLE_RADIUS + (bubble_y/ACCEL_PIXEL_RATIO)));
 }


// When the phone sends us data, save the settings in persistant storage
static void in_received_handler(DictionaryIterator *iter, void *context) {
  Tuple *highlight_tuple = dict_find(iter, BUBBLE_KEY_HIGHLIGHT);
  Tuple *orientation_tuple = dict_find(iter, BUBBLE_KEY_ORIENTATION);
	
  if (highlight_tuple) {
    persist_write_int(BUBBLE_KEY_HIGHLIGHT, highlight_tuple->value->uint8);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Pebble setting highlight=%d",highlight_tuple->value->uint8);
  }
  if (orientation_tuple) {
	mode = orientation_tuple->value->uint8 % 3;
    persist_write_int(BUBBLE_KEY_ORIENTATION, mode);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Pebble setting orientation=%d",mode);
  }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Dropped!");
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Failed to Send!");
}

// Register message handlers
static void app_message_init(void) {
  app_message_register_inbox_received(in_received_handler);
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_outbox_failed(out_failed_handler);
  // Init buffers
  app_message_open(64, 64);
}

// This monitors the accelerometer to perform the tilt-to-change-bubble
static void handle_accel(AccelData *accel_data, uint32_t num_samples) {
	uint32_t i;
	int lastx;
	int lastz;
	int lasty = lastx = lastz = 999999;
	for (i=0; i < num_samples; i++)
	{
		if (accel_data[i].x != lastx || accel_data[i].y != lasty || accel_data[i].z != lastz)
		{
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Marking bubble_layer dirty");
			bubble_x_target = lastx = -accel_data[i].x;
			bubble_y_target = lasty = accel_data[i].y;
			bubble_z_target = lastz = accel_data[i].z;
			layer_mark_dirty(bubble_layer);	
		}
	}
}

// Change the display mode based on the select button
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (mode >= MAX_MODES) mode = 0;
	else mode++;
 }

static void config_provider(Window *window) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Setup Bubble Layer -> Window Layer
  bubble_layer = layer_create(bounds);
  layer_set_update_proc(bubble_layer, bubble_layer_update_callback);
  layer_add_child(window_layer, bubble_layer);
	
	window_set_click_config_provider(window, (ClickConfigProvider) config_provider);
}

static void window_unload(Window *window) {
	layer_destroy(bubble_layer);
	app_message_deregister_callbacks();
}

// Handle the start-up of the app
static void init(void) {

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
	
  // Push the window onto the stack
	const bool animated = true;
	window_stack_push(window, animated);

	accel_data_service_subscribe(50, handle_accel);
	accel_service_set_sampling_rate(50);

	//tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	app_message_init();
	
	mode = persist_read_int(BUBBLE_KEY_ORIENTATION);
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  accel_data_service_unsubscribe();
  window_destroy(window);
}

// The main event/run loop for our app
int main(void) {
  init();
  app_event_loop();
  deinit();
}
