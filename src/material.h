#pragma once
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "common.h"

namespace material {

enum class Property { ro, l0, l90, l, cp, ko, ea, null };

const std::vector<std::string> PROP_NAMES{
    "ro, kg/m^3", "l0, W/m K", "l90, W/m K", "Cp, J/kg K", "ko", "ea"};

using Thermal = std::unordered_map<Property, std::map<double, double>>;

std::optional<Property> stoe(const std::string& name);

class Material {
 private:
  std::string name_;
  Thermal therm_props_;

 public:
  Material(const std::string& name, double angle);
  Material(std::istream& fin, double angle);
  void Initialize(std::istream& fin, double angle);

  double GetProperty(double T, Property prop_name) const;
  const Thermal& GetProps() const { return therm_props_; }
  const std::string& GetName() const { return name_; }

};

}  // namespace material