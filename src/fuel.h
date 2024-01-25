#pragma once

#include <map>
#include <string>
#include <vector>

namespace fuel {

enum FuelProp {
  p,
  t,
  v,
  m,
  cp_fr,
  k_fr,
  cp_eq,
  k_eq,
  cp_gas_fr,
  k_gas_fr,
  cp_gas_eq,
  k_gas_eq,
  mu,
  lt_gas,
  lt_total,
  pr_fr,
  pr_eq,
  a,
  z,
  bm,
  m_frac_o,
  m_frac_o2,
  m_frac_h2o,
  m_frac_co2,
  m_frac_n,
  null
};

const std::vector<std::string> PROP_NAMES{"P, Pa",
                                          "T, K",
                                          "V, 1/m^3",
                                          "M, mol/kg",
                                          "cp_fr, J/kg K",
                                          "k_fr",
                                          "cp_eq, J/kg K",
                                          "k_eq",
                                          "cp_gas_fr, J/kg K",
                                          "k_gas_fr",
                                          "cp_gas_eq, J/kg K",
                                          "k_gas_eq",
                                          "mu, Pa s",
                                          "lt_gas, W/m K",
                                          "lt_total, W/m K",
                                          "pr_fr",
                                          "pr_eq",
                                          "A, m/s",
                                          "z",
                                          "bm",
                                          "m_frac_o",
                                          "m_frac_o2",
                                          "m_frac_h2o",
                                          "m_frac_co2",
                                          "m_frac_n"};

using PropValues = std::map<FuelProp, double>;
using Properties = std::map<double, std::map<double, PropValues>>;
using TotalTemp = std::map<double, double>;

class Fuel {
 private:
  TotalTemp total_temp_;
  Properties properties_;
  void Initialize(std::ifstream& fin);
 public:
  Fuel(const TotalTemp &total_temp, const Properties &properties)
      : total_temp_(total_temp), properties_(properties) {}
  Fuel(const std::string &name);
  Fuel(std::ifstream &fin);
  PropValues GetProperties(double P, double T) const;
  double GetTotalTemp(double p) const;
  const Properties &GetProperties() const { return properties_; }
  const TotalTemp &GetTotalTemperatures() const { return total_temp_; }
};

}  // namespace fuel
