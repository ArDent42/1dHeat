#include "common.h"

#include <algorithm>

namespace parsing {

std::string RemoveSpaces(const std::string &str) {
  std::string new_str;
  std::remove_copy(str.begin(), str.end(), std::back_inserter(new_str), ' ');
  return new_str;
}

std::vector<std::optional<std::string>> ParseString(const std::string &str) {
  std::vector<std::optional<std::string>> results;
  std::string value;
  std::string new_str = RemoveSpaces(str);
  for (const char c : new_str) {
    if (c != ' ' && c != ';' && c != ',' && c != '\n' && c != '\r') {
      value += c;
    } else {
      if (value.empty()) {
        results.push_back(std::nullopt);
        continue;
      } else {
        results.push_back(value);
      }
      value.clear();
    }
  }
  if (!value.empty()) {
    results.push_back(value);
  }
  return results;
}

ParsedFile ParsingFuelCSV(const std::string &file_name) {
  std::ifstream fin(file_name);
  std::string str;
  ParsedFile parsed_file;
  while (getline(fin, str)) {
    ParsedString parsed_string = ParseString(str);
    if (parsed_string.empty() || (!parsed_string.front().has_value() ||
                                  !parsed_string.back().has_value())) {
      continue;
    } else {
      parsed_file.push_back(parsed_string);
    }
  }
  return parsed_file;
}

ParsedFile ParsingMatCSV(const std::string &file_name) {
  std::ifstream fin(file_name);
  std::string str;
  ParsedFile parsed_file;
  while (getline(fin, str)) {
    ParsedString parsed_string = ParseString(str);
    parsed_file.push_back(parsed_string);
  }
  return parsed_file;
}

}  // namespace parsing

std::string FileName(const std::string &name, char entry_type) {
  std::string filename;
  if (entry_type == 'f') {
    filename = "fuels/" + name + ".json";
  } else if (entry_type == 'm') {
    filename = "materials/" + name + ".json";
  }
  return filename;
}

std::map<double, double> JSON2DArray2Map(const nlohmann::json &array2d) {
  std::map<double, double> res;
  std::vector<double> t;
  std::vector<double> v;
  for (size_t i = 0; i < array2d.size(); ++i) {
    if (i == 0) {
      t = JSONArray2Vector<double>(array2d[i]);
    } else if (i == 1) {
      v = JSONArray2Vector<double>(array2d[i]);
    }
  }
  for (size_t i = 0; i < t.size(); ++i) {
    res[t[i]] = v[i];
  }
  return res;
}

double math::Linterp(double x1, double x2, double y1, double y2, double x) {
  double res;
  if (x1 != x2) {
    res = y1 + (y2 - y1) / (x2 - x1) * (x - x1);
  } else {
    res = y1;
  }
  return res;
}

double math::Linterp(const std::map<double, double> &c, double p) {
  auto bounds = math::GetParamBounds(c, p);
  return math::Linterp(bounds.first->first, bounds.second->first,
                       bounds.first->second, bounds.second->second, p);
}