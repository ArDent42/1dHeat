#pragma once
#include <sstream>
#include <string>

#include "mesh.h"

class Logger {
 private:
  bool logging = 0;
  std::stringstream log_;
  std::string output_name;

 public:
  Logger(const std::string& name)
      : output_name(name) {}

  void Time(double time) {
    if (logging) {
      log_ << '\n' << "Time: " << time << " s" << '\n';
    }
  }
  void SimpleIter(size_t iter, double max, double max_1, double max_N,
                  const Mesh& mesh) {
    log_.setf(std::ios_base::left);
    log_.precision(2);
    int w = 14;
    log_ << "Iter: " << std::setw(w) << iter << "Max: " << std::setw(w) << max
         << "Max left: " << std::setw(w) << max_1
         << "Max right: " << std::setw(w) << max_N << "T left: " << std::setw(w)
         << mesh.GetVolumes().front().t_curr_ << "T right: " << std::setw(w)
         << mesh.GetVolumes().back().t_curr_ << '\n';
  }

  void Msg(const std::string& msg) { log_ << msg << '\n'; }

  void Domain(const IniData::Domain& domain) {
    log_ << "Name: " << domain.name << '\n';
    log_ << "Fuel: " << domain.fuel_name << '\n';
    log_ << "Layers count: " << domain.layers_count << '\n';
    for (size_t i = 0; i < domain.layers_count; ++i) {
      log_ << "Material: " << domain.mat_names[i];
      log_ << " Thickness: " << domain.thickness[i];
      log_ << " Angle: " << domain.angles[i];
      log_ << " Subdivisions " << domain.subdivisions[i] << '\n';
    }
    if (domain.initial_radius) {
      log_ << "Initial radius: " << domain.initial_radius.value() << '\n';
    }
    if (domain.throat_radius) {
      log_ << "Throat radius: " << domain.throat_radius.value() << '\n';
    }
    if (domain.blayer_length) {
      log_ << "Blayer length: " << domain.blayer_length.value() << '\n';
    }
    log_ << '\n';
  }

  void InitialState(const IniData::InitialState& state) {
    log_.setf(std::ios_base::left);
    int w = 10;
    log_ << std::setw(w) << "t, s:";
    for (const auto [t, temp] : state.t_initial) {
      log_ << std::setw(w) << t;
    }
    log_ << '\n';
    log_ << std::setw(w) << "T, K:";
    for (const auto [t, temp] : state.t_initial) {
      log_ << std::setw(w) << temp;
    }
    log_ << '\n' << '\n';
  }

  void Logging(bool b) { logging = b; }
  ~Logger() {
    std::ofstream fout(output_name);
    fout << log_.str();
  }
};