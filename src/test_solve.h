#pragma once

#include "ini_data.h"
#include "logger.h"
#include "mesh.h"
#include "solver.h"
#include "handler.h"

extern LogDuration dur_solver;

void TestSolver() {
  std::stringstream ss;
  IniData ini("ini_data");
  Logger logger("output/" + ini.GetDomainSettings().name + "/LOG.txt", 1);
  base::Database base(ini);
  flow::Flow1D flow(base.GetFuel(ini.GetDomainSettings().fuel_name));
  Mesh mesh(base, ini);
  Results res(ini);
  logger.Domain(ini.GetDomainSettings());
  logger.InitialState(ini.GetInitialState());
  MainSolve solver(mesh, ini, res, logger);
  dur_solver.Start();
  solver.solve_impl();
  dur_solver.Stop();
  res.Print(ss);
  std::ofstream out("output/" + ini.GetDomainSettings().name + "/" + "results.txt");
  out << ss.str();
  res.ExportResults(ini);
}

void TestSolver() {
  Handler handler("ini_data.json", true);
  handler.SolveiImpl().EportResults().ExportPlots();
}


