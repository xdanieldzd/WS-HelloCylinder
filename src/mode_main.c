#include <wonderful.h>
#include <ws.h>
#include <wse.h>

// project sources
#include "main.h"
#include "system.h"
#include "text.h"
#include "input.h"

// strings for sprite prio text
const char __far str_front[] = "Front";
const char __far str_back[] = "Back";

ws_sprite_t* main_sprite = NULL;

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

	// print a bunch of text
	print_string(1, 1, "~ Hello WonderSwan World ~");

	print_string(1, 3, "Vblank count: %u", vbl_ticks);

	print_string(1, 5, "BG: X=%03i(%03i) Y=%03i(%03i)", bg_scroll_position_x >> 8, bg_scroll_position_x & 0xFF, bg_scroll_position_y >> 8, bg_scroll_position_y & 0xFF);

	print_string(1, 7, "Sprite pos: X=%i Y=%i", main_sprite->x, main_sprite->y);
	print_string(1, 8, "Sprite prio: %s (%04X)", main_sprite->attr & WS_SPRITE_ATTR_PRIORITY ? str_front : str_back, main_sprite->attr & WS_SPRITE_ATTR_PRIORITY);

	print_string(1, 10, "Btn held/down now/down prv");
	print_string(1, 11, "    %04X/    %04X/    %04X", buttons_held_now, buttons_pressed_now, buttons_pressed_last);

	print_string(1, 13, "X-pad to move sprite");
	print_string(1, 14, "Y-pad to scroll background");
	print_string(1, 15, "Start to change spr prio");
	print_string(1, 16, "A to toggle BG autoscroll");

	// this one is off-screen; 0x11 is a heart in the font tileset <3
	print_string(1, 18, "February 2026 by xdaniel \x11");
}
