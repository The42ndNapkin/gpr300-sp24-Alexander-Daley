#pragma once

#include "../ew/external/glad.h"

namespace daley {
	struct Framebuffer {
		unsigned int fbo;
		unsigned int colorBuffer[8];
		unsigned int depthBuffer;
		unsigned int width;
		unsigned int height;
	};
	Framebuffer createFrameBuffer(unsigned int width, unsigned int height, int colorFormat);
}