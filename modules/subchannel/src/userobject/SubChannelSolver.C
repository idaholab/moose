#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include "SubChannelSolver.h"
#include "iapws.h"
#include "SolutionHandle.h"

using namespace Eigen;

registerMooseObject("SubChannelApp", SubChannelSolver);

InputParameters
SubChannelSolver::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredCoupledVar("mdot", "axial mass flow rate");
  params.addRequiredCoupledVar("SumWij", "Sum of cross flows for each channel");
  params.addRequiredCoupledVar("SumWijh", "Sum of enthaly crossflow flux for each channel");
  params.addRequiredCoupledVar("SumWijPrimeDhij",
                               "Sum of enthaly turbulent crossflow flux for each channel");
  params.addRequiredCoupledVar("SumWijPrimeDUij",
                               "Sum of velocity turbulent crossflow flux for each channel");
  params.addRequiredCoupledVar("P", "pressure");
  params.addRequiredCoupledVar("h", "specific enthalpy");
  params.addRequiredCoupledVar("T", "fluid temperature");
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("flow_area", "");
  params.addRequiredCoupledVar("cross_flow_area", "");
  params.addRequiredCoupledVar("wetted_perimeter", "");
  params.addRequiredCoupledVar("q_prime", "linear heat rate [W/m]");
  params.addRequiredParam<Real>("mflux_in", "Inlet coolant mass flux [kg/m^2-s]");
  params.addParam<Real>("T_in", 566.3, "Inlet coolant temperature in [K]");
  params.addRequiredParam<Real>("P_out", "Outlet coolant pressure in [Pa]");
  return params;
}

SubChannelSolver::SubChannelSolver(const InputParameters & params)
  : GeneralUserObject(params),
    Coupleable(this, "true"),
    _mdot_var(*getFieldVar("mdot", 0)),
    _SumWij_var(*getFieldVar("SumWij", 0)),
    _SumWijh_var(*getFieldVar("SumWijh", 0)),
    _SumWijPrimeDhij_var(*getFieldVar("SumWijPrimeDhij", 0)),
    _SumWijPrimeDUij_var(*getFieldVar("SumWijPrimeDUij", 0)),
    _P_var(*getFieldVar("P", 0)),
    _h_var(*getFieldVar("h", 0)),
    _T_var(*getFieldVar("T", 0)),
    _rho_var(*getFieldVar("rho", 0)),
    _S_flow_var(*getFieldVar("flow_area", 0)),
    _S_crossflow_var(*getFieldVar("cross_flow_area", 0)),
    _w_perim_var(*getFieldVar("wetted_perimeter", 0)),
    _q_prime_var(*getFieldVar("q_prime", 0)),
    _mflux_in(getParam<Real>("mflux_in")),
    _T_in(getParam<Real>("T_in")),
    _P_out(getParam<Real>("P_out"))
{
}

void
SubChannelSolver::initialize()
{
  _mesh = dynamic_cast<SubChannelMesh *>(&_fe_problem.mesh());
  if (!_mesh)
  {
    mooseError("Must use a SubChannelMesh");
  }
}

void
SubChannelSolver::execute()
{
  _console << "Executing subchannel solver\n";

  // Get handles for each variable's part of the solution vector.
  auto mdot_soln = SolutionHandle(_mdot_var);
  auto SumWij_soln = SolutionHandle(_SumWij_var);
  auto SumWijh_soln = SolutionHandle(_SumWijh_var);
  auto SumWijPrimeDhij_soln = SolutionHandle(_SumWijPrimeDhij_var);
  auto SumWijPrimeDUij_soln = SolutionHandle(_SumWijPrimeDUij_var);
  auto P_soln = SolutionHandle(_P_var);
  auto h_soln = SolutionHandle(_h_var);
  auto T_soln = SolutionHandle(_T_var);
  auto rho_soln = SolutionHandle(_rho_var);
  auto S_flow_soln = SolutionHandle(_S_flow_var);
  auto S_crossflow_soln = SolutionHandle(_S_crossflow_var);
  auto w_perim_soln = SolutionHandle(_w_perim_var);
  auto q_prime_soln = SolutionHandle(_q_prime_var);

  constexpr Real g_grav = 9.87; // m/sec^2

  // Set the inlet/outlet/guess for each channel.
  {
    for (int iz = 0; iz < _mesh->nz_ + 1; iz++) // nz + 1 nodes
    {
      for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++) // n_channels_ = number of channels
      {
        // creates node
        auto * node = _mesh->nodes_[i_ch][iz];
        // Initial enthalpy same everywhere
        h_soln.set(node, iapws::h1(_P_out * 1e-6, _T_in) * 1e3);
        T_soln.set(node, _T_in);
        P_soln.set(node, _P_out);
        // Initial Density is the same everywhere
        rho_soln.set(node, 1.0 / iapws::nu1(_P_out * 1e-6, _T_in));
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
  Eigen::VectorXd Wij(_mesh->n_gaps_);                        // Crossflow vector
  Eigen::VectorXd WijPrime(_mesh->n_gaps_);                   // turbulent Crossflow vector
  Eigen::VectorXd Wij_old(_mesh->n_gaps_);                    // Crossflow vector
  Eigen::MatrixXd Wij_global(_mesh->n_gaps_, _mesh->nz_ + 1); // Crossflow Matrix nz + 1 axial nodes
  Eigen::VectorXd mdot(_mesh->n_channels_);                   // Mass Flow Vector
  Eigen::VectorXd mdot_old(_mesh->n_channels_);               // Mass Flow Vector
  Eigen::MatrixXd mdot_global(_mesh->n_channels_,
                              _mesh->nz_ + 1); // Mass Flow Matrix nz + 1 axial nodes
  Eigen::VectorXd P(_mesh->n_channels_);       // Pressure Vector
  Eigen::MatrixXd P_global_old(_mesh->n_channels_,
                               _mesh->nz_ + 1); // Pressure Matrix nz + 1 axial nodes
  Eigen::MatrixXd P_global(_mesh->n_channels_, _mesh->nz_ + 1); // Pressure Matrix nz + 1 axial
                                                                // nodes
  Eigen::MatrixXd PCYCLES(_mesh->nz_, 2);
  Eigen::MatrixXd Temp_out(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Temp_in(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Enthalpy_out(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Pressure_out(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd mdotin(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd mdotout(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Pressure_in(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Enthalpy_in(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd rho_in(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd rho_out(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Area(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Gin(_mesh->ny_, _mesh->nx_);
  Eigen::MatrixXd Gout(_mesh->ny_, _mesh->nx_);

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
  mdot *= _mflux_in * S_flow_soln(_mesh->nodes_[_mesh->nx_ + 1][0]);
  mdot_old.setOnes();
  mdot_old *= _mflux_in * S_flow_soln(_mesh->nodes_[_mesh->nx_ + 1][0]);
  mdot_global.setOnes();
  mdot_global *= _mflux_in * S_flow_soln(_mesh->nodes_[_mesh->nx_ + 1][0]);
  // _mesh->nz_ + 1

  for (int axial_level = 1; axial_level < _mesh->nz_ + 1; axial_level++) // nz level calculations
  {
    _console << "AXIAL LEVEL: " << axial_level << std::endl;

    double PError = 1.0;
    int max_axial_cycles = 200;
    int axial_cycles = 0;
    int max_level_cycles = 200;
    int bottom_limiter;
    while (PError > 1E-10 && axial_cycles < max_axial_cycles)
    {
      if (axial_level < 5)
      {
        bottom_limiter = 1;
      }
      else
      {
        bottom_limiter = axial_level - 4;
      }

      axial_cycles++;
      PCYCLES(axial_level - 1, 0) = axial_cycles;
      // Sweep upwards through the channels.
      for (int iz = bottom_limiter; iz < axial_level + 1; iz++)
      {
        // Compute the height of this element.
        auto dz = _mesh->z_grid_[iz] - _mesh->z_grid_[iz - 1];
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
          for (int i_gap = 0; i_gap < _mesh->n_gaps_; i_gap++) // number of gaps = _mesh->n_gaps_
          {
            int i_ch = _mesh->gap_to_chan_map_[i_gap].first;
            int j_ch = _mesh->gap_to_chan_map_[i_gap].second;
            auto * node_in_i = _mesh->nodes_[i_ch][iz - 1];
            auto * node_out_i = _mesh->nodes_[i_ch][iz];
            auto * node_in_j = _mesh->nodes_[j_ch][iz - 1];
            auto * node_out_j = _mesh->nodes_[j_ch][iz];

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
            auto Sij = dz * _mesh->gij_map_[i_gap];
            // hydraulic diameter in the ij direction
            auto Dh_ij = 4.0 * Sij / (2 * dz);
            auto Lij = _mesh->pitch_;
            // local form loss in the ij direction
            auto kij =
                2.0 * std::pow((1 - std::pow(Lij, 2) / std::pow(Lij - _mesh->rod_diameter_, 2)), 2);
            // assumed symmetry (that's why there is a two in the denominator)
            auto Mass_Termi = ((mdot_soln(node_out_i) - mdot_soln(node_in_i)) * Lij) / (2 * Si);
            auto Mass_Termj = ((mdot_soln(node_out_j) - mdot_soln(node_in_j)) * Lij) / (2 * Sj);
            auto Mass_Term = (Mass_Termi + Mass_Termj) * 2 * Sij / dz; // (kg/sec)^2
            auto Pressure_Term =
                std::pow(Sij, 2) * std::abs(P_soln(node_in_i) - P_soln(node_in_j)) * rho_bar;
            auto mu = iapws::mu((T_i + T_j) / 2, rho_bar / 2);
            double a;
            double b;

            // Calculation of Turbulent Crossflow
            double abeta = 0.08; // thermal diffusion coefficient
            WijPrime(i_gap) = abeta * 0.25 *
                              (mdot_soln(node_in_i) / Si + mdot_soln(node_out_i) / Si +
                               mdot_soln(node_in_j) / Sj + mdot_soln(node_out_j) / Sj) *
                              Sij; // Kg/sec

            if (Wij_old(i_gap) == 0)
            {
              Wij(i_gap) = std::sqrt(Pressure_Term / kij); // INITIAL GUESS
            }
            else
            {
              continue;
            }

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
              else if (Re >= 1 && Re < 4000)
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
              auto dfijdW = -b * a * std::pow(Wolder, -b - 1) * std::pow(Dh_ij / (Sij * mu), -b);
              auto Kij = fij * Lij / Dh_ij + kij;
              auto dKijdW = (Lij / Dh_ij) * dfijdW;
              auto Residual = Mass_Term * Wolder + Kij * std::pow(Wolder, 2) - Pressure_Term;
              auto derivative = Mass_Term + dKijdW * std::pow(Wolder, 2) + 2 * Kij * Wolder;
              Wij(i_gap) = Wolder - Residual / (derivative + 1e-10);
              newton_error = std::abs(Residual);
            }

            // apply global sign to crossflow
            Wij(i_gap) = (-2 * signbit(P_soln(node_in_i) - P_soln(node_in_j)) + 1) * (Wij(i_gap));
          }

          Wij_global.col(iz) = Wij;
          double SumSumWij = 0.0;
          for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++)
          {
            auto * node_out = _mesh->nodes_[i_ch][iz];
            auto * node_in = _mesh->nodes_[i_ch][iz - 1];
            auto rho = rho_soln(node_in);
            auto S = S_flow_soln(node_in);
            double SumWij = 0.0;
            double SumWijh = 0.0;
            double SumWijPrimeDhij = 0.0;
            double SumWijPrimeDUij = 0.0;
            // Calculate sum of crossflow into channel i from channels j around i
            unsigned int counter = 0;
            for (auto i_gap : _mesh->chan_to_gap_map_[i_ch])
            {
              int ii_ch = _mesh->gap_to_chan_map_[i_gap].first;
              // i is always the smallest and first index in the mapping
              int jj_ch = _mesh->gap_to_chan_map_[i_gap].second;
              auto * node_in_i = _mesh->nodes_[ii_ch][iz - 1];
              auto * node_in_j = _mesh->nodes_[jj_ch][iz - 1];
              auto * node_out_i = _mesh->nodes_[ii_ch][iz];
              auto * node_out_j = _mesh->nodes_[jj_ch][iz];

              auto rho_i = rho_soln(node_in_i);
              auto rho_j = rho_soln(node_in_j);
              auto Si = S_flow_soln(node_in_i); // area of channel i
              auto Sj = S_flow_soln(node_in_j); // area of channel j

              // apply local sign to crossflow
              SumWij += _mesh->sign_id_crossflow_map_[i_ch][counter] * Wij(i_gap);
              // take care of the sign by applying the map, use donor cell
              SumWijh += _mesh->sign_id_crossflow_map_[i_ch][counter] * Wij(i_gap) *
                         (h_soln(node_in_i) + h_soln(node_in_j) + h_soln(node_out_i) +
                          h_soln(node_out_j)) /
                         4;
              SumWijPrimeDhij += WijPrime(i_gap) * ((h_soln(node_in) + h_soln(node_out)) -
                                                    (h_soln(node_in_j) + h_soln(node_out_j)) / 2 -
                                                    (h_soln(node_in_i) + h_soln(node_out_i)) / 2);
              SumWijPrimeDUij += WijPrime(i_gap) *
                                 ((mdot_soln(node_in) + mdot_soln(node_out)) / rho / S -
                                  (mdot_soln(node_in_j) + mdot_soln(node_out_j)) / 2 / rho_j / Sj -
                                  (mdot_soln(node_in_i) + mdot_soln(node_out_i)) / 2 / rho_i / Si);
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
          for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++)
          {
            // Start with applying mass-conservation equation & energy - conservation equation
            // Find the nodes for the top and bottom of this element.
            auto * node_in = _mesh->nodes_[i_ch][iz - 1];
            auto * node_out = _mesh->nodes_[i_ch][iz];
            // Copy the variables at the inlet (bottom) of this element.
            auto mdot_in = mdot_soln(node_in);
            auto h_in = h_soln(node_in); // J/kg

            // Wij positive out of i into j;
            auto mdot_out = mdot_in - SumWij_soln(node_out);
            auto h_out = std::pow(mdot_out, -1) *
                         (mdot_in * h_in - SumWijh_soln(node_out) - SumWijPrimeDhij_soln(node_out) +
                          q_prime_soln(node_out) * dz);
            auto T_out = iapws::T_from_p_h(P_soln(node_out) * 1e-6, h_out * 1e-3);
            auto rho_out = 1.0 / iapws::nu1(P_soln(node_out) * 1e-6, T_out);

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

      if (axial_level == _mesh->nz_)
      {
        bottom_limiter = 1;
      }

      P_global_old = P_global;
      // Sweep downwards through the channels. level by level
      for (int iz = axial_level; iz > bottom_limiter - 1; iz--) // nz calculations
      {
        auto dz = _mesh->z_grid_[iz] - _mesh->z_grid_[iz - 1];
        // Sweep through the channels of level
        for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++)
        {
          // Find the nodes for the top and bottom of this element.
          auto * node_in = _mesh->nodes_[i_ch][iz - 1];
          auto * node_out = _mesh->nodes_[i_ch][iz];
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
          for (auto i_gap : _mesh->chan_to_gap_map_[i_ch])
          {
            int ii_ch = _mesh->gap_to_chan_map_[i_gap].first;
            int jj_ch = _mesh->gap_to_chan_map_[i_gap].second;
            auto * node_in_i = _mesh->nodes_[ii_ch][iz - 1];
            auto * node_in_j = _mesh->nodes_[jj_ch][iz - 1];
            auto * node_out_i = _mesh->nodes_[ii_ch][iz];
            auto * node_out_j = _mesh->nodes_[jj_ch][iz];
            SumCrossflow +=
                0.25 * _mesh->sign_id_crossflow_map_[i_ch][counter] * Wij_global(i_gap, iz) *
                (mdot_soln(node_in_i) / S_flow_soln(node_in_i) / rho_soln(node_in_i) +
                 mdot_soln(node_out_i) / S_flow_soln(node_out_i) / rho_soln(node_out_i) +
                 mdot_soln(node_in_j) / S_flow_soln(node_in_j) / rho_soln(node_in_j) +
                 mdot_soln(node_out_j) / S_flow_soln(node_out_j) / rho_soln(node_out_j));
            counter++;
          }

          auto mu = iapws::mu(T_i, rho_i);
          auto Re = ((mdot_soln(node_in) / Si) * Dh_i / mu);
          auto fi = 0.184 * std::pow(Re, -0.2);
          auto Friction = (fi * dz / Dh_i) * 0.5 * (std::pow(mdot_soln(node_in), 2)) /
                          (std::pow(Si, 2) * rho_i); // Pa
          auto Gravity = g_grav * rho_i * dz;        // Pa
          auto Pin =
              P_soln(node_out) + Friction + Gravity +
              std::pow(Si, -1) * (DeltaMass + SumCrossflow + SumWijPrimeDUij_soln(node_out)); // Pa
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

  for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++)
  {
    auto * node_out = _mesh->nodes_[i_ch][_mesh->nz_];
    auto * node_in = _mesh->nodes_[i_ch][0];
    auto * node_20 = _mesh->nodes_[i_ch][20];
    int i = (i_ch / _mesh->nx_);           // row
    int j = i_ch - i * _mesh->nx_;         // column
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

  for (int iz = 0; iz < _mesh->nz_ + 1; iz++)
  {
    double Total_crossflow = 0.0;
    for (int i_ch = 0; i_ch < _mesh->n_channels_; i_ch++)
    {
      auto * node = _mesh->nodes_[i_ch][iz];
      Total_crossflow += SumWij_soln(node);
    }
  }

  _console << "Finished executing subchannel solver\n";
}

void
SubChannelSolver::finalize()
{
}
