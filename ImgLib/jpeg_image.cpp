#include "jpeg_image.h"

#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdio.h>

#include <array>
#include <cstddef>  // for std::byte and std::to_integer
#include <fstream>

using namespace std;

namespace img_lib {
struct my_error_mgr {
  struct jpeg_error_mgr pub;
  jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr* my_error_ptr;
METHODDEF(void)
my_error_exit(j_common_ptr cinfo) {
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  (*cinfo->err->output_message)(cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

void SaveScanlineToImage(const JSAMPLE* row, int y, Image& out_image) {
  Color* line = out_image.GetLine(y);
  for (int x = 0; x < out_image.GetWidth(); ++x) {
    const JSAMPLE* pixel = row + x * 3;
    line[x] = Color{byte{pixel[0]}, byte{pixel[1]}, byte{pixel[2]}, byte{255}};
  }
}

Image LoadJPEG(const Path& file) {
  jpeg_decompress_struct cinfo;
  my_error_mgr jerr;

  FILE* infile;
  JSAMPARRAY buffer;
  int row_stride;

#ifdef _MSC_VER
  if ((infile = _wfopen(file.wstring().c_str(), "rb")) == NULL) {
#else
  if ((infile = fopen(file.string().c_str(), "rb")) == NULL) {
#endif
    return {};
  }

  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return {};
  }

  jpeg_create_decompress(&cinfo);

  jpeg_stdio_src(&cinfo, infile);

  (void)jpeg_read_header(&cinfo, TRUE);

  cinfo.out_color_space = JCS_RGB;
  cinfo.output_components = 3;

  (void)jpeg_start_decompress(&cinfo);

  row_stride = cinfo.output_width * cinfo.output_components;

  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE,
                                      row_stride, 1);

  Image result(cinfo.output_width, cinfo.output_height, Color::Black());

  while (cinfo.output_scanline < cinfo.output_height) {
    int y = cinfo.output_scanline;
    (void)jpeg_read_scanlines(&cinfo, buffer, 1);

    SaveScanlineToImage(buffer[0], y, result);
  }

  (void)jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  return result;
}

bool SaveJPEG(const Path& file, const Image& image) {
  jpeg_compress_struct cinfo;
  jpeg_error_mgr jerr;

  FILE* outfile;
#ifdef _MSC_VER
  outfile = _wfopen(file.wstring().c_str(), L"wb");
#else
  outfile = fopen(file.string().c_str(), "wb");
#endif
  if (!outfile) {
    return false;
  }

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo, outfile);

  cinfo.image_width = image.GetWidth();
  cinfo.image_height = image.GetHeight();
  cinfo.input_components = 3;
  cinfo.in_color_space = JCS_RGB;

  jpeg_set_defaults(&cinfo);
  jpeg_start_compress(&cinfo, TRUE);

  int row_stride = image.GetWidth() * 3;
  JSAMPROW row_pointer[1];
  JSAMPLE* row = new JSAMPLE[row_stride];

  while (cinfo.next_scanline < cinfo.image_height) {
    const Color* srcLine = image.GetLine(cinfo.next_scanline);
    for (int x = 0; x < image.GetWidth(); ++x) {
      int idx = x * 3;
      row[idx] = std::to_integer<JSAMPLE>(srcLine[x].r);
      row[idx + 1] = std::to_integer<JSAMPLE>(srcLine[x].g);
      row[idx + 2] = std::to_integer<JSAMPLE>(srcLine[x].b);
    }
    row_pointer[0] = row;
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  delete[] row;

  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  fclose(outfile);

  return true;
}
}  // namespace img_lib
