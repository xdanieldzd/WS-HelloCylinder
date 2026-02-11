local process = require("wf.api.v1.process")
local superfamiconv = require("wf.api.v1.process.tools.superfamiconv")

local background2 = superfamiconv.convert_tilemap(
	"background2.png",
	superfamiconv.config()
		:mode("wsc"):bpp(4)
		:tile_base(384):palette_base(1)
)

process.emit_symbol("gfx_background2", background2)
