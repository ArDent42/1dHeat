#include "fuel.h"

#include <cmath>

#include "common.h"

namespace fuel {

Fuel::Fuel(const std::string& name) {
  std::string str = FileName(name, 'f');
  std::ifstream fin(str);
  Initialize(fin);
}

Fuel::Fuel(std::ifstream& fin) { Initialize(fin); }

void Fuel::Initialize(std::ifstream& fin) {
  nlohmann::json res = nlohmann::json::parse(fin);
  nlohmann::json& total_temp = res.at("Total temperature");
  for (size_t i = 0; i < total_temp.front().size(); ++i) {
    total_temp_[total_temp.front()[i]] = total_temp.back()[i];
  }
  for (const auto& props : res.at("Properties").items()) {
    double press = props.value().at("0");
    double temp = props.value().at("1");
    for (const auto& [name, prop] : props.value().items()) {
      if (name == "0" || name == "1") {
        continue;
      }
      properties_[press][temp]
                 [static_cast<fuel::FuelProp>(std::stoi(name))] =
                     prop;
    }
  }
}

PropValues Fuel::GetProperties(double p, double t) const {
  auto p_b = math::GetParamBounds(properties_, p);
  auto t1_b = math::GetParamBounds(p_b.first->second, t);
  auto t2_b = math::GetParamBounds(p_b.second->second, t);
  PropValues res;
  for (int i = FuelProp::v; i < FuelProp::null; ++i) {
    double param1 =
        math::Linterp(t1_b.first->first, t1_b.second->first,
                      t1_b.first->second.at(static_cast<FuelProp>(i)),
                      t1_b.second->second.at(static_cast<FuelProp>(i)), t);
    double param2 =
        math::Linterp(t2_b.first->first, t2_b.second->first,
                      t2_b.first->second.at(static_cast<FuelProp>(i)),
                      t2_b.second->second.at(static_cast<FuelProp>(i)), t);
    res[static_cast<FuelProp>(i)] =
        math::Linterp(p_b.first->first, p_b.second->first, param1, param2, p);
  }
  return res;
}

double Fuel::GetTotalTemp(double p) const {
  auto bounds = math::GetParamBounds(total_temp_, p);
  return math::Linterp(bounds.first->first, bounds.second->first,
                       bounds.first->second, bounds.second->second, p);
}

}  // namespace fuel
