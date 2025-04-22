#include "bmp_image.h"

#include <array>
#include <fstream>
#include <string_view>

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
  uint16_t bf_type;
  uint32_t bf_size;
  uint32_t bf_reserved;
  uint32_t bf_off_bits;
}
PACKED_STRUCT_END;

PACKED_STRUCT_BEGIN BitmapInfoHeader {
  uint32_t bi_size;
  int32_t bi_width;
  int32_t bi_height;
  uint16_t bi_planes;
  uint16_t bi_bit_count;
  uint32_t bi_compression;
  uint32_t bi_size_image;
  int32_t bi_x_pels_per_meter;
  int32_t bi_y_pels_per_meter;
  uint32_t bi_clr_used;
  uint32_t bi_clr_important;
}
PACKED_STRUCT_END;

static int GetBMPStride(int w) { return 4 * ((w * 3 + 3) / 4); }
bool SaveBMP(const Path& file, const Image& image) {
  int stride = GetBMPStride(image.GetWidth());
  uint32_t data_size = stride * image.GetHeight();

  std::ofstream ofs(file, std::ios::binary);
  if (!ofs) {
    return false;
  }

  BitmapFileHeader file_header;
  file_header.bf_type = static_cast<uint16_t>('B') | static_cast<uint16_t>('M')
                                                         << 8;
  file_header.bf_reserved = 0;
  file_header.bf_off_bits = sizeof(file_header) + sizeof(BitmapInfoHeader);
  file_header.bf_size = file_header.bf_off_bits + data_size;

  BitmapInfoHeader info_header;
  info_header.bi_size = sizeof(info_header);
  info_header.bi_width = image.GetWidth();
  info_header.bi_height = image.GetHeight();
  info_header.bi_planes = 1;
  info_header.bi_bit_count = 24;
  info_header.bi_compression = 0;
  info_header.bi_size_image = data_size;
  info_header.bi_x_pels_per_meter = 11811;
  info_header.bi_y_pels_per_meter = 11811;
  info_header.bi_clr_used = 0;
  info_header.bi_clr_important = 0x1000000;

  ofs.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
  ofs.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

  std::vector<char> row(stride);
  for (int y = image.GetHeight() - 1; y >= 0; --y) {
    const Color* line = image.GetLine(y);
    for (int x = 0; x < image.GetWidth(); ++x) {
      int idx = x * 3;
      row[idx + 0] = static_cast<char>(line[x].b);
      row[idx + 1] = static_cast<char>(line[x].g);
      row[idx + 2] = static_cast<char>(line[x].r);
    }
    for (int i = image.GetWidth() * 3; i != stride; ++i) {
      row[i] = 0;
    }
    ofs.write(row.data(), stride);
  }

  return true;
}

Image LoadBMP(const Path& file) {
  std::ifstream ifs(file, std::ios::binary);

  BitmapFileHeader file_header;
  ifs.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));

  BitmapInfoHeader info_header;
  ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

  ifs.seekg(file_header.bf_off_bits, std::ios::beg);

  Image result(info_header.bi_width, info_header.bi_height, Color::Black());

  int stride = GetBMPStride(info_header.bi_width);
  std::vector<std::byte> row(stride);
  for (int y = 0; y != info_header.bi_height; ++y) {
    ifs.read(reinterpret_cast<char*>(row.data()), stride);
    Color* line = result.GetLine(info_header.bi_height - y - 1);
    for (int x = 0; x < info_header.bi_width; ++x) {
      int idx = x * 3;
      line[x].b = row[idx + 0];
      line[x].g = row[idx + 1];
      line[x].r = row[idx + 2];
    }
  }

  return result;
}

}  // namespace img_lib
