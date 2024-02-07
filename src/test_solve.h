#pragma once

#include "ini_data.h"
#include "logger.h"
#include "mesh.h"
#include "solver.h"

extern Logger logger;
extern LogDuration dur_solver;

void TestSolver() {
  std::stringstream ss;
  IniData ini("ini_data");
  base::Database base("ini_data");
  Mesh mesh(base, ini);
  logger.Domain(ini.GetDomainSettings());
  logger.InitialState(ini.GetInitialState());
  Results res;
  MainSolve solver(mesh, ini, res, logger);
  dur_solver.Start();
  solver.solve_impl();
  dur_solver.Stop();
  res.PrintBoundsDistr(ss, ini);
  res.PrintXDistr(ss);
  std::ofstream out("res.txt");
  out << ss.str();
}