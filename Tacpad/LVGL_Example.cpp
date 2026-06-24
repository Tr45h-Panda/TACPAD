#include "LVGL_Example.h"
#include <demos/lv_demos.h>
#include "BLE_Driver.h"
#include "Stratagem_Data.h"   
#include "SD_Card.h"
#include "Audio_PCM5101.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void Automatic_grid_create(lv_obj_t * parent);
static void grid_btn_event_cb(lv_event_t * e);
static void hotspot_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

lv_obj_t * tv;
static lv_obj_t * calendar;
lv_style_t style_text_muted;
lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;

static lv_obj_t * chart1;
static lv_obj_t * chart2;
static lv_obj_t * chart3;

static void sd_image_btn_event_cb(lv_event_t * e);

static lv_chart_series_t * ser1;
static lv_chart_series_t * ser2;
static lv_chart_series_t * ser3;
static lv_chart_series_t * ser4;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

static lv_timer_t * auto_step_timer;
static lv_color_t original_screen_bg_color;

static lv_timer_t * meter2_timer;

lv_obj_t * SD_Size;
lv_obj_t * FlashSize;
lv_obj_t * BAT_Volts;
lv_obj_t * Board_angle;
lv_obj_t * RTC_Time;
lv_obj_t * Wireless_Scan;
lv_obj_t * Backlight_add;
lv_obj_t * Backlight_subtract;
lv_obj_t * Backlight_slider;

lv_obj_t *Page_panel[50];
lv_obj_t *Simulated_panel1[100];
size_t Simulated_panel1_Size;

static void Settings_panel_create(lv_obj_t * parent);
static void loadout_dropdown_event_cb(lv_event_t * e);
static void play_sfx_async(void * data);
static lv_obj_t * screens[3];
static uint8_t active_screen = 0;

static void show_screen(uint8_t idx)
{
  for (int i = 0; i < 3; i++) {
    if (i == idx) lv_obj_clear_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
    else          lv_obj_add_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
  }
  active_screen = idx;
}

static lv_obj_t * splash_bg = NULL;
static lv_obj_t * splash_img = NULL;

static void Lvgl_Pump(uint32_t duration_ms)
{
  uint32_t start = millis();
  while (millis() - start < duration_ms) {
    lv_timer_handler();   // manually drives rendering/animation since loop() hasn't started yet
    delay(5);
  }
}

void Splash_Show(void)
{
  splash_bg = lv_obj_create(lv_scr_act());
  lv_obj_set_size(splash_bg, LVGL_WIDTH, LVGL_HEIGHT);
  lv_obj_set_pos(splash_bg, 0, 0);
  lv_obj_clear_flag(splash_bg, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(splash_bg, 0, 0);
  lv_obj_set_style_radius(splash_bg, 0, 0);
  lv_obj_set_style_bg_color(splash_bg, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(splash_bg, LV_OPA_COVER, 0);   // background: always solid black

  splash_img = lv_img_create(splash_bg);
  lv_img_set_src(splash_img, "S:/images/Super_Earth.bin");
  lv_obj_center(splash_img);
  lv_obj_set_style_opa(splash_img, LV_OPA_TRANSP, 0);    // image starts invisible

  lv_obj_fade_in(splash_img, 600, 0);   // fade the IMAGE in, revealing it over the solid black
  Lvgl_Pump(700);
}

void Splash_Hide(void)
{
  if (!splash_bg) return;

  lv_obj_move_foreground(splash_bg);    // Lvgl_Example1() just built 3 screens on top of this
  lv_obj_fade_out(splash_img, 600, 0);  // fade the IMAGE back out, leaving solid black showing
  Lvgl_Pump(700);

  show_screen(0);                       // land on Automatic, now that we're at black

  lv_obj_del(splash_bg);
  splash_bg = NULL;
  splash_img = NULL;
}

static void tab_icon_event_cb(lv_event_t * e)
{
  uint32_t idx = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  show_screen(idx);
}

static void swipe_event_cb(lv_event_t * e)
{
  lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
  if (dir == LV_DIR_LEFT)       show_screen((active_screen + 1) % 3);
  else if (dir == LV_DIR_RIGHT) show_screen((active_screen + 2) % 3);  // (active-1) mod 3
}

static lv_obj_t * loadout_status_label = NULL;

static void refresh_grid_async(void * data)
{
  Automatic_grid_refresh();
  uint8_t idx = (uint8_t)(uintptr_t)data;
  if (loadout_status_label && idx < loadout_count) {
    char buf[48];
    snprintf(buf, sizeof(buf), "Loaded: %s", loadouts[idx].name);
    lv_label_set_text(loadout_status_label, buf);
  }
}

static void loadout_dropdown_event_cb(lv_event_t * e)
{
  lv_obj_t * dd = lv_event_get_target(e);
  uint16_t idx = lv_dropdown_get_selected(dd);

  if (loadout_status_label) lv_label_set_text(loadout_status_label, "Updating...");

  if (Stratagem_SelectLoadout((uint8_t)idx)) {
    lv_async_call(refresh_grid_async, (void*)(uintptr_t)idx);
  }
}

static void Settings_panel_create(lv_obj_t * parent)
{
  lv_obj_t * title = lv_label_create(parent);
  lv_label_set_text(title, "Loadout");
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  char options[512] = {0};
  for (int i = 0; i < loadout_count; i++) {
    strlcat(options, loadouts[i].name, sizeof(options));
    if (i < loadout_count - 1) strlcat(options, "\n", sizeof(options));
  }
  if (loadout_count == 0) strlcpy(options, "No loadouts found", sizeof(options));

  lv_obj_t * dd = lv_dropdown_create(parent);
  lv_dropdown_set_options(dd, options);
  lv_dropdown_set_selected(dd, 0);
  lv_obj_set_width(dd, 180);
  lv_obj_align(dd, LV_ALIGN_TOP_MID, 0, 40);
  lv_obj_add_flag(dd, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_add_event_cb(dd, loadout_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

  loadout_status_label = lv_label_create(parent);
  lv_label_set_text(loadout_status_label, "");
  lv_obj_align(loadout_status_label, LV_ALIGN_TOP_MID, 0, 75);
}


static lv_obj_t * automatic_grid = NULL;

static void Automatic_grid_populate(lv_obj_t * grid)
{
  lv_obj_clean(grid);   // wipe existing buttons before rebuilding

  for (int i = 0; i < 9; i++) {
    Stratagem * s = Stratagem_GetById(grid_loadout[i]);
    lv_obj_t * btn;

    if (s == NULL) {
      btn = lv_label_create(grid);
      lv_label_set_text(btn, LV_SYMBOL_WARNING);
      lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    } 
    else {
      const char * slash = strrchr(s->img_path, '/');
      const char * filename_only = slash ? slash + 1 : s->img_path;

      if (s->icon_exists) 
      {
        btn = lv_imgbtn_create(grid);
        lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, s->img_path, NULL);
      } 
      else 
      {
        printf("SD : %s not found, falling back to placeholder button\r\n", s->img_path);
        btn = lv_label_create(grid);
        lv_label_set_text(btn, LV_SYMBOL_IMAGE);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_14, 0);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
      }
      lv_obj_add_event_cb(btn, grid_btn_event_cb, LV_EVENT_CLICKED, (void*)s->command);
    }

    lv_obj_set_size(btn, 70, 70);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_GESTURE_BUBBLE);
  }
}

static void Automatic_grid_create(lv_obj_t * parent)
{
  automatic_grid = lv_obj_create(parent);
  lv_obj_set_size(automatic_grid, LV_PCT(100), LV_PCT(100));
  lv_obj_center(automatic_grid);
  lv_obj_clear_flag(automatic_grid, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(automatic_grid, 0, 0);
  lv_obj_set_style_pad_all(automatic_grid, 2, 0);
  lv_obj_set_style_pad_row(automatic_grid, 2, 0);
  lv_obj_set_style_pad_column(automatic_grid, 2, 0);
  lv_obj_set_flex_flow(automatic_grid, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(automatic_grid, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(automatic_grid, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_set_style_bg_opa(automatic_grid, LV_OPA_TRANSP, 0);

  Automatic_grid_populate(automatic_grid);
}

void Automatic_grid_refresh(void)
{
  if (automatic_grid) Automatic_grid_populate(automatic_grid);
}

static void play_sfx_async(void * data)
{
  Play_SFX("helldivers-2-arrow.mp3");
}

static void grid_btn_event_cb(lv_event_t * e)
{
  const char * command = (const char*)lv_event_get_user_data(e);
  if (command) {
    printf("Stratagem grid: queuing %s\r\n", command);
    Queue_KeySequence(command);
    lv_async_call(play_sfx_async, NULL);
  }
}


static void hotspot_event_cb(lv_event_t * e)
{
  uint32_t id = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  static const char * names[5] = {"UP", "DOWN", "LEFT", "RIGHT", "CENTER"};

  if (id < 5) {
    printf("Manual hotspot: %s pressed\r\n", names[id]);
  } 
  else {
    printf("Manual hotspot: unknown id %u pressed\r\n", id);
  }

  switch (id) {
    case 0:  // UP
      // TODO: jog/move up action
      Queue_KeySequence("U");
      break;
    case 1:  // DOWN
      // TODO: jog/move down action
      Queue_KeySequence("D");
      break;
    case 2:  // LEFT
      // TODO: jog/move left action
      Queue_KeySequence("L");
      break;
    case 3:  // RIGHT
      // TODO: jog/move right action
      Queue_KeySequence("R");
      break;
    case 4:  // CENTER
      // TODO: select/confirm/stop action
      Queue_KeySequence("U");
      break;
    default:
      break;
  }
}

static lv_obj_t * Hotspot_create(lv_obj_t * parent, lv_coord_t x, lv_coord_t y,
                                  lv_coord_t w, lv_coord_t h, uint32_t id)
{
  lv_obj_t * hs = lv_obj_create(parent);
  lv_obj_set_pos(hs, x, y);
  lv_obj_set_size(hs, w, h);
  lv_obj_set_style_bg_opa(hs, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(hs, 0, 0);
  lv_obj_clear_flag(hs, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(hs, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(hs, LV_OBJ_FLAG_GESTURE_BUBBLE);   // same swipe requirement as your other buttons
  lv_obj_add_event_cb(hs, hotspot_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)id);
  return hs;
}

static void Manual_arrows_create(lv_obj_t * parent)
{
  lv_obj_t * bg = lv_img_create(parent);
  lv_img_set_src(bg, "S:/images/ManualArrows.bin");
  lv_obj_center(bg);

  Hotspot_create(parent, 101, 32,  46, 29, 0);   // up
  Hotspot_create(parent, 103, 178, 45, 29, 1);   // down
  Hotspot_create(parent, 35,  96,  26, 47, 2);   // left
  Hotspot_create(parent, 188, 96,  28, 47, 3);   // right
  Hotspot_create(parent, 89,  89,  71, 61, 4);   // center button
}



void Lvgl_Example1(void){

  disp_size = DISP_SMALL;                            

  font_large = LV_FONT_DEFAULT;                             
  font_normal = LV_FONT_DEFAULT;                         
  
  lv_coord_t tab_h;
  tab_h = 45;

  #if LV_FONT_MONTSERRAT_18
    font_large     = &lv_font_montserrat_18;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  #if LV_FONT_MONTSERRAT_12
    font_normal    = &lv_font_montserrat_12;
  #else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
  #endif
  
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x303030), 0);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

  lv_style_init(&style_text_muted);
  lv_style_set_text_opa(&style_text_muted, LV_OPA_90);

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, font_large);

  lv_style_init(&style_icon);
  lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
  lv_style_set_text_font(&style_icon, font_large);

  lv_style_init(&style_bullet);
  lv_style_set_border_width(&style_bullet, 0);
  lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

  lv_coord_t tab_w = 70;

  for (int i = 0; i < 3; i++) {
    screens[i] = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screens[i], LV_HOR_RES - tab_w, LV_VER_RES);
    lv_obj_align(screens[i], LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_clear_flag(screens[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(screens[i], swipe_event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_set_style_bg_opa(screens[i], LV_OPA_TRANSP, 0);   // let the screen's dark grey show through
  }

  Automatic_grid_create(screens[0]);
  Manual_arrows_create(screens[1]);
  Settings_panel_create(screens[2]);

  show_screen(0);   // start on screen 0; hides the other two

  // the same right-edge icon strip as before — only the callback's target changed
  lv_obj_t * tab_strip = lv_obj_create(lv_scr_act());
  lv_obj_set_size(tab_strip, tab_w, LV_PCT(100));
  lv_obj_align(tab_strip, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_clear_flag(tab_strip, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(tab_strip, 0, 0);
  lv_obj_set_flex_flow(tab_strip, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(tab_strip, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  // lv_obj_set_style_bg_opa(tab_strip, LV_OPA_TRANSP, 0);
  lv_obj_set_style_bg_color(tab_strip, lv_color_hex(0x202020), 0);
  lv_obj_set_style_bg_opa(tab_strip, LV_OPA_COVER, 0);

  static const char * tab_icon_files[3] = { "icon1.bin", "icon2.bin", "icon3.bin" };
  static const char * tab_icon_paths[3] = { "S:/images/icon1.bin", "S:/images/icon2.bin", "S:/images/icon3.bin" };
  
  for (int i = 0; i < 3; i++) {
    if (!File_Search("/images", tab_icon_files[i]))
      printf("SD : %s not found for tab icon %d\r\n", tab_icon_files[i], i);
    lv_obj_t * btn = lv_imgbtn_create(tab_strip);
    lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, tab_icon_paths[i], NULL);
    lv_obj_set_size(btn, tab_w - 16, tab_w - 16);
    lv_obj_add_event_cb(btn, tab_icon_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
  }

  lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);

}

void Lvgl_Example1_close(void)
{
  /*Delete all animation*/
  lv_anim_del(NULL, NULL);

  lv_timer_del(meter2_timer);
  meter2_timer = NULL;

  lv_obj_clean(lv_scr_act());

  lv_style_reset(&style_text_muted);
  lv_style_reset(&style_title);
  lv_style_reset(&style_icon);
  lv_style_reset(&style_bullet);
}


/**********************
*   STATIC FUNCTIONS
**********************/


static void sd_image_btn_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    printf("SD image button pressed\r\n");
    // TODO: whatever this button should actually trigger
  }
}
static lv_obj_t * Image_tab_create(lv_obj_t * parent, const char * fileName, const char * imgPath)
{
  lv_obj_t * img_btn;

  if (File_Search("/images", fileName))
  {
    img_btn = lv_imgbtn_create(parent);
    lv_imgbtn_set_src(img_btn, LV_IMGBTN_STATE_RELEASED, NULL, imgPath, NULL);
    lv_obj_set_size(img_btn, 64, 64);   // match THIS image's actual pixel size
  }
  else
  {
    printf("SD : %s not found, falling back to placeholder button\r\n", fileName);
    img_btn = lv_label_create(parent);
    lv_label_set_text(img_btn, LV_SYMBOL_IMAGE);
    lv_obj_set_style_text_font(img_btn, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(img_btn, LV_OBJ_FLAG_CLICKABLE);
  }

  lv_obj_add_event_cb(img_btn, sd_image_btn_event_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_center(img_btn);
  return img_btn;
}


void Backlight_adjustment_event_cb(lv_event_t * e) {

  lv_obj_t * target = lv_event_get_target(e);
  uint8_t Backlight = lv_slider_get_value(Backlight_slider);  
  if(target == Backlight_add){
    if(Backlight < 99)
      Backlight = Backlight + 2;
    else{
      printf("Backlight out of range: %d\n", Backlight);
      Backlight = 100;
    }
    
  }
  else if(target == Backlight_subtract){
    if(Backlight > 1)
      Backlight = Backlight - 2;
    else{
      printf("Backlight out of range: %d\n", Backlight);
      Backlight = 0;
    }
  }
  if (Backlight >= 0 && Backlight <= Backlight_MAX)  {
    lv_slider_set_value(Backlight_slider, Backlight, LV_ANIM_ON); 
    LCD_Backlight = Backlight;
    LVGL_Backlight_adjustment(Backlight);
  }

}


