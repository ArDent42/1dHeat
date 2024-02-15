#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "common.h"
#include "flow.h"
#include "nlohmann/json.hpp"

using ordered_json = nlohmann::ordered_json;
using json = nlohmann::json;

class IniData {
 public:
  struct HeatTransfer {
    std::map<double, double> alpha;
    std::map<double, double> te;
    std::map<double, double> trad;
    std::map<double, double> eps;
    std::map<double, double> q;
  };

  struct Domain {
    std::string name;
    bool axis_symmetry;
    std::string fuel_name;
    std::size_t layers_count;
    std::vector<uint> subdivisions;
    std::vector<double> thickness;
    std::vector<double> angles;
    std::vector<std::string> mat_names;
    std::optional<double> initial_radius;
    std::optional<double> throat_radius;
    std::optional<double> blayer_length;
  };

  struct SolverSettings {
    double solve_time;
    double solve_timestep;
    double output_timestep;
    std::optional<std::vector<double>> output_times;
  };

  struct InitialState {
    std::optional<std::map<double, double>> pressure;
    std::map<double, double> t_initial;
  };

  struct BoundaryConditions {
    flow::Flow1D::FlowType flow_type;
    HeatTransfer heat_left;
    HeatTransfer heat_right;
  };

 private:
  ordered_json ini_data_;
  Domain domain_;
  SolverSettings solver_settings_;
  InitialState initial_state_;
  BoundaryConditions boundary_conditions_;

 public:
  IniData() {
    std::ofstream fout("empty_ini.json");
    CreateEmpty(fout);
  }

  void ProcessDomainData() {
    domain_.name = ini_data_.at("name");
    domain_.axis_symmetry = ini_data_.at("axis symmetry");
    domain_.fuel_name = ini_data_.at("fuel");
    domain_.layers_count = ini_data_.at("layers count");
    domain_.subdivisions = JSONArray2Vector<uint>(ini_data_.at("subdivisions"));
    domain_.thickness = JSONArray2Vector<double>(ini_data_.at("thickness"));
    domain_.angles = JSONArray2Vector<double>(ini_data_.at("angles"));
    domain_.mat_names =
        JSONArray2Vector<std::string>(ini_data_.at("materials"));

    // for (const std::string& name : domain_.mat_names) {
    //   if (!domain_.mat_ids.count(name)) {
    //     size_t i = domain_.mat_ids.size();
    //     domain_.mat_ids[name] = i + 1;
    //   }
    // }

    domain_.initial_radius = ini_data_.at("initial radius");
    domain_.throat_radius = ini_data_.at("throat radius");
    domain_.blayer_length = ini_data_.at("blayer length");
  }

  void ProcessSolverSettings() {
    solver_settings_.solve_time = ini_data_.at("solve time");
    solver_settings_.solve_timestep = ini_data_.at("solve time step");
    solver_settings_.output_timestep = ini_data_.at("output time step");
    solver_settings_.output_times =
        JSONArray2Vector<double>(ini_data_.at("output times"));
  }

  void ProcessInitialData() {
    if (ini_data_.count("pressure")) {
      initial_state_.pressure = JSON2DArray2Map(ini_data_.at("pressure"));
    }
    if (ini_data_.at("initial temp").is_array()) {
      initial_state_.t_initial = JSON2DArray2Map(ini_data_.at("initial temp"));
    } else {
      initial_state_.t_initial[0.0] = ini_data_.at("initial temp");
    }
  }

  void ProcessBoundaryData() {
    auto flow_type = [](const std::string& str) {
      if (str == "subsonic") {
        return flow::Flow1D::FlowType::subsonic;
      } else if (str == "sonic") {
        return flow::Flow1D::FlowType::sonic;
      } else {
        return flow::Flow1D::FlowType::supersonic;
      };
    };

    boundary_conditions_.flow_type = flow_type(ini_data_.at("flow type"));
    boundary_conditions_.heat_left.alpha =
        JSON2DArray2Map(ini_data_.at("boundary left").at("alpha"));
    boundary_conditions_.heat_left.te =
        JSON2DArray2Map(ini_data_.at("boundary left").at("te"));
    boundary_conditions_.heat_left.trad =
        JSON2DArray2Map(ini_data_.at("boundary left").at("trad"));
    boundary_conditions_.heat_left.eps =
        JSON2DArray2Map(ini_data_.at("boundary left").at("eps"));
    boundary_conditions_.heat_left.q =
        JSON2DArray2Map(ini_data_.at("boundary left").at("q"));
    boundary_conditions_.heat_right.alpha =
        JSON2DArray2Map(ini_data_.at("boundary right").at("alpha"));
    boundary_conditions_.heat_right.te =
        JSON2DArray2Map(ini_data_.at("boundary right").at("te"));
    boundary_conditions_.heat_right.trad =
        JSON2DArray2Map(ini_data_.at("boundary right").at("trad"));
    boundary_conditions_.heat_right.eps =
        JSON2DArray2Map(ini_data_.at("boundary right").at("eps"));
    boundary_conditions_.heat_right.q =
        JSON2DArray2Map(ini_data_.at("boundary right").at("q"));
  }

  void ProcessData() {
    ProcessDomainData();
    ProcessSolverSettings();
    ProcessInitialData();
    ProcessBoundaryData();
  }

  IniData(std::ifstream& fin) {
    ini_data_ = ordered_json::parse(fin);
    ProcessData();
  }

  IniData(const std::string& name) {
    std::ifstream fin(name + ".json");
    ini_data_ = ordered_json::parse(fin);
    ProcessData();
  }

  void CreateEmpty(std::ofstream& fout) {
    ini_data_ = {{"name", {}},
                 {"axis symmetry", {}},
                 {"layers count", {}},
                 {"subdivisions", json::array()},
                 {"thickness", json::array()},
                 {"angles", json::array()},
                 {"materials", json::array()},
                 {"solve time", {}},
                 {"solve time step", {}},
                 {"output time step", {}},
                 {"output times", json::array()},
                 {"fuel", {}},
                 {"initial radius", {}},
                 {"throat radius", {}},
                 {"pressure", json::array({json::array(), json::array()})},
                 {"blayer length", {}},
                 {"flow type", {}},
                 {"boundary left",
                  {{"alpha", json::array({json::array(), json::array()})},
                   {"te", json::array({json::array(), json::array()})},
                   {"trad", json::array({json::array(), json::array()})},
                   {"eps", json::array({json::array(), json::array()})},
                   {"q", json::array({json::array(), json::array()})}}},
                 {"boundary right",
                  {{"alpha", json::array({json::array(), json::array()})},
                   {"te", json::array({json::array(), json::array()})},
                   {"trad", json::array({json::array(), json::array()})},
                   {"eps", json::array({json::array(), json::array()})},
                   {"q", json::array({json::array(), json::array()})}},
                  {"initial temp", {json::array(), json::array()}}}};
    fout << ini_data_.dump(4);
  };

  const Domain& GetDomainSettings() const { return domain_; }
  const InitialState& GetInitialState() const { return initial_state_; }
  const SolverSettings& GetSolverSettings() const { return solver_settings_; }
  const BoundaryConditions& GetBoundaryTable() const {
    return boundary_conditions_;
  }
};
