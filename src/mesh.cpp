#include "mesh.h"

#include <cmath>

double Volume::LambdaEff(Volume* volume, Layer* layer) const {
  auto lambda = [&volume, &layer, this](double x) -> double {
    return 1 /
           layer->mat->GetProperty(math::Linterp(x_, volume->x_, t_prev_step_,
                                                 volume->t_prev_step_, x),
                                   material::Property::l);
  };
  return volume->dx_ / math::Integral(std::min(volume->x_, x_),
                                      std::max(volume->x_, x_), lambda);
}

double Volume::RoCpIntegral(Volume* volume, Layer* layer,
                            material::Property prop) const {
  auto f = [&layer, &prop, &volume, this](double x) {
    return layer->mat->GetProperty(
        math::Linterp(x_, (x_ + volume->x_) / 2.0, t_prev_step_,
                      volume->t_prev_step_, x),
        prop);
  };
  return math::Integral(std::min(x_, (x_ + volume->x_) / 2.0),
                        std::max(x_, (x_ + volume->x_) / 2.0), f);
}

void Volume::CalcProps() {
  l_eff_left_ = 0.0;
  l_eff_right_ = 0.0;
  ro_sr_ = 0.0;
  cp_sr_ = 0.0;
  double l = 0.0;
  if (volume_left_) {
    l_eff_left_ = LambdaEff(volume_left_, layer_left_);
    cp_sr_ += RoCpIntegral(volume_left_, layer_left_, material::Property::cp);
    ro_sr_ += RoCpIntegral(volume_left_, layer_left_, material::Property::ro);
    l += std::abs(x_ - volume_left_->x_) / 2.0;
  }
  if (volume_right_) {
    l_eff_right_ = LambdaEff(volume_right_, layer_right_);
    cp_sr_ += RoCpIntegral(volume_right_, layer_right_, material::Property::cp);
    ro_sr_ += RoCpIntegral(volume_right_, layer_right_, material::Property::ro);
    l += std::abs(x_ - volume_right_->x_) / 2.0;
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
  const IniData::Domain& domain = ini_data_.GetDomainSettings();
  for (size_t i = 0; i < domain.layers_count; ++i) {
    layers_.push_back(Layer{domain.thickness[i], domain.angles[i],
                            domain.subdivisions[i],
                            &database_.GetMaterial(domain.mat_names[i])});
  }
  InitializeMesh(domain, ini_data_.GetInitialState());
}

void Mesh::InitializeMesh(const IniData::Domain& domain,
                          const IniData::InitialState& ini_state) {
  volumes_.clear();
  uint sum_nodes = std::transform_reduce(
      layers_.begin(), layers_.end(), 1, std::plus(),
      [](const Layer& layer) { return layer.num_subdiv; });
  volumes_.reserve(sum_nodes);
  AddVolume(ini_state.t_initial, domain.initial_radius.value());
  for (int i = 0; i < layers_.size(); ++i) {
    double dx = layers_[i].thickness / layers_[i].num_subdiv;
    for (int j = 1; j <= layers_[i].num_subdiv; ++j) {
      AddVolume(ini_state.t_initial, domain.initial_radius.value(), dx);
      volumes_.back().layer_left_ = &layers_[i];
      (&volumes_.back() - 1)->layer_right_ = &layers_[i];

      volumes_.back().volume_left_ = &volumes_.back() - 1;
      (&volumes_.back() - 1)->volume_right_ = &volumes_.back();
    }
  }
  dxCalc();
  if (ini_data_.GetDomainSettings().axis_symmetry) {
    rCalc();
  }
}

void Mesh::dxCalc() {
  volumes_.front().dx_ = volumes_[1].x_ - volumes_[0].x_;
  for (size_t i = 1; i < volumes_.size() - 1; ++i) {
    volumes_[i].dx_ = (volumes_[i + 1].x_ - volumes_[i - 1].x_) / 2.0;
  }
  volumes_.back().dx_ = (volumes_.end() - 2)->dx_;
}

void Mesh::rCalc() {
  if (volumes_.front().x_ < EPS) {
    volumes_.front().r_right_ = 0.0;
  } else {
    volumes_.front().r_right_ =
        (volumes_.front().x_ + volumes_.front().x_ + volumes_.front().dx_) /
        2.0 / volumes_.front().x_;
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
        << vol.dx_;
    if (vol.layer_left_) {
      out << std::setw(w) << vol.layer_left_->mat->GetName();
    } else {
      out << std::setw(w) << "";
    }
    if (vol.layer_right_) {
      out << std::setw(w) << vol.layer_right_->mat->GetName();
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
    if (vol.layer_left_) {
      out << std::setw(w) << vol.layer_left_->mat->GetName();
    } else {
      out << std::setw(w) << "";
    }
    if (vol.layer_right_) {
      out << std::setw(w) << vol.layer_right_->mat->GetName();
    } else {
      out << std::setw(w) << "";
    }
    out << '\n';
  }
}