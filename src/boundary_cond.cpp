#include "boundary_cond.h"

namespace heat_exchange {

double HeatExchange::CalcAlphaAvd() const {
  double ro = 1 / fuel_props_wall_.at(fuel::FuelProp::v);
  double mu = fuel_props_wall_.at(fuel::FuelProp::mu);
  double lt;
  double cp;
  if (ini_.GetBoundaryTable().flow_type == flow::Flow1D::FlowType::subsonic) {
    cp = fuel_props_wall_.at(fuel::FuelProp::cp_eq);
    lt = fuel_props_wall_.at(fuel::FuelProp::lt_total);
  } else {
    cp = fuel_props_wall_.at(fuel::FuelProp::cp_fr);
    lt = fuel_props_wall_.at(fuel::FuelProp::lt_gas);
  }
  double Re =
      ro * flow_.u() * ini_.GetDomainSettings().blayer_length.value() / mu;
  double Pr = mu * cp / lt;
  double Te = CalcTe();
  double m = Te / flow_.t_static();
  double alpha = 0.0296 * pow(Re, -0.2) * pow(Pr, -0.6) * ro * cp *
                 flow_.u() * pow((0.9 * flow_.t_total() / Te), 0.39) *
                 pow(m, 0.11);
  return alpha;
}

// double HeatExchange::CalcAlphaCrit(double p, double t_w, double r0, double u,
//                                    flow::FlowType flow_type) const {
//   fuel::PropValues fuel_props = fuel_.GetProperties(p, t_w);
//   double ro = 1 / fuel_props.at(fuel::FuelProp::v);
//   double mu = fuel_props.at(fuel::FuelProp::mu);
//   double lt = fuel_props.at(fuel::FuelProp::lt_total);
//   double cp;
//   if (flow_type == flow::FlowType::subsonic) {
//     cp = fuel_props.at(fuel::FuelProp::cp_eq);
//   } else {
//     cp = fuel_props.at(fuel::FuelProp::cp_fr);
//   }
//   double Re = ro * u * r0 * 2.0 / fuel_props.at(fuel::FuelProp::mu);
//   double Pr = mu * cp / lt;
//   double alpha = 0.023 * lt / r0 / 2.0 * pow(Re, 0.8) * pow(Pr, 0.3);
//   return alpha;
// }

double HeatExchange::CalcTe() const {
  double mu = fuel_props_static_.at(fuel::FuelProp::mu);
  double lt;
  double cp;
  if (ini_.GetBoundaryTable().flow_type == flow::Flow1D::FlowType::subsonic) {
    cp = fuel_props_static_.at(fuel::FuelProp::cp_eq);
    lt = fuel_props_static_.at(fuel::FuelProp::lt_total);
  } else {
    cp = fuel_props_static_.at(fuel::FuelProp::cp_fr);
    lt = fuel_props_static_.at(fuel::FuelProp::lt_gas);
  }
  double Pr = mu * cp / lt;
  double m =
      1 + (flow_.k() - 1) / 2 * pow(Pr, 1.0 / 3.0) * pow(flow_.mach(), 2);
  double Te = flow_.t_static() * m;
  return Te;
}

double HeatExchange::CalcTRad() const {
  return flow_
      .t_static();
}
double HeatExchange::CalcEmissitivity() const {
  return 0.229 + 0.0616 * 5 + 0.00011 * flow_.t_static() -
         0.3684 * fuel_props_static_.at(fuel::FuelProp::z) +
         0.00502 * flow_.p_static() / math::MEGA -
         0.00338 * ini_.GetDomainSettings().initial_radius.value() * 2.0;
}

Result HeatExchange::CalcAutomatic(double pressure, double t_wall) {
  Result res;
  flow_.CalcFlowParams(pressure, ini_.GetDomainSettings().initial_radius.value() / ini_.GetDomainSettings().throat_radius.value(), ini_.GetBoundaryTable().flow_type);
  fuel_props_wall_ = fuel_.GetProperties(flow_.p_static(), t_wall);
  fuel_props_static_ = fuel_.GetProperties(flow_.p_static(), flow_.t_static());
  res.alfa = CalcAlphaAvd();
  res.t_e = CalcTe();
  res.t_rad = CalcTRad();
  res.eps_gas = CalcEmissitivity();

  return res;
}

}  // namespace heat_exchange
