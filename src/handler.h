#pragma once
#include <filesystem>
#include <string>

#include "boundary_cond.h"
#include "flow.h"
#include "ini_data.h"
#include "logger.h"
#include "mesh.h"
#include "solver.h"

class Handler {
  std::unique_ptr<IniData> ini_data_;
  std::unique_ptr<Logger> logger_;
  std::unique_ptr<base::Database> database_;
  std::unique_ptr<flow::Flow1D> flow_;
  std::unique_ptr<heat_exchange::HeatExchange> heat_exchange_;
  std::unique_ptr<Mesh> mesh_;
  std::unique_ptr<Results> res_;
  std::unique_ptr<MainSolve> solver_;
  std::string output_path_;

 public:
  Handler(const std::string& ini_path, bool logging) {
    ini_data_ = std::make_unique<IniData>(ini_path);
    output_path_ = "output/" + ini_data_->GetDomainSettings().name + '/';
    logger_ = std::make_unique<Logger>(output_path_ + "log.txt");
    database_ = std::make_unique<base::Database>(*ini_data_);
    flow_ = std::make_unique<flow::Flow1D>(database_->GetFuel());
    heat_exchange_ = std::make_unique<heat_exchange::HeatExchange>(*ini_data_,
        database_->GetFuel());
    mesh_ = std::make_unique<Mesh>(*database_, *ini_data_);
    res_ = std::make_unique<Results>(*ini_data_);
    solver_ = std::make_unique<MainSolve>(*mesh_, *ini_data_, *res_, *logger_);
    std::filesystem::create_directories(output_path_);
  }

  heat_exchange::Result CalcAvd() {
    return heat_exchange_->Calc(*ini_data_, heat_exchange::CalcType::avd);
  }

  Handler& Logging(bool b) {
    logger_->Logging(b);
    return *this;
  }
  Handler& SolveiImpl() {
    solver_->solve_impl();
    return *this;
  }
  Handler& ExportPlots() {
    res_->ExportResults(*ini_data_, output_path_);
    return *this;
  }
  Handler& ExportResults() {
    std::ofstream os(output_path_ + "res.csv");
    res_->ExportText(os, ',');
    return *this;
  }
};