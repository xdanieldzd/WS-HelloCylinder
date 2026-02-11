local process = require("wf.api.v1.process")
local superfamiconv = require("wf.api.v1.process.tools.superfamiconv")

local font = superfamiconv.convert_tileset(
	"font.png",
	superfamiconv.config()
		:mode("wsc"):bpp(4)
		:no_discard():no_flip()
)

process.emit_symbol("gfx_font", font)
process.emit_symbol("gfx_font_palette", font.palette)
