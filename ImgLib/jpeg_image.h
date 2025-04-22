#pragma once

#include <filesystem>

#include "img_lib.h"

namespace img_lib {
using Path = std::filesystem::path;

Image LoadJPEG(const Path& file);
bool SaveJPEG(const Path& file, const Image& image);

}  // namespace img_lib