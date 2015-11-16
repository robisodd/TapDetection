#include "main.h"
// ------------------------------------------------------------------------ //
//  Variables and Structures
// ------------------------------------------------------------------------ //
static Window *main_window;
static Layer *graphics_layer;

int16_t max_width=143, max_height=167;  // CURSOR STUFF: should be changed in init function in case of chalk
bool emulator;  // whether in emulator or not
GPoint cursor;  // crosshairs

#define NUMBER_OF_CIRCLES 10
typedef struct circleStruct {
  int16_t x, y, r;
} circleStruct;
circleStruct circles[NUMBER_OF_CIRCLES];
uint8_t cIndex = 0;

typedef struct TapStruct {
  int64_t timestamp;
  uint8_t strength;
  int16_t x, y;  // probably could be uint8, but leaving as int16 as that's what the cursor uses
} TapStruct;
TapStruct tap;


// ------------------------------------------------------------------------ //
//  Timer Functions
// ------------------------------------------------------------------------ //
#define UPDATE_MS 50 // Screen refresh rate in milliseconds
static void timer_callback(void *data) {
  layer_mark_dirty(graphics_layer);                    // Schedule redraw of screen
  app_timer_register(UPDATE_MS, timer_callback, NULL); // Schedule a callback
}

// ------------------------------------------------------------------------ //
//  Tapping Functions
// ------------------------------------------------------------------------ //
#define SUBTLE_LENGTH        6  // how long (in samples) does it have to be quiet for to mean tap is over
#define SUBTLE_THRESHOLD    12  // how quiet does it need to be (after a tap has started) in order for it to be not-a-tap
#define INITIAL_SENSITIVITY 30  // how hard to hit to start a tap
#define MAX_LENGTH          30  // maximum length (in samples) before a tap is now just moving a lot
void accel_data_handler(AccelRawData *data, uint32_t num_samples, uint64_t timestamp) {
  static TapStruct temp;      // holds data in case current possible tap actually is a tap
  static int16_t current = 0; // current accelerometer sample.  static so it remembers where the accelerometer was between calls
  static uint8_t gap = 255;   // big gap means tap is not currently happening

  GPoint movecursor = (GPoint){.x=0, .y=0};  // CURSOR STUFF: update cursor position with average of accelerometer readings
  
  for(uint32_t i=0; i<num_samples; i++) {
    movecursor.x+=data[i].x >> 3;  // CURSOR STUFF: Get average of cursor position update
    movecursor.y+=data[i].y >> 3;  // CURSOR STUFF: Get sum of all accelerometer data to average to update cursor
    
    int16_t diff = current;                             // backup most recent accelerometer sample
    current = data[i].z >> 3;                           // get new accelerometer sample (all samples seem to be evenly divisible by 8, hence the >>3)
    if(gap==255) {                                      // if this is the first sample taken,
      diff = current;                                   //   there is no difference yet, so set diff to current sample
      gap = 0;                                          //   this stops a tap happening at program start
      temp.strength=MAX_LENGTH;                         //   because current starts at 0, 
    }                                                   //   but the accelerometer probably won't be near 0
    diff = diff>current ? diff-current : current-diff;  // Find the delta between the two samples
    
    if(gap<SUBTLE_LENGTH) {                             // tapping still going
      if(temp.strength<MAX_LENGTH)                        // if length didn't max out
        temp.strength++;                                    // then it's one longer

      if(diff < SUBTLE_THRESHOLD) {                  // if subtle vibrations (but tapping might still be going)
        gap++;                                         // measure how long it's been subtle for
        if(gap>=SUBTLE_LENGTH) {                       // if it's been subtle for too long, then the tap is over
          if(temp.strength<MAX_LENGTH) {                 // if vibration didn't last too long then make it a tap.  30 (not MAX_LENGTH) samples@100Hz is too long.
            tap.strength  = temp.strength - SUBTLE_LENGTH; // strength = how long the tap lasted.  Think of listening to how long a metal pole vibrates for after getting struck.
            tap.timestamp = temp.timestamp;                // time tap started.  good to determine time between taps.
            tap.x = temp.x; // CURSOR STUFF:  x position the tap occurred at
            tap.y = temp.y; // CURSOR STUFF:  y position the tap occurred at
            
            // New tap?  New circle!
            circles[cIndex].x = tap.x;
            circles[cIndex].y = tap.y;
            circles[cIndex].r = tap.strength;
            LOG("R: %d", circles[cIndex].r);
            cIndex = (cIndex+1) % NUMBER_OF_CIRCLES;
          }
        }
      } else {           // else: still major vibrations (tap IS still going on)
        gap=0;
      }
    } else {             // else: tapping not currently going
      if(diff >= INITIAL_SENSITIVITY) {       // if major movement, tapping begins!
        gap = 0;                              // reset gap
        //temp.strength = diff;               // initial strength of first hit.  pointless metric.  TODO: Remove
        temp.strength = 1;                    // strength is how long (in samples) tap lasts
        temp.timestamp = timestamp + (i*10);  // time sample was taken (in ms): time sample group started + 10ms (@100Hz) for each sample
        temp.x = cursor.x; // CURSOR STUFF: Store x/y of where tap started (in case it's actually a tap) cause cursor will move
        temp.y = cursor.y; // CURSOR STUFF: ... another way would be to not have cursor move during suspected taps
      }
    }
  }
  
  // CURSOR STUFF: move crosshairs
  cursor.x += (movecursor.x>>5);
  cursor.y -= (movecursor.y>>5);
  cursor.x = (cursor.x<0) ? 0 : (cursor.x>max_width)  ? max_width  : cursor.x;
  cursor.y = (cursor.y<0) ? 0 : (cursor.y>max_height) ? max_height : cursor.y;
}

// ------------------------------------------------------------------------ //
//  Button Functions
// ------------------------------------------------------------------------ //
void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_long_pulse();
}
void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {

}
void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  vibes_short_pulse();
}
  
void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
}
  
// ------------------------------------------------------------------------ //
//  Drawing Functions
// ------------------------------------------------------------------------ //
static void graphics_layer_update(Layer *me, GContext *ctx) {
  static char text[100];  //Buffer to hold text
  
  // Draw text box
  GRect textframe = GRect(25-2, 28+2, 90+4, 27+2);  // Text Box Position and Size: x, y, w, h
  graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_rect(ctx, textframe, 0, GCornerNone);  //Black Filled Rectangle
  graphics_context_set_stroke_color(ctx, GColorWhite); graphics_draw_rect(ctx, textframe);                //White Rectangle Border
  
  // Fill it with text
  textframe = GRect(25, 28, 90, 27);  // Text Box Position and Size: x, y, w, h
  graphics_draw_text(ctx, "Strength:\nTime:", fonts_get_system_font(FONT_KEY_GOTHIC_14), textframe, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);  //Write Text
  snprintf(text, sizeof(text), "%d\n%ld", tap.strength, (long)(tap.timestamp&0x7FFFFFFF));  // What text to draw
  graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_14), textframe, GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);  //Write Text
  
  // Draw Plus and Tap Circle
  graphics_context_set_stroke_color(ctx, GColorWhite); // Line Color
  graphics_draw_line(ctx, GPoint(cursor.x-10, cursor.y), GPoint(cursor.x+10, cursor.y));
  graphics_draw_line(ctx, GPoint(cursor.x, cursor.y-10), GPoint(cursor.x, cursor.y+10));
  //graphics_draw_circle(ctx, GPoint(tap.x, tap.y), tap.length);  // Draw the tap (commented out since now we draw the 8 previous taps below)
   for(uint8_t i=0; i<NUMBER_OF_CIRCLES; i++)
     if(circles[i].r<100) // Make sure we didn't somehow make a giant circle
       graphics_draw_circle(ctx, GPoint(circles[i].x, circles[i].y), circles[i].r);
}


// ------------------------------------------------------------------------ //
//  Main Functions
// ------------------------------------------------------------------------ //
void log_app_info() {
  extern const PebbleProcessInfo __pbl_app_info;
  LOG("APP Version: %d.%d", __pbl_app_info.process_version.major, __pbl_app_info.process_version.minor);
  LOG("Name: %s", __pbl_app_info.name);
  LOG("Company: %s", __pbl_app_info.company);
  char uuidString[UUID_STRING_BUFFER_LENGTH];
  uuid_to_string((Uuid*)&__pbl_app_info.uuid, uuidString);
  LOG("UUID: %s", uuidString);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  graphics_layer = layer_create(layer_get_frame(window_layer));
  layer_set_update_proc(graphics_layer, graphics_layer_update);
  layer_add_child(window_layer, graphics_layer);
  max_width  = layer_get_bounds(graphics_layer).size.w - 1;
  max_height = layer_get_bounds(graphics_layer).size.h - 1;
  timer_callback(NULL);  // Start the main loop!
}

static void main_window_unload(Window *window) {
  layer_destroy(graphics_layer);
}

static void init(void) {
  // Set up and push main window
  main_window = window_create();
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_set_click_config_provider(main_window, click_config_provider);
  window_set_background_color(main_window, GColorBlack);
  IF_SDK2(window_set_fullscreen(main_window, true));
  
  // Detect Emulation
  emulator = watch_info_get_model()==WATCH_INFO_MODEL_UNKNOWN;
  if(emulator) {
    light_enable(true);  // Good colors on emulator
    LOG("Emulator Detected: Turning Backlight On");
  }

  //Set up other stuff
  cursor.x = 72; cursor.y = 44;  // Cursor starting position
  srand(time(NULL));             // Seed randomizer (just for good measure)
  accel_raw_data_service_subscribe(5, accel_data_handler);  // We will be using the accelerometer: 5 samples_per_update
  accel_service_set_sampling_rate(ACCEL_SAMPLING_100HZ);    // 100 samples per second = 20 updates per second
  
  //Begin
  window_stack_push(main_window, true /* Animated */); // Display window.  Callback will be called once layer is dirtied then written
}
  
static void deinit(void) {
  accel_data_service_unsubscribe();
  window_destroy(main_window);
}

int main(void) {
  log_app_info();
  init();
  app_event_loop();
  deinit();
}