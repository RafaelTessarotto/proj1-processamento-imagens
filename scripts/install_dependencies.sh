#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update
sudo apt-get install -y build-essential pkg-config cmake ninja-build curl \
    libfreetype-dev libharfbuzz-dev libx11-dev libxext-dev libxrandr-dev \
    libxcursor-dev libxi-dev libxfixes-dev libxss-dev libxtst-dev \
    libxkbcommon-dev libgl1-mesa-dev libegl1-mesa-dev

build_dir=$(mktemp -d /tmp/compvis-sdl.XXXXXX)

build_library() {
    local repository="$1" archive="$2" version="$3" package="$4"
    shift 4
    if pkg-config --atleast-version="$version" "$package"; then
        printf '%s ja instalado: ' "$package"
        pkg-config --modversion "$package"
        return
    fi
    curl -fL --retry 2 \
        "https://github.com/libsdl-org/$repository/releases/download/release-$version/$archive-$version.tar.gz" \
        -o "$build_dir/$archive.tar.gz"
    tar -xzf "$build_dir/$archive.tar.gz" -C "$build_dir"
    cmake -S "$build_dir/$archive-$version" -B "$build_dir/$archive-build" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local "$@"
    cmake --build "$build_dir/$archive-build" --parallel 4
    sudo cmake --install "$build_dir/$archive-build"
    sudo ldconfig
}

build_library SDL SDL3 3.4.16 sdl3 -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
build_library SDL_image SDL3_image 3.4.6 sdl3-image -DSDLIMAGE_SAMPLES=OFF -DSDLIMAGE_TESTS=OFF
build_library SDL_ttf SDL3_ttf 3.2.2 sdl3-ttf -DSDLTTF_SAMPLES=OFF
pkg-config --modversion sdl3 sdl3-image sdl3-ttf
printf 'Fontes e builds temporarios: %s\n' "$build_dir"
