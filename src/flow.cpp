#include <iostream>
#include "flow.h"

namespace flow {

void Flow1D::CalcLambda(FlowParams &flow_params, FlowType flow_type) const {
	double n = 1 / (flow_params.k - 1.0);
	double k1 = flow_params.k + 1.0;
	double k2 = flow_params.k - 1.0;
	double prev_value;
	do {
		prev_value = flow_params.lambda;
		if (flow_type == FlowType::subsonic) {
			flow_params.lambda = 1.0
					/ (pow(k1 / 2.0, n) * pow(1.0 - prev_value * prev_value * k2 / k1, n) * flow_params.ksi
							* flow_params.ksi);
		} else if (flow_type == FlowType::supersonic) {
			flow_params.lambda = k1 / k2 / prev_value
					- 2 / pow(prev_value, flow_params.k) / k2 / pow(flow_params.ksi, 2 * flow_params.k - 2);
		} else {
			flow_params.lambda = 1.0;
			return;
		}
	} while (std::abs(flow_params.lambda - prev_value) > 0.00001);
}

void Flow1D::CalcPressStatic(FlowParams &flow_params) const {
	flow_params.p_static = flow_params.p_total
			* pow(
					(1
							- flow_params.lambda * flow_params.lambda * (flow_params.k - 1)
									/ (flow_params.k + 1)), (flow_params.k / (flow_params.k - 1)));
}

void Flow1D::CalcTempStatic(FlowParams &flow_params) const {
	flow_params.t_static = flow_params.t_total
			* (1
					- flow_params.lambda * flow_params.lambda * (flow_params.k - 1)
							/ (flow_params.k + 1));
}

void Flow1D::CalcVelocity(FlowParams &flow_params) const {
	auto fuel_props_static = fuel_.GetProperties(flow_params.p_static, flow_params.t_static);
	double a = pow(flow_params.k * flow_params.p_static * fuel_props_static.at(fuel::FuelProp::v),
			0.5);
	flow_params.mach = flow_params.lambda * pow(2 / (flow_params.k + 1), 0.5)
			/ pow(
					1
							- (flow_params.k - 1) / (flow_params.k + 1) * flow_params.lambda
									* flow_params.lambda, 0.5);
	flow_params.u = a * flow_params.mach;
}


FlowParams Flow1D::GetParams(double p_total, double ksi, FlowType flow_type) const {
	FlowParams res;
	res.p_total = p_total;
	res.ksi = ksi;
	res.t_total = fuel_.GetTotalTemp(p_total);
	res.k = fuel_.GetProperties(res.p_total, res.t_total).at(fuel::FuelProp::k_eq);
	CalcLambda(res, flow_type);
	CalcPressStatic(res);
	CalcTempStatic(res);
	CalcVelocity(res);
	return res;
}

}
