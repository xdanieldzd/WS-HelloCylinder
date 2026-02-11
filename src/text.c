#include <stdio.h>
#include <wonderful.h>
#include <ws.h>
#include <wse.h>

// pointer & buffer for printing
const char* ptr;
char buf[512];

// temporary variables
uint8_t x_pos, len;

void print_string(uint8_t x, uint8_t y, const char* fmt, ...)
{
	// handle variadic function arguments & generate string in buffer
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);

	// print string while also clearing current line with spaces
	for (ptr = buf, x_pos = 0, len = 0; (x_pos >= x ? len++ : len) < WS_SCREEN_WIDTH_TILES; x_pos++)
		ws_screen_put_tile(&wse_screen2, (x_pos >= x && *ptr != '\0' ? *ptr++ : 0x020) | WS_SCREEN_ATTR_BANK(0) | WS_SCREEN_ATTR_PALETTE(4), x_pos, y);
}
