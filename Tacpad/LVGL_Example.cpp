#include "LVGL_Example.h"
#include "LVGL_Music.h"
#include <demos/lv_demos.h>
#include "BLE_Driver.h"
// #include <demos/music/lv_demo_music_main.h>
// #include <demos/music/lv_demo_music_list.h>


/*********************
 *      STRAT ARRAY
 **********************/
const char* btnArray[] = {
  "DDUR", //resuply
  "DULDURDU", //hellbomb
  "DURULU", //autocannon sentry
  "DURRU", //machine gun sentry
  "DDLUR", //EAT
  "URUD", //eagle smoke
  "RDURD", //orbital laser
  "DUUDU", //jump pack
  "DURL", //gatling sentry
};

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 *  STATIC PROTOTYPESc:\Users\C_Horton\Downloads\CSE_CST328-main\CSE_CST328-main\src\CSE_CST328.h
 **********************/
static void Onboard_create(lv_obj_t * parent);
static void Music_create(lv_obj_t * parent);
static void color_changer_create(lv_obj_t * parent);

static void color_changer_event_cb(lv_event_t * e);
static void color_event_cb(lv_event_t * e);
static void ta_event_cb(lv_event_t * e);
static void birthday_event_cb(lv_event_t * e);
static void calendar_event_cb(lv_event_t * e);
void IRAM_ATTR example1_increase_lvgl_tick(lv_timer_t * t);
static void Automatic_grid_create(lv_obj_t * parent);
static void grid_btn_event_cb(lv_event_t * e);
static lv_obj_t * DPad_btn_create(lv_obj_t * parent, const char * fileName, const char * imgPath,
                                   lv_coord_t x_ofs, lv_coord_t y_ofs, uint32_t dir_idx);
static void dpad_btn_event_cb(lv_event_t * e);
static void Manual_dpad_create(lv_obj_t * parent);

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

static void Automatic_grid_create(lv_obj_t * parent)
{
  lv_obj_t * grid = lv_obj_create(parent);
  lv_obj_set_size(grid, LV_PCT(100), LV_PCT(100));
  lv_obj_center(grid);
  lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(grid, 0, 0);
  lv_obj_set_style_pad_all(grid, 2, 0);
  lv_obj_set_style_pad_row(grid, 2, 0);
  lv_obj_set_style_pad_column(grid, 2, 0);
  lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(grid, LV_OBJ_FLAG_GESTURE_BUBBLE);   // see note below
  lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
  
  static const char * grid_files[9] = {
    "grid1.bin","grid2.bin","grid3.bin",
    "grid4.bin","grid5.bin","grid6.bin",
    "grid7.bin","grid8.bin","grid9.bin"
  };
  static const char * grid_paths[9] = {
    "S:/images/grid1.bin","S:/images/grid2.bin","S:/images/grid3.bin",
    "S:/images/grid4.bin","S:/images/grid5.bin","S:/images/grid6.bin",
    "S:/images/grid7.bin","S:/images/grid8.bin","S:/images/grid9.bin"
  };

  for (int i = 0; i < 9; i++) {
    lv_obj_t * btn;
    if (File_Search("/images", grid_files[i])) {
      btn = lv_imgbtn_create(grid);
      lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, grid_paths[i], NULL);
    } else {
      printf("SD : %s not found, falling back to placeholder button\r\n", grid_files[i]);
      btn = lv_label_create(grid);
      lv_label_set_text(btn, LV_SYMBOL_IMAGE);
      lv_obj_set_style_text_font(btn, &lv_font_montserrat_14, 0);
      lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    }
    lv_obj_set_size(btn, 70, 70);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_GESTURE_BUBBLE);   // see note below
    lv_obj_add_event_cb(btn, grid_btn_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
  }
}

static void grid_btn_event_cb(lv_event_t * e)
{
  uint32_t idx = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  printf("Automatic grid button %d pressed\r\n", idx);
  // TODO: whatever each of the 9 buttons should actually do
    if (isBleConnected && !sent) {
    delay(20);
    sendKeySequence(btnArray[idx]);
    printf("Sent key sequence\r\n", btnArray[idx]);  
    // sent = true;
    }
}

static lv_obj_t * DPad_btn_create(lv_obj_t * parent, const char * fileName, const char * imgPath,
                                   lv_coord_t x_ofs, lv_coord_t y_ofs, uint32_t dir_idx)
{
  lv_obj_t * btn;

  if (File_Search("/images", fileName)) {
    btn = lv_imgbtn_create(parent);
    lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, imgPath, NULL);
  } else {
    printf("SD : %s not found, falling back to placeholder button\r\n", fileName);
    btn = lv_label_create(parent);
    lv_label_set_text(btn, LV_SYMBOL_IMAGE);
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
  }

  lv_obj_set_size(btn, 50, 50);
  lv_obj_align(btn, LV_ALIGN_CENTER, x_ofs, y_ofs);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_GESTURE_BUBBLE);   // keeps swipe-to-switch-screens working here too
  lv_obj_add_event_cb(btn, dpad_btn_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)dir_idx);
  return btn;
}

static void dpad_btn_event_cb(lv_event_t * e)
{
  uint32_t dir = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
  static const char * names[4] = {"UP", "DOWN", "LEFT", "RIGHT"};
  printf("Manual D-pad: %s pressed\r\n", names[dir]);
  // TODO: actual jog/move action per direction
}

static void Manual_dpad_create(lv_obj_t * parent)
{
  const lv_coord_t OFS = 60;   // center-to-center distance; bump up/down to taste

  DPad_btn_create(parent, "UpArrow.bin",    "S:/images/UpArrow.bin",     0,   -OFS, 0);
  DPad_btn_create(parent, "DownArrow.bin",  "S:/images/DownArrow.bin",   0,    OFS, 1);
  DPad_btn_create(parent, "LeftArrow.bin",  "S:/images/LeftArrow.bin", -OFS,    0,  2);
  DPad_btn_create(parent, "RightArrow.bin", "S:/images/RightArrow.bin", OFS,    0,  3);
}

void Lvgl_Example1(void){

  disp_size = DISP_SMALL;                            

  font_large = LV_FONT_DEFAULT;                             
  font_normal = LV_FONT_DEFAULT;                         
  
  lv_coord_t tab_h;
  tab_h = 45;
  // lv_coord_t tab_w;
  // tab_w = 70;

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

  lv_obj_t * img_btn1 = Image_tab_create(screens[0], "AutomaticIcon_64.bin", "S:/images/AutomaticIcon_64.bin");
  //lv_obj_t * img_btn2 = Image_tab_create(screens[1], "ManualIcon_64.bin", "S:/images/ManualIcon_64.bin");
  // lv_obj_t * img_btn3 = Image_tab_create(screens[2], "icon3.bin", "S:/images/icon3.bin");
  
  Manual_dpad_create(screens[1]);
  Automatic_grid_create(screens[2]);


  // gestures only bubble up if the touched child opts in
  lv_obj_add_flag(img_btn1, LV_OBJ_FLAG_GESTURE_BUBBLE);
  // lv_obj_add_flag(img_btn2, LV_OBJ_FLAG_GESTURE_BUBBLE);
  // lv_obj_add_flag(img_btn3, LV_OBJ_FLAG_GESTURE_BUBBLE);

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

static void Onboard_create(lv_obj_t * parent)
{

  /*Create a panel*/
  lv_obj_t * panel1 = lv_obj_create(parent);
  lv_obj_set_height(panel1, LV_SIZE_CONTENT);

  lv_obj_t * panel1_title = lv_label_create(panel1);
  lv_label_set_text(panel1_title, "Onboard parameter");
  lv_obj_add_style(panel1_title, &style_title, 0);

  lv_obj_t * SD_label = lv_label_create(panel1);
  lv_label_set_text(SD_label, "SD Card");
  lv_obj_add_style(SD_label, &style_text_muted, 0);

  SD_Size = lv_textarea_create(panel1);
  lv_textarea_set_one_line(SD_Size, true);
  lv_textarea_set_placeholder_text(SD_Size, "SD Size");
  lv_obj_add_event_cb(SD_Size, ta_event_cb, LV_EVENT_ALL, NULL);


  lv_obj_t * Image_label = NULL;
  lv_obj_t * sd_image_btn = NULL;

  const char *directory = "/images";
  const char *fileName  = "btn_icon.bin";

  if (File_Search(directory, fileName))
  {
    Image_label = lv_label_create(panel1);
    lv_label_set_text(Image_label, "SD Image Button");
    lv_obj_add_style(Image_label, &style_text_muted, 0);

    static char imgPath[64];   // static: must outlive this function, LVGL only stores the pointer
    snprintf(imgPath, sizeof(imgPath), "S:%s/%s", directory, fileName);

    sd_image_btn = lv_imgbtn_create(panel1);
    lv_imgbtn_set_src(sd_image_btn, LV_IMGBTN_STATE_RELEASED, NULL, imgPath, NULL);
    lv_obj_set_size(sd_image_btn, 64, 64);
    lv_obj_add_event_cb(sd_image_btn, sd_image_btn_event_cb, LV_EVENT_CLICKED, NULL);
  }
  else
  {
    printf("SD : btn_icon.bin not found, falling back to placeholder button\r\n");

    Image_label = lv_label_create(panel1);
    lv_label_set_text(Image_label, "SD Image Button (missing)");
    lv_obj_add_style(Image_label, &style_text_muted, 0);

    sd_image_btn = lv_label_create(panel1);
    lv_label_set_text(sd_image_btn, LV_SYMBOL_IMAGE);
    lv_obj_set_style_text_font(sd_image_btn, &lv_font_montserrat_14, 0);
    lv_obj_add_flag(sd_image_btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(sd_image_btn, sd_image_btn_event_cb, LV_EVENT_CLICKED, NULL);
  }
  static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
  lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);


  auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 500, NULL);
  

}

void IRAM_ATTR example1_increase_lvgl_tick(lv_timer_t * t)
{
  char buf[100];

  snprintf(buf, sizeof(buf), "%d MB\r\n", SDCard_Size);
  lv_textarea_set_placeholder_text(SD_Size, buf);

  LVGL_Backlight_adjustment(LCD_Backlight);   // hardware PWM call, no LVGL object involved — safe to keep
}
static void Music_create(lv_obj_t * parent)
{
  static lv_obj_t * ctrl;
  original_screen_bg_color = lv_obj_get_style_bg_color(parent, 0);
  lv_obj_set_style_bg_color(parent, lv_color_hex(0x343247), 0);

  ctrl = _lv_demo_music_main_create(parent);
}

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void color_changer_create(lv_obj_t * parent)
{
    static lv_palette_t palette[] = {
        LV_PALETTE_BLUE, LV_PALETTE_GREEN, LV_PALETTE_BLUE_GREY,  LV_PALETTE_ORANGE,
        LV_PALETTE_RED, LV_PALETTE_PURPLE, LV_PALETTE_TEAL, _LV_PALETTE_LAST
    };

    lv_obj_t * color_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(color_cont);
    lv_obj_set_flex_flow(color_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(color_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(color_cont, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(color_cont, lv_color_white(), 0);
    lv_obj_set_style_pad_right(color_cont, disp_size == DISP_SMALL ? LV_DPX(47) : LV_DPX(55), 0);
    lv_obj_set_style_bg_opa(color_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(color_cont, LV_RADIUS_CIRCLE, 0);

    if(disp_size == DISP_SMALL) lv_obj_set_size(color_cont, LV_DPX(52), LV_DPX(52));
    else lv_obj_set_size(color_cont, LV_DPX(60), LV_DPX(60));

    lv_obj_align(color_cont, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));

    uint32_t i;
    for(i = 0; palette[i] != _LV_PALETTE_LAST; i++) {
        lv_obj_t * c = lv_btn_create(color_cont);
        lv_obj_set_style_bg_color(c, lv_palette_main(palette[i]), 0);
        lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_size(c, 20, 20);
        lv_obj_add_event_cb(c, color_event_cb, LV_EVENT_ALL, &palette[i]);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }

    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(btn, 10, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_event_cb(btn, color_changer_event_cb, LV_EVENT_ALL, color_cont);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_TINT, 0);

    if(disp_size == DISP_SMALL) {
        lv_obj_set_size(btn, LV_DPX(42), LV_DPX(42));
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
    }
    else {
        lv_obj_set_size(btn, LV_DPX(50), LV_DPX(50));
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
    }
}

static void color_changer_anim_cb(void * var, int32_t v)
{
    lv_obj_t * obj =(lv_obj_t *) var;
    lv_coord_t max_w = lv_obj_get_width(lv_obj_get_parent(obj)) - LV_DPX(20);
    lv_coord_t w;

    if(disp_size == DISP_SMALL) {
        w = lv_map(v, 0, 256, LV_DPX(52), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));
    }
    else {
        w = lv_map(v, 0, 256, LV_DPX(60), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));
    }

    if(v > LV_OPA_COVER) v = LV_OPA_COVER;

    uint32_t i;
    for(i = 0; i < lv_obj_get_child_cnt(obj); i++) {
        lv_obj_set_style_opa(lv_obj_get_child(obj, i), v, 0);
    }

}

static void color_changer_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t * color_cont = (lv_obj_t *)lv_event_get_user_data(e);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
        else {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 256, 0);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
}
static void color_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_t * color_cont = lv_obj_get_parent(obj);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
    else if(code == LV_EVENT_CLICKED) {
        lv_palette_t * palette_primary = (lv_palette_t *)lv_event_get_user_data(e);
        lv_palette_t palette_secondary =(lv_palette_t) ((*palette_primary) + 3); /*Use another palette as secondary*/
        if(palette_secondary >= _LV_PALETTE_LAST) palette_secondary =(lv_palette_t)(0);
#if LV_USE_THEME_DEFAULT
        lv_theme_default_init(NULL, lv_palette_main(*palette_primary), lv_palette_main(palette_secondary),
                              LV_THEME_DEFAULT_DARK, font_normal);
#endif
        lv_color_t color = lv_palette_main(*palette_primary);
        lv_style_set_text_color(&style_icon, color);
        lv_chart_set_series_color(chart1, ser1, color);
        lv_chart_set_series_color(chart2, ser3, color);
    }
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



static void ta_event_cb(lv_event_t * e)
{
}

static void birthday_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) == LV_INDEV_TYPE_POINTER) {
            if(calendar == NULL) {
                lv_obj_add_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
                calendar = lv_calendar_create(lv_layer_top());
                lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);
                lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0);
                if(disp_size == DISP_SMALL) lv_obj_set_size(calendar, 180, 200);
                else if(disp_size == DISP_MEDIUM) lv_obj_set_size(calendar, 200, 220);
                else  lv_obj_set_size(calendar, 300, 330);
                lv_calendar_set_showed_date(calendar, 1990, 01);
                lv_obj_align(calendar, LV_ALIGN_CENTER, 0, 30);
                lv_obj_add_event_cb(calendar, calendar_event_cb, LV_EVENT_ALL, ta);

                lv_calendar_header_dropdown_create(calendar);
            }
        }
    }
}


static void calendar_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t * obj = (lv_obj_t *)lv_event_get_current_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t d;
        lv_calendar_get_pressed_date(obj, &d);
        char buf[32];
        lv_snprintf(buf, sizeof(buf), "%02d.%02d.%d", d.day, d.month, d.year);
        lv_textarea_set_text(ta, buf);

        lv_obj_del(calendar);
        calendar = NULL;
        lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);
    }
}


void LVGL_Backlight_adjustment(uint8_t Backlight) {
  Set_Backlight(Backlight);                                 
}




