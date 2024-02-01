#include "logger.h"
#include "test_inidata.h"
#include "test_mat.h"
#include "test_mesh.h"
#include "test_solve.h"

Logger logger("LOG.txt", 1);
LogDuration dur_res_write("Results write");
LogDuration dur_solver("Solving");
LogDuration dur_prevsteps_update("Prev steps update");
LogDuration dur_volumeprops_update("Volume props update");
LogDuration dur_abi("a b i");
LogDuration dur_t_calc("T calc");

int main() {
  // material::TestOpen();
  // material::TestGetProperty();
  // TestCreateEmptyIni();
  // TestParse();
  // TestMesh();
  // TestLeff();
  TestSolver();
}