#pragma once
#include <filesystem>

#include "img_lib.h"
#include "pack_defines.h"

namespace img_lib {

using Path = std::filesystem::path;

bool SaveBMP(const Path& file, const Image& image);
Image LoadBMP(const Path& file);
}  // namespace img_lib