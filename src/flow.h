#ifndef FLOW_H_
#define FLOW_H_
#include <cmath>
#include "fuel.h"

namespace flow {

enum FlowType {
	subsonic, sonic, supersonic,
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
	double Lambda(double prev_value, FlowType flow_type) const;
	void CalcLambda(FlowParams& flow_params, FlowType flow_type) const;
	void CalcPressStatic(FlowParams& flow_params) const;
	void CalcTempStatic(FlowParams& flow_params) const;
	void CalcVelocity(FlowParams& flow_params) const;

	const fuel::Fuel &fuel_;

public:
	Flow1D(const fuel::Fuel &fuel) :
			fuel_(fuel) {}
	FlowParams GetParams(double p_total, double ksi, FlowType flow_type) const;
};

}

#endif
