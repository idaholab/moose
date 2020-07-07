
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

registerMooseObject("SubChannelApp", SubChannel1PhaseProblem);

InputParameters
SubChannel1PhaseProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredParam<Real>("mflux_in", "Inlet coolant mass flux [kg/m^2-s]");
  params.addRequiredParam<Real>("T_in", "Inlet coolant temperature in [K]");
  params.addRequiredParam<Real>("P_out", "Outlet coolant pressure in [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  return params;
}

SubChannel1PhaseProblem::SubChannel1PhaseProblem(const InputParameters & params)
  : ExternalProblem(params),
  _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh)),
  _mflux_in(getParam<Real>("mflux_in")),
  _T_in(getParam<Real>("T_in")),
  _P_out(getParam<Real>("P_out"))
{
}

bool
SubChannel1PhaseProblem::converged() {return true;}

void
SubChannel1PhaseProblem::externalSolve()
{
  _console << "Executing subchannel solver\n";
  const SinglePhaseFluidProperties & _fp = getUserObject<SinglePhaseFluidProperties>(getParam<UserObjectName>("fp"));
  auto mdot_soln = SolutionHandle(getVariable(0, "mdot"));
  auto SumWij_soln = SolutionHandle(getVariable(0, "SumWij"));
  auto SumWijh_soln = SolutionHandle(getVariable(0, "SumWijh"));
  auto SumWijPrimeDhij_soln = SolutionHandle(getVariable(0, "SumWijPrimeDhij"));
  auto SumWijPrimeDUij_soln = SolutionHandle(getVariable(0, "SumWijPrimeDUij"));
  auto P_soln = SolutionHandle(getVariable(0, "P"));
  auto h_soln = SolutionHandle(getVariable(0, "h"));
  auto T_soln = SolutionHandle(getVariable(0, "T"));
  auto rho_soln = SolutionHandle(getVariable(0, "rho"));
  auto S_flow_soln = SolutionHandle(getVariable(0, "S"));
  auto S_crossflow_soln = SolutionHandle(getVariable(0, "Sij"));
  auto w_perim_soln = SolutionHandle(getVariable(0, "w_perim"));
  auto q_prime_soln = SolutionHandle(getVariable(0, "q_prime"));
  constexpr Real g_grav = 9.87; // m/sec^2

  // Set the inlet/outlet/guess for each channel.
  {
    for (unsigned int iz = 0; iz < _subchannel_mesh._nz + 1; iz++) // nz + 1 nodes
    {
      for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++) // _n_channels = number of channels
      {
        // creates node
        auto * node = _subchannel_mesh._nodes[i_ch][iz];
        // Initial enthalpy same everywhere
        h_soln.set(node, _fp.h_from_p_T(_P_out, _T_in));
        T_soln.set(node, _T_in);
        P_soln.set(node, _P_out);
        // Initial density is the same everywhere
        rho_soln.set(node, _fp.rho_from_p_T(_P_out, _T_in));
        SumWij_soln.set(node, 0.0);
        SumWijh_soln.set(node, 0.0);
        SumWijPrimeDhij_soln.set(node, 0.0);
        SumWijPrimeDUij_soln.set(node, 0.0);
        mdot_soln.set(node, _mflux_in * S_flow_soln(node)); // kg/sec
        P_soln.set(node, _P_out);
      }
    }
  }

  // Initialize  crossflow / Pressure matrixes and vectors to use in calculation set
  // Crossflow
  Eigen::VectorXd Wij(_subchannel_mesh._n_gaps);
  Eigen::VectorXd Wij_old(_subchannel_mesh._n_gaps);
  Eigen::MatrixXd Wij_global(_subchannel_mesh._n_gaps, _subchannel_mesh._nz + 1);
  // turbulent Crossflow
  Eigen::VectorXd WijPrime(_subchannel_mesh._n_gaps);
  // Mass Flow
  Eigen::VectorXd mdot(_subchannel_mesh._n_channels);
  Eigen::VectorXd mdot_old(_subchannel_mesh._n_channels);
  Eigen::MatrixXd mdot_global(_subchannel_mesh._n_channels,  _subchannel_mesh._nz + 1);
  // Pressure
  Eigen::VectorXd P(_subchannel_mesh._n_channels);
  Eigen::MatrixXd P_global_old(_subchannel_mesh._n_channels, _subchannel_mesh._nz + 1);
  Eigen::MatrixXd P_global(_subchannel_mesh._n_channels, _subchannel_mesh._nz + 1);
  Eigen::MatrixXd PCYCLES(_subchannel_mesh._nz, 2);
  Eigen::MatrixXd Temp_out(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd Temp_in(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd Enthalpy_out(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd Pressure_out(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd mdotin(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd mdotout(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd Pressure_in(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd Enthalpy_in(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd rho_in(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd rho_out(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd Area(_subchannel_mesh._ny, _subchannel_mesh._nx);
  // mass flux
  Eigen::MatrixXd Gin(_subchannel_mesh._ny, _subchannel_mesh._nx);
  Eigen::MatrixXd Gout(_subchannel_mesh._ny, _subchannel_mesh._nx);
  // Initialize
  PCYCLES.setZero();
  Area.setZero();
  Gin.setZero();
  Gout.setZero();
  Temp_out.setZero();
  Temp_in.setZero();
  rho_in.setZero();
  rho_out.setZero();
  Enthalpy_out.setZero();
  Pressure_out.setZero();
  Enthalpy_in.setZero();
  Pressure_in.setZero();
  mdotin.setZero();
  mdotout.setZero();
  Wij.setZero();
  WijPrime.setZero();
  Wij_old.setZero();
  Wij_global.setZero();
  P.setOnes();
  P_global.setOnes();
  P_global_old.setOnes();
  P *= _P_out;
  P_global *= _P_out;
  P_global_old *= _P_out;
  mdot.setOnes();
  // flow profile same as the inlet on all axial levels
  mdot *= _mflux_in * S_flow_soln(_subchannel_mesh._nodes[_subchannel_mesh._nx + 1][0]);
  mdot_old.setOnes();
  mdot_old *= _mflux_in * S_flow_soln(_subchannel_mesh._nodes[_subchannel_mesh._nx + 1][0]);
  mdot_global.setOnes();
  mdot_global *= _mflux_in * S_flow_soln(_subchannel_mesh._nodes[_subchannel_mesh._nx + 1][0]);

  // nz level calculations
  for (unsigned int axial_level = 1; axial_level < _subchannel_mesh._nz + 1; axial_level++)
  {
    _console << "AXIAL LEVEL: " << axial_level << std::endl;
    double PError = 1.0;
    int max_axial_cycles = 200;
    int axial_cycles = 0;
    int max_level_cycles = 200;
    int bottom_limiter;
    while (PError > 1E-10 && axial_cycles < max_axial_cycles)
    {
      if (axial_level < 5) bottom_limiter = 1;
      else bottom_limiter = axial_level - 4;
      axial_cycles++;
      PCYCLES(axial_level - 1, 0) = axial_cycles;
      // Sweep upwards through the channels.
      for (unsigned int iz = bottom_limiter; iz < axial_level + 1; iz++)
      {
        // Compute the height of this element.
        auto dz = _subchannel_mesh._z_grid[iz] - _subchannel_mesh._z_grid[iz - 1];
        double WError = 1.0;
        double MError = 1.0;
        int level_cycles = 0;
        Wij = Wij_global.col(iz);
        mdot = mdot_global.col(iz);
        mdot_old = mdot_global.col(iz);
        // Lateral Loop... crossflow calculation
        while ((MError > 1E-9 || WError > 1E-8) && (level_cycles < max_level_cycles))
        {
          level_cycles++;
          Wij_old = Wij;
          mdot_old = mdot;
          // Calculate crossflow between channel i-j using crossflow momentum equation
          // number of gaps = _subchannel_mesh._n_gaps
          for (unsigned int i_gap = 0; i_gap < _subchannel_mesh._n_gaps; i_gap++)
          {
            unsigned int i_ch = _subchannel_mesh._gap_to_chan_map[i_gap].first;
            unsigned int j_ch = _subchannel_mesh._gap_to_chan_map[i_gap].second;
            auto * node_in_i = _subchannel_mesh._nodes[i_ch][iz - 1];
            auto * node_out_i = _subchannel_mesh._nodes[i_ch][iz];
            auto * node_in_j = _subchannel_mesh._nodes[j_ch][iz - 1];
            auto * node_out_j = _subchannel_mesh._nodes[j_ch][iz];
            auto rho_i = rho_soln(node_in_i);
            auto rho_j = rho_soln(node_in_j);
            auto rho_bar = rho_i + rho_j;
            auto T_i = T_soln(node_in_i);
            auto T_j = T_soln(node_in_j);
            // area of channel i
            auto Si = S_flow_soln(node_in_i);
            // area of channel j
            auto Sj = S_flow_soln(node_in_j);
            // crossflow area between channels i,j dz*(pitch - rod diameter)
            auto Sij = dz * _subchannel_mesh._gij_map[i_gap];
            // hydraulic diameter in the ij direction
            auto Dh_ij = 4.0 * Sij / (2.0 * dz);
            auto Lij = _subchannel_mesh._pitch;
            // local form loss in the ij direction
            auto kij =
                2.0 * std::pow((1.0 - std::pow(Lij, 2.0) / std::pow(Lij - _subchannel_mesh._rod_diameter, 2.0)), 2.0);
            // assumed symmetry (that's why there is a two in the denominator)
            auto Mass_Termi = ((mdot_soln(node_out_i) - mdot_soln(node_in_i)) * Lij) / (2.0 * Si);
            auto Mass_Termj = ((mdot_soln(node_out_j) - mdot_soln(node_in_j)) * Lij) / (2.0 * Sj);
            auto Mass_Term = (Mass_Termi + Mass_Termj) * 2.0 * Sij / dz; // (kg/sec)^2
            auto Pressure_Term =
                std::pow(Sij, 2.0) * std::abs(P_soln(node_in_i) - P_soln(node_in_j)) * rho_bar;
            auto mu = _fp.mu_from_rho_T(rho_bar / 2.0, (T_i + T_j) / 2.0);
            double a;
            double b;

            // Calculation of Turbulent Crossflow
            double abeta = 0.08; // thermal diffusion coefficient
            WijPrime(i_gap) = abeta * 0.25 *
                              (mdot_soln(node_in_i) / Si + mdot_soln(node_out_i) / Si +
                               mdot_soln(node_in_j) / Sj + mdot_soln(node_out_j) / Sj) *
                              Sij; // Kg/sec
            // INITIAL GUESS
            if (Wij_old(i_gap) == 0) Wij(i_gap) = std::sqrt(Pressure_Term / kij);
            else continue;

            auto newton_error = 1.0;
            auto newton_tolerance = 1e-10;
            int newton_cycles = 0;
            int max_newton_cycles = 100;
            while (newton_error > newton_tolerance && newton_cycles < max_newton_cycles)
            {
              auto Re = std::abs((Wij(i_gap) / Sij) * Dh_ij / mu);
              if (Re < 1)
              {
                b = -1.0;
                a = 1e5;
              }
              else if (Re >= 1.0 && Re < 4000)
              {
                b = 1.0;
                a = 64.0;
              }
              else
              {
                b = 0.2;
                a = 0.184;
              }
              newton_cycles++;
              auto Wolder = Wij(i_gap);
              auto fij = a * std::pow(Wolder * Dh_ij / (Sij * mu), -b);
              auto dfijdW = -b * a * std::pow(Wolder, -b - 1.0) * std::pow(Dh_ij / (Sij * mu), -b);
              auto Kij = fij * Lij / Dh_ij + kij;
              auto dKijdW = (Lij / Dh_ij) * dfijdW;
              auto Residual = Mass_Term * Wolder + Kij * std::pow(Wolder, 2.0) - Pressure_Term;
              auto derivative = Mass_Term + dKijdW * std::pow(Wolder, 2.0) + 2.0 * Kij * Wolder;
              Wij(i_gap) = Wolder - Residual / (derivative + 1e-10);
              newton_error = std::abs(Residual);
            }
            // apply global sign to crossflow
            Wij(i_gap) = (-2.0 * signbit(P_soln(node_in_i) - P_soln(node_in_j)) + 1.0) * (Wij(i_gap));
          }
          Wij_global.col(iz) = Wij;
          double SumSumWij = 0.0;
          for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
          {
            auto * node_out = _subchannel_mesh._nodes[i_ch][iz];
            auto * node_in = _subchannel_mesh._nodes[i_ch][iz - 1];
            auto rho = rho_soln(node_in);
            auto S = S_flow_soln(node_in);
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
              auto rho_i = rho_soln(node_in_i);
              auto rho_j = rho_soln(node_in_j);
              // area of channel i
              auto Si = S_flow_soln(node_in_i);
              // area of channel j
              auto Sj = S_flow_soln(node_in_j);
              // apply local sign to crossflow
              SumWij += _subchannel_mesh._sign_id_crossflow_map[i_ch][counter] * Wij(i_gap);
              // take care of the sign by applying the map, use donor cell
              SumWijh += _subchannel_mesh._sign_id_crossflow_map[i_ch][counter] * Wij(i_gap) *
                         (h_soln(node_in_i) + h_soln(node_in_j) + h_soln(node_out_i) +
                          h_soln(node_out_j)) / 4.0;
              SumWijPrimeDhij += WijPrime(i_gap) * ((h_soln(node_in) + h_soln(node_out)) -
                                                    (h_soln(node_in_j) + h_soln(node_out_j)) / 2.0 -
                                                    (h_soln(node_in_i) + h_soln(node_out_i)) / 2.0);
              SumWijPrimeDUij += WijPrime(i_gap) *
                                 ((mdot_soln(node_in) + mdot_soln(node_out)) / rho / S -
                                  (mdot_soln(node_in_j) + mdot_soln(node_out_j)) / 2.0 / rho_j / Sj -
                                  (mdot_soln(node_in_i) + mdot_soln(node_out_i)) / 2.0 / rho_i / Si);
              counter++;
            }
            SumSumWij += SumWij;
            // The total crossflow coming out of cell i [kg/sec]
            SumWij_soln.set(node_out, SumWij);
            // The total enthalpy crossflow coming out of cell i [J/sec]
            SumWijh_soln.set(node_out, SumWijh);
            // The total turbulent enthalpy crossflow coming out of cell i [J/sec]
            SumWijPrimeDhij_soln.set(node_out, SumWijPrimeDhij);
            // The total turbulent velocity crossflow coming out of cell i [kg m/sec^2]
            SumWijPrimeDUij_soln.set(node_out, SumWijPrimeDUij);
          }

          // go through the channels of the level.
          for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
          {
            // Start with applying mass-conservation equation & energy - conservation equation
            // Find the nodes for the top and bottom of this element.
            auto * node_in = _subchannel_mesh._nodes[i_ch][iz - 1];
            auto * node_out = _subchannel_mesh._nodes[i_ch][iz];
            // Copy the variables at the inlet (bottom) of this element.
            auto mdot_in = mdot_soln(node_in);
            auto h_in = h_soln(node_in); // J/kg
            // Wij positive out of i into j;
            auto mdot_out = mdot_in - SumWij_soln(node_out);
            // note use of trapezoidal rule concistent with axial power rate calculation (PowerIC.C)
            auto h_out = std::pow(mdot_out, -1) *
                         (mdot_in * h_in - SumWijh_soln(node_out) - SumWijPrimeDhij_soln(node_out) +
                          (q_prime_soln(node_out) + q_prime_soln(node_in))* dz / 2.0);
            auto T_out = _fp.T_from_p_h(P_soln(node_out), h_out);
            auto rho_out = _fp.rho_from_p_T(P_soln(node_out), T_out);

            // Update the solution vectors.
            mdot_soln.set(node_out, mdot_out); // kg/sec
            h_soln.set(node_out, h_out);       // J/kg
            T_soln.set(node_out, T_out);       // Kelvin
            rho_soln.set(node_out, rho_out);   // Kg/m3 (This line couples density)
            mdot(i_ch) = mdot_out;
          }
          mdot_global.col(iz) = mdot;
          MError = std::sqrt((mdot - mdot_old).squaredNorm() / (mdot_old.squaredNorm() + 1E-14));
          WError = SumSumWij;
        }
      }

      if (axial_level == _subchannel_mesh._nz) bottom_limiter = 1;
      P_global_old = P_global;
      // Sweep downwards through the channels. level by level
      for (int iz = axial_level; iz > bottom_limiter - 1; iz--) // nz calculations
      {
        auto dz = _subchannel_mesh._z_grid[iz] - _subchannel_mesh._z_grid[iz - 1];
        // Sweep through the channels of level
        for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
        {
          // Find the nodes for the top and bottom of this element.
          auto * node_in = _subchannel_mesh._nodes[i_ch][iz - 1];
          auto * node_out = _subchannel_mesh._nodes[i_ch][iz];
          auto rho_i = rho_soln(node_in);
          auto T_i = T_soln(node_in);
          auto Si = S_flow_soln(node_in);
          auto w_perim = w_perim_soln(node_in);
          // hydraulic diameter in the i direction
          auto Dh_i = 4.0 * Si / w_perim;
          auto DeltaMass = std::pow(mdot_soln(node_out), 2) / (Si * rho_soln(node_out)) -
                           std::pow(mdot_soln(node_in), 2) / (Si * rho_soln(node_in));
          double SumCrossflow = 0.0;
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
                (mdot_soln(node_in_i) / S_flow_soln(node_in_i) / rho_soln(node_in_i) +
                 mdot_soln(node_out_i) / S_flow_soln(node_out_i) / rho_soln(node_out_i) +
                 mdot_soln(node_in_j) / S_flow_soln(node_in_j) / rho_soln(node_in_j) +
                 mdot_soln(node_out_j) / S_flow_soln(node_out_j) / rho_soln(node_out_j));
            counter++;
          }

          auto mu = _fp.mu_from_rho_T(rho_i, T_i);
          auto Re = ((mdot_soln(node_in) / Si) * Dh_i / mu);
          auto fi = 0.184 * std::pow(Re, -0.2);
          auto Friction = (fi * dz / Dh_i) * 0.5 * (std::pow(mdot_soln(node_in), 2.0)) /
                          (std::pow(Si, 2.0) * rho_i); // Pa
          auto Gravity = g_grav * rho_i * dz;        // Pa
          auto Pin =
              P_soln(node_out) + Friction + Gravity +
              std::pow(Si, -1.0) * (DeltaMass + SumCrossflow + SumWijPrimeDUij_soln(node_out)); // Pa
          // Relaxation
          Pin = 0.0 * P_global_old(i_ch, iz - 1) + 1.0 * Pin;
          // update solution
          P_soln.set(node_in, Pin);
          // update solution vector
          P(i_ch) = Pin;
        }
        P_global.col(iz - 1) = P;
      }
      // Calculate pressure Error
      PError =
          std::sqrt((P_global - P_global_old).squaredNorm() / (P_global_old.squaredNorm() + 1E-14));
      _console << "- PError: " << PError << std::endl;
      PCYCLES(axial_level - 1, 1) = PError;
    }
  }

  double hinMdotin = 0.0;
  double houtMdotout = 0.0;
  double Total_crossflow_in = 0.0;
  double Total_crossflow_out = 0.0;
  double Total_crossflow_20 = 0.0;

  for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
  {
    auto * node_out = _subchannel_mesh._nodes[i_ch][_subchannel_mesh._nz];
    auto * node_in = _subchannel_mesh._nodes[i_ch][0];
    auto * node_20 = _subchannel_mesh._nodes[i_ch][20];
    unsigned int i = (i_ch / _subchannel_mesh._nx);           // row
    unsigned int j = i_ch - i * _subchannel_mesh._nx;         // column
    Temp_out(i, j) = T_soln(node_out);     // Kelvin
    Temp_in(i, j) = T_soln(node_in);       // Kelvin
    rho_in(i, j) = rho_soln(node_in);      // Kg/m3
    rho_out(i, j) = rho_soln(node_out);    // Kg/m3
    Pressure_out(i, j) = P_soln(node_out); // Pa
    Enthalpy_out(i, j) = h_soln(node_out); // J/kg
    Pressure_in(i, j) = P_soln(node_in);   // Pa
    Enthalpy_in(i, j) = h_soln(node_in);   // J/kg
    mdotin(i, j) = mdot_soln(node_in);     // Kg/sec
    mdotout(i, j) = mdot_soln(node_out);   // Kg/sec
    Area(i, j) = S_flow_soln(node_in);
    Gin(i, j) = mdot_soln(node_in) / S_flow_soln(node_in);
    Gout(i, j) = mdot_soln(node_out) / S_flow_soln(node_out);
    hinMdotin += mdot_soln(node_in) * h_soln(node_in);
    houtMdotout += mdot_soln(node_out) * h_soln(node_out);
    Total_crossflow_out += SumWij_soln(node_out);
    Total_crossflow_in += SumWij_soln(node_in);
    Total_crossflow_20 += SumWij_soln(node_20);
  }

  for (unsigned int iz = 0; iz < _subchannel_mesh._nz + 1; iz++)
  {
    double Total_crossflow = 0.0;
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh._n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh._nodes[i_ch][iz];
      Total_crossflow += SumWij_soln(node);
    }
  }

  _console << "Finished executing subchannel solver\n";
}

void
SubChannel1PhaseProblem::syncSolutions(Direction /*direction*/)
{
}
