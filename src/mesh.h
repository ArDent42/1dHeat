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

struct Layer {
  double thickness;
  double angle;
  uint num_subdiv;
  const material::Material* mat;
};

struct Volume {
  double x_;
  double dx_ = 0.0;
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

  Volume* volume_left_ = nullptr;
  Volume* volume_right_ = nullptr;

  Layer* layer_left_ = nullptr;
  Layer* layer_right_ = nullptr;

  Volume(double x, double t)
      : x_(x), t_curr_(t), t_prev_iter_(t), t_prev_step_(t) {}

  double LambdaEff(Volume* volume, Layer* layer) const;
  double RoCpIntegral(Volume* volume, Layer* layer,
                      material::Property prop) const;

  void CalcProps();
};

class Mesh {
 protected:
  std::vector<Volume> volumes_;
  std::vector<Layer> layers_;
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

struct Results {};

class BaseVolume {
 protected:
  double x_;
  double dx_ = 0.0;
  double ro_ = 0.0;
  double cp_ = 0.0;
  double t_curr_;
  double t_prev_step_;
  double t_prev_iter_;

 public:
  BaseVolume(double x, double t)
      : x_(x), t_curr_(t), t_prev_iter_(t), t_prev_step_(t) {}

  virtual double LambdaEff() const;
  virtual double RoCpIntegral() const;
  virtual void CalcProps();
};

class SideVolume : public BaseVolume {
 private:
  material::Material* mat_ = nullptr;
  double l_eff_ = 0.0;
  double r_ = 0.0;

 public:
  SideVolume(double x, double t, material::Material* mat)
      : BaseVolume(x, t), mat_(mat) {}
};

class InnerVolume : public BaseVolume {
 private:
  material::Material* mat_ = nullptr;
  double l_eff_left_ = 0.0;
  double l_eff_right_ = 0.0;
  double r_left_ = 0.0;
  double r_right_ = 0.0;
  double r = 0.0;
};