#include "ini_data.h"
#include <fstream>

void TestCreateEmptyIni() {
  IniData ini_data;
  std::ofstream fout("empty_ini.json");
  ini_data.CreateEmpty(fout);
}
void TestParse() {
  IniData ini_data("ini_data");
}