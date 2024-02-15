#include "flow.h"

#include <iostream>

#include "common.h"

namespace flow {

void Flow1D::CalcLambda() {
  double n = 1 / (k_ - 1.0);
  double k1 = k_ + 1.0;
  double k2 = k_ - 1.0;
  double prev_value = 1.0;
  do {
    prev_value = lambda_;
    if (flow_type_ == FlowType::subsonic) {
      lambda_ =
          1.0 / (pow(k1 / 2.0, n) *
                 pow(1.0 - prev_value * prev_value * k2 / k1, n) * ksi_ * ksi_);
    } else if (flow_type_ == FlowType::supersonic) {
      lambda_ = k1 / k2 / prev_value -
                2 / pow(prev_value, k_) / k2 / pow(ksi_, 2 * k_ - 2);
    } else {
      lambda_ = 1.0;
      return;
    }
  } while (std::abs(lambda_ - prev_value) > EPS);
}

void Flow1D::CalcPressStatic() {
  p_static_ = p_total_ * pow((1 - lambda_ * lambda_ * (k_ - 1) / (k_ + 1)),
                             (k_ / (k_ - 1)));
}

void Flow1D::CalcTempStatic() {
  t_static_ = t_total_ * (1 - lambda_ * lambda_ * (k_ - 1) / (k_ + 1));
}

void Flow1D::CalcVelocity() {
  auto fuel_props_static =
      fuel_.GetProperties(p_static_, t_static_);
  double a =
      pow(k_ * p_static_ * fuel_props_static.at(fuel::FuelProp::v),
          0.5);
  mach_ =
      lambda_ * pow(2 / (k_ + 1), 0.5) /
      pow(1 - (k_ - 1) / (k_ + 1) * lambda_ * lambda_,
          0.5);
  u_ = a * mach_;
}

void Flow1D::CalcFlowParams(double p_total, double ksi, FlowType flow_type) {
  if (std::abs(p_total_ - p_total) <= EPS && std::abs(ksi_ - ksi) <= EPS && flow_type_ == flow_type) {
    return;
  }
  flow_type_ = flow_type;
  p_total_ = p_total;
  ksi_ = ksi;
  t_total_ = fuel_.GetTotalTemp(p_total);
  k_ =
      fuel_.GetProperties(p_total_, t_total_)
          .at(fuel::FuelProp::k_eq);
  CalcLambda();
  CalcPressStatic();
  CalcTempStatic();
  CalcVelocity();
}
}  // namespace flow
