#include "boundary_cond.h"

namespace heat_exchange {

double HeatExchange::CalcAlphaAvd() const {
  double ro = 1 / fuel_props_Tw_.at(fuel::FuelProp::v);
  double mu = fuel_props_Tw_.at(fuel::FuelProp::mu);
  double lt;
  double cp;
  if (ini_.GetBoundaryTable().flow_type == flow::FlowType::subsonic) {
    cp = fuel_props_Tw_.at(fuel::FuelProp::cp_eq);
    lt = fuel_props_Tw_.at(fuel::FuelProp::lt_total);
  } else {
    cp = fuel_props_Tw_.at(fuel::FuelProp::cp_fr);
    lt = fuel_props_Tw_.at(fuel::FuelProp::lt_gas);
  }
  double Re = ro * flow_params_.u * ini_.GetDomainSettings().blayer_length.value() / mu;
  double Pr = mu * cp / lt;
  double Te = CalcTe();
  double m = Te / flow_params_.t_static;
  double alpha = 0.0296 * pow(Re, -0.2) * pow(Pr, -0.6) * ro * cp *
                 flow_params_.u * pow((0.9 * flow_params_.t_total / Te), 0.39) *
                 pow(m, 0.11);
  return alpha;
}

double HeatExchange::CalcAlphaCrit(double p, double t_w, double r0, double u,
                                   flow::FlowType flow_type) const {
  fuel::PropValues fuel_props = fuel_.GetProperties(p, t_w);
  double ro = 1 / fuel_props.at(fuel::FuelProp::v);
  double mu = fuel_props.at(fuel::FuelProp::mu);
  double lt = fuel_props.at(fuel::FuelProp::lt_total);
  double cp;
  if (flow_type == flow::FlowType::subsonic) {
    cp = fuel_props.at(fuel::FuelProp::cp_eq);
  } else {
    cp = fuel_props.at(fuel::FuelProp::cp_fr);
  }
  double Re = ro * u * r0 * 2.0 / fuel_props.at(fuel::FuelProp::mu);
  double Pr = mu * cp / lt;
  double alpha = 0.023 * lt / r0 / 2.0 * pow(Re, 0.8) * pow(Pr, 0.3);
  return alpha;
}
double HeatExchange::CalcTe() const {
  double mu = fuel_props_Tstatic_.at(fuel::FuelProp::mu);
  double lt;
  double cp;
  double k = flow_params_.k;
  if (ini_.GetBoundaryTable().flow_type == flow::FlowType::subsonic) {
    cp = fuel_props_Tstatic_.at(fuel::FuelProp::cp_eq);
    lt = fuel_props_Tstatic_.at(fuel::FuelProp::lt_total);
  } else {
    cp = fuel_props_Tstatic_.at(fuel::FuelProp::cp_fr);
    lt = fuel_props_Tstatic_.at(fuel::FuelProp::lt_gas);
  }
  double Pr = mu * cp / lt;
  double m = 1 + (k - 1) / 2 * pow(Pr, 1.0 / 3.0) * pow(flow_params_.mach, 2);
  double Te = flow_params_.t_static * m;
  return Te;
}
double HeatExchange::CalcTRad(IniDataAvd ini_data) const {
  return flow_
      .GetParams(ini_data.p_total, ini_data.r0 / ini_data.r_kr,
                 ini_data.flow_type)
      .t_static;
}
double HeatExchange::CalcEmissitivity(IniDataAvd ini_data) const {
  flow::FlowParams flow_params = flow_.GetParams(
      ini_data.p_total, ini_data.r0 / ini_data.r_kr, ini_data.flow_type);
  return 0.229 + 0.0616 * 5 + 0.00011 * flow_params.t_static -
         0.3684 * fuel_props_Tstatic_.at(fuel::FuelProp::z) +
         0.00502 * flow_params.p_static / math::MEGA -
         0.00338 * ini_data.r0 * 2.0;
}

}  // namespace heat_exchange
