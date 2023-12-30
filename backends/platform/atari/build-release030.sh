#!/bin/bash -eux
# -e: Exit immediately if a command exits with a non-zero status.
# -u: Treat unset variables as an error when substituting.
# -x: Display expanded script commands

mkdir -p build-release030
cd build-release030

PLATFORM=m68k-atari-mintelf

export ASFLAGS="-m68030"
export CXXFLAGS="-m68030 -DDISABLE_FANCY_THEMES"
export LDFLAGS="-m68030"
export PKG_CONFIG_LIBDIR="$(${PLATFORM}-gcc -print-sysroot)/usr/lib/m68020-60/pkgconfig"

if [ ! -f config.log ]
then
../configure \
	--backend=atari \
	--host=${PLATFORM} \
	--enable-release \
	--disable-mt32emu \
	--disable-lua \
	--disable-nuked-opl \
	--disable-16bit \
	--disable-highres \
	--disable-scalers \
	--disable-translation \
	--disable-eventrecorder \
	--disable-tts \
	--disable-bink \
	--opengl-mode=none \
	--enable-verbose-build \
	--enable-text-console \
	--disable-engine=hugo,director,cine
fi

make -j 16
rm -rf dist-generic
make dist-generic

# make memory protection friendly
${PLATFORM}-flags -S dist-generic/scummvm/scummvm.ttp

# create symbol file and strip
${PLATFORM}-nm -C dist-generic/scummvm/scummvm.ttp | grep -vF ' .L' | grep ' [TtWV] ' | ${PLATFORM}-c++filt | sort -u > dist-generic/scummvm/scummvm.sym
${PLATFORM}-strip -s dist-generic/scummvm/scummvm.ttp

# remove unused files
rm -f dist-generic/scummvm/data/*.zip dist-generic/scummvm/data/{gui-icons,achievements,macgui,shaders}.dat

# readme.txt
cp ../backends/platform/atari/readme.txt dist-generic/scummvm
unix2dos dist-generic/scummvm/readme.txt

cd dist-generic
zip -r -9 scummvm-slim.zip scummvm
cd -

mv dist-generic/scummvm-slim.zip ..
