#include "SubChannel1PhaseProblem.h"
#include "SystemBase.h"
#include "libmesh/petsc_vector.h"
#include <petscdm.h>
#include <petscdmda.h>
#include <petscksp.h>
#include <petscsys.h>
#include <petscvec.h>
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include "SinglePhaseFluidProperties.h"
#include "SolutionHandle.h"
#include "AuxiliarySystem.h"

registerMooseObject("SubChannelApp", SubChannel1PhaseProblem);

InputParameters
SubChannel1PhaseProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  return params;
}

SubChannel1PhaseProblem::SubChannel1PhaseProblem(const InputParameters & params)
  : ExternalProblem(params),
    _g_grav(9.87),
    _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh)),
    _fp(nullptr)
{
  // Turbulent crossflow
  WijPrime.resize(_subchannel_mesh._n_gaps);
  Wij.resize(_subchannel_mesh._n_gaps);
  Wij_old.resize(_subchannel_mesh._n_gaps);
  WijPrime_global.resize(_subchannel_mesh._n_gaps, _subchannel_mesh._nz + 1);
  Wij_global.resize(_subchannel_mesh._n_gaps, _subchannel_mesh._nz + 1);
  Wij_global_old.resize(_subchannel_mesh._n_gaps, _subchannel_mesh._nz + 1);
  Wij_global.setZero();
  Wij_global_old.setZero();
  WijPrime.setZero();
  WijPrime_global.setZero();
}

void
SubChannel1PhaseProblem::initialSetup()
{
  ExternalProblem::initialSetup();
  // Read in Wij_global for null transient only at the first run of externalSolve
  if (isTransient())
  {
    std::ifstream file("Wij_global");
    std::string line_str;
    int row_index = 0;

    while (std::getline(file, line_str))
    {
      int column_index = 0;
      std::stringstream str_strm(line_str);
      std::string tmp;
      char delim = ' '; // Define the delimiter to split by
      while (std::getline(str_strm, tmp, delim))
      {
        // Provide proper checks here for tmp like if empty
        // Also strip down symbols like !, ., ?, etc.
        // Finally push it.
        if (tmp.size() != 0)
        {
          Wij_global(row_index, column_index) = std::stof(tmp);
          Wij_global_old(row_index, column_index) = std::stof(tmp);
          column_index += 1;
        }
      }
      row_index += 1;
    }
  }

  _fp = &getUserObject<SinglePhaseFluidProperties>(getParam<UserObjectName>("fp"));
  mdot_soln = new SolutionHandle(getVariable(0, "mdot"));
  SumWij_soln = new SolutionHandle(getVariable(0, "SumWij"));
  SumWijh_soln = new SolutionHandle(getVariable(0, "SumWijh"));
  SumWijPrimeDhij_soln = new SolutionHandle(getVariable(0, "SumWijPrimeDhij"));
  SumWijPrimeDUij_soln = new SolutionHandle(getVariable(0, "SumWijPrimeDUij"));
  P_soln = new SolutionHandle(getVariable(0, "P"));
  DP_soln = new SolutionHandle(getVariable(0, "DP"));
  h_soln = new SolutionHandle(getVariable(0, "h"));
  T_soln = new SolutionHandle(getVariable(0, "T"));
  rho_soln = new SolutionHandle(getVariable(0, "rho"));
  S_flow_soln = new SolutionHandle(getVariable(0, "S"));
  w_perim_soln = new SolutionHandle(getVariable(0, "w_perim"));
  q_prime_soln = new SolutionHandle(getVariable(0, "q_prime"));
}

SubChannel1PhaseProblem::~SubChannel1PhaseProblem()
{
  delete mdot_soln;
  delete SumWij_soln;
  delete SumWijh_soln;
  delete SumWijPrimeDhij_soln;
  delete SumWijPrimeDUij_soln;
  delete P_soln;
  delete DP_soln;
  delete h_soln;
  delete T_soln;
  delete rho_soln;
  delete S_flow_soln;
  delete w_perim_soln;
  delete q_prime_soln;
}

bool
SubChannel1PhaseProblem::converged()
{
  return true;
}

double
SubChannel1PhaseProblem::computeFrictionFactor(double Re)
{
  double a, b;
  if (Re < 1.0)
  {
    return 64.0;
  }
  else if (Re >= 1.0 && Re < 5000.0)
  {
    a = 64.0;
    b = -1.0;
  }
  else if (Re >= 5000.0 && Re < 30000.0)
  {
    a = 0.316;
    b = -0.25;
  }
  else
  {
    a = 0.184;
    b = -0.20;
  }
  return a * std::pow(Re, b);
}

void
SubChannel1PhaseProblem::computeWij(int iz)
{
  auto dz = _subchannel_mesh._z_grid[iz] - _subchannel_mesh._z_grid[iz - 1];
  for (unsigned int i_gap = 0; i_gap < _subchannel_mesh._n_gaps; i_gap++)
  {
    unsigned int i_ch = _subchannel_mesh._gap_to_chan_map[i_gap].first;
    unsigned int j_ch = _subchannel_mesh._gap_to_chan_map[i_gap].second;
    auto * node_in_i = _subchannel_mesh._nodes[i_ch][iz - 1];
    auto * node_out_i = _subchannel_mesh._nodes[i_ch][iz];
    auto * node_in_j = _subchannel_mesh._nodes[j_ch][iz - 1];
    auto * node_out_j = _subchannel_mesh._nodes[j_ch][iz];
    auto rho_i = (*rho_soln)(node_in_i);
    auto rho_j = (*rho_soln)(node_in_j);
    auto rho_bar = rho_i + rho_j;
    auto T_i = (*T_soln)(node_in_i);
    auto T_j = (*T_soln)(node_in_j);
    // area of channel i
    auto Si = (*S_flow_soln)(node_in_i);
    // area of channel j
    auto Sj = (*S_flow_soln)(node_in_j);
    // crossflow area between channels i,j dz*(pitch - rod diameter)
    auto Sij = dz * _subchannel_mesh._gij_map[i_gap];
    // hydraulic diameter in the ij direction
    auto Dh_ij = 4.0 * Sij / (2.0 * dz);
    auto Lij = _subchannel_mesh._pitch;
    // local form loss in the ij direction
    auto kij =
        2.0 *
        std::pow((1.0 - std::pow(Lij, 2.0) / std::pow(Lij - _subchannel_mesh._rod_diameter, 2.0)),
                 2.0);
    // assumed symmetry (that's why there is a two in the denominator)
    auto Mass_Termi = (((*mdot_soln)(node_out_i) - (*mdot_soln)(node_in_i)) * Lij) / (2.0 * Si);
    auto Mass_Termj = (((*mdot_soln)(node_out_j) - (*mdot_soln)(node_in_j)) * Lij) / (2.0 * Sj);
    auto Mass_Term = (Mass_Termi + Mass_Termj) * 2.0 * Sij / dz; // (kg/sec)^2
    auto Pressure_Term =
        std::pow(Sij, 2.0) * ((*P_soln)(node_in_i) - (*P_soln)(node_in_j)) * rho_bar;
    auto sign = (-2.0 * signbit((*P_soln)(node_in_i) - (*P_soln)(node_in_j)) + 1.0);
    auto mu = _fp->mu_from_rho_T(rho_bar / 2.0, (T_i + T_j) / 2.0);
    double a;
    double b;
    // Calculation of Turbulent Crossflow
    double abeta = 0.08; // thermal diffusion coefficient
    WijPrime(i_gap) = abeta * 0.25 *
                      ((*mdot_soln)(node_in_i) / Si + (*mdot_soln)(node_out_i) / Si +
                       (*mdot_soln)(node_in_j) / Sj + (*mdot_soln)(node_out_j) / Sj) *
                      Sij; // Kg/sec
    // INITIAL GUESS (eventually the continue statement will be removed)
    auto Wijguess = 0.0;
    if (Wij(i_gap) == 0.0)
    {
      // I am gonna solve a "fake problem" where Wij is always positive going from i->j
      // (Pressure_Term always positive) and apply the correct global sign afterwards
      if (isTransient())
        Wijguess = std::abs(Wij(i_gap));
      else
        Wijguess = std::sqrt(sign * Pressure_Term / kij);
    }
    else
      continue;

    auto newton_error = 1.0;
    auto newton_tolerance = 1e-10;
    int newton_cycles = 0;
    int max_newton_cycles = 100;
    while (newton_error > newton_tolerance && newton_cycles <= max_newton_cycles)
    {
      auto Re = std::abs(Wijguess / Sij) * Dh_ij / mu;
      if (Re < 1.0)
      {
        a = 0.0;
        b = 2.0; // doesn't matter in this case
      }
      else if (Re >= 1.0 && Re < 5000.0)
      {
        a = 64.0;
        b = -1.0;
      }
      else if (Re >= 5000.0 && Re < 30000.0)
      {
        a = 0.316;
        b = -0.25;
      }
      else
      {
        a = 0.184;
        b = -0.20;
      }
      newton_cycles++;
      if (newton_cycles == max_newton_cycles)
      {
        mooseError(
            name(), " CrossFlow Calculation didn't converge, newton_cycles: ", newton_cycles);
      }
      auto fij = computeFrictionFactor(Re);
      auto dRedW = Dh_ij / (Sij * mu);
      auto dfijdW = b * a * std::pow(Re, b - 1.0) * dRedW;
      auto Kij = fij * Lij / Dh_ij + kij;
      auto dKijdW = (Lij / Dh_ij) * dfijdW;
      auto derivativeTerm = (Wijguess - std::abs(Wij_old(i_gap))) * Lij * Sij * rho_bar / dt();
      auto Residual = 0.0;
      auto derivative = 0.0;
      if (isTransient())
      {
        Residual = derivativeTerm + Mass_Term * Wijguess + Kij * std::pow(Wijguess, 2.0) -
                   sign * Pressure_Term;
        derivative = Lij * Sij * rho_bar / dt() + Mass_Term + dKijdW * std::pow(Wijguess, 2.0) +
                     2.0 * Kij * Wijguess;
      }
      else
      {
        Residual = Mass_Term * Wijguess + Kij * std::pow(Wijguess, 2.0) - sign * Pressure_Term;
        derivative = Mass_Term + dKijdW * std::pow(Wijguess, 2.0) + 2.0 * Kij * Wijguess;
      }
      Wijguess = Wijguess - Residual / (derivative + 1e-10);
      newton_error = std::abs(Residual);
    }
    // apply global sign to crossflow
    Wij(i_gap) = sign * Wijguess;
  }
  /// Update global Matrix
  Wij_global.col(iz) = Wij;
}

void
SubChannel1PhaseProblem::computeSumWij(double SumSumWij, int iz)
{
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
  {
    auto * node_out = _subchannel_mesh._nodes[i_ch][iz];
    auto * node_in = _subchannel_mesh._nodes[i_ch][iz - 1];
    // upwind density
    auto rho = (*rho_soln)(node_in);
    auto S = (*S_flow_soln)(node_in);
    double SumWij = 0.0;
    double SumWijh = 0.0;
    double SumWijPrimeDhij = 0.0;
    double SumWijPrimeDUij = 0.0;
    // Calculate sum of crossflow into channel i from channels j around i
    unsigned int counter = 0;
    for (auto i_gap : _subchannel_mesh._chan_to_gap_map[i_ch])
    {
      unsigned int ii_ch = _subchannel_mesh._gap_to_chan_map[i_gap].first;
      // i is always the smallest and first index in the mapping
      unsigned int jj_ch = _subchannel_mesh._gap_to_chan_map[i_gap].second;
      auto * node_in_i = _subchannel_mesh._nodes[ii_ch][iz - 1];
      auto * node_in_j = _subchannel_mesh._nodes[jj_ch][iz - 1];
      auto * node_out_i = _subchannel_mesh._nodes[ii_ch][iz];
      auto * node_out_j = _subchannel_mesh._nodes[jj_ch][iz];
      auto rho_i = (*rho_soln)(node_in_i);
      auto rho_j = (*rho_soln)(node_in_j);
      // area of channel i
      auto Si = (*S_flow_soln)(node_in_i);
      // area of channel j
      auto Sj = (*S_flow_soln)(node_in_j);
      // apply local sign to crossflow
      SumWij += _subchannel_mesh._sign_id_crossflow_map[i_ch][counter] * Wij(i_gap);
      // take care of the sign by applying the map, use donor cell
      SumWijh += _subchannel_mesh._sign_id_crossflow_map[i_ch][counter] * Wij(i_gap) *
                 ((*h_soln)(node_in_i) + (*h_soln)(node_in_j) + (*h_soln)(node_out_i) +
                  (*h_soln)(node_out_j)) /
                 4.0;
      SumWijPrimeDhij += WijPrime(i_gap) * (((*h_soln)(node_in) + (*h_soln)(node_out)) -
                                            ((*h_soln)(node_in_j) + (*h_soln)(node_out_j)) / 2.0 -
                                            ((*h_soln)(node_in_i) + (*h_soln)(node_out_i)) / 2.0);
      SumWijPrimeDUij += WijPrime(i_gap) *
                         (((*mdot_soln)(node_in) + (*mdot_soln)(node_out)) / rho / S -
                          ((*mdot_soln)(node_in_j) + (*mdot_soln)(node_out_j)) / 2.0 / rho_j / Sj -
                          ((*mdot_soln)(node_in_i) + (*mdot_soln)(node_out_i)) / 2.0 / rho_i / Si);
      counter++;
    }
    SumSumWij += SumWij;
    // The total crossflow coming out of cell i [kg/sec]
    SumWij_soln->set(node_out, SumWij);
    // The total enthalpy crossflow coming out of cell i [J/sec]
    SumWijh_soln->set(node_out, SumWijh);
    // The total turbulent enthalpy crossflow coming out of cell i [J/sec]
    SumWijPrimeDhij_soln->set(node_out, SumWijPrimeDhij);
    // The total turbulent velocity crossflow coming out of cell i [kg m/sec^2]
    SumWijPrimeDUij_soln->set(node_out, SumWijPrimeDUij);
  }
}

void
SubChannel1PhaseProblem::computeMdot(int iz)
{
  auto dz = _subchannel_mesh._z_grid[iz] - _subchannel_mesh._z_grid[iz - 1];
  // go through the channels of the level.
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
  {
    // Start with applying mass-conservation equation & energy - conservation equation
    // Find the nodes for the top and bottom of this element.
    auto * node_in = _subchannel_mesh._nodes[i_ch][iz - 1];
    auto * node_out = _subchannel_mesh._nodes[i_ch][iz];
    // Copy the variables at the inlet (bottom) of this element.
    auto mdot_in = (*mdot_soln)(node_in);
    auto h_in = (*h_soln)(node_in); // J/kg
    auto volume = dz * (*S_flow_soln)(node_in);
    auto mdot_out = 0.0;
    auto h_out = 0.0;
    // Wij positive out of i into j;
    if (isTransient())
    {
      mdot_out = mdot_in - (*SumWij_soln)(node_out) -
                 ((*rho_soln)(node_in)-rho_soln->old(node_in)) * volume / dt();
      h_out = std::pow(mdot_out, -1) *
              (mdot_in * h_in - (*SumWijh_soln)(node_out) - (*SumWijPrimeDhij_soln)(node_out) +
               ((*q_prime_soln)(node_out) + (*q_prime_soln)(node_in)) * dz / 2.0 -
               ((*rho_soln)(node_in)*h_in - rho_soln->old(node_in) * h_soln->old(node_in)) *
                   volume / dt());
    }
    else
    {
      mdot_out = mdot_in - (*SumWij_soln)(node_out);
      // note use of trapezoidal rule concistent with axial power rate calculation
      // (PowerIC.C)
      h_out = std::pow(mdot_out, -1) *
              (mdot_in * h_in - (*SumWijh_soln)(node_out) - (*SumWijPrimeDhij_soln)(node_out) +
               ((*q_prime_soln)(node_out) + (*q_prime_soln)(node_in)) * dz / 2.0);
    }
    if (h_out < 0)
    {
      _console << "Wij = : " << Wij << "\n";
      mooseError(
          name(), " : Calculation of negative Enthalpy h_out = : ", h_out, " Axial Level= : ", iz);
    }
    if (mdot_out < 0)
    {
      _console << "Wij = : " << Wij << "\n";
      mooseError(name(),
                 " : Calculation of negative mass flow mdot_out = : ",
                 mdot_out,
                 " Axial Level= : ",
                 iz);
    }
    auto T_out = _fp->T_from_p_h((*P_soln)(node_out), h_out);
    auto rho_out = _fp->rho_from_p_T((*P_soln)(node_out), T_out);
    // Update the solution vectors at the outlet of the cell
    // (mass,density,Temperature,Enthalpy is upwinded).
    mdot_soln->set(node_out, mdot_out); // kg/sec
    h_soln->set(node_out, h_out);       // J/kg
    T_soln->set(node_out, T_out);       // Kelvin
    rho_soln->set(node_out, rho_out);   // Kg/m3 (This line couples density)
    // Update the solution vectors at the inlet of the whole assembly.
    // These values will be updated just 5 times depending on the bottom limiter value
    if (iz == 1)
    {
      h_soln->set(node_in, _fp->h_from_p_T((*P_soln)(node_in), (*T_soln)(node_in)));
      rho_soln->set(node_in, _fp->rho_from_p_T((*P_soln)(node_in), (*T_soln)(node_in)));
    }
  }
}

void
SubChannel1PhaseProblem::computeDP(int iz)
{
  auto dz = _subchannel_mesh._z_grid[iz] - _subchannel_mesh._z_grid[iz - 1];
  // Sweep through the channels of level
  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
  {
    // Find the nodes for the top and bottom of this element.
    auto * node_in = _subchannel_mesh._nodes[i_ch][iz - 1];
    auto * node_out = _subchannel_mesh._nodes[i_ch][iz];
    auto rho_i = (*rho_soln)(node_in);
    auto T_i = (*T_soln)(node_in);
    auto Si = (*S_flow_soln)(node_in);
    // _console << "Si: " << Si << std::endl;
    auto w_perim = (*w_perim_soln)(node_in);
    // _console << "w_perim: " << w_perim << std::endl;
    // hydraulic diameter in the i direction
    auto Dh_i = 4.0 * Si / w_perim;
    auto DeltaMass = std::pow((*mdot_soln)(node_out), 2) / (Si * (*rho_soln)(node_out)) -
                     std::pow((*mdot_soln)(node_in), 2) / (Si * (*rho_soln)(node_in));
    auto derivativeTerm = ((*mdot_soln)(node_in)-mdot_soln->old(node_in)) * dz / dt();
    double SumCrossflow = 0.0;
    auto DP = 0.0;
    unsigned int counter = 0;
    for (auto i_gap : _subchannel_mesh._chan_to_gap_map[i_ch])
    {
      unsigned int ii_ch = _subchannel_mesh._gap_to_chan_map[i_gap].first;
      unsigned int jj_ch = _subchannel_mesh._gap_to_chan_map[i_gap].second;
      auto * node_in_i = _subchannel_mesh._nodes[ii_ch][iz - 1];
      auto * node_in_j = _subchannel_mesh._nodes[jj_ch][iz - 1];
      auto * node_out_i = _subchannel_mesh._nodes[ii_ch][iz];
      auto * node_out_j = _subchannel_mesh._nodes[jj_ch][iz];
      SumCrossflow +=
          0.25 * _subchannel_mesh._sign_id_crossflow_map[i_ch][counter] * Wij_global(i_gap, iz) *
          ((*mdot_soln)(node_in_i) / (*S_flow_soln)(node_in_i) / (*rho_soln)(node_in_i) +
           (*mdot_soln)(node_out_i) / (*S_flow_soln)(node_out_i) / (*rho_soln)(node_out_i) +
           (*mdot_soln)(node_in_j) / (*S_flow_soln)(node_in_j) / (*rho_soln)(node_in_j) +
           (*mdot_soln)(node_out_j) / (*S_flow_soln)(node_out_j) / (*rho_soln)(node_out_j));
      counter++;
    }
    auto mu = _fp->mu_from_rho_T(rho_i, T_i);
    auto Re = (((*mdot_soln)(node_in) / Si) * Dh_i / mu);
    auto fi = computeFrictionFactor(Re);
    auto Friction = (fi * dz / Dh_i) * 0.5 * (std::pow((*mdot_soln)(node_in), 2.0)) /
                    (std::pow(Si, 2.0) * rho_i); // Pa
    auto Gravity = _g_grav * rho_i * dz;         // Pa
    if (isTransient())
    {
      DP = Friction + Gravity +
           std::pow(Si, -1.0) * (derivativeTerm + DeltaMass + SumCrossflow +
                                 (*SumWijPrimeDUij_soln)(node_out)); // Pa
    }
    else
    {
      DP =
          Friction + Gravity +
          std::pow(Si, -1.0) * (DeltaMass + SumCrossflow + (*SumWijPrimeDUij_soln)(node_out)); // Pa
    }
    // update solution
    DP_soln->set(node_out, DP);
  }
}

void
SubChannel1PhaseProblem::externalSolve()
{
  _console << "Executing subchannel solver\n";
  Eigen::MatrixXd PCYCLES(_subchannel_mesh._nz, 2);
  // Initialize
  PCYCLES.setZero();
  // nz level calculations
  for (unsigned int axial_level = 1; axial_level < _subchannel_mesh._nz + 1; axial_level++)
  {
    _console << "AXIAL LEVEL: " << axial_level << std::endl;
    double PError = 1.0;
    unsigned int stencilSize = 5;
    int max_axial_cycles = 200;
    int axial_cycles = 0;
    int max_level_cycles = 200;
    int bottom_limiter;
    while (PError > 1E-10 && axial_cycles <= max_axial_cycles)
    {
      if (axial_level < stencilSize)
        bottom_limiter = 1;
      else
        bottom_limiter = axial_level - stencilSize + 1;
      axial_cycles++;
      PCYCLES(axial_level - 1, 0) = axial_cycles;
      if (axial_cycles == max_axial_cycles)
      {
        mooseError(name(), " Pressure loop didn't converge, axial_cycles: ", axial_cycles);
      }
      /// L2 norm of old pressure solution vector
      auto P_L2norm_old = P_soln->L2norm();
      // Sweep upwards through the channels.
      for (unsigned int iz = bottom_limiter; iz < axial_level + 1; iz++)
      {
        double WError = 1.0;
        double MError = 1.0;
        int level_cycles = 0;
        Wij = Wij_global.col(iz);
        Wij_old = Wij_global_old.col(iz);
        // Lateral Loop... crossflow calculation
        while ((MError > 1E-9 || WError > 1E-8) && (level_cycles <= max_level_cycles))
        {
          level_cycles++;
          if (level_cycles == max_level_cycles)
          {
            mooseError(name(), " Level loop didn't converge, level_cycles: ", level_cycles);
          }
          // L2 norm of old mass flow solution vector
          auto mdot_L2norm_old = mdot_soln->L2norm();
          // Calculate crossflow between channel i-j using crossflow momentum equation
          computeWij(iz);
          // calculate Sum values per subchannel
          double SumSumWij = 0.0;
          computeSumWij(SumSumWij, iz);
          // Calculate mass flow, enthalpy, density, Temperature using the mass and energy
          // conseravation equations
          computeMdot(iz);
          // Calculate Error per level
          auto mdot_L2norm_new = mdot_soln->L2norm();
          MError = std::abs((mdot_L2norm_new - mdot_L2norm_old) / (mdot_L2norm_old + 1E-14));
          WError = SumSumWij;
        }
        /// Populate Pressure Drop vector
        computeDP(iz);
      }
      /// Now backsolve
      if (axial_level == _subchannel_mesh._nz)
        bottom_limiter = 1;
      // Calculate pressure everywhere (in the stencil) using the axial momentum equation
      // Sweep downwards through the channels level by level
      for (int iz = axial_level; iz > bottom_limiter - 1; iz--) // nz calculations
      {
        for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh._nodes[i_ch][iz];
          auto * node_in = _subchannel_mesh._nodes[i_ch][iz - 1];
          // update Pressure solution
          P_soln->set(node_in, (*P_soln)(node_out) + (*DP_soln)(node_out));
        }
      }
      //      // Calculate pressure Error
      auto P_L2norm_new = P_soln->L2norm();
      PError = std::abs((P_L2norm_new - P_L2norm_old) / (P_L2norm_old + 1E-14));
      _console << "- PError: " << PError << std::endl;
      PCYCLES(axial_level - 1, 1) = PError;
    }
  }
  /// update old crossflow matrix
  Wij_global_old = Wij_global;
  /// Save Wij_global
  std::ofstream myfile1;
  myfile1.open("Wij_global", std::ofstream::trunc);
  myfile1 << std::setprecision(12) << std::scientific << Wij_global << "\n";
  myfile1.close();

  _console << "Finished executing subchannel solver\n";
  _aux->solution().close();
}

void SubChannel1PhaseProblem::syncSolutions(Direction /*direction*/) {}
