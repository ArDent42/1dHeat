#include "material.h"

#include "common.h"

namespace material {

Material::Material(const std::string& name, double angle = 0.0) {
  std::string str = FileName(name, 'm');
  std::ifstream fin(str);
  Initialize(fin, angle);
}

Material::Material(std::istream& fin, double angle = 0.0) { Initialize(fin, angle); }

void Material::Initialize(std::istream& fin, double angle) {
  nlohmann::json json = nlohmann::json::parse(fin);
  name_ = json.at("name");
  for (const auto& [name, props] : json.items()) {
    for (size_t i = 0; i < props.back().size(); i++) {
      if (stoe(name)) {
        therm_props_[stoe(name).value()][props.front()[i]] = props.back()[i];
      }
    }
  }
  std::vector<std::pair<double, double>> l0(
      therm_props_.at(Property::l0).begin(),
      therm_props_.at(Property::l0).end());
  std::vector<std::pair<double, double>> l90(
      therm_props_.at(Property::l90).begin(),
      therm_props_.at(Property::l90).end());
  for (size_t i = 0; i < l0.size(); ++i) {
    therm_props_[Property::l][l0[i].first] = l0[i].second * std::pow(std::cos(angle * M_PI / 180.0), 2) + l90[i].second * std::pow(std::cos((90.0 - angle) * M_PI / 180.0), 2);
  }
}

double Material::GetProperty(double T, Property prop_name) const {
  double res;
  return math::Linterp(therm_props_.at(prop_name), T);
}

std::optional<Property> stoe(const std::string& name) {
  static const std::unordered_map<std::string, Property> converter{
      {"ro", Property::ro},         {"lambda_0", Property::l0},
      {"lambda_90", Property::l90}, {"cp", Property::cp},
      {"ko", Property::ko},         {"ea", Property::ea}};
  if (converter.count(name)) {
    return converter.at(name);
  }
  return std::nullopt;
}

}  // namespace material
