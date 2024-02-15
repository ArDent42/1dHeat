#pragma once
#include <cmath>

#include "fuel.h"

namespace flow {

class Flow1D {
 public:
  enum FlowType {
    subsonic,
    sonic,
    supersonic,
  };

 private:
  double t_total_ = 0.0;
  double t_static_ = 0.0;
  double p_total_ = 0.0;
  double p_static_ = 0.0;
  double u_ = 0.0;
  double mach_ = 0.0;
  double k_ = 0.0;
  double lambda_ = 1.0;
  double ksi_ = 0.0;

  FlowType flow_type_;

  void CalcLambda();
  void CalcPressStatic();
  void CalcTempStatic();
  void CalcVelocity();

  const fuel::Fuel &fuel_;

 public:
  Flow1D(const fuel::Fuel &fuel) : fuel_(fuel) {}
  void CalcFlowParams(double p_total, double ksi, FlowType flow_type);
  double p_static() const {
    return p_static_;
  }
  double t_static() const {
    return t_static_;
  }
  double t_total() const {
    return t_total_;
  }
  double k() const {
    return k_;
  }
  double mach() const {
    return mach_;
  }
  double u() const {
    return u_;
  }
};

}  // namespace flow
