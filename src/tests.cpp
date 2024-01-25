#include "logger.h"
#include "test_inidata.h"
#include "test_mat.h"
#include "test_mesh.h"
#include "test_solve.h"

Logger logger("LOG.txt", 1);

int main() {
  // material::TestOpen();
  // material::TestGetProperty();
  // TestCreateEmptyIni();
  // TestParse();
  // TestMesh();
  // TestLeff();
  TestSolver();
}