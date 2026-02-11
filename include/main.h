#pragma once

#define BG_SCROLL_SPEED 0x0140;

extern volatile uint16_t vbl_ticks;

enum prog_modes
{
	MODE_MAIN_INIT = 0,
	MODE_MAIN_RUN,
	MODE_CYLINDER_INIT,
	MODE_CYLINDER_RUN,
	MODE_MAX
};

extern uint8_t mode;
extern uint8_t do_scroll_bg;
extern uint16_t bg_scroll_position_x, bg_scroll_position_y;
