#include "mesh.h"

#include <cmath>
#include <execution>

double Volume::LambdaEff(const material::Material* mat,
                         const Volume* vol) const {
  auto lambda = [&vol, &mat, this](double x) -> double {
    return 1 / mat->GetProperty(math::Linterp(x_, vol->x_, t_prev_step_,
                                              vol->t_prev_step_, x),
                                material::Property::l);
  };
  return (std::abs(x_ - vol->x_)) /
         math::Integral(std::min(vol->x_, x_), std::max(vol->x_, x_), lambda);
}

double Volume::RoCpIntegral(const material::Material* mat, const Volume* vol,
                            material::Property prop) const {
  auto f = [&mat, &prop, &vol, this](double x) {
    return mat->GetProperty(math::Linterp(x_, (x_ + vol->x_) / 2.0,
                                          t_prev_step_, vol->t_prev_step_, x),
                            prop);
  };
  return math::Integral(std::min(x_, (x_ + vol->x_) / 2.0),
                        std::max(x_, (x_ + vol->x_) / 2.0), f);
}

void Volume::CalcProps() {
  l_eff_left_ = 0.0;
  l_eff_right_ = 0.0;
  ro_sr_ = 0.0;
  cp_sr_ = 0.0;
  double l = 0.0;
  if (mat_left_) {
    l_eff_left_ = LambdaEff(mat_left_, this - 1);
    cp_sr_ += RoCpIntegral(mat_left_, this - 1, material::Property::cp);
    ro_sr_ += RoCpIntegral(mat_left_, this - 1, material::Property::ro);
    l += std::abs(x_ - (this - 1)->x_) / 2.0;
  }
  if (mat_right_) {
    l_eff_right_ = LambdaEff(mat_right_, this + 1);
    cp_sr_ += RoCpIntegral(mat_right_, this + 1, material::Property::cp);
    ro_sr_ += RoCpIntegral(mat_right_, this + 1, material::Property::ro);
    l += std::abs(x_ - (this + 1)->x_) / 2.0;
  }
  cp_sr_ /= l;
  ro_sr_ /= l;
}

void Mesh::AddVolume(const std::map<double, double>& t_init, double r0,
                     double dx) {
  double x;
  if (volumes_.empty()) {
    x = r0;
  } else {
    x = volumes_.back().x_ + dx;
  }
  if (t_init.size() == 1) {
    volumes_.emplace_back(x, t_init.begin()->second);
  } else {
    volumes_.emplace_back(x, math ::Linterp(t_init, x));
  }
}

Mesh::Mesh(const base::Database& base, const IniData& ini_data)
    : database_(base), ini_data_(ini_data) {
  InitializeMesh(ini_data_.GetDomainSettings(), ini_data_.GetInitialState());
}

void Mesh::InitializeMesh(const IniData::Domain& domain,
                          const IniData::InitialState& ini_state) {
  const auto& ini = ini_data_.GetDomainSettings();
  volumes_.clear();
  volumes_.reserve(
      std::accumulate(ini.subdivisions.begin(), ini.subdivisions.end(), 0));

  AddVolume(ini_state.t_initial, domain.initial_radius.value());

  for (int i = 0; i < ini.layers_count; ++i) {
    double dx = ini.thickness[i] / ini.subdivisions[i];
    for (int j = 1; j <= ini.subdivisions[i]; ++j) {
      AddVolume(ini_state.t_initial, domain.initial_radius.value(), dx);
      volumes_.back().mat_left_ = &database_.GetMaterial(ini.mat_names[i]);
      (&volumes_.back() - 1)->mat_right_ =
          &database_.GetMaterial(ini.mat_names[i]);
    }
  }

  dxCalc();

  if (ini_data_.GetDomainSettings().axis_symmetry) {
    rCalc();
  }
}

void Mesh::dxCalc() {
  for (size_t i = 1; i < volumes_.size() - 1; ++i) {
    volumes_[i].dx_left_ = (volumes_[i].x_ - volumes_[i - 1].x_) / 2.0;
    volumes_[i].dx_right_ = (volumes_[i + 1].x_ - volumes_[i].x_) / 2.0;
    if (i == 1) {
      volumes_[i - 1].dx_right_ = volumes_[i].dx_left_;
      continue;
    }
    if (i == volumes_.size() - 2) {
      volumes_[i + 1].dx_left_ = volumes_[i].dx_right_;
    }
  }
}

void Mesh::rCalc() {
  if (volumes_.front().x_ < EPS) {
    volumes_.front().r_right_ = 0.0;
  } else {
    volumes_.front().r_right_ =
        (volumes_.front().x_ + volumes_.front().dx_right_) /
        volumes_.front().x_;
  }
  for (size_t i = 1; i < volumes_.size() - 1; ++i) {
    volumes_[i].r_left_ =
        (volumes_[i - 1].x_ + volumes_[i].x_) / 2.0 / volumes_[i].x_;
    volumes_[i].r_right_ =
        (volumes_[i].x_ + volumes_[i + 1].x_) / 2.0 / volumes_[i].x_;
    volumes_[i].r_ = volumes_[i].r_left_ + volumes_[i].r_right_;
  }
  volumes_.back().r_left_ = ((volumes_.end() - 2)->x_ + volumes_.back().x_) /
                            2.0 / volumes_.back().x_;
}

void Mesh::UpdateVolumeProps() {
  for (Volume& vol : volumes_) {
    vol.CalcProps();
  }
  // for_each(std::execution::par, volumes_.begin(), volumes_.end(),
  //          [](Volume& vol) { vol.CalcProps(); });
}

void Mesh::TPrevStepUpdate() {
  for (Volume& vol : volumes_) {
    vol.t_prev_step_ = vol.t_curr_;
  }
}

void Mesh::TPrevIterUpdate() {
  for (Volume& vol : volumes_) {
    vol.t_prev_iter_ = vol.t_curr_;
  }
}

void Mesh::PrintGeomDebug(std::ostream& out) const {
  out.setf(std::ios_base::left);
  int w = 15;
  out << std::setw(w) << "x, m" << std::setw(w) << "T_curr, K" << std::setw(w)
      << "T_prev_step, K" << std::setw(w) << "T_prev_iter, K" << std::setw(w)
      << "dx" << std::setw(w) << "Mat_left" << std::setw(w) << "Mat_right"
      << '\n';
  for (const Volume& vol : volumes_) {
    out << std::setw(w) << vol.x_ << std::setw(w) << vol.t_curr_ << std::setw(w)
        << vol.t_prev_step_ << std::setw(w) << vol.t_prev_iter_ << std::setw(w)
        << vol.dx_left_ + vol.dx_right_;
    if (vol.mat_left_) {
      out << std::setw(w) << vol.mat_left_->GetName();
    } else {
      out << std::setw(w) << "";
    }
    if (vol.mat_right_) {
      out << std::setw(w) << vol.mat_right_->GetName();
    } else {
      out << std::setw(w) << "";
    }
    out << '\n';
  }
}

void Mesh::Print(std::ostream& out) const {
  out.setf(std::ios_base::left);
  int w = 15;
  out << "x, m" << ';' << "T, K" << '\n';
  for (const Volume& vol : volumes_) {
    out << vol.x_ << ';' << vol.t_curr_ << '\n';
  }
}

void Mesh::PrintThermDebug(std::ostream& out) const {
  out.setf(std::ios_base::left);
  int w = 15;
  out << std::setw(w) << "x, m" << std::setw(w) << "l_eff_left" << std::setw(w)
      << "l_eff_right" << std::setw(w) << "cp_sr" << std::setw(w) << "ro_sr"
      << '\n';
  for (const Volume& vol : volumes_) {
    out << std::setw(w) << vol.x_ << std::setw(w) << vol.l_eff_left_
        << std::setw(w) << vol.l_eff_right_ << std::setw(w) << vol.cp_sr_
        << std::setw(w) << vol.ro_sr_;
    if (vol.mat_left_) {
      out << std::setw(w) << vol.mat_left_->GetName();
    } else {
      out << std::setw(w) << "";
    }
    if (vol.mat_right_) {
      out << std::setw(w) << vol.mat_right_->GetName();
    } else {
      out << std::setw(w) << "";
    }
    out << '\n';
  }
}