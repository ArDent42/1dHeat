// #pragma once

// #include <string>
// #include <vector>

// using std::string;
// using std::vector;

// class MainSolve {
//     string name = "";
//     vector<double> l_ = {0.005, 0.01, 0.005};
//     vector<int> subdiv_ = {50, 50, 50};
//     int N_ = 0;
//     int n_ = 3;
//     vector<double> h_;
//     double r0 = 0;
//     double a1 = 4298.59;
//     double a2 = 5;
//     double Te1 = 3350.7;
//     double Te2 = 293;
//     double Tr1 = 3156.3;
//     double Tr2 = 293;
//     double e1 = 0.57;
//     double e2 = 0.8;
//     double q1 = 0;
//     double q2 = 0;
//     double t_end_ = 30;
//     double t_step_ = 0.01;
//     double time_ = 0.0;
//     int n_iter = 0;
//     int count_ = 0;
//     double cp_1 = 0, cp_i = 0, cp_N = 0, lambda_1 = 0, l1 = 0, l2 = 0,
//     lambda_N = 0; vector<double> x_; vector<double> T_; vector<double> Tn_;
//     vector<double> Ts_;
//     vector<double> A, B, C, F;
//     vector<double> alfa, beta;
//     vector<vector<double>> l { {0.55, 0.57, 0.6, 0.5, 0.3, 0.3, 0.37,
//     0.8, 2.85, 5.1, 7.1, 9.2, 12.5, 14.8, 16}, 		{0.55, 0.57,
//     0.6, 0.5, 0.3, 0.3, 0.37, 0.8, 2.85, 5.1, 7.1, 9.2, 12.5, 14.8, 16},
// 			{0.55, 0.57, 0.6, 0.5, 0.3, 0.3, 0.37,
// 0.8, 2.85, 5.1, 7.1, 9.2, 12.5, 14.8, 16} };
//     vector<vector<double>> T_l { {273, 300, 400, 600, 800, 1000, 1200, 1500,
//     1800, 2100, 2400, 2700, 3000, 3300, 4000} }; vector<vector<double>> cp {
//     {629, 715, 1045, 1420, 1500, 1550, 1560, 1590, 1670, 1775, 1800, 1820,
//     1840, 1880}, 		{629, 715, 1045, 1420, 1500, 1550, 1560, 1590,
//     1670, 1775, 1800, 1820, 1840, 1880},
// 			{629, 715, 1045, 1420, 1500, 1550, 1560, 1590, 1670,
// 1775, 1800, 1820, 1840, 1880} };
//     vector<vector<double>>  Tcp { {273, 300, 400, 600, 800, 1000, 1200, 1500,
//     1800, 2100, 2400, 2700, 3000, 3300} }; double ro = 1450; char D = ';';
// public:
//     MainSolve();
//     string file_name() { return name; }
//     void ini_temp(double T0);
//     void ab_0();
//     void ab_i_impl();
//     void ab_i_CN();
//     void T_N();
//     void T();
//     double Max_1();
//     double Max_N();
//     double Max();
//     double lambda_ef(const double &a, const double &b, const int& N);
//     double cp_sr(const double& a, const double& b, const int& n);
//     void solve_impl();
//     void solve_CN();
//     int count() { return count_; }
//     friend std::ostream &operator<<(std::ostream &os, MainSolve &M);
// };

#pragma once

#include "ini_data.h"
#include "logger.h"
#include "mesh.h"

class MainSolve {
  Mesh& mesh_;
  const IniData& ini_;
  Logger& log_;
  Results& res_;
  std::vector<double> alfa;
  std::vector<double> beta;
  std::vector<double> A;
  std::vector<double> B;
  std::vector<double> C;
  std::vector<double> F;
  double lambda, lambda_l, lambda_r, ro, cp, dx, t_step, a, te, eps, trad, q;
  size_t iter = 0;
  std::list<double> times_output;
  std::stringstream log;

 public:
  MainSolve(Mesh& mesh, const IniData& ini, Results& res, Logger& log);
  void ab_0();
  void ab_i_impl();
  void T_N();
  void T();
  double Max_1();
  double Max_N();
  double Max();
  void solve_impl(bool logging = 0);
  void Print(std::ostream& out) const;
  void AddResults(double time);
};