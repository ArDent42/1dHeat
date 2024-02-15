#include "solver.h"

#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "common.h"
#include "log_duration.h"

extern LogDuration dur_res_write;
extern LogDuration dur_solver;
extern LogDuration dur_prevsteps_update;
extern LogDuration dur_volumeprops_update;
extern LogDuration dur_abi;
extern LogDuration dur_t_calc;

MainSolve::MainSolve(const IniData& ini, Mesh& mesh,
                     heat_exchange::HeatExchange& heat, Results& res,
                     Logger& log)
    : ini_(ini), mesh_(mesh), heat_(heat), res_(res), log_(log) {
  size_t size = mesh_.GetVolumes().size();
  alfa.resize(size);
  beta.resize(size);
  A.resize(size);
  B.resize(size);
  C.resize(size);
  F.resize(size);

  out_time_ = ini_.GetSolverSettings().output_timestep;

  for (double time = ini_.GetSolverSettings().output_timestep;
       ini_.GetSolverSettings().solve_time >= time;
       time += ini_.GetSolverSettings().output_timestep) {
    times_output.push_back(time);
  }

  if (ini_.GetSolverSettings().output_times) {
    std::copy(ini_.GetSolverSettings().output_times->begin(),
              ini_.GetSolverSettings().output_times->end(),
              std::back_inserter(times_output));
    auto last = std::unique(
        times_output.begin(), times_output.end(),
        [](double lhs, double rhs) { return std::abs(lhs - rhs) < EPS; });
    times_output.erase(last, times_output.end());
    std::sort(times_output.begin(), times_output.end());
  }
}

void MainSolve::ab_0() {
  double aa;
  lambda = mesh_.GetVolumes().front().l_eff_right_;
  cp = mesh_.GetVolumes().front().cp_sr_;
  ro = mesh_.GetVolumes().front().ro_sr_;
  t_step = ini_.GetSolverSettings().solve_timestep;
  dx = mesh_.GetVolumes().front().dx_right_ * 2.0;
  double r = mesh_.GetVolumes().front().r_right_;
  a = math::Linterp(ini_.GetBoundaryTable().heat_left.alpha,
                    mesh_.GetVolumes().front().t_curr_);
  te = math::Linterp(ini_.GetBoundaryTable().heat_left.te,
                     mesh_.GetVolumes().front().t_curr_);
  eps = math::Linterp(ini_.GetBoundaryTable().heat_left.eps,
                      mesh_.GetVolumes().front().t_curr_);
  trad = math::Linterp(ini_.GetBoundaryTable().heat_left.trad,
                       mesh_.GetVolumes().front().t_curr_);
  q = math::Linterp(ini_.GetBoundaryTable().heat_left.q,
                    mesh_.GetVolumes().front().t_curr_);

  aa = 2 * lambda * r * t_step /
       (2 * lambda * r * t_step + cp * ro * dx * dx + 2 * a * dx * t_step);
  alfa.front() = aa;
  beta.front() =
      aa * dx / lambda / r *
      (q + a * te +
       cp * ro * dx * mesh_.GetVolumes().front().t_prev_step_ / 2 / t_step +
       eps * math::SIGMA *
           (pow(te, 4) - pow(mesh_.GetVolumes().front().t_prev_iter_, 4)));
}

void MainSolve::ab_i_impl() {
  for (size_t i = 1; i < mesh_.GetVolumes().size() - 1; ++i) {
    lambda_l = mesh_.GetVolumes()[i].l_eff_left_;
    lambda_r = mesh_.GetVolumes()[i].l_eff_right_;
    cp = mesh_.GetVolumes()[i].cp_sr_;
    ro = mesh_.GetVolumes()[i].ro_sr_;
    dx = mesh_.GetVolumes()[i].dx_left_ + mesh_.GetVolumes()[i].dx_right_;
    t_step = ini_.GetSolverSettings().solve_timestep;
    double r_l = mesh_.GetVolumes()[i].r_left_;
    double r_r = mesh_.GetVolumes()[i].r_right_;
    double r = mesh_.GetVolumes()[i].r_;
    A.at(i) = lambda_r * r_r / (dx * dx);
    B.at(i) = (lambda_r + lambda_l) / (dx * dx) + ro * cp / t_step;
    C.at(i) = lambda_l * r_l / (dx * dx);
    F.at(i) = -ro * cp * mesh_.GetVolumes()[i].t_prev_step_ / t_step;
    alfa.at(i) = A.at(i) / (B.at(i) - C.at(i) * alfa.at(i - 1));
    beta.at(i) = (C.at(i) * beta.at(i - 1) - F.at(i)) /
                 (B.at(i) - C.at(i) * alfa.at(i - 1));
  }
}

void MainSolve::T_N() {
  double bn;
  lambda = mesh_.GetVolumes().back().l_eff_left_;
  cp = mesh_.GetVolumes().back().cp_sr_;
  ro = mesh_.GetVolumes().back().ro_sr_;
  double r = mesh_.GetVolumes().back().r_left_;
  a = math::Linterp(ini_.GetBoundaryTable().heat_right.alpha,
                    mesh_.GetVolumes().back().t_curr_);
  te = math::Linterp(ini_.GetBoundaryTable().heat_right.te,
                     mesh_.GetVolumes().back().t_curr_);
  eps = math::Linterp(ini_.GetBoundaryTable().heat_right.eps,
                      mesh_.GetVolumes().back().t_curr_);
  trad = math::Linterp(ini_.GetBoundaryTable().heat_right.trad,
                       mesh_.GetVolumes().back().t_curr_);
  q = math::Linterp(ini_.GetBoundaryTable().heat_right.q,
                    mesh_.GetVolumes().back().t_curr_);
  t_step = ini_.GetSolverSettings().solve_timestep;
  bn = 2 * lambda * r * t_step * (1 - alfa[alfa.size() - 2]) +
       cp * ro * dx * dx + 2 * a * dx * t_step;
  mesh_.TCurr(mesh_.GetVolumes().size() - 1) =
      2 * dx * t_step / bn *
      (lambda * r * beta[beta.size() - 2] / dx + q + a * te +
       ro * cp * dx * mesh_.GetVolumes().back().t_prev_step_ / 2 / t_step +
       eps * math::SIGMA *
           (pow(te, 4) - pow(mesh_.GetVolumes().back().t_prev_iter_, 4)));
}

void MainSolve::T() {
  for (int i = mesh_.GetVolumes().size() - 2; i >= 0; --i) {
    mesh_.TCurr(i) = alfa[i] * mesh_.GetVolumes()[i + 1].t_curr_ + beta[i];
  }
}

double MainSolve::Max() {
  double max1 = abs(mesh_.GetVolumes().front().t_curr_ -
                    mesh_.GetVolumes().front().t_curr_);
  for (size_t i = 1; i < mesh_.GetVolumes().size(); ++i) {
    if (max1 < abs(mesh_.GetVolumes()[i].t_curr_ -
                   mesh_.GetVolumes()[i].t_prev_iter_)) {
      max1 = abs(mesh_.GetVolumes()[i].t_curr_ -
                 mesh_.GetVolumes()[i].t_prev_iter_);
    }
  }
  double max2 = abs(mesh_.GetVolumes().front().t_curr_);
  for (int i = 1; i < mesh_.GetVolumes().size(); ++i) {
    if (max2 < abs(mesh_.GetVolumes()[i].t_curr_)) {
      max2 = abs(mesh_.GetVolumes()[i].t_curr_);
    }
  }
  return abs(max1 / max2);
}

double MainSolve::Max_1() {
  double max = abs(mesh_.GetVolumes().front().t_curr_ -
                   mesh_.GetVolumes().front().t_prev_iter_);
  return max;
}

double MainSolve::Max_N() {
  double max = abs(mesh_.GetVolumes().back().t_curr_ -
                   mesh_.GetVolumes().back().t_prev_iter_);
  return max;
}

void MainSolve::solve_impl(bool logging) {
  double max, max_1, max_N, time = 0.0, prev_time = 0.0;
  while ((ini_.GetSolverSettings().solve_time - time) > EPS) {
    prev_time = time;
    time += ini_.GetSolverSettings().solve_timestep;
    log_.Time(time);
    mesh_.TPrevStepUpdate();
    mesh_.UpdateVolumeProps();
    iter = 0;
    if (ini_.GetInitialState().pressure.has_value()) {
      double pressure =
          math::Linterp(ini_.GetInitialState().pressure.value(), time);
      heat_exchange::Result res = heat_.CalcAutomatic(
          pressure, mesh_.GetVolumes().front().t_prev_step_);
      log_.Msg("Pressure: " + std::to_string(pressure));
      log_.Msg((std::stringstream{} << res).str());
    }
    do {
      ++iter;
      mesh_.TPrevIterUpdate();
      ab_0();
      ab_i_impl();
      T_N();
      T();
      max = Max();
      max_1 = Max_1();
      max_N = Max_N();
      log_.SimpleIter(iter, max, max_1, max_N, mesh_);
    } while (max >= EPS_ITER || max_1 >= EPS_ITER || max_N >= EPS_ITER);
    if (out_time_ > prev_time && out_time_ <= time) {
      AddResults(prev_time, out_time_, time);
      out_time_ += ini_.GetSolverSettings().output_timestep;
    }
  }
}

void MainSolve::AddResults(double prev_time, double time, double curr_time) {
  res_.time.push_back(time);
  res_.bound_temp_distr.push_back({});
  TempToCoord temp_coord;
  for (const Volume& vol : mesh_.GetVolumes()) {
    temp_coord.x.push_back(vol.x_);
    temp_coord.temp.push_back(math::Linterp(
        prev_time, curr_time, vol.t_prev_step_, vol.t_curr_, time));
    if (vol.mat_left_ != vol.mat_right_) {
      res_.bound_temp_distr.back().push_back(math::Linterp(
          prev_time, curr_time, vol.t_prev_step_, vol.t_curr_, time));
    }
  }
  res_.temp_coord_distr.push_back(std::move(temp_coord));
}

// void MainSolve::AddResults(double time) {
//   res_.time_.push_back(time);
//   res_.temp_x_.push_back({});
//   res_.temp_t_.push_back({});
//   const auto& vols = mesh_.GetVolumes();
//   size_t size = vols.size();
//   for (size_t i = 0; i < size; ++i) {
//     res_.temp_x_.back()[vols[i].x_] = vols[i].t_curr_;
//     if (vols[i].layer_left_ != vols[i].layer_right_) {
//       res_.temp_t_.back().push_back(vols[i].t_curr_);
//     }
//   }
// }

// void MainSolve::AddTBounds(double time) {
//   res_.temp_t_.push_back({});
//   const auto& vols = mesh_.GetVolumes();
//   size_t size = vols.size();
//   for (size_t i = 0; i < size; ++i) {
//     res_.temp_x_.back()[vols[i].x_] = vols[i].t_curr_;
//     if (vols[i].layer_left_ != vols[i].layer_right_) {
//       res_.temp_t_.back().push_back(vols[i].t_curr_);
//     }
//   }
// }

void MainSolve::Print(std::ostream& out) const {}