/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "backends/platform/libretro/include/libretro-global-options.h"

#ifdef LRHARD
#include <atomic>
#include "common/config-manager.h"
#include "common/language.h"
#include "common/translation.h"
#include "engines/engine.h"
#ifdef USE_FREETYPE2
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"
#endif
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-threads.h"

namespace {

struct GlobalOption {
	const char *key;
	const char *setting;
	const char *defaultValue;
	bool requiresRestart;
	retro_core_option_value values[RETRO_NUM_CORE_OPTION_VALUES_MAX];
	Common::String selected;
};

#define LR_GLOBAL_OPTION(key, setting, title, info, category, defaultValue, ...) \
	{"scummvm_global_" key, setting, defaultValue, true, {__VA_ARGS__}, Common::String()},
#define LR_GLOBAL_LIVE_OPTION(key, setting, title, info, category, defaultValue, ...) \
	{"scummvm_global_" key, setting, defaultValue, false, {__VA_ARGS__}, Common::String()},
GlobalOption options[] = {
#include "backends/platform/libretro/include/libretro-global-options.inc"
};
#undef LR_GLOBAL_LIVE_OPTION
#undef LR_GLOBAL_OPTION

std::atomic<unsigned> fluidEffects(RETRO_FLUID_CHORUS | RETRO_FLUID_REVERB);
std::atomic<bool> mt32LcdEnabled(false);

Common::String reportedPaths;
Common::String requestedGuiLanguage;
Common::String reportedGui;
bool warnedGuiFonts = false;

} // namespace
#endif

unsigned retro_global_options_fluid_effects() {
#ifdef LRHARD
	return fluidEffects.load(std::memory_order_relaxed);
#else
	return RETRO_FLUID_CHORUS | RETRO_FLUID_REVERB;
#endif
}

bool retro_global_options_mt32_lcd_enabled() {
#ifdef LRHARD
	return mt32LcdEnabled.load(std::memory_order_relaxed);
#else
	return true;
#endif
}

void retro_global_options_update(retro_environment_t environment) {
#ifdef LRHARD
	bool changed = false;
	bool soundChanged = false;
	unsigned effects = 0;
	bool lcdEnabled = false;
	for (auto &option : options) {
		retro_variable variable = {option.key, nullptr};
		const char *value = option.defaultValue;
		if (environment(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value) {
			for (const auto &allowed : option.values) {
				if (!allowed.value)
					break;
				if (!strcmp(allowed.value, variable.value)) {
					value = variable.value;
					break;
				}
			}
		}
		if (!option.selected.empty() && option.selected != value) {
			if (option.requiresRestart)
				changed = true;
			else if (retro_emu_thread_started()) {
				ConfMan.set(option.setting, value, Common::ConfigManager::kSessionDomain);
				if (!strcmp(option.setting, "autosave_period") && g_engine)
					g_engine->setAutosaveInterval(ConfMan.getInt("autosave_period"));
				if (!strcmp(option.setting, "music_volume") ||
					!strcmp(option.setting, "sfx_volume") ||
					!strcmp(option.setting, "speech_volume"))
					soundChanged = true;
			}
		}
		if (!strcmp(option.setting, "fluidsynth_chorus_activate") && !strcmp(value, "true"))
			effects |= RETRO_FLUID_CHORUS;
		if (!strcmp(option.setting, "fluidsynth_reverb_activate") && !strcmp(value, "true"))
			effects |= RETRO_FLUID_REVERB;
		if (!strcmp(option.setting, "mt32_lcd"))
			lcdEnabled = !strcmp(value, "true");
		option.selected = value;
	}
	fluidEffects.store(effects, std::memory_order_relaxed);
	mt32LcdEnabled.store(lcdEnabled, std::memory_order_relaxed);
	if (soundChanged && g_engine)
		g_engine->syncSoundSettings();
	if (changed && retro_emu_thread_started())
		retro_osd_notification("Restart content to apply ScummVM global options.", RETRO_LOG_INFO);
#endif
}

void retro_global_options_init() {
#ifdef LRHARD
	reportedPaths.clear();
	requestedGuiLanguage.clear();
	reportedGui.clear();
	warnedGuiFonts = false;
	// Session settings override game/global INI values without being persisted.
	// Native game options
	// can still replace them through ConfigManager's normal setters.
	for (const auto &option : options) {
		const Common::String value = option.selected.empty() ? option.defaultValue : option.selected;
		if (!strcmp(option.setting, "text_and_speech")) {
			if (value == "inherit") {
				ConfMan.removeKey("subtitles", Common::ConfigManager::kSessionDomain);
				ConfMan.removeKey("speech_mute", Common::ConfigManager::kSessionDomain);
			} else {
				ConfMan.setBool("subtitles", value != "speech", Common::ConfigManager::kSessionDomain);
				ConfMan.setBool("speech_mute", value == "subtitles", Common::ConfigManager::kSessionDomain);
			}
		} else if (value == "inherit") {
			ConfMan.removeKey(option.setting, Common::ConfigManager::kSessionDomain);
		} else {
			ConfMan.set(option.setting, value, Common::ConfigManager::kSessionDomain);
		}
	}
#ifndef DISABLE_FANCY_THEMES
	ConfMan.set("gui_renderer", "antialias", Common::ConfigManager::kSessionDomain);
#else
	ConfMan.set("gui_renderer", "normal", Common::ConfigManager::kSessionDomain);
#endif
	ConfMan.set("gui_theme", "builtin", Common::ConfigManager::kSessionDomain);
	ConfMan.setBool("gui_disable_fixed_font_scaling", true, Common::ConfigManager::kSessionDomain);
	ConfMan.setBool("gui_kinetic_scrolling", true, Common::ConfigManager::kSessionDomain);
	ConfMan.set("gui_language", "en", Common::ConfigManager::kSessionDomain);
	ConfMan.setBool("gui_use_game_language", false, Common::ConfigManager::kSessionDomain);
	ConfMan.setBool("filtering", false, Common::ConfigManager::kSessionDomain);
	ConfMan.setBool("mute", false, Common::ConfigManager::kSessionDomain);
	ConfMan.setBool("gui_return_to_launcher_at_exit", false, Common::ConfigManager::kSessionDomain);
	ConfMan.setBool("confirm_exit", false, Common::ConfigManager::kSessionDomain);
#endif
}

void retro_global_options_log_paths(const char *paths) {
#ifdef LRHARD
	if (retro_log_cb && reportedPaths != paths) {
		retro_log_cb(RETRO_LOG_INFO, "%s", paths);
		reportedPaths = paths;
	}
#endif
}

void retro_global_options_prepare_game(const char *detectedLanguage) {
#ifdef LRHARD
	if (ConfMan.getBool("gui_use_game_language")) {
		// Auto-detected content need not have a language saved in its INI target.
		if (!ConfMan.hasKey("language") && detectedLanguage && *detectedLanguage)
			ConfMan.set("language", detectedLanguage, Common::ConfigManager::kSessionDomain);
		if (ConfMan.hasKey("language")) {
			// Game identifiers (e.g. br/cn/tw) differ from GUI translation locales.
			const Common::Language language = Common::parseLanguage(ConfMan.get("language"));
			const char *locale = Common::getLanguageLocale(language);
			if (language == Common::ZH_CHN)
				locale = "zh_Hans";
			else if (language == Common::ZH_TWN)
				locale = "zh_Hant";
			ConfMan.set("gui_language", locale ? Common::String(locale) : ConfMan.get("language"), Common::ConfigManager::kSessionDomain);
		}
	}
	requestedGuiLanguage = ConfMan.get("gui_language");
#endif
}

void retro_global_options_log_gui() {
#if defined(LRHARD) && defined(USE_TRANSLATION)
	const Common::String requested = requestedGuiLanguage.empty() ? ConfMan.get("gui_language") : requestedGuiLanguage;
	const Common::String effective = TransMan.getCurrentLanguage();
	const Common::String report = Common::String::format("[ScummVM GUI] language requested: %s; effective: %s; use game language: %s; game language: %s\n",
		requested.empty() ? "automatic" : requested.c_str(), effective.c_str(),
		ConfMan.getBool("gui_use_game_language") ? "yes" : "no",
		ConfMan.hasKey("language") ? ConfMan.get("language").c_str() : "unknown");
	if (retro_log_cb && reportedGui != report) {
		retro_log_cb(RETRO_LOG_INFO, "%s", report.c_str());
		if (!requested.empty() && effective == "en" && requested != "en")
			retro_log_cb(RETRO_LOG_WARN, "[ScummVM GUI] Translation unavailable. Check translations.dat and the theme's language fonts in the configured theme/extra paths.\n");
		reportedGui = report;
	}
#endif
}

const Graphics::Font *retro_global_options_gui_font(const char *bitmapFont, int pointSize, bool monospace, bool localizedDefault) {
#if defined(LRHARD) && defined(USE_FREETYPE2)
	Common::String bitmap(bitmapFont);
	bitmap.toLowercase();
	const bool bold = bitmap.contains("helvb") || bitmap.contains("bold");
	Common::String family = "LiberationSans";
	Common::String extension = ".ttf";
#ifdef USE_TRANSLATION
	const Common::String language = TransMan.getCurrentLanguage();
	if (language.hasPrefix("ja")) {
		family = "NotoSansJP";
		extension = ".otf";
	} else if (language.hasPrefix("ko")) {
		family = "NotoSansKR";
		extension = ".otf";
	} else if (language.hasPrefix("zh_Hans")) {
		family = "NotoSansSC";
		extension = ".otf";
	} else if (language.hasPrefix("zh_Hant")) {
		family = "NotoSansTC";
		extension = ".otf";
	} else if (language.hasPrefix("hi")) {
		family = "NotoSans";
	} else if (language.hasPrefix("ar")) {
		family = "NotoNaskhArabic";
	} else if (language.hasPrefix("he")) {
		family = "NotoSansHebrew";
	} else if (language.hasPrefix("ka")) {
		family = "NotoSansGeorgian";
	}
#endif
	const Common::String filename = monospace ? "SourceCodeVariable-Roman.ttf" : family + (bold ? "-Bold" : "-Regular") + extension;
	const Common::String name = Common::String::format("%s@%d", filename.c_str(), pointSize);
	const Graphics::Font *font = FontMan.getFontByName(name);
	if (!font) {
		font = Graphics::loadTTFFontFromArchive(filename, pointSize, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeLight);
		if (font)
			FontMan.assignFontToName(name, font);
	}
	if (font && localizedDefault) {
		FontMan.setLocalizedFont(name);
		if (retro_log_cb)
			retro_log_cb(RETRO_LOG_INFO, "[ScummVM GUI] Scalable font: %s (%d pixels)\n", filename.c_str(), pointSize);
	} else if (!font && !warnedGuiFonts && retro_log_cb) {
		retro_log_cb(RETRO_LOG_WARN, "[ScummVM GUI] Scalable font %s unavailable; keeping the theme's bitmap fallback. Install fonts.dat (and fonts-cjk.dat for CJK) in the configured extra-data path.\n", filename.c_str());
		warnedGuiFonts = true;
	}
	return font;
#else
	return nullptr;
#endif
}
