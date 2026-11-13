/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef LIBRETRO_GLOBAL_OPTIONS_H
#define LIBRETRO_GLOBAL_OPTIONS_H

#include <libretro.h>

namespace Graphics {
class Font;
}

enum {
	RETRO_FLUID_CHORUS = 1,
	RETRO_FLUID_REVERB = 2
};
unsigned retro_global_options_fluid_effects();
bool retro_global_options_mt32_lcd_enabled();

void retro_global_options_update(retro_environment_t environment);
void retro_global_options_init();
void retro_global_options_prepare_game(const char *detectedLanguage);
void retro_global_options_log_gui();
void retro_global_options_log_paths(const char *paths);
const Graphics::Font *retro_global_options_gui_font(const char *bitmapFont, int pointSize, bool monospace, bool localizedDefault);

#endif
