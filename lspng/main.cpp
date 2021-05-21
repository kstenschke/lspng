/* lspng
 * Copyright 2021 Kay Stenschke <info@stenschke.com>
 *
 * License GPLv3+: GNU GPL version 3 or later
 * <http://gnu.org/licenses/gpl.html>.
 *
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 *
 * Written by Kay Stenschke, see <https://github.com/kstenschke/grepc>.
 */

#define cimg_use_png 1

#include <cstdlib>
#include <string>
#include <png.h>
#include <vector>
#include <tuple>
#include <lspng/../CImg/CImg.h>

using namespace std;
using namespace cimg_library;

std::vector<std::string> GetPngsInPath(const char *path);
std::string GetFilenameFromPath(const std::string &path);
float GetAvgLuminance(const std::string &path_png);

bool compareAsc(std::tuple<std::string, float> a,
                std::tuple<std::string, float> b);

bool compareDesc(std::tuple<std::string, float> a,
                 std::tuple<std::string, float> b);

int main(int argc, char **argv) {
  uint8_t amount_digits_min = 1;
  bool descending = false;

  if (argc > 1) {
    if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--desc") == 0)
      descending = true;
  }

  std::string path_bin = argv[0];
  path_bin = path_bin.substr(0, path_bin.length() - 5);
  auto png_files = GetPngsInPath(path_bin.c_str());

  std::vector<std::tuple<std::string, float>> tuples_png_and_luminance;

  for (auto const path_png : png_files) {
    auto luminance_total = GetAvgLuminance(path_png);
    auto luminance_total_str = std::to_string(luminance_total);

    if (luminance_total_str.find("-nan") == 0)
      continue;  // determining brightness failed: dont rename

    auto filename = GetFilenameFromPath(path_png);
    tuples_png_and_luminance.emplace_back(
        std::make_tuple(filename, luminance_total));
  }

  std::sort(tuples_png_and_luminance.begin(),
            tuples_png_and_luminance.end(),
            descending ? compareDesc : compareAsc);

  uint32_t index = 0;
  auto amount_digits = std::to_string(png_files.size()).length();

  for (auto const path_png : png_files) {
    auto prefix = std::to_string(index);
    while (prefix.size() < amount_digits) prefix = "0" + prefix;

    auto filename = GetFilenameFromPath(path_png);

    rename(path_png.c_str(),
           std::string(path_bin).append("/").append(prefix)
           .append("_").append(filename).c_str());
    ++index;
  }

  return 0;
}

std::vector<std::string> GetPngsInPath(const char *path) {
  struct dirent **namelist;
  int filecount;

  std::vector<std::string> files;

  filecount = scandir(path, &namelist, nullptr, alphasort);

  if (filecount > 0) {
    for (int i = 0; i < filecount; i++) {
      if (namelist[i]->d_name[0] == '.'
          || std::string(namelist[i]->d_name).find(".png") !=
              strlen(namelist[i]->d_name) - 4
          )
        continue;

      files.push_back(std::string(path) + namelist[i]->d_name);
    }

    while (filecount--) {
      free(namelist[filecount]);
    }

    free(namelist);
  }

  return files;
}

std::string GetFilenameFromPath(const std::string &path) {
  auto offset_slash = path.find_last_of('/');

  return path.substr(offset_slash + 1, path.size() - offset_slash + 1);
}

float GetAvgLuminance(const std::string &path_png) {
  CImg<float> png_img;
  png_img.load_png(path_png.c_str());

  int width = png_img.width();
  int height = png_img.height();

  float luminance_total = 0;

  for (uint32_t x = 0; x < width; ++x) {
    for (uint32_t y = 0; y < height; ++y) {
      auto red = png_img.data(x, y, 0, 0);
      auto green = png_img.data(x, y, 0, 1);
      auto blue = png_img.data(x, y, 0, 2);

      auto luminance_pixel = (.2126 * *red)
          + (0.7152 * *green)
          + 0.0722 * *blue;

      luminance_total += luminance_pixel;
    }
  }

  auto amount_pixels = width * height;

  return luminance_total / amount_pixels;
}

bool compareAsc(std::tuple<std::string, float> a,
                std::tuple<std::string, float> b) {
  float luminance_a =  std::get<1>(a);
  float luminance_b =  std::get<1>(b);

  if (luminance_a < luminance_b) return true;

  if (luminance_a == luminance_b) {
    std::string filename_a = std::get<0>(a);
    std::string filename_b = std::get<0>(b);

    return filename_a < filename_b;
  }

  return false;
}

bool compareDesc(std::tuple<std::string, float> a,
                 std::tuple<std::string, float> b) {
  float luminance_a =  std::get<1>(a);
  float luminance_b =  std::get<1>(b);

  if (luminance_b < luminance_a) return true;

  if (luminance_b == luminance_a) {
    std::string filename_a = std::get<0>(a);
    std::string filename_b = std::get<0>(b);

    return filename_b < filename_a;
  }

  return false;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc30-c"
#pragma clang diagnostic pop
