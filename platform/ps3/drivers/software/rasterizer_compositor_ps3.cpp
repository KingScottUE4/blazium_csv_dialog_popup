/**************************************************************************/
/*  rasterizer_compositor_ps3.cpp                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/

#include "rasterizer_compositor_ps3.h"

#include "display_server_ps3.h"
#include "drivers/norsx/norsx_framebuffer.h"
#include "drivers/software/canvas_blit_ps3.h"

void RasterizerCompositorPS3::_blit_image_to_framebuffer(const Ref<Image> &p_image, const Rect2i &p_dst) {
	DisplayServerPS3 *ds = DisplayServerPS3::get_singleton();
	if (!ds || p_image.is_null() || p_image->is_empty()) {
		return;
	}

	NoRSXFramebuffer *fb = ds->get_framebuffer();
	if (!fb || !fb->is_initialized()) {
		return;
	}

	CanvasBlitPS3::State state;
	state.pixels = reinterpret_cast<uint32_t *>(fb->get_buffer());
	state.width = NoRSXFramebuffer::WIDTH;
	state.height = NoRSXFramebuffer::HEIGHT;

	Rect2i dst = p_dst;
	if (dst.size.x <= 0 || dst.size.y <= 0) {
		dst = Rect2i(0, 0, NoRSXFramebuffer::WIDTH, NoRSXFramebuffer::HEIGHT);
	}

	const Rect2i src(0, 0, p_image->get_width(), p_image->get_height());
	CanvasBlitPS3::blit_image_rect(state, p_image, dst, src, Color(1, 1, 1, 1), false, false);
}

void RasterizerCompositorPS3::blit_render_targets_to_screen(int p_screen, const BlitToScreen *p_render_targets, int p_amount) {
	for (int i = 0; i < p_amount; i++) {
		const BlitToScreen &blit = p_render_targets[i];
		const Ref<Image> image = texture_storage.get_render_target_image(blit.render_target);
		if (image.is_null()) {
			continue;
		}
		_blit_image_to_framebuffer(image, blit.dst_rect);
	}
}

void RasterizerCompositorPS3::end_frame(bool p_swap_buffers) {
	if (p_swap_buffers) {
		DisplayServer::get_singleton()->swap_buffers();
	}
}
