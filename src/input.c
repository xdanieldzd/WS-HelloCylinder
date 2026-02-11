#include <wonderful.h>
#include <ws.h>

// buttons down at last update
uint16_t buttons_last = 0;

// buttons held down now
uint16_t buttons_held_now = 0;
// buttons pressed now
uint16_t buttons_pressed_now = 0;
// buttons pressed at last update
uint16_t buttons_pressed_last = 0;

void update_inputs(void)
{
	buttons_held_now = ws_keypad_scan();
	buttons_pressed_now = (buttons_held_now ^ buttons_last) & buttons_held_now;

	if (buttons_pressed_now != 0)
		buttons_pressed_last = buttons_pressed_now;

	buttons_last = buttons_held_now;
}
