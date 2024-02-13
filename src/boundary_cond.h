#pragma once

#include "fuel.h"
#include "flow.h"
#include "common.h"

namespace heat_exchange {

enum class CalcType {
	avd, crit
};

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
	const fuel::Fuel &fuel_;
	flow::Flow1D flow_;
	flow::FlowParams flow_params_;
	fuel::PropValues fuel_props_Tw_;
	fuel::PropValues fuel_props_Tstatic_;
	double CalcAlphaAvd(IniDataAvd ini_data) const;
	double CalcAlphaCrit(double p, double t_w, double r0, double u, flow::FlowType flow_type) const;
	double CalcTe(IniDataAvd ini_data) const;
	double CalcTRad(IniDataAvd ini_data) const;
	double CalcEmissitivity(IniDataAvd ini_data) const;

public:
	HeatExchange(const fuel::Fuel &fuel) :
			fuel_(fuel), flow_(fuel) {
	}
	template<typename T>
	Result Calc(T ini_data, CalcType calc_type);
};

template<typename T>
Result HeatExchange::Calc(T ini_data, CalcType calc_type) {
	Result res;
	if (calc_type == CalcType::avd) {
		flow_params_ = flow_.GetParams(ini_data.p_total, ini_data.r0 / ini_data.r_kr,
			ini_data.flow_type);
		fuel_props_Tw_ = fuel_.GetProperties(flow_params_.p_static, flow_params_.t_total * 0.8);
		fuel_props_Tstatic_ = fuel_.GetProperties(flow_params_.p_static, flow_params_.t_static);
		res.alfa = CalcAlphaAvd(ini_data);
		res.t_e = CalcTe(ini_data);
		res.t_rad = CalcTRad(ini_data);
		res.eps_gas = CalcEmissitivity(ini_data);
	}
	return res;
}

}
