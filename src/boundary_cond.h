#pragma once

#include "common.h"
#include "flow.h"
#include "fuel.h"
#include "ini_data.h"

namespace heat_exchange {

enum class CalcType { avd, crit };

struct Result {
  double alfa;
  double t_e;
  double t_rad;
  double eps_gas;
  friend std::ostream& operator<<(std::ostream& os, const Result& res) {
    os << "Alpha: " << res.alfa << " W/m^2 K" << '\n';
    os << "Te: " << res.t_e << " K" << '\n';
    os << "Trad: " << res.t_rad << " K" << '\n';
    os << "Eps: " << res.eps_gas << '\n';
    return os;
  }
};

class HeatExchange {
 private:
  const fuel::Fuel& fuel_;
  const IniData& ini_;
  flow::Flow1D& flow_;

  fuel::PropValues fuel_props_static_;
  fuel::PropValues fuel_props_wall_;

  double CalcAlphaAvd() const;
  double CalcAlphaCrit(double p, double t_w, double r0, double u,
                       flow::Flow1D::FlowType flow_type) const;
  double CalcTe() const;
  double CalcTRad() const;
  double CalcEmissitivity() const;

 public:
  HeatExchange(const IniData& ini, const fuel::Fuel& fuel,
               flow::Flow1D& flow)
      : ini_(ini), fuel_(fuel), flow_(flow) {}
  Result CalcAutomatic(double pressure, double t_wall);
};

}  // namespace heat_exchange
