/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Shared by the frontend definitions and the native configuration adapter.
 * Define LR_GLOBAL_OPTION and LR_GLOBAL_LIVE_OPTION before including this file.
 */

#define LR_GLOBAL_INHERIT {"inherit", "ScummVM CFG"}
#define LR_GLOBAL_END {nullptr, nullptr}
#define LR_GLOBAL_BOOL(key, setting, title, info, category, defaultValue) \
	LR_GLOBAL_OPTION(key, setting, title, info, category, defaultValue, \
		LR_GLOBAL_INHERIT, {"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
#define LR_GLOBAL_VOLUME(key, setting, title, defaultValue) \
	LR_GLOBAL_LIVE_OPTION(key, setting, title, "Set the native mixer volume. Applied immediately.", "audio", defaultValue, \
		{"0", "0%"}, {"26", "10%"}, {"51", "20%"}, \
		{"77", "30%"}, {"102", "40%"}, {"128", "50%"}, {"153", "60%"}, \
		{"179", "70%"}, {"204", "80%"}, {"230", "90%"}, {"255", "100%"}, LR_GLOBAL_END)

LR_GLOBAL_LIVE_OPTION("aspect_ratio", "aspect_ratio", "Game aspect ratio correction",
	"Report 4:3 for supported legacy game resolutions when enabled, or the native pixel ratio when disabled. The frontend scales the image; framebuffer dimensions remain unchanged.", "video", "false",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
LR_GLOBAL_OPTION("render_mode", "render_mode", "Legacy graphics mode",
	"Select a legacy graphics mode supported by the game. Automatic uses the engine's normal mode for the detected game and platform.", "video", "default",
	{"default", "Automatic"}, {"cga", "CGA"}, {"cgaComp", "CGA composite"},
	{"cgaBW", "CGA monochrome"}, {"ega", "EGA"}, {"vga", "VGA"},
	{"hercGreen", "Hercules green"}, {"hercAmber", "Hercules amber"}, {"amiga", "Amiga"},
	{"fmtowns", "FM-TOWNS"}, {"pc98-256c", "PC-9821 (256 colors)"}, {"pc98-16c", "PC-9801 (16 colors)"},
	{"pc98-8c", "PC-9801 (8 colors)"}, {"2gs", "Apple IIgs"}, {"atari", "Atari ST"},
	{"macintosh", "Macintosh"}, {"macintoshbw", "Macintosh monochrome"}, {"cpc", "Amstrad CPC"},
	{"zx", "ZX Spectrum"}, {"c64", "Commodore 64"}, {"vgaGrey", "VGA grayscale"},
	{"win256c", "Windows (256 colors)"}, {"win16c", "Windows (16 colors)"}, LR_GLOBAL_END)

LR_GLOBAL_OPTION("music_driver", "music_driver", "Music device",
	"Select the preferred emulated music device. Availability also depends on the game and installed ROMs or SoundFont.", "audio", "auto",
	{"auto", "Automatic"}, {"null", "No music device"}, {"adlib", "AdLib"},
	{"pcspk", "PC speaker"}, {"pcjr", "IBM PCjr"}, {"cms", "Creative Music System"},
	{"libretro_midi", "Frontend MIDI output"},
#ifdef USE_FLUIDSYNTH
	{"fluidsynth", "FluidSynth"},
#endif
#ifdef USE_MT32EMU
	{"mt32", "MT-32 emulator"},
#endif
	LR_GLOBAL_END)
LR_GLOBAL_OPTION("opl_driver", "opl_driver", "AdLib emulator",
	"Select the emulator used for AdLib music.", "audio", "auto",
	{"auto", "Automatic"},
#ifndef DISABLE_DOSBOX_OPL
	{"db", "DOSBox"},
#endif
#ifndef DISABLE_NUKED_OPL
	{"nuked", "Nuked"},
#endif
#ifndef DISABLE_MAME_OPL
	{"mame", "MAME (OPL2 only)"},
#endif
	LR_GLOBAL_END)
LR_GLOBAL_VOLUME("music_volume", "music_volume", "Music volume", "255")
LR_GLOBAL_VOLUME("sfx_volume", "sfx_volume", "Effects volume", "204")
LR_GLOBAL_VOLUME("speech_volume", "speech_volume", "Speech volume", "204")
LR_GLOBAL_OPTION("text_and_speech", "text_and_speech", "Text and speech",
	"Select spoken dialogue, subtitles, or both where the game supports them.", "audio", "both",
	{"speech", "Speech"}, {"subtitles", "Subtitles"}, {"both", "Both"}, LR_GLOBAL_END)
LR_GLOBAL_OPTION("talkspeed", "talkspeed", "Subtitle speed",
	"Set native dialogue text speed. Interpretation depends on the engine; 60 is ScummVM's default.", "audio", "60",
	{"0", nullptr}, {"25", nullptr}, {"50", nullptr}, {"60", nullptr},
	{"75", nullptr}, {"100", nullptr}, {"125", nullptr}, {"150", nullptr}, {"175", nullptr},
	{"200", nullptr}, {"225", nullptr}, {"255", nullptr}, LR_GLOBAL_END)

LR_GLOBAL_OPTION("gm_device", "gm_device", "General MIDI device",
	"Select the preferred General MIDI synthesizer. SoundFont paths keep their existing configuration.", "midi", "auto",
	{"auto", "Automatic"}, {"null", "No General MIDI"}, {"libretro_midi", "Frontend MIDI output"},
#ifdef USE_FLUIDSYNTH
	{"fluidsynth", "FluidSynth"},
#endif
	LR_GLOBAL_END)
#ifdef USE_MT32EMU
#define LR_GLOBAL_MT32_DEFAULT "mt32"
#else
#define LR_GLOBAL_MT32_DEFAULT "auto"
#endif
LR_GLOBAL_OPTION("mt32_device", "mt32_device", "MT-32 device",
	"Select the preferred Roland MT-32 device. The emulator requires compatible ROM files.", "midi", LR_GLOBAL_MT32_DEFAULT,
	{"auto", "Automatic"}, {"null", "No MT-32"}, {"libretro_midi", "Frontend MIDI output"},
#ifdef USE_MT32EMU
	{"mt32", "MT-32 emulator"},
#endif
#ifdef USE_FLUIDSYNTH
	{"fluidsynth", "FluidSynth (GM emulation)"},
#endif
	LR_GLOBAL_END)
#undef LR_GLOBAL_MT32_DEFAULT
#ifdef USE_MT32EMU
LR_GLOBAL_LIVE_OPTION("mt32_lcd", "mt32_lcd", "MT-32 LCD messages",
	"Display the emulated MT-32's LCD messages onscreen. This does not affect music playback.", "midi", "false",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
#endif
LR_GLOBAL_OPTION("multi_midi", "multi_midi", "Mixed AdLib/MIDI mode",
	"Use both MIDI and AdLib sound generation in supported games.", "midi", "true",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
LR_GLOBAL_OPTION("native_mt32", "native_mt32", "True Roland MT-32",
	"Disable GM emulation for a real Roland-compatible device. Usually leave disabled for software synthesis.", "midi", "false",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
LR_GLOBAL_OPTION("enable_gs", "enable_gs", "Roland GS mappings",
	"Enable MT-32 instrument mappings on Roland GS devices.", "midi", "false",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
LR_GLOBAL_OPTION("midi_gain", "midi_gain", "MIDI gain",
	"Set synthesizer gain. 1.00 is ScummVM's default; high gain may cause clipping.", "midi", "100",
	{"0", "0.00"}, {"25", "0.25"}, {"50", "0.50"}, {"75", "0.75"},
	{"100", "1.00"}, {"125", "1.25"}, {"150", "1.50"}, {"200", "2.00"}, {"300", "3.00"},
	{"400", "4.00"}, {"500", "5.00"}, {"750", "7.50"}, {"1000", "10.00"}, LR_GLOBAL_END)

#ifdef USE_FLUIDSYNTH
LR_GLOBAL_LIVE_OPTION("fluidsynth_chorus_activate", "fluidsynth_chorus_activate", "FluidSynth chorus",
	"Enable the chorus effect for FluidSynth music. Other chorus parameters retain their configured values.", "midi", "true",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
LR_GLOBAL_LIVE_OPTION("fluidsynth_reverb_activate", "fluidsynth_reverb_activate", "FluidSynth reverb",
	"Enable the reverb effect for FluidSynth music. Other reverb parameters retain their configured values.", "midi", "true",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
#endif

LR_GLOBAL_LIVE_OPTION("gui_scale", "gui_scale", "Scale",
	"Scale ScummVM's native dialogs. Applied on the next GUI refresh without restarting content.", "interface", "175",
	{"100", "100%"}, {"125", "125%"}, {"150", "150%"}, {"175", "175%"}, {"200", "200%"}, LR_GLOBAL_END)
LR_GLOBAL_OPTION("startup_splash", "libretro_startup_splash", "Startup splash screen",
	"Display the ScummVM logo before starting a game. This does not affect game intros or error messages.", "system", "false",
	{"false", "Disabled"}, {"true", "Enabled"}, LR_GLOBAL_END)
LR_GLOBAL_LIVE_OPTION("autosave_period", "autosave_period", "Native autosave interval",
	"Set ScummVM's autosave interval for supported engines. Changing the interval restarts the timer.", "system", "300",
	{"0", "Disabled"}, {"300", "5 minutes"}, {"600", "10 minutes"},
	{"900", "15 minutes"}, {"1800", "30 minutes"}, LR_GLOBAL_END)

#undef LR_GLOBAL_VOLUME
#undef LR_GLOBAL_BOOL
#undef LR_GLOBAL_END
#undef LR_GLOBAL_INHERIT
