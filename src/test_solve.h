#pragma once

#include "ini_data.h"
#include "logger.h"
#include "mesh.h"
#include "solver.h"

extern Logger logger;

void TestSolver() {
  IniData ini("ini_data");
  base::Database base("ini_data");
  Mesh mesh(base, ini);
  logger.Domain(ini.GetDomainSettings());
  logger.InitialState(ini.GetInitialState());
  Results res;
  MainSolve solver(mesh, ini, res, logger);
  solver.solve_impl();
}