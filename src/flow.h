#ifndef FLOW_H_
#define FLOW_H_
#include <cmath>

#include "fuel.h"

namespace flow {

enum FlowType {
  subsonic,
  sonic,
  supersonic,
};

struct FlowParams {
  double t_total;
  double t_static;
  double p_total;
  double p_static;
  double u;
  double mach;
  double k;
  double lambda = 1.0;
  double ksi;
};

class Flow1D {
 private:
  void CalcLambda(FlowType flow_type);
  void CalcPressStatic();
  void CalcTempStatic();
  void CalcVelocity();

  const fuel::Fuel &fuel_;
  FlowParams flow_params_;

 public:
  Flow1D(const fuel::Fuel &fuel) : fuel_(fuel) {}
  void CalcFlowParams(double p_total, double ksi, FlowType flow_type) {}
  const FlowParams &GetParams() const { return flow_params_; }
};

}  // namespace flow

#endif
