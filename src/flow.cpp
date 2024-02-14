#include "flow.h"

#include <iostream>

namespace flow {

void Flow1D::CalcLambda(FlowType flow_type) {
  double n = 1 / (flow_params_.k - 1.0);
  double k1 = flow_params_.k + 1.0;
  double k2 = flow_params_.k - 1.0;
  double prev_value;
  do {
    prev_value = flow_params_.lambda;
    if (flow_type == FlowType::subsonic) {
      flow_params_.lambda =
          1.0 /
          (pow(k1 / 2.0, n) * pow(1.0 - prev_value * prev_value * k2 / k1, n) *
           flow_params_.ksi * flow_params_.ksi);
    } else if (flow_type == FlowType::supersonic) {
      flow_params_.lambda = k1 / k2 / prev_value -
                            2 / pow(prev_value, flow_params_.k) / k2 /
                                pow(flow_params_.ksi, 2 * flow_params_.k - 2);
    } else {
      flow_params_.lambda = 1.0;
      return;
    }
  } while (std::abs(flow_params_.lambda - prev_value) > 0.00001);
}

void Flow1D::CalcPressStatic() {
  flow_params_.p_static =
      flow_params_.p_total *
      pow((1 - flow_params_.lambda * flow_params_.lambda *
                   (flow_params_.k - 1) / (flow_params_.k + 1)),
          (flow_params_.k / (flow_params_.k - 1)));
}

void Flow1D::CalcTempStatic() {
  flow_params_.t_static = flow_params_.t_total *
                          (1 - flow_params_.lambda * flow_params_.lambda *
                                   (flow_params_.k - 1) / (flow_params_.k + 1));
}

void Flow1D::CalcVelocity() {
  auto fuel_props_static =
      fuel_.GetProperties(flow_params_.p_static, flow_params_.t_static);
  double a = pow(flow_params_.k * flow_params_.p_static *
                     fuel_props_static.at(fuel::FuelProp::v),
                 0.5);
  flow_params_.mach = flow_params_.lambda * pow(2 / (flow_params_.k + 1), 0.5) /
                      pow(1 - (flow_params_.k - 1) / (flow_params_.k + 1) *
                                  flow_params_.lambda * flow_params_.lambda,
                          0.5);
  flow_params_.u = a * flow_params_.mach;
}

void Flow1D::CalcFlowParams(double p_total, double ksi, FlowType flow_type) {
  flow_params_.p_total = p_total;
  flow_params_.ksi = ksi;
  flow_params_.t_total = fuel_.GetTotalTemp(p_total);
  flow_params_.k =
      fuel_.GetProperties(flow_params_.p_total, flow_params_.t_total)
          .at(fuel::FuelProp::k_eq);
  CalcLambda(flow_type);
  CalcPressStatic();
  CalcTempStatic();
  CalcVelocity();
}
}  // namespace flow
