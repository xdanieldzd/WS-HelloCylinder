#include <wonderful.h>
#include <ws.h>
#include <wse.h>

// project sources
#include "main.h"
#include "utils.h"
#include "math.h"
#include "text.h"
#include "input.h"

// project assets
#include "graphics/background2.h"

// wave parameters
#define SINE_SPRITE_COUNT (WS_SPRITE_MAX_COUNT - 16)
#define SINE_Y_INCREMENT 2
#define SINE_X_INCREMENT (64 * SINE_Y_INCREMENT)
#define NUM_EASTER_EGGS 4

// scanlines for line match interrupt
#define SPLIT_LINE_TOP_TEXT 158
#define SPLIT_LINE_CYLINDER_SCROLL_START 8
#define SPLIT_LINE_BOTTOM_TEXT 135

// number of cylinder colors
#define CYLINDER_COLOR_COUNT 64

// colors for cylinder
const uint16_t colors[CYLINDER_COLOR_COUNT] =
{
	WS_RGB(15,  0, 15), WS_RGB(15, 15,  3), WS_RGB(15, 15,  3), WS_RGB(15, 15,  4),
	WS_RGB(15, 15,  4), WS_RGB(15, 15,  5), WS_RGB(15, 15,  6), WS_RGB(15, 15,  6),
	WS_RGB(15, 15,  7), WS_RGB(15, 15,  7), WS_RGB(15, 15,  8), WS_RGB(15, 15,  8),
	WS_RGB(15, 15,  9), WS_RGB(15, 15,  9), WS_RGB(15, 15, 10), WS_RGB(15, 15, 11),
	WS_RGB(15,  0, 15), WS_RGB(15, 15, 11), WS_RGB(15, 15, 12), WS_RGB(15, 15, 12),
	WS_RGB(15, 15, 13), WS_RGB(15, 15, 13), WS_RGB(15, 15, 14), WS_RGB(15, 15, 14),
	WS_RGB(15, 14, 14), WS_RGB(15, 14, 14), WS_RGB(14, 13, 14), WS_RGB(14, 13, 14),
	WS_RGB(14, 12, 14), WS_RGB(13, 12, 13), WS_RGB(13, 11, 13), WS_RGB(13, 11, 13),
	WS_RGB(15,  0, 15), WS_RGB(12, 10, 13), WS_RGB(12, 10, 13), WS_RGB(12,  9, 13),
	WS_RGB(11,  9, 13), WS_RGB(11,  8, 13), WS_RGB(11,  8, 13), WS_RGB(10,  7, 13),
	WS_RGB(10,  7, 13), WS_RGB(10,  6, 13), WS_RGB(10,  6, 13), WS_RGB( 9,  5, 13),
	WS_RGB( 9,  5, 12), WS_RGB( 9,  5, 11), WS_RGB( 8,  5, 11), WS_RGB( 8,  5, 10),
	WS_RGB(15,  0, 15), WS_RGB( 7,  4, 10), WS_RGB( 7,  4,  9), WS_RGB( 7,  4,  9),
	WS_RGB( 6,  4,  8), WS_RGB( 6,  4,  8), WS_RGB( 6,  4,  7), WS_RGB( 5,  4,  7),
	WS_RGB( 5,  3,  6), WS_RGB( 5,  3,  6), WS_RGB( 4,  3,  5), WS_RGB( 4,  3,  5),
	WS_RGB( 4,  3,  4), WS_RGB( 3,  3,  4), WS_RGB( 3,  3,  3), WS_RGB( 3,  2,  3),
};

// strings
const char __far str_cylinder_top[] = "The Mighty Non-Binary Cylinder!";
const char __far str_cylinder_bottom[] = "My 1st demoscene-ish effect ^_^";

// set to line match interrupt scanlines, used to run different logic per screen area
uint8_t next_line_match_scanline = SPLIT_LINE_TOP_TEXT;

// coarse/fine scroll -- increment by arbitrary amount, then use upper 8 bits for scroll register
uint16_t top_text_scroll_x = 0;
uint16_t bottom_text_scroll_x = 0;
uint16_t cylinder_scroll_y = 0;

// coarse/fine offset -- same as scrolling above
uint16_t sine_offset_x = 0;
uint16_t sine_offset_y = 64;

uint8_t sprites_easter_egg = 1;
uint8_t show_text = 1;

// line match interrupt handler
__attribute__((assume_ss_data, interrupt)) void __far line_int_handler(void)
{
	if (next_line_match_scanline == SPLIT_LINE_TOP_TEXT)
	{
		// screen split 1: top row of text -- no Y scroll, set X scroll to scroll text to the left
		ws_display_scroll_screen2_to(top_text_scroll_x >> 8, 0);

		// set next line match interrupt position & split area
		outportb(WS_DISPLAY_LINE_IRQ_PORT, next_line_match_scanline = SPLIT_LINE_CYLINDER_SCROLL_START);
	}
	else if (next_line_match_scanline == SPLIT_LINE_CYLINDER_SCROLL_START)
	{
		// screen split 2: cylinder area -- no X scroll, set Y scroll to move cylinder up and down
		ws_display_scroll_screen2_to(0, cylinder_scroll_y >> 8);

		// configure display window to start 3 tiles down and end 3 tiles before the edge of the screen (Y=24, height=96)
		ws_display_set_screen2_window(0, WS_DISPLAY_TILE_HEIGHT * 3, WS_DISPLAY_WIDTH_PIXELS, WS_DISPLAY_HEIGHT_PIXELS - (WS_DISPLAY_TILE_HEIGHT * 6));

		// set next line match interrupt position & split area
		outportb(WS_DISPLAY_LINE_IRQ_PORT, next_line_match_scanline = SPLIT_LINE_BOTTOM_TEXT);
	}
	else if (next_line_match_scanline == SPLIT_LINE_BOTTOM_TEXT)
	{
		// screen split 3: bottom row of text -- no Y scroll, set X scroll to scroll text to the right
		ws_display_scroll_screen2_to(bottom_text_scroll_x >> 8, 0);

		// configure display window to cover whole screen
		ws_display_set_screen2_window(0, 0, WS_DISPLAY_WIDTH_PIXELS, WS_DISPLAY_HEIGHT_PIXELS);

		// set next line match interrupt position & split area
		outportb(WS_DISPLAY_LINE_IRQ_PORT, next_line_match_scanline = SPLIT_LINE_TOP_TEXT);
	}

	// acknowledge interrupt
	ws_int_ack(WS_INT_ACK_LINE_MATCH);

	// enable interrupts
	ia16_enable_irq();
}

void print_text(void)
{
	if (show_text)
	{
		// if text is enabled, print it
		print_string_static(8, 0, str_cylinder_top);
		print_string_static(20, 17, str_cylinder_bottom);
	}
	else
	{
		// else, clear both lines of the tilemap
		ws_screen_fill_tiles(&wse_screen2, 0x020 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(0), 0, 0, WS_SCREEN_WIDTH_TILES, 1);
		ws_screen_fill_tiles(&wse_screen2, 0x020 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(0), 0, 17, WS_SCREEN_WIDTH_TILES, 1);
	}
}

void init_mode_cylinder(void)
{
	// disable LCD output
	ws_lcd_control_disable();

	// enable screen 1, screen 2, screen 2 window, sprites
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE | WS_DISPLAY_CTRL_SCR2_WIN_INSIDE | WS_DISPLAY_CTRL_SPR_ENABLE);

	// set line match interrupt handler & set first line match interrupt position
	ws_int_set_handler(WS_INT_LINE_MATCH, line_int_handler);
	outportb(WS_DISPLAY_LINE_IRQ_PORT, next_line_match_scanline);

	wait_for_vblank();

	// fill screen 2 with spaces
	ws_screen_fill_tiles(&wse_screen2, 0x020 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(15), 0, 0, WS_SCREEN_WIDTH_TILES, WS_SCREEN_HEIGHT_TILES);

	// copy cylinder tiles via WSC GDMA
	ws_gdma_copy(WS_TILE_4BPP_MEM(0x180), gfx_background2_tiles, gfx_background2_tiles_size);

	// fill tilemap with gradient tiles w/ correct attributes
	ws_screen_fill_tiles(&wse_screen2, 0x180 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(4), 0, 5, WS_SCREEN_WIDTH_TILES, 1);
	ws_screen_fill_tiles(&wse_screen2, 0x181 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(4), 0, 6, WS_SCREEN_WIDTH_TILES, 1);
	ws_screen_fill_tiles(&wse_screen2, 0x180 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(5), 0, 7, WS_SCREEN_WIDTH_TILES, 1);
	ws_screen_fill_tiles(&wse_screen2, 0x181 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(5), 0, 8, WS_SCREEN_WIDTH_TILES, 1);
	ws_screen_fill_tiles(&wse_screen2, 0x180 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(6), 0, 9, WS_SCREEN_WIDTH_TILES, 1);
	ws_screen_fill_tiles(&wse_screen2, 0x181 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(6), 0, 10, WS_SCREEN_WIDTH_TILES, 1);
	ws_screen_fill_tiles(&wse_screen2, 0x180 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(7), 0, 11, WS_SCREEN_WIDTH_TILES, 1);
	ws_screen_fill_tiles(&wse_screen2, 0x181 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(7), 0, 12, WS_SCREEN_WIDTH_TILES, 1);

	// copy cylinder colors to palettes 4-7
	ws_gdma_copy(WS_DISPLAY_COLOR_MEM(4), &colors[0], sizeof(uint16_t) * 16);
	ws_gdma_copy(WS_DISPLAY_COLOR_MEM(5), &colors[16], sizeof(uint16_t) * 16);
	ws_gdma_copy(WS_DISPLAY_COLOR_MEM(6), &colors[32], sizeof(uint16_t) * 16);
	ws_gdma_copy(WS_DISPLAY_COLOR_MEM(7), &colors[48], sizeof(uint16_t) * 16);

	// set sprite table address to 2nd default
	ws_display_set_sprite_address(&wse_sprites2);

	// initialize sprites
	ws_sprite_t* cylinder_sprite = NULL;
	uint8_t i, j;

	for (i = 0, j = SINE_SPRITE_COUNT - 1; i < SINE_SPRITE_COUNT; i++, j--)
	{
		cylinder_sprite = &wse_sprites2.entry[j];
		cylinder_sprite->x = (i << 1) - 2;
	}
	// set sprite count
	outportb(WS_SPR_COUNT_PORT, SINE_SPRITE_COUNT);

	// print text once
	print_text();

	// enable LCD output
	ws_lcd_control_enable();

	// next mode is main cylinder loop
	mode = MODE_CYLINDER_RUN;
}

void run_mode_cylinder(void)
{
	// update sprites
	ws_sprite_t* cylinder_sprite = NULL;
	uint8_t i, j;

	for (i = 0; i < SINE_SPRITE_COUNT; i++)
	{
		j = ((sine_offset_x >> 8) + (i << 3)) & 0xFF;
		cylinder_sprite = &wse_sprites2.entry[i];
		cylinder_sprite->y = (sin(j) >> 1) + 4;
		cylinder_sprite->attr = (0x010 + (i % sprites_easter_egg)) | WS_SPRITE_ATTR_PALETTE(15);

		// sprites appear in front of cylinder
		if (j > 64 && j < 192)
			cylinder_sprite->attr |= WS_SPRITE_ATTR_PRIORITY;
	}

	// A button: toggle text, then print it once
	if (buttons_pressed_now & WS_KEY_A)
	{
		show_text = !show_text;
		print_text();
	}

	if (buttons_pressed_now & WS_KEY_START)
	{
		sprites_easter_egg++;
		if (sprites_easter_egg > NUM_EASTER_EGGS) sprites_easter_egg = 1;
	}

	// update various variables
	sine_offset_x += SINE_X_INCREMENT;
	sine_offset_y += SINE_Y_INCREMENT;

	cylinder_scroll_y = 0x1000 - (sin(sine_offset_y) << 5);
	top_text_scroll_x += 96;
	bottom_text_scroll_x -= 96;
}
