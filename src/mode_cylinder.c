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

// sprites in wave
#define SINE_SPRITE_COUNT (WS_SPRITE_MAX_COUNT - 16)

// scanlines for line match interrupt
#define SPLIT_LINE_1 158
#define SPLIT_LINE_2 8
#define SPLIT_LINE_3 135

// strings
const char __far str_cylinder_top[] = "The Mighty Non-Binary Cylinder!";
const char __far str_cylinder_bottom[] = "My 1st demoscene-ish effect ^_^";

// set to line match interrupt scanlines, used to run different logic per screen area
uint8_t next_line_match_scanline = SPLIT_LINE_1;

// coarse/fine scroll -- increment by arbitrary amount, then use upper 8 bits for scroll register
uint16_t top_text_scroll_x = 0;
uint16_t bottom_text_scroll_x = 0;
uint16_t cylinder_scroll_y = 0;

// coarse/fine offset -- same as scrolling above
uint16_t sine_offset_x = 0;
uint16_t sine_offset_y = 64;

uint8_t show_text = 1;

// line match interrupt handler
__attribute__((assume_ss_data, interrupt)) void __far line_int_handler(void)
{
	if (next_line_match_scanline == SPLIT_LINE_1)
	{
		// screen split 1: top row of text -- no Y scroll, set X scroll to scroll text to the left
		ws_display_scroll_screen2_to(top_text_scroll_x >> 8, 0);

		// set next line match interrupt position & split area
		outportb(WS_DISPLAY_LINE_IRQ_PORT, next_line_match_scanline = SPLIT_LINE_2);
	}
	else if (next_line_match_scanline == SPLIT_LINE_2)
	{
		// screen split 2: cylinder area -- no X scroll, set Y scroll to move cylinder up and down
		ws_display_scroll_screen2_to(0, cylinder_scroll_y >> 8);

		// configure display window to start 3 tiles down and end 3 tiles before the edge of the screen (Y=24, height=96)
		ws_display_set_screen2_window(0, WS_DISPLAY_TILE_HEIGHT * 3, WS_DISPLAY_WIDTH_PIXELS, WS_DISPLAY_HEIGHT_PIXELS - (WS_DISPLAY_TILE_HEIGHT * 6));

		// set next line match interrupt position & split area
		outportb(WS_DISPLAY_LINE_IRQ_PORT, next_line_match_scanline = SPLIT_LINE_3);
	}
	else if (next_line_match_scanline == SPLIT_LINE_3)
	{
		// screen split 3: bottom row of text -- no Y scroll, set X scroll to scroll text to the right
		ws_display_scroll_screen2_to(bottom_text_scroll_x >> 8, 0);

		// configure display window to cover whole screen
		ws_display_set_screen2_window(0, 0, WS_DISPLAY_WIDTH_PIXELS, WS_DISPLAY_HEIGHT_PIXELS);

		// set next line match interrupt position & split area
		outportb(WS_DISPLAY_LINE_IRQ_PORT, next_line_match_scanline = SPLIT_LINE_1);
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
		ws_screen_fill_tiles(&wse_screen2, 0x020 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(4), 0, 0, WS_SCREEN_WIDTH_TILES, 1);
		ws_screen_fill_tiles(&wse_screen2, 0x020 | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(4), 0, 17, WS_SCREEN_WIDTH_TILES, 1);
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

	// copy cylinder tiles, palette and tilemap via WSC GDMA
	ws_gdma_copy(WS_TILE_4BPP_MEM(0x180), gfx_background2_tiles, gfx_background2_tiles_size);
	ws_gdma_copy(WS_SCREEN_COLOR_MEM(1), gfx_background2_palette, gfx_background2_palette_size);
	ws_gdma_copy(&wse_screen2, gfx_background2_map, gfx_background2_map_size);

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
		cylinder_sprite->attr = 0x011 | WS_SPRITE_ATTR_PALETTE(4);

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

	// update various variables
	sine_offset_x += 128;
	sine_offset_y += 2;

	cylinder_scroll_y = 0x1000 - (sin(sine_offset_y) << 5);
	top_text_scroll_x += 96;
	bottom_text_scroll_x -= 96;
}
