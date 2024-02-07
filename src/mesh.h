#pragma once
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "common.h"
#include "data_base.h"
#include "ini_data.h"
#include "material.h"
#include "math.h"

struct Volume {
  double x_;
  double dx_left_ = 0.0;
  double dx_right_ = 0.0;

  double t_curr_;
  double t_prev_step_;
  double t_prev_iter_;

  double l_eff_left_ = 0.0;
  double l_eff_right_ = 0.0;
  double r_left_ = 1.0;
  double r_right_ = 1.0;
  double r_ = 1.0;
  double cp_sr_ = 0.0;
  double ro_sr_ = 0.0;

  const material::Material* mat_left_ = nullptr;
  const material::Material* mat_right_ = nullptr;

  Volume(double x, double t)
      : x_(x), t_curr_(t), t_prev_iter_(t), t_prev_step_(t) {}

  double LambdaEff(const material::Material* mat, const Volume* vol) const;
  double RoCpIntegral(const material::Material* mat, const Volume* vol,
                      material::Property prop) const;

  void CalcProps();
};

class Mesh {
 protected:
  std::vector<Volume> volumes_;
  const base::Database& database_;
  const IniData& ini_data_;

  void AddVolume(const std::map<double, double>& t_init, double r0,
                 double dx = 0.0);

 public:
  Mesh(const base::Database& base, const IniData& ini_data);
  void InitializeMesh(const IniData::Domain& domain,
                      const IniData::InitialState& ini_state);

  void dxCalc();

  void rCalc();

  void TPrevStepUpdate();

  void TPrevIterUpdate();

  void UpdateVolumeProps();

  double& TCurr(size_t id) { return volumes_[id].t_curr_; }

  const std::vector<Volume>& GetVolumes() const { return volumes_; }

  void AddTx();
  void AddTt();

  void PrintGeomDebug(std::ostream& out) const;
  void PrintThermDebug(std::ostream& out) const;
  void Print(std::ostream& out) const;
};

struct TempCoord {
  std::list<double> x;
  std::list<double> temp;
};

struct Results {
  std::list<double> time;
  std::list<TempCoord> temp_coord_distr;
  std::list<std::list<double>> bound_temp_distr;
  void PrintBoundsDistr(std::ostream& os, const IniData& ini) const {
    int w = 20;
    auto time_it = time.begin();
    auto distr_it = bound_temp_distr.begin();
    os.setf(std::ios_base::internal);
    os << std::setw(w) << "t, s" << std::setw(w)
       << "liquid - " + ini.GetDomainSettings().mat_names.front();
    for (size_t i = 0; i < ini.GetDomainSettings().mat_names.size() - 1; ++i) {
      os << std::setw(w)
         << ini.GetDomainSettings().mat_names[i] + " - " +
                ini.GetDomainSettings().mat_names[i + 1];
    }
    os << '\n';
    while (time_it != time.end()) {
      os << std::setw(w) << std::fixed << std::setprecision(2) << *time_it;
      for (double val : *distr_it) {
        os << std::setw(w) << std::fixed << std::setprecision(2) << val;
      }
      os << '\n';
      ++time_it;
      ++distr_it;
    }
  }
  void PrintXDistr(std::ostream& os) const {
    int w = 10;
    auto time_it = time.begin();
    auto coord_it = temp_coord_distr.begin();
    while (time_it != time.end()) {
      os << "Time: " << std::fixed << std::setprecision(3) << *time_it << " sec"
         << '\n';
      auto x_it = coord_it->x.begin();
      auto temp_it = coord_it->temp.begin();
      os << std::setw(w) << "x, mm" << std::setw(w) << "T, K" << '\n';
      while (x_it != coord_it->x.end()) {
        os << std::setw(w) << std::fixed << std::setprecision(2)
           << *x_it * 1000.0 << std::setw(w) << std::fixed
           << std::setprecision(2) << *temp_it << '\n';
        ++x_it;
        ++temp_it;
      }
      ++time_it;
      ++coord_it;
    }
  }
};