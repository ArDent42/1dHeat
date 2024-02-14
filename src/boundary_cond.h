#pragma once

#include "common.h"
#include "flow.h"
#include "fuel.h"
#include "ini_data.h"

namespace heat_exchange {

enum class CalcType { avd, crit };

struct IniDataAvd {
  double p_total;
  double r_kr;
  double r0;
  double s_ef;
  flow::FlowType flow_type;
};

struct IniDataCrit {
  double p;
  double t_w;
  double r0;
  double u;
  flow::FlowType flow_type;
};

struct Result {
  double alfa;
  double t_e;
  double t_rad;
  double eps_gas;
};

class HeatExchange {
 private:
  const fuel::Fuel& fuel_;
  const IniData& ini_;
  const flow::Flow1D& flow_;
  flow::FlowParams flow_params_;
  fuel::PropValues fuel_props_Tw_;
  fuel::PropValues fuel_props_Tstatic_;
  double CalcAlphaAvd() const;
  double CalcAlphaCrit(double p, double t_w, double r0, double u,
                       flow::FlowType flow_type) const;
  double CalcTe() const;
  double CalcTRad() const;
  double CalcEmissitivity() const;

 public:
  HeatExchange(const IniData& ini, const fuel::Fuel& fuel, const flow::Flow1D& flow)
      : ini_(ini), fuel_(fuel), flow_(flow) {}
  Result CalcAvd(double pressure, double t_wall);
};

Result HeatExchange::CalcAvd(double pressure, double t_wall) {
  Result res;
  flow_params_ = flow_.GetParams(pressure, ini_.GetDomainSettings().initial_radius.value() / ini_.GetDomainSettings().throat_radius.value(),
                                 ini_.GetBoundaryTable().flow_type);
  fuel_props_Tw_ =
      fuel_.GetProperties(flow_params_.p_static, t_wall);
  fuel_props_Tstatic_ =
      fuel_.GetProperties(flow_params_.p_static, flow_params_.t_static);
  res.alfa = CalcAlphaAvd();
  res.t_e = CalcTe();
  res.t_rad = CalcTRad();
  res.eps_gas = CalcEmissitivity(ini_data);

  return res;
}

}  // namespace heat_exchange
