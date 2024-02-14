#pragma once
#include <matplot/matplot.h>

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

struct TempToCoord {
  std::list<double> x;
  std::list<double> temp;
};

struct Results {
  const IniData& ini_;
  std::vector<double> time;
  std::vector<TempToCoord> temp_coord_distr;
  std::vector<std::vector<double>> bound_temp_distr;

  Results(const IniData& ini) : ini_(ini) {}

  void PrintBoundsDistr(std::ostream& os, const char delimiter) const {
    auto time_it = time.begin();
    auto distr_it = bound_temp_distr.begin();
    os << "t, s" << delimiter << "liquid";
    for (size_t i = 0; i < ini_.GetDomainSettings().mat_names.size(); ++i) {
      os << delimiter << ini_.GetDomainSettings().mat_names[i];
    }
    os << '\n';
    for (size_t i = 0; i < ini_.GetDomainSettings().mat_names.size(); ++i) {
      os << delimiter << ini_.GetDomainSettings().mat_names[i];
    }
    os << delimiter << "liquid";
    os << '\n';
    while (time_it != time.end()) {
      os << std::fixed << std::setprecision(2) << *time_it;
      for (double val : *distr_it) {
        os << delimiter << std::fixed << std::setprecision(2) << val;
      }
      os << '\n';
      ++time_it;
      ++distr_it;
    }
    os << '\n';
  }

  void PrintXDistr(std::ostream& os, const char delimiter) const {
    auto time_it = time.begin();
    auto coord_it = temp_coord_distr.begin();
    while (time_it != time.end()) {
      os << "Time: " << std::setprecision(3) << *time_it << " sec"
         << '\n';
      auto x_it = coord_it->x.begin();
      auto temp_it = coord_it->temp.begin();
      os << "x, mm" << delimiter << "T, K" << '\n';
      while (x_it != coord_it->x.end()) {
        os << std::fixed << std::setprecision(2)
           << *x_it * 1000.0 << delimiter << std::fixed
           << std::setprecision(2) << *temp_it << '\n';
        ++x_it;
        ++temp_it;
      }
      ++time_it;
      ++coord_it;
    }
  }

  void ExportText(std::ostream& os, const char delimiter) {
    PrintBoundsDistr(os, delimiter);
    PrintXDistr(os, delimiter);
  }

  template <typename X, typename Y>
  void SavePlot(const X& x, const Y& y, const std::string& title,
                const std::string& label_x, const std::string& label_y,
                const std::string& output_path) {
    matplot::plot(x, y)->line_width(2.0);
    matplot::grid(matplot::on);
    matplot::gca()->minor_grid(true);
    matplot::title(title);
    matplot::xlabel(label_x);
    matplot::ylabel(label_y);
    matplot::save(output_path + title + ".jpg");
  }

  void ExportTtoT(const IniData& ini, const std::string& output_path) {
    auto it = ini.GetDomainSettings().mat_names.begin();
    std::string path = output_path + "T(t)/";
    for (size_t i = 0; i < bound_temp_distr.front().size(); ++i) {
      std::string title;
      std::vector<double> res;
      std::transform(this->bound_temp_distr.begin(),
                     this->bound_temp_distr.end(), std::back_inserter(res),
                     [i](const auto& temp) { return temp[i]; });
      if (it == ini.GetDomainSettings().mat_names.begin()) {
        title = "liquid - ";
        SavePlot(time, res, title + *it, "t, s", "T, K", path);
      } else if (it == ini.GetDomainSettings().mat_names.end()) {
        title = " - liquid";
        SavePlot(time, res, *std::prev(it) + title, "t, s", "T, K", path);
      } else {
        SavePlot(time, res, *std::prev(it) + " - " + *it, "t, s", "T, K", path);
      }
      ++it;
    }
  }

  void ExportTtoX(const IniData& ini, const std::string& output_path) {
    auto it = time.begin();
    std::string path = output_path + "T(x)/";
    for (const auto& temp : temp_coord_distr) {
      SavePlot(temp.x, temp.temp, std::to_string(*it), "x, m", "T, K", path);
      ++it;
    }
  }

  void ExportResults(const IniData& ini, const std::string& output_path) {
    ExportTtoT(ini, output_path);
    ExportTtoX(ini, output_path);
  }

  void ShowPlot() const {
    using namespace matplot;
    auto handle =
        matplot::plot(matplot::transform(
                          std::vector<double>(temp_coord_distr.back().x.begin(),
                                              temp_coord_distr.back().x.end()),
                          [](auto x) { return 1000.0 * x; }),
                      temp_coord_distr.back().temp)
            ->line_width(3.0);
    matplot::grid(matplot::on);
    matplot::gca()->minor_grid(matplot::on);
    matplot::show();
  }
};