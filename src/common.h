#pragma once
#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

const double EPS = 0.00001;
const double EPS_ITER = 1E-4;

namespace parsing {

using ParsedString = std::vector<std::optional<std::string>>;
using ParsedFile = std::vector<ParsedString>;

std::vector<std::optional<std::string>> ParseString(const std::string &str);
ParsedFile ParsingFuelCSV(const std::string &file_name);
ParsedFile ParsingMatCSV(const std::string &file_name);
std::string RemoveSpaces(const std::string &str);

}  // namespace parsing

namespace math {

const double MEGA = 1000000;
const double KILO = 1000;
const uint N_INTEGRATION = 5;
const double SIGMA = 5.67E-08;

double Linterp(double x1, double x2, double y1, double y2, double x);
double Linterp(const std::map<double, double> &c, double p);

template <typename T1, typename T2>
auto GetParamBounds(const std::map<T1, T2> &c, T1 t) {
  if (c.size() == 1) {
    auto it1 = c.begin();
    auto it2 = c.begin();
    return std::make_pair(it1, it2);
  }
  auto it2 = c.upper_bound(t);
  auto it1 = it2;
  if (it2 == c.end()) {
    std::advance(it2, -1);
    it1 = std::prev(it1);
  } else if (it2 != c.begin()) {
    it1 = std::prev(it2);
  }
  return std::make_pair(it1, it2);
}

template <typename Func>
double Integral(double a, double b, Func fx) {
  double dx = (b - a) / N_INTEGRATION;
  double area = 0;
  for (size_t i = 0; i < N_INTEGRATION; ++i) {
    area += fx(a + dx * i + 0.5 * dx) * dx;
  }
  return area;
}

}  // namespace math

std::string FileName(const std::string &name, char entry_type);

template <typename T>
std::vector<T> JSONArray2Vector(const nlohmann::json &array) {
  std::vector<T> res;
  for (auto element : array.items()) {
    res.push_back(element.value());
  }
  return res;
}

std::map<double, double> JSON2DArray2Map(const nlohmann::json &array2d);