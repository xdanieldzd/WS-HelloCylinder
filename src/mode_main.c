#include <wonderful.h>
#include <ws.h>
#include <wse.h>

// project sources
#include "main.h"
#include "utils.h"
#include "text.h"
#include "input.h"

// strings
const char __far str_line_1[] = "~ Hello WonderSwan World ~";

const char __far str_line_2[] = "Vblank count: %u";

const char __far str_line_3[] = "BG: X=%03i(%03i) Y=%03i(%03i)";

const char __far str_line_4[] = "Sprite pos: X=%i Y=%i";
const char __far str_line_5[] = "Sprite prio: %s (%04X)";

const char __far str_line_6[] = "Btn held/down now/down prv";
const char __far str_line_7[] = "    %04X/    %04X/    %04X";

const char __far str_line_8[] = "X-pad to move sprite";
const char __far str_line_9[] = "Y-pad to scroll background";
const char __far str_line_10[] = "Start to change spr prio";
const char __far str_line_11[] = "A to toggle BG autoscroll";

// this one will be off-screen; 0x11 is a heart in the font tileset <3
const char __far str_line_12[] = "February 2026 by xdaniel \x11";

// strings for sprite prio text
const char __far str_front[] = "Front";
const char __far str_back[] = "Back";

ws_sprite_t* main_sprite = NULL;

void print_static_text(void)
{
	print_string_static(1, 1, str_line_1);

	print_string_static(1, 10, str_line_6);

	print_string_static(1, 13, str_line_8);
	print_string_static(1, 14, str_line_9);
	print_string_static(1, 15, str_line_10);
	print_string_static(1, 16, str_line_11);

	print_string_static(1, 18, str_line_12);
}

void print_dynamic_text(void)
{
	print_string(1, 3, str_line_2, vbl_ticks);

	print_string(1, 5, str_line_3, bg_scroll_position_x >> 8, bg_scroll_position_x & 0xFF, bg_scroll_position_y >> 8, bg_scroll_position_y & 0xFF);

	print_string(1, 7, str_line_4, main_sprite->x, main_sprite->y);
	print_string(1, 8, str_line_5, main_sprite->attr & WS_SPRITE_ATTR_PRIORITY ? str_front : str_back, main_sprite->attr & WS_SPRITE_ATTR_PRIORITY);

	print_string(1, 11, str_line_7, buttons_held_now, buttons_pressed_now, buttons_pressed_last);
}

void init_mode_main(void)
{
	// disable LCD output
	ws_lcd_control_disable();

	// enable screen 1, screen 2, sprites
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE | WS_DISPLAY_CTRL_SPR_ENABLE);

	// set line match interrupt handler to default
	ws_int_set_default_handler_line();

	wait_for_vblank();

	// clear text to spaces
	ws_screen_fill_tiles(&wse_screen2, 0x020 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(4), 0, 0, WS_SCREEN_WIDTH_TILES, WS_SCREEN_HEIGHT_TILES);

	// reset scroll
	ws_display_scroll_screen2_to(0, 0);

	// set sprite table address to 1st default
	ws_display_set_sprite_address(&wse_sprites1);

	// initialize sprite
	main_sprite = &wse_sprites1.entry[0];
	main_sprite->x = 224 - 12;
	main_sprite->y = 144 - 12;
	// 0x10 is a smiley face in the font tileset
	main_sprite->attr = 0x010 | WS_SPRITE_ATTR_PALETTE(4);

	// set sprite count
	outportb(WS_SPR_COUNT_PORT, 0x01);

	// print all static text once
	print_static_text();

	// enable LCD output
	ws_lcd_control_enable();

	// next mode is main mode loop
	mode = MODE_MAIN_RUN;
}

void run_mode_main(void)
{
	// X-pad: move sprite
	if (buttons_held_now & WS_KEY_X1) main_sprite->y--;
	if (buttons_held_now & WS_KEY_X2) main_sprite->x++;
	if (buttons_held_now & WS_KEY_X3) main_sprite->y++;
	if (buttons_held_now & WS_KEY_X4) main_sprite->x--;

	// Y-pad: scroll background
	if (buttons_held_now & WS_KEY_Y1) bg_scroll_position_y -= BG_SCROLL_SPEED;
	if (buttons_held_now & WS_KEY_Y2) bg_scroll_position_x += BG_SCROLL_SPEED;
	if (buttons_held_now & WS_KEY_Y3) bg_scroll_position_y += BG_SCROLL_SPEED;
	if (buttons_held_now & WS_KEY_Y4) bg_scroll_position_x -= BG_SCROLL_SPEED;

	// Start button: toggle sprite prio
	if (buttons_pressed_now & WS_KEY_START) main_sprite->attr = main_sprite->attr ^ WS_SPRITE_ATTR_PRIORITY;

	// A button: toggle BG scroll
	if (buttons_pressed_now & WS_KEY_A) do_scroll_bg = !do_scroll_bg;

	// print all dynamic text
	print_dynamic_text();
}
