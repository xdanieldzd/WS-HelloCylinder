// Hello WonderSwan World (aka The Mighty Non-Binary Cylinder! [aka 2-bettertest])
// February 2026 by xdaniel
// https://bsky.app/profile/xdaniel.neocities.org

#include <wonderful.h>
#include <ws.h>
#include <wse.h>

// project sources
#include "main.h"
#include "utils.h"
#include "input.h"
#include "mode_main.h"
#include "mode_cylinder.h"

// project assets
#include "graphics/font.h"
#include "graphics/background.h"

volatile uint16_t vbl_ticks;

uint8_t mode;
uint8_t do_scroll_bg;
uint16_t bg_scroll_position_x, bg_scroll_position_y;

// vblank handler
__attribute__((assume_ss_data, interrupt)) void __far vblank_int_handler(void)
{
	vbl_ticks++;

	// scroll background if enabled
	if (do_scroll_bg) 
	{
		bg_scroll_position_x += BG_SCROLL_SPEED;
		bg_scroll_position_y += BG_SCROLL_SPEED;
	}
	ws_display_scroll_screen1_to(bg_scroll_position_x >> 8, bg_scroll_position_y >> 8);

	// acknowledge interrupt
	ws_int_ack(WS_INT_ACK_VBLANK);

	// enable interupts
	ia16_enable_irq();
}

void init_system(void)
{
	// set system to 4bpp color more
	ws_system_set_mode(WS_MODE_COLOR_4BPP);

	// set vblank interrupt handler to function above
	ws_int_set_handler(WS_INT_VBLANK, vblank_int_handler);
}

void init_video(void)
{
	// disable LCD output
	ws_lcd_control_disable();

	// set screen addresses to defaults
	ws_display_set_screen_addresses(&wse_screen1, &wse_screen2);

	// reset scrolling to 0
	ws_display_scroll_screen1_to(0, 0);
	ws_display_scroll_screen2_to(0, 0);

	// enable screen 1, screen 2, sprites
	ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE | WS_DISPLAY_CTRL_SCR2_ENABLE | WS_DISPLAY_CTRL_SPR_ENABLE);

	// enable LCD output
	ws_lcd_control_enable();
}

void main(void)
{
	// disable interrupts
	disable_interrupts();

	// initialize system and display via functions above
	init_system();
	init_video();

	// copy font tiles and palette via WSC GDMA
	ws_gdma_copy(WS_TILE_4BPP_MEM(0x000), gfx_font_tiles, gfx_font_tiles_size);
	ws_gdma_copy(WS_DISPLAY_COLOR_MEM(15), gfx_font_palette, gfx_font_palette_size);

	// copy background tiles and palette via WSC GDMA
	ws_gdma_copy(WS_TILE_4BPP_MEM(0x100), gfx_background_tiles, gfx_background_tiles_size);
	ws_gdma_copy(WS_DISPLAY_COLOR_MEM(0), gfx_background_palette, gfx_background_palette_size);

	// copy background tilemap via WSC GDMA
	ws_gdma_copy(&wse_screen1, gfx_background_map, gfx_background_map_size);

	// set background color to color 0 of background palette (0)
	outportb(WS_DISPLAY_BACK_PORT, 0x00);

	// start in cylinder mode, with BG scrolling enabled
	mode = MODE_CYLINDER_INIT;
	do_scroll_bg = 1;

	// enable interrupts now
	enable_interrupts();

	// main loop
	while (1)
	{
		wait_for_vblank();

		// update button state
		update_inputs();

		// B button: advance mode
		if (buttons_pressed_now & WS_KEY_B)
		{
			mode++;
			if (mode == MODE_MAX)
				mode = MODE_MAIN_INIT;
		}

		// run appropriate function depending on mode
		switch (mode)
		{
			case MODE_MAIN_INIT:
				init_mode_main();
				break;

			case MODE_MAIN_RUN:
				run_mode_main();
				break;

			case MODE_CYLINDER_INIT:
				init_mode_cylinder();
				break;

			case MODE_CYLINDER_RUN:
				run_mode_cylinder();
				break;
		}
	}
}
