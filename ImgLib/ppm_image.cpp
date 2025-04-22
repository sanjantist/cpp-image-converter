#include "ppm_image.h"

#include <fstream>
#include <ios>
#include <string_view>

using namespace std;

namespace img_lib {

static const string_view PPM_SIG = "P6"sv;
static const int PPM_MAX = 255;

bool SavePPM(const Path& file, const Image& image) {
  ofstream ofs(file, ios::binary);
  if (!ofs) {
    return false;
  }

  const int width = image.GetWidth();
  const int height = image.GetHeight();

  ofs << "P6\n" << width << " " << height << "\n255\n";
  if (!ofs) {
    return false;
  }

  std::vector<char> buffer(static_cast<size_t>(width) * 3);
  for (int y = 0; y < height; ++y) {
    const Color* line = image.GetLine(y);
    for (int x = 0; x < width; ++x) {
      buffer[x * 3 + 0] =
          static_cast<char>(std::to_integer<unsigned int>(line[x].r));
      buffer[x * 3 + 1] =
          static_cast<char>(std::to_integer<unsigned int>(line[x].g));
      buffer[x * 3 + 2] =
          static_cast<char>(std::to_integer<unsigned int>(line[x].b));
    }
    ofs.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    if (!ofs) {
      return false;
    }
  }

  return true;
}

Image LoadPPM(const Path& file) {
  ifstream ifs(file, ios::binary);
  std::string sign;
  int w, h, color_max;
  ifs >> sign >> w >> h >> color_max;

  if (sign != PPM_SIG || color_max != PPM_MAX) {
    return {};
  }

  const char next = ifs.get();
  if (next != '\n') {
    return {};
  }

  Image result(w, h, Color::Black());
  std::vector<char> buff(w * 3);

  for (int y = 0; y < h; ++y) {
    Color* line = result.GetLine(y);
    ifs.read(buff.data(), w * 3);

    for (int x = 0; x < w; ++x) {
      line[x].r = static_cast<byte>(buff[x * 3 + 0]);
      line[x].g = static_cast<byte>(buff[x * 3 + 1]);
      line[x].b = static_cast<byte>(buff[x * 3 + 2]);
    }
  }

  return result;
}
}  // namespace img_lib
