#include <wonderful.h>
#include <ws.h>

void enable_interrupts(void)
{
	// acknowledge all interrupts
    ws_int_ack_all();

	// enable WonderSwan Vblank and line match interrupts
	ws_int_enable(WS_INT_ENABLE_VBLANK | WS_INT_ENABLE_LINE_MATCH);

	// enable CPU interrupt handling
	ia16_enable_irq();
}

void disable_interrupts(void)
{
	// disable all WonderSwan interrupts
	ws_int_disable_all();

	// disable CPU interrupt handling
	ia16_disable_irq();
}

void wait_for_vblank(void)
{
	// halt CPU until next interrupt
	ia16_halt();
}
