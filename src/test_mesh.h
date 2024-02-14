#pragma once

#include <execution>

#include "data_base.h"
#include "log_duration.h"
#include "mesh.h"

// void TestMesh() {
//   IniData ini("ini_data");
//   base::Database database("ini_data");
//   Mesh mesh(database, ini);
//   mesh.UpdateVolumeProps();
//   mesh.PrintGeomDebug(std::cout);
//   mesh.PrintThermDebug(std::cout);
// }

// void TestLeff() {
//   IniData ini("ini_data");
//   base::Database database("ini_data");
//   Mesh mesh(database, ini);
//   int n = 100;
//   {
//     LOG_DURATION("Sequence");
//     for (int i = 0; i < n; ++i)
//       std::for_each(std::execution::seq, mesh.GetVolumes().begin(),
//                     mesh.GetVolumes().end(),
//                     [&](Volume& vol) { vol.CalcProps(); });
//   }
//   {
//     LOG_DURATION("Parallel");
//     for (int i = 0; i < n; ++i)
//       std::for_each(std::execution::par, mesh.GetVolumes().begin(),
//                     mesh.GetVolumes().end(),
//                     [&](Volume& vol) { vol.CalcProps(); });
//   }
//   // mesh.PrintLeff();
// }