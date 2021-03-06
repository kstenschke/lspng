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
#include <dirent.h>
#include <fstream>
#include <netinet/in.h>

using namespace std;
using namespace cimg_library;

void ParseArguments(
    uint8_t argc, char *const *argv,
    bool *descending,
    uint8_t *amount_digits_min,
    bool *numeric_only_filenames,
    bool *append_luminance_to_filename, bool *append_wid_hgt_to_filename);

void PrintVersionAndExit();
std::string EvalInCli(const char *command);
std::vector<std::string> GetPngsInPath(const char *path);
std::string GetFilenameFromPath(const std::string &path);
float GetAvgLuminance(const std::string &path_png);

bool compareAsc(std::tuple<std::string, float> a,
                std::tuple<std::string, float> b);

bool compareDesc(std::tuple<std::string, float> a,
                 std::tuple<std::string, float> b);

void CollectPngsWithLuminanceInVector(
    const vector<std::string> &png_files,
    vector<std::tuple<std::string, float>> &tuples_png_and_luminance);

string GetPercentFromLuminanceFloat(const float &luminance,
                                    int amount_digits = 3);

int GetPngHeight(const std::string& path_png);
int GetPngWidth(const std::string& path_png);

int main(int argc, char **argv) {
  uint8_t amount_digits_min = 1;
  bool descending = false;
  bool numeric_only_filenames = false;
  bool append_luminance_to_filename = false;
  bool append_wid_and_hgt_to_filename = false;

  if (argc > 1) ParseArguments(
      argc, argv,
      &descending,
      &amount_digits_min,
      &numeric_only_filenames,
      &append_luminance_to_filename, &append_wid_and_hgt_to_filename);

  auto cwd = EvalInCli("echo $PWD");
  cwd = cwd.substr(0, cwd.length() - 1) + "/";  // remove trailing "\n"
  auto png_files = GetPngsInPath(cwd.c_str());

  std::vector<std::tuple<std::string, float>> tuples_png_and_luminance;

  CollectPngsWithLuminanceInVector(png_files, tuples_png_and_luminance);

  std::sort(tuples_png_and_luminance.begin(),
            tuples_png_and_luminance.end(),
            descending ? compareDesc : compareAsc);

  uint32_t index = 0;
  auto amount_digits = std::to_string(png_files.size()).length();
  if (amount_digits < amount_digits_min) amount_digits = amount_digits_min;

  for (auto const& tuple_png_and_luminance : tuples_png_and_luminance) {
    const auto& path_png = std::get<0>(tuple_png_and_luminance);
    const auto& luminance = std::get<1>(tuple_png_and_luminance);

    auto prefix = std::to_string(index);

    while (prefix.size() < amount_digits)
      prefix = std::string("0").append(prefix);

    auto filename = GetFilenameFromPath(path_png);

    std::string path_png_new = std::string(cwd).append("/").append(prefix);

    path_png_new = numeric_only_filenames
        ? path_png_new.append(".png")
        : path_png_new.append("_").append(filename);

    if (append_luminance_to_filename) {
      path_png_new = path_png_new.substr(0, path_png_new.length() - 4)
          + "_" + GetPercentFromLuminanceFloat(luminance);
    } else {
      path_png_new = path_png_new.substr(0, path_png_new.length() - 4);
    }

    if (append_wid_and_hgt_to_filename) {
      int wid = GetPngWidth(path_png);
      int hgt = GetPngHeight(path_png);

      path_png_new += "_" + to_string(wid) + "x" + to_string(hgt);
    }

    path_png_new += ".png";

    rename(path_png.c_str(), path_png_new.c_str());
    ++index;
  }

  return 0;
}

// Get percentage value (0-100) from given luminance float: 0.0-255.0
string GetPercentFromLuminanceFloat(const float &luminance, int amount_digits) {
  // |V1 - V2|/ [(V1 + V2)/2] ?? 100
  float float_percent = (255.0 - luminance) / ((255.0 + luminance) / 2) * 100;
  auto percent = 100 - ((int)float_percent / 2);

  auto rc = to_string(percent);

  while (rc.size() < 3) rc = "0" + rc;

  return rc;
}

void CollectPngsWithLuminanceInVector(
    const vector<std::string> &png_files,
    vector<std::tuple<std::string, float>> &tuples_png_and_luminance) {
  for (auto const& path_png : png_files) {
    auto luminance_total = GetAvgLuminance(path_png);
    auto luminance_total_str = to_string(luminance_total);

    if (luminance_total_str.find("-nan") == 0)
      continue;  // determining brightness failed: dont rename

    auto filename = GetFilenameFromPath(path_png);
    tuples_png_and_luminance.emplace_back(
        make_tuple(filename, luminance_total));
  }
}

void ParseArguments(
    uint8_t argc, char *const *argv,
    bool *descending,
    uint8_t *amount_digits_min,
    bool *numeric_only_filenames,
    bool *append_luminance_to_filename, bool *append_wid_hgt_to_filename) {
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

    if (strcmp(argv[index_arg], "-l") == 0
        || strcmp(argv[index_arg], "--append_luminance") == 0) {
      *append_luminance_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-p") == 0
        || strcmp(argv[index_arg], "--append_px_wid_and_hgt") == 0) {
      *append_wid_hgt_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-nd") == 0
        || strcmp(argv[index_arg], "-dn") == 0) {
      *numeric_only_filenames = true;
      *descending = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-nl") == 0
        || strcmp(argv[index_arg], "-ln") == 0) {
      *numeric_only_filenames = true;
      *append_luminance_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-np") == 0
        || strcmp(argv[index_arg], "-pn") == 0) {
      *numeric_only_filenames = true;
      *append_wid_hgt_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-dl") == 0
        || strcmp(argv[index_arg], "-ld") == 0) {
      *descending = true;
      *append_luminance_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-dp") == 0
        || strcmp(argv[index_arg], "-pd") == 0) {
      *descending = true;
      *append_wid_hgt_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-ndl") == 0
        || strcmp(argv[index_arg], "-dnl") == 0
        || strcmp(argv[index_arg], "-nld") == 0) {
      *numeric_only_filenames = true;
      *descending = true;
      *append_luminance_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-ndp") == 0
        || strcmp(argv[index_arg], "-dnp") == 0
        || strcmp(argv[index_arg], "-npd") == 0) {
      *numeric_only_filenames = true;
      *descending = true;
      *append_wid_hgt_to_filename = true;
      continue;
    }

    if (strcmp(argv[index_arg], "-ndlp") == 0
        || strcmp(argv[index_arg], "-pndl") == 0
        || strcmp(argv[index_arg], "-dnlp") == 0
        || strcmp(argv[index_arg], "-pdnl") == 0
        || strcmp(argv[index_arg], "-pnld") == 0
        || strcmp(argv[index_arg], "-nldp") == 0) {
      *numeric_only_filenames = true;
      *descending = true;
      *append_luminance_to_filename = true;
      *append_wid_hgt_to_filename = true;
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
      << "lspng 0.0.4\n"
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

  if (filecount == 0) return files;

  for (int i = 0; i < filecount; i++) {
    if (namelist[i]->d_type == DT_DIR
        || namelist[i]->d_name[0] == '.'
        || std::string(namelist[i]->d_name).find(".png") !=
            strlen(namelist[i]->d_name) - 4
        ) continue;

    files.push_back(std::string(path) + namelist[i]->d_name);
  }

  while (filecount--) free(namelist[filecount]);
  free(namelist);

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

int GetPngWidth(const std::string& path_png) {
  std::ifstream in(path_png.c_str());
  unsigned int width, height;

  in.seekg(16);
  in.read((char *)&width, 4);
  in.read((char *)&height, 4);

  return ntohl(width);
}

int GetPngHeight(const std::string& path_png) {
  std::ifstream in(path_png.c_str());
  unsigned int width, height;

  in.seekg(16);
  in.read((char *)&width, 4);
  in.read((char *)&height, 4);

  return ntohl(height);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc30-c"
#pragma clang diagnostic pop
