/* lspng
 * Copyright 2021 Kay Stenschke <info@stenschke.com>
 *
 * License GPLv3+: GNU GPL version 3 or later
 * <http://gnu.org/licenses/gpl.html>.
 *
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 *
 * Written by Kay Stenschke, see <https://github.com/kstenschke/lspng>.
 */

#define cimg_use_png 1

#include <iostream>
#include <cstdlib>
#include <string>
#include <png.h>
#include <vector>
#include <tuple>
#include <lspng/../CImg/CImg.h>

using namespace std;
using namespace cimg_library;

void ParseArguments(uint8_t argc,
                    char *const *argv,
                    bool *descending,
                    uint8_t *amount_digits_min,
                    bool *numeric_only_filenames);

void PrintVersionAndExit();
std::string EvalInCli(const char *command);
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
  bool numeric_only_filenames = false;

  if (argc > 1) ParseArguments(argc,
                               argv,
                               &descending,
                               &amount_digits_min,
                               &numeric_only_filenames);
  auto cwd = EvalInCli("echo $PWD");
  cwd = cwd.substr(0, cwd.length() - 1) + "/";  // remove trailing "\n"
  auto png_files = GetPngsInPath(cwd.c_str());

  std::vector<std::tuple<std::string, float>> tuples_png_and_luminance;

  for (auto const& path_png : png_files) {
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
  if (amount_digits < amount_digits_min) amount_digits = amount_digits_min;

  for (auto const& tuple_png_and_luminance : tuples_png_and_luminance) {
    const auto& path_png = std::get<0>(tuple_png_and_luminance);
    auto prefix = std::to_string(index);

    while (prefix.size() < amount_digits)
      prefix = std::string("0").append(prefix);

    auto filename = GetFilenameFromPath(path_png);

    std::string path_png_new = std::string(cwd).append("/")
        .append(prefix);

    if (!numeric_only_filenames)
      path_png_new = path_png_new.append("_").append(filename);
    else
      path_png_new = path_png_new.append(".png");

    rename(path_png.c_str(), path_png_new.c_str());
    ++index;
  }

  return 0;
}

void ParseArguments(uint8_t argc,
                    char *const *argv,
                    bool *descending,
                    uint8_t *amount_digits_min,
                    bool *numeric_only_filenames) {
  for (uint8_t index_arg = 0; index_arg < argc; ++index_arg) {
    if (strcmp(argv[index_arg], "-V") == 0
        || strcmp(argv[index_arg], "--version") == 0) PrintVersionAndExit();

    if (strcmp(argv[index_arg], "-d") == 0
        || strcmp(argv[index_arg], "--desc") == 0) {
      *descending = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-n") == 0
        || strcmp(argv[index_arg], "--numeric_only") == 0) {
      *numeric_only_filenames = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-nd") == 0
        || strcmp(argv[index_arg], "-dn") == 0) {
      *numeric_only_filenames = true;
      *descending = true;
      continue;
    }

    auto v = std::string(argv[index_arg]);
    if (v.find("-a=") == 0) {
      *amount_digits_min = std::stoi(v.substr(3));
      continue;
    }

    if (v.find("--amount_digits_min=") == 0) {
      *amount_digits_min = std::stoi(v.substr(20));
      continue;
    }
  }
}

void PrintVersionAndExit() {
  std::cout
      << "lspng 0.0.1\n"
         "License GPLv3+: GNU GPL version 3 or later "
         "<http://gnu.org/licenses/gpl.html>.\n"
         "This is free software: you are free to change and redistribute it.\n"
         "There is NO WARRANTY, to the extent permitted by law.\n"
         "\n"
         "Written by Kay Stenschke, see <https://github.com/kstenschke/lspng>."
         "\n";

  exit(0);
}

std::string EvalInCli(const char *command) {
  FILE *fp;
  char path[1035];

  fp = popen(command, "r");

  if (fp ==nullptr) return "Failed to run command";

  std::string result;

  while (fgets(path, sizeof(path), fp) !=nullptr) result += path;

  pclose(fp);

  return result;
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
          ) continue;

      files.push_back(std::string(path) + namelist[i]->d_name);
    }

    while (filecount--) free(namelist[filecount]);
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

  if (luminance_a != luminance_b) return false;

  return std::get<0>(a) < std::get<0>(b);
}

bool compareDesc(std::tuple<std::string, float> a,
                 std::tuple<std::string, float> b) {
  float luminance_a =  std::get<1>(a);
  float luminance_b =  std::get<1>(b);

  if (luminance_b < luminance_a) return true;

  if (luminance_b != luminance_a) return false;

  return std::get<0>(b) < std::get<0>(a);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc30-c"
#pragma clang diagnostic pop
