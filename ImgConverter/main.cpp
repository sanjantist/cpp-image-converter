#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <iostream>
#include <string_view>

#include "bmp_image.h"
using namespace std::literals::string_view_literals;
enum class Format { JPEG, PPM, BMP, UNKNOWN };

Format GetFormatByExtension(const img_lib::Path& input_file) {
  const std::string ext = input_file.extension().string();
  if (ext == ".jpg"sv || ext == ".jpeg"sv) return Format::JPEG;
  if (ext == ".ppm"sv) return Format::PPM;
  if (ext == ".bmp"sv) return Format::BMP;
  return Format::UNKNOWN;
}

class ImageFormatInterface {
 public:
  virtual ~ImageFormatInterface() = default;
  virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
  virtual bool SaveImage(const img_lib::Path& file,
                         const img_lib::Image& image) const = 0;
};

class PPMFormat : public ImageFormatInterface {
 public:
  img_lib::Image LoadImage(const img_lib::Path& file) const override {
    return img_lib::LoadPPM(file);
  }
  bool SaveImage(const img_lib::Path& file,
                 const img_lib::Image& image) const override {
    return img_lib::SavePPM(file, image);
  }
};

class JPEGFormat : public ImageFormatInterface {
 public:
  img_lib::Image LoadImage(const img_lib::Path& file) const override {
    return img_lib::LoadJPEG(file);
  }
  bool SaveImage(const img_lib::Path& file,
                 const img_lib::Image& image) const override {
    return img_lib::SaveJPEG(file, image);
  }
};

class BMPFormat : public ImageFormatInterface {
 public:
  img_lib::Image LoadImage(const img_lib::Path& file) const override {
    return img_lib::LoadBMP(file);
  }
  bool SaveImage(const img_lib::Path& file,
                 const img_lib::Image& image) const override {
    return img_lib::SaveBMP(file, image);
  }
};

ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
  static PPMFormat ppmFormat;
  static JPEGFormat jpegFormat;
  static BMPFormat bmpFormat;
  switch (GetFormatByExtension(path)) {
    case Format::PPM:
      return &ppmFormat;
    case Format::JPEG:
      return &jpegFormat;
    case Format::BMP:
      return &bmpFormat;
    default:
      return nullptr;
  }
}

using namespace std;

int main(int argc, const char** argv) {
  if (argc != 3) {
    cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
    return 1;
  }

  img_lib::Path in_path = argv[1];
  img_lib::Path out_path = argv[2];

  auto* inputIF = GetFormatInterface(in_path);
  if (!inputIF) {
    cerr << "Unknown format of the input file."sv << endl;
    return 2;
  }
  img_lib::Image image = inputIF->LoadImage(in_path);
  if (!image) {
    cerr << "Loading failed"sv << endl;
    return 4;
  }

  auto* outputIF = GetFormatInterface(out_path);
  if (!outputIF) {
    cerr << "Unknown format of the output file."sv << endl;
    return 3;
  }
  if (!outputIF->SaveImage(out_path, image)) {
    cerr << "Saving failed"sv << endl;
    return 5;
  }

  cout << "Successfully converted"sv << endl;
}