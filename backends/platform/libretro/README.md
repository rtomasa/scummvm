# ScummVM libretro port
## Building ScummVM core
To build the core with the default configuration, type in a shell the following:
```
git clone https://github.com/scummvm/scummvm
cd scummvm/backends/platform/libretro
make
```
Use `make all` to build the core along with datafiles/themes (`scummvm.zip`) and core info file (which can be built separately with `make datafiles`/`make coreinfo`).

"Work in progress" engines are not built by default, to include them pass `NO_WIP=0` to make.

To crossbuild for specific platforms, pass the relevant `platform` variable to make (refer to Makefile for supported ones).

### Build for Android
To build for Android:
* install android-sdk
* install android-ndk (e.g. through sdkmanager included in android-sdk/cmdline-tools)
* make sure all needed sdk/ndk paths are included in PATH
* type in a shell the following:
```
git clone https://github.com/scummvm/scummvm
cd scummvm/backends/platform/libretro/jni
ndk-build
```
Core will be built for all available android targets by default

### Data files and themes
[Data files](https://wiki.scummvm.org/index.php/Datafiles) required for certain games and core functions (e.g. virtual keyboard) and default [ScummVM GUI themes](https://wiki.scummvm.org/index.php/GUI_Themes) are bundled during the build process in `scummvm.zip` file. Libretro core info file is created during build as well.
Extract `scummvm.zip` and select relevant paths in ScummVM GUI (or modify `scummvm.ini` accordingly) to load properly needed data files/themes in the core.

Note that both datafiles and themes included in `scummvm.zip` need to be consistent with ScummVM version in use, hence need to be generally rebuilt and replaced for each new version.

## Native frontend integration (`LRHARD=1`)

`LRHARD` builds keep ScummVM's native Save, Load and game-specific Options
dialogs, and hide Help, About, Return to Launcher, Quit and Global Options. The fallback
launcher's Global Options entry is hidden too. Its Backend playlist generator
is not exposed in this mode; use the frontend's library management instead.

The native in-game Options dialog retains game-specific settings, keymaps and
any engine-provided tabs. Its Audio tab is omitted in LRHARD builds, including
Mute all; volume, text/speech mode and subtitle speed use the core options.

Failed content detection or loading returns an error to the frontend instead
of opening the ScummVM launcher. Errors after engine startup are reported through
the frontend and finish normal ScummVM cleanup before requesting shutdown.
Return to launcher on exit is disabled. LRHARD builds do not advertise no-content
support and reject null-content loads, so empty frontend launch files cannot
open the launcher. Standard builds retain no-content launcher support.

Native global settings are available through core options in Video, Audio and
subtitles, MIDI and MT-32, ScummVM interface, and System categories. Native
configuration options marked "restart" require restarting content. System is the first category, followed by Video.
There is no separate Timing category. Older frontends receive the same settings
through the existing v1/v0 fallback, without categories. Missing translations
use the English definitions.

For options offering **ScummVM CFG**, that default leaves existing global and game-specific
settings in control. Explicit frontend selections override them for the
session, without writing those selections to `scummvm.ini`. Native game
Options can still change settings using ScummVM's usual behavior. Disable
fixed font scaling defaults to enabled. Exit confirmation and return to the
launcher on game exit are disabled in this mode.

Video options use explicit defaults: game aspect ratio correction is disabled
and legacy graphics mode is Automatic. They do not offer ScummVM CFG.
Graphics filtering is disabled for the session and its core option is omitted.
The frame rate cap option is omitted too; the core follows the frontend's
reported refresh rate (60 Hz if unavailable), ignoring any saved cap selection.
With game aspect ratio correction enabled, legacy resolutions recognized by
ScummVM (320x200, 640x400, 640x350, 720x348 and 720x350) are reported as 4:3.
With it disabled, the native width/height ratio is reported. The framebuffer
is never resized for this correction; the frontend performs the scaling.
Native dialogs and the launcher keep their configured GUI aspect ratio.
Game aspect ratio correction applies immediately without restarting content.
If changed while a native dialog is open, the selected game ratio applies when
the dialog closes. Legacy graphics mode still requires restarting content.
Hardware rendering synchronizes geometry updates before drawing, since a
frontend may clear or replace its framebuffer in the geometry callback.
The GL pipeline is deactivated before handing control to the frontend so the
next draw restores the core's viewport, projection, shader and framebuffer.

Audio and subtitles options use explicit defaults without ScummVM CFG: Music
device and AdLib emulator are Automatic, music volume is 100% (255), effects
and speech volumes are 80% (204), Text and speech is Both, and subtitle speed
is 60. Volume choices use 10% steps; the old 75% choice is omitted. Music,
effects and speech volume changes apply immediately without restarting content.
The other Audio and subtitles options still require restarting content. Mute all
audio is disabled for the session and its core option is omitted. The core
sample rate is fixed at 48000 Hz, ignoring any saved sample-rate option, and
its option is omitted too.

MIDI and MT-32 options use explicit defaults without ScummVM CFG: General MIDI
device is Automatic, MT-32 device is MT-32 emulator, Mixed AdLib/MIDI mode is
enabled, True Roland MT-32 and Roland GS mappings are disabled, and MIDI gain
is 1.00 (100). Builds without the MT-32 emulator default its device to Automatic.
MT-32 emulator builds expose MT-32 LCD messages, disabled by default. This toggle
applies live to new onscreen LCD messages without changing MT-32 audio.
FluidSynth builds also expose chorus and reverb toggles, both enabled by default.
These two toggles apply live to active synthesizers during audio generation,
without restarting content. Detailed parameters are initialized even when an
effect starts disabled, so enabling it retains its configured sound. Other MIDI
and MT-32 options require restarting content. Existing SoundFont paths and detailed
chorus/reverb parameters are retained.

The System category exposes Startup splash screen and Native autosave interval.
Startup splash screen defaults to Disabled; enabling restores the orange ScummVM
logo and its 600 ms display delay on the next content load. Game intros and
error dialogs are unaffected. Native autosave interval defaults to
5 minutes, with Disabled, 5, 10, 15 and 30 minute choices and no ScummVM CFG
value. Changes apply live and restart the running engine's autosave timer;
disabling prevents subsequent native autosaves. Engine support and existing
save-slot protection are unchanged. Browsing mode is hidden in LRHARD builds,
which use local filesystem browsing except on Android, where authorized storage
remains the default. Saved browsing-mode choices are ignored in these builds.

The ScummVM interface category exposes only Scale, Aspect ratio and Resolution.
Scale defaults to 175%, with 100%, 125%, 150%, 175% and 200% choices. Aspect ratio defaults
to 16:9 and Resolution to HD (1280x720 at 16:9). These options have no ScummVM
CFG choice and apply without restarting content: scale updates on the next GUI
refresh, and aspect ratio/resolution apply when opening a dialog. RePlay's LCD
and CRT profiles use the same aspect ratio and resolution defaults.

The Antialiased renderer, disabled fixed font scaling, enabled kinetic scrolling,
English interface language and disabled game-language override are fixed for the
session and are not exposed as core options. Builds without fancy-theme support
use the Standard renderer. Antialiased rendering smooths widget edges and uses
scalable fonts, including for the built-in Classic theme. Install fonts.dat under
scummvm/extra in the frontend system directory. Missing fonts retain the theme's
normal fallback. The built-in Classic theme uses charcoal panels, off-white text and blue filled
button highlights with dark labels. The built-in theme is enforced and saved external-theme
selections are ignored. Existing 3D renderer configuration is retained;
fullscreen and shader paths remain frontend settings.

The backend switches output geometry when a dialog opens and restores the
engine's original geometry when it closes; the game surface is not resized.
Native GUI overlays and hover tooltips are opaque. Only the top native dialog
is drawn; closing a nested dialog redraws its parent. No external theme archive
or theme directory is required. Explicit session settings
override saved INI values without changing them.

Path resolution and existing path overrides are unchanged. Effective config,
game, save, theme, extra-data, SoundFont, icon, browser and playlist paths are
logged at `RETRO_LOG_INFO` at startup and when they change, prefixed with
`[ScummVM paths]`, for inclusion in issue reports. Advanced settings without a
core option (for example custom SoundFonts, FluidSynth tuning, arbitrary random
seeds and keymaps) retain their existing ScummVM configuration.

The option list and configuration adapter live in
`include/libretro-global-options.inc` and `src/libretro-global-options.cpp`.
Keep native settings behavior in this backend adapter when updating upstream;
shared hooks cover menu visibility, dialog stacking, font selection and game-language startup. Non-`LRHARD`
builds retain upstream menus and do not register these extra core options.

For a single ARM64 build compatible with Raspberry Pi 3, 4 and 5, use
`platform=rpi3_64 LRHARD=1`. This selects Cortex-A53-compatible instructions
and OpenGLES2. `rpi4_64` and `rpi5_64` select newer CPUs and should not be used
for a binary intended to run on Pi 3. A 32-bit operating system requires a
separate 32-bit build. Keep the same engine selection (for example `LITE=1`)
when updating an existing installation. Use a toolchain/sysroot matching the
target operating system; RePlayOS 2 uses Debian Trixie. CPU compatibility alone
does not ensure that a binary linked against a newer host libc can load there.

## Libretro playlists for ScummVM core

Playlists used in Libretro frontends (e.g. Retroarch) are plain text lists used to directly launch a game with a specific core from the user interface. Those lists are structured to pass to the core the path of a specific content file to be loaded (e.g. a ROM zip fiile). Detailed info can be found in [Libretro documentation](https://docs.libretro.com/guides/roms-playlists-thumbnails/).

> WARNING: ScummVM core playlist generation with Retroarch Scanner is **NOT recommended** as it is based on a third party database instead of ScummVM game detection system, hence it is not guaranteed to work properly.

ScummVM core can accept as content the playlist-provided path to any of the files inside a valid game folder, the ScummVM internal detection system will try to autodetect the game from the content file parent folder and run the game with default ScummVM options.

The core also supports dedicated per game **hook** plain text files with `.scummvm` or `.svm` extension, which behave identically and can be used as target paths in the playlist. An empty file or a whitespace-only first line triggers game detection in the hook file's parent folder. Otherwise, the first line specifies one of the following ScummVM identifiers:

  - **target**: this is the game identifier of each entry in the internal Launcher list, hence corresponding to entries in ScummVM configuration file (e.g. 'scummvm.ini'). In this case the game must be added from ScummVM GUI first, and the hook files can be placed anywhere, as the path for the game files is already part of the target configuration. The game will be launched with the options set in ScummVM

  - **game ID**: this is a unique identifier for any game supported by ScummVM. This identifier is hard coded in each engine source and can be subject to change, hence it is **not a recommended choice**. A list of current game IDs is available [here](https://scummvm.org/compatibility). In this case the game will be launched even if not added in ScummVM Launcher, the hook file must be placed in the game folder and the game will be launched with default ScummVM options

Game IDs can be specified on their own (for example `monkey`) or qualified with an engine ID (`scumm:monkey`) to resolve ambiguity. A matching target in `scummvm.ini` takes precedence over a game ID.

### Loading ZIP games

The core accepts `.zip` and `.zsvm` files; both must contain a ZIP archive.
The archive is passed intact to the core and exposed as a read-only game
folder, without extracting files to disk. Detection uses the contents, not
the archive filename. Place one game's data at the archive root or inside a
single containing folder (up to 16 nested containing folders are supported).
Preserve the game's subdirectory structure. Archives with several separate
game folders are not searched recursively.

The archive uses the same libretro VFS access as other content. ScummVM's
ZIP reader decompresses accessed members into memory to support seeking.
The backend retains recently accessed files within a 128 MiB cache, plus
one file larger than that budget, to avoid decompressing resources each
time an engine reopens them. These retained buffers are shared with open
streams and released when the archive is unmounted and its nodes are freed.
Large game data files can therefore require substantial RAM. Games that
bypass ScummVM's filesystem or need writable game data may require an
unpacked folder. Encrypted or unsupported ZIP variants are not supported.
Saves continue to use the configured save directory. Engine compatibility
must be verified with actual games.

### Creating ScummVM core playlist
The recommended way to generate a ScummVM core playlist is from within the ScummVM Launcher (i.e. the interal core GUI) with the Playlist Generator tool. Both needed hook files and the playlist are created automatically, based on ScummVM Launcher games list.
  - Load the core from RetroArch and start it to reach the ScummVM GUI (i.e. the Launcher)
  - Add games to the list as required using the GUI buttons ('Mass Add' available).
  - Select **Global Options** and then the **Backend** tab.
  - Check or select the path of frontend playlists. A `ScummVM.lpl` file will be created or overwritten in there.
  - Check the 'Hooks location' setting, to have one `.scummvm` in each game folder or all of them in a `scummvm_hooks` folder in the `save` path.
  - Check the 'Playlist version' setting. JSON format should be selected, 6-lines format is deprecated and provided for backwards compatibility only.
  - Select the 'Clear existing hooks' checkbox to remove any existing `.scummvm` file in the working folders.
  - Press the 'Generate playlist' button.

Operation status will be shown in the same dialog, while details will be given in the frontend logs.

## Core options
### Cursor Movement
  - **Pointer Device**: allows switching which device to use to move the cursor.
  - **Gamepad Cursor Speed**: sets the mouse cursor speed multiplier when moving the cursor with the RetroPad left analog stick or D-Pad. The default value of '1.0' is optimised for games that have a native resolution of '320x200' or '320x240'. When running 'high definition' games with a resolution of '640x400' or '640x480', a Gamepad Cursor Speed of '2.0' is recommended.
  - **Gamepad Cursor Acceleration**: the amount of time (In seconds) it takes for the cursor to reach full speed
  - **Analog Cursor Response**: determines how the speed of the cursor varies when tilting the RetroPad left analog stick. 'Linear': Speed is directly proportional to analog stick displacement. This is standard behaviour with which most users will be familiar. 'Quadratic': Speed increases quadratically with analog stick displacement. This allows for greater precision when making small movements without sacrificing maximum speed at full analog range. This mode may require practice for effective use.
  - **Analog Deadzone**: sets the deadzone in percentage of the RetroPad analog sticks. Used to eliminate cursor drift/unwanted input.
  - **Mouse Speed**: sets the mouse cursor speed multiplier when moving the cursor with the RetroMouse.
  - **Mouse Fine Control Speed Reduction**: sets the mouse cursor speed reduction as percentage of normal speed when fine control is activated.
### Video settings
  - **Hardware acceleration**: request video hardware acceleration (OpenGL or OpenGLES2) to the frontend if supported. It is needed to reload the core to apply this setting.
  - **ScummVM Launcher aspect ratio**: set ScummVM Launcher aspect ratio.
  - **ScummVM Launcher resolution**: set ScummVM Launcher aspect ratio.
### Timing
  - **Frame rate cap**: set core frame rate upper limit. Reducing the limit will improve the performance on lower end devices. Changing this setting will reset the core.
  - **Sample rate**: set core sample rate. Reducing the rate will slightly improve the performance on lower end devices. Changing this setting will reset the core.
### RetroPad mapping
Settings to map each RetroPad key to ScummVM controls.

## Android storage access
On modern Android versions apps can only access folders explicitly authorized by the user through the system file picker (Storage Access Framework, SAF).

The core can browse both the standard local filesystem and the folders authorized through the frontend (e.g. RetroArch). The starting location of the ScummVM file browser is controlled by the **Browsing mode** core option.

### Browsing mode
Available in standard builds under the frontend core options (e.g. RetroArch `Quick Menu > Core Options > System`); LRHARD builds use the platform default:

  - **Authorized storage**: the file browser starts from a virtual root listing only the folders authorized through the frontend. Default on Android.
  - **Local filesystem**: the file browser starts from the standard local path.

### Authorizing folders
  - Open the frontend file browser (e.g. in RetroArch, "Load Content") and use the option to open/add a new folder; the system file picker will appear.
  - Grant access to the folder(s) containing your games and exit the file browser (no need to actually select any content at this time). The authorization is persistent across reboots.
  - Start the core; the authorized folders will be listed by the ScummVM file browser when **Browsing mode** is set to `Authorized storage`.

### Notes
  - The authorized folder list is read when the core starts. If you authorize new folders while the core is running, restart (reload) the core to make them available.
  - If **Browsing mode** is `Authorized storage` but no folders have been authorized, the core falls back to the local filesystem and shows a notification.
