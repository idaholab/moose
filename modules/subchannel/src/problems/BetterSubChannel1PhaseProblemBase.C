#include "BetterSubChannel1PhaseProblemBase.h"
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
#include "AuxiliarySystem.h"

InputParameters
BetterSubChannel1PhaseProblemBase::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredParam<Real>("beta",
                                "Thermal diffusion coefficient used in turbulent crossflow");
  params.addRequiredParam<Real>("CT", "Turbulent modeling parameter");
  params.addRequiredParam<bool>("enforce_uniform_pressure",
                                "Flag that enables uniform inlet pressure");
  params.addRequiredParam<bool>("Density", "Flag that enables the calculation of density");
  params.addRequiredParam<bool>("Viscosity", "Flag that enables the calculation of viscosity");
  params.addRequiredParam<bool>(
      "Power", "Flag that informs whether we solve the Enthalpy/Temperature equations or not");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  return params;
}

BetterSubChannel1PhaseProblemBase::BetterSubChannel1PhaseProblemBase(const InputParameters & params)
  : ExternalProblem(params),
    _g_grav(9.87),
    _one(1.0),
    _TR(isTransient() ? 1. : 0.),
    _Density(getParam<bool>("Density")),
    _Viscosity(getParam<bool>("Viscosity")),
    _Power(getParam<bool>("Power")),
    _dt(isTransient() ? dt() : _one),
    _subchannel_mesh(dynamic_cast<BetterSubChannelMeshBase &>(_mesh)),
    _beta(getParam<Real>("beta")),
    _CT(getParam<Real>("CT")),
    _enforce_uniform_pressure(getParam<bool>("enforce_uniform_pressure")),
    _fp(nullptr)
{
  n_cells = _subchannel_mesh.getNumOfAxialCells();
  n_blocks = _subchannel_mesh.getNumOfAxialBlocks();
  n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();
  block_size = n_cells / n_blocks;
  // Turbulent crossflow (stuff that live on the gaps)
  Wij.resize(n_gaps, n_cells + 1);
  Wij_old.resize(n_gaps, n_cells + 1);
  WijPrime.resize(n_gaps, n_cells + 1);
  Wij.setZero();
  Wij_old.setZero();
  WijPrime.setZero();
}

void
BetterSubChannel1PhaseProblemBase::initialSetup()
{
  ExternalProblem::initialSetup();
  // Read in Wij for null transient only at the first run of externalSolve
  if (isTransient())
  {
    std::ifstream file("Wij_SS");
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
          Wij(row_index, column_index) = std::stof(tmp);
          Wij_old(row_index, column_index) = std::stof(tmp);
          column_index += 1;
        }
      }
      row_index += 1;
    }
  }

  _fp = &getUserObject<SinglePhaseFluidProperties>(getParam<UserObjectName>("fp"));
  mdot_soln = new SolutionHandle(getVariable(0, "mdot"));
  SumWij_soln = new SolutionHandle(getVariable(0, "SumWij"));
  P_soln = new SolutionHandle(getVariable(0, "P"));
  DP_soln = new SolutionHandle(getVariable(0, "DP"));
  h_soln = new SolutionHandle(getVariable(0, "h"));
  T_soln = new SolutionHandle(getVariable(0, "T"));
  rho_soln = new SolutionHandle(getVariable(0, "rho"));
  Mu_soln = new SolutionHandle(getVariable(0, "Mu"));
  S_flow_soln = new SolutionHandle(getVariable(0, "S"));
  w_perim_soln = new SolutionHandle(getVariable(0, "w_perim"));
  q_prime_soln = new SolutionHandle(getVariable(0, "q_prime"));
}

BetterSubChannel1PhaseProblemBase::~BetterSubChannel1PhaseProblemBase()
{
  delete mdot_soln;
  delete SumWij_soln;
  delete P_soln;
  delete DP_soln;
  delete h_soln;
  delete T_soln;
  delete rho_soln;
  delete Mu_soln;
  delete S_flow_soln;
  delete w_perim_soln;
  delete q_prime_soln;
}

bool
BetterSubChannel1PhaseProblemBase::converged()
{
  return true;
}

double
BetterSubChannel1PhaseProblemBase::computeFrictionFactor(double Re)
{
  double a, b;
  if (Re < 1)
  {
    return 64.0;
  }
  else if (Re >= 1 and Re < 5000)
  {
    a = 64.0;
    b = -1.0;
  }
  else if (Re >= 5000 and Re < 30000)
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
BetterSubChannel1PhaseProblemBase::computeWij(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;
  Eigen::VectorXd solution_seed = Wij.block(0, first_node, n_gaps, block_size);

  /// Solving the combined lateral momentum equation for Wij using a PETSc solver yet to be defined
  root = PETSC_SNES_SOLVER();
  /// Update crossflow using root (root is a PETSc vector i need to turn into eigen Vector and then put into an eigen Matrix)
  // Assign the solution to the cross-flow matrix
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
    {
      int i = n_gaps * iz + i_gap; // column wise transfer
      Wij(i_gap, iz) = root(i);
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeSumWij(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      double SumWij = 0.0;
      // Calculate sum of crossflow into channel i from channels j around i
      unsigned int counter = 0;
      for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
      {
        SumWij += _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz);
        counter++;
      }
      // The net crossflow coming out of cell i [kg/sec]
      SumWij_soln->set(node_out, SumWij);
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeMdot(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // go through the channels of the level.
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      // Start with applying mass-conservation equation & energy - conservation equation
      // Find the nodes for the top and bottom of this element.
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto volume = dz * (*S_flow_soln)(node_in);
      // mass damping
      double am = 1.0; // means no damping
      auto time_term = _TR * ((*rho_soln)(node_out)-rho_soln->old(node_out)) * volume / _dt;
      // Wij positive out of i into j;
      auto mdot_out = am * ((*mdot_soln)(node_in) - (*SumWij_soln)(node_out)-time_term) +
                      (1.0 - am) * (*mdot_soln)(node_out);
      if (mdot_out < 0)
      {
        _console << "Wij = : " << Wij << "\n";
        mooseError(name(),
                   " : Calculation of negative mass flow mdot_out = : ",
                   mdot_out,
                   " Axial Level= : ",
                   iz);
      }
      // Update solution vector
      mdot_soln->set(node_out, mdot_out); // kg/sec
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeWijPrime(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;
  unsigned int n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int i_ch = chans.first;
      unsigned int j_ch = chans.second;
      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
      auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
      // area of channel i
      auto Si_in = (*S_flow_soln)(node_in_i);
      // area of channel j
      auto Sj_in = (*S_flow_soln)(node_in_j);
      // area of channel i
      auto Si_out = (*S_flow_soln)(node_out_i);
      // area of channel j
      auto Sj_out = (*S_flow_soln)(node_out_j);
      // crossflow area between channels i,j dz*gap_width
      auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
      // Calculation of Turbulent Crossflow
      for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
      {
        WijPrime(i_gap, iz) =
            _beta * 0.5 *
            (((*mdot_soln)(node_in_i) + (*mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
             ((*mdot_soln)(node_out_i) + (*mdot_soln)(node_out_j)) / (Si_out + Sj_out)) *
            Sij; // Kg/sec
      }
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeDP(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // Sweep through the channels of level
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      // Find the nodes for the top and bottom of this element.
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto rho_in = (*rho_soln)(node_in);
      auto rho_out = (*rho_soln)(node_out);
      auto Mu_in = (*Mu_soln)(node_in);
      auto S = (*S_flow_soln)(node_in);
      auto w_perim = (*w_perim_soln)(node_in);
      // hydraulic diameter in the i direction
      auto Dh_i = 4.0 * S / w_perim;
      auto time_term =
          _TR * ((*mdot_soln)(node_out)-mdot_soln->old(node_out)) * dz / _dt -
          dz * 2.0 * (*mdot_soln)(node_out) * (rho_out - rho_soln->old(node_out)) / rho_in / _dt;

      auto Mass_Term1 =
          std::pow((*mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
      auto Mass_Term2 = -2.0 * (*mdot_soln)(node_out) * (*SumWij_soln)(node_out) / S / rho_in;

      auto CrossFlow_Term = 0.0;
      auto Turbulent_Term = 0.0;

      unsigned int counter = 0;
      for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
      {
        auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
        unsigned int ii_ch = chans.first;
        unsigned int jj_ch = chans.second;
        auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
        auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
        auto * node_out_i = _subchannel_mesh.getChannelNode(ii_ch, iz);
        auto * node_out_j = _subchannel_mesh.getChannelNode(jj_ch, iz);
        auto rho_i = (*rho_soln)(node_in_i);
        auto rho_j = (*rho_soln)(node_in_j);
        auto Si = (*S_flow_soln)(node_in_i);
        auto Sj = (*S_flow_soln)(node_in_j);
        auto U_star = 0.0;
        // figure out donor axial velocity
        if (Wij(i_gap, iz) > 0.0)
        {
          U_star = (*mdot_soln)(node_out_i) / Si / rho_i;
        }
        else
        {
          U_star = (*mdot_soln)(node_out_j) / Sj / rho_j;
        }

        CrossFlow_Term +=
            _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz) * U_star;

        Turbulent_Term += WijPrime(i_gap, iz) * (2 * (*mdot_soln)(node_out) / rho_in / S -
                                                 (*mdot_soln)(node_out_j) / Sj / rho_j -
                                                 (*mdot_soln)(node_out_i) / Si / rho_i);
        counter++;
      }
      Turbulent_Term *= _CT;

      auto Re = (((*mdot_soln)(node_in) / S) * Dh_i / Mu_in);
      auto fi = computeFrictionFactor(Re);
      auto Friction_Term = (fi * dz / Dh_i) * 0.5 * (std::pow((*mdot_soln)(node_out), 2.0)) /
                           (S * (*rho_soln)(node_out));
      auto Gravity_Term = _g_grav * (*rho_soln)(node_out)*dz * S;
      auto DP = std::pow(S, -1.0) * (time_term + Mass_Term1 + Mass_Term2 + CrossFlow_Term +
                                     Turbulent_Term + Friction_Term + Gravity_Term); // Pa
      // update solution
      DP_soln->set(node_out, DP);
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeP(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  for (unsigned int iz = last_node; iz > first_node - 1; iz--)
  {
    // Calculate pressure in the inlet of the cell assuming known outlet
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      // update Pressure solution
      P_soln->set(node_in, (*P_soln)(node_out) + (*DP_soln)(node_out));
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeh(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      h_soln->set(node, _fp->h_from_p_T((*P_soln)(node), (*T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // go through the channels of the level.
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      // Start with applying mass-conservation equation & energy - conservation equation
      // Find the nodes for the top and bottom of this element.
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      // Copy the variables at the inlet (bottom) of this element.
      auto mdot_in = (*mdot_soln)(node_in);
      auto h_in = (*h_soln)(node_in); // J/kg
      auto volume = dz * (*S_flow_soln)(node_in);
      auto mdot_out = (*mdot_soln)(node_out);
      auto h_out = 0.0;
      double SumWijh = 0.0;
      double SumWijPrimeDhij = 0.0;
      // Calculate sum of crossflow into channel i from channels j around i
      unsigned int counter = 0;
      for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
      {
        auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
        unsigned int ii_ch = chans.first;
        // i is always the smallest and first index in the mapping
        unsigned int jj_ch = chans.second;
        auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
        auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
        // Define donor enthalpy
        auto h_star = 0.0;
        if (Wij(i_gap, iz) > 0.0)
        {
          h_star = (*h_soln)(node_in_i);
        }
        else if (Wij(i_gap, iz) < 0.0)
        {
          h_star = (*h_soln)(node_in_j);
        }
        // take care of the sign by applying the map, use donor cell
        SumWijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * Wij(i_gap, iz) * h_star;
        SumWijPrimeDhij += WijPrime(i_gap, iz) *
                           (2 * (*h_soln)(node_in) - (*h_soln)(node_in_j) - (*h_soln)(node_in_i));
        counter++;
      }

      h_out = (mdot_in * h_in - SumWijh - SumWijPrimeDhij +
               ((*q_prime_soln)(node_out) + (*q_prime_soln)(node_in)) * dz / 2.0 +
               _TR * rho_soln->old(node_out) * h_soln->old(node_out) * volume / _dt) /
              (mdot_out + _TR * (*rho_soln)(node_out)*volume / _dt);

      if (h_out < 0)
      {
        _console << "Wij = : " << Wij << "\n";
        mooseError(name(),
                   " : Calculation of negative Enthalpy h_out = : ",
                   h_out,
                   " Axial Level= : ",
                   iz);
      }
      // Update the solution vectors at the outlet of the cell
      h_soln->set(node_out, h_out); // J/kg
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeT(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      T_soln->set(node, _fp->T_from_p_h((*P_soln)(node), (*h_soln)(node)));
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeRho(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      rho_soln->set(node, _fp->rho_from_p_T((*P_soln)(node), (*T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      // Find the node
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      rho_soln->set(node, _fp->rho_from_p_T((*P_soln)(node), (*T_soln)(node)));
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeMu(int iblock)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      Mu_soln->set(node, _fp->mu_from_rho_T((*rho_soln)(node), (*T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _subchannel_mesh.getNumOfChannels(); i_ch++)
    {
      // Find the node
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      Mu_soln->set(node, _fp->mu_from_rho_T((*rho_soln)(node), (*T_soln)(node)));
    }
  }
}

Eigen::VectorXd
BetterSubChannel1PhaseProblemBase::computeResidualFunction(int iblock, Eigen::VectorXd solution)
{
  int last_node = (iblock + 1) * block_size;
  int first_node = iblock * block_size + 1;

  Eigen::MatrixXd Wij_residual_matrix(n_gaps, block_size);
  Wij_residual_matrix.setZero();
  Eigen::VectorXd Wij_residual_vector(n_gaps * block_size);
  Wij_residual_vector.setZero();

  // Assign the solution to the cross-flow matrix
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
    {
      int i = n_gaps * iz + i_gap; // column wise transfer
      Wij(i_gap, iz) = solution(i);
    }
  }

  // Calculating Sum Crossflows
  computeSumWij(iblock);

  // Solving axial flux
  computeMdot(iblock);

  // Calculation of Turbulent Crossflow
  computeWijPrime(iblock);

  // Solving for Pressure Drop
  computeDP(iblock);

  // Solving for pressure
  computeP(iblock);

  // Cross flow residual
  auto z_grid = _subchannel_mesh.getZGrid();
  const Real & pitch = _subchannel_mesh.getPitch();
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto dz = z_grid[iz] - z_grid[iz - 1];
    for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int i_ch = chans.first;
      unsigned int j_ch = chans.second;
      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
      auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
      auto rho_i = (*rho_soln)(node_in_i);
      auto rho_j = (*rho_soln)(node_in_j);
      // area of channel i
      auto Si = (*S_flow_soln)(node_in_i);
      // area of channel j
      auto Sj = (*S_flow_soln)(node_in_j);
      // crossflow area between channels i,j dz*gap_width
      auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
      // hydraulic diameter in the ij direction
      auto Lij = pitch;
      // total local form loss in the ij direction
      auto Kij = 0.5;

      // apply lateral pressure difference damping
      auto asp = 1.0; // means no damping
      auto DPi = (*DP_soln)(node_out_i);
      auto DPj = (*DP_soln)(node_out_j);
      auto DPij_out = (*P_soln)(node_out_i) - (*P_soln)(node_out_j);
      auto DPij_in = (*P_soln)(node_in_i) - (*P_soln)(node_in_j);
      auto DPij = (1 - asp) * (DPij_out + DPi - DPj) + asp * DPij_in;

      // Figure out donor cell density
      auto rho_star = 0.0;
      if (Wij(i_gap, iz) > 0.0)
      {
        rho_star = rho_i;
      }
      else if (Wij(i_gap, iz) < 0.0)
      {
        rho_star = rho_j;
      }
      else
      {
        rho_star = (rho_i + rho_j) / 2.0;
      }

      auto Mass_Term_out =
          (*mdot_soln)(node_out_i) / Si / rho_i + (*mdot_soln)(node_out_j) / Sj / rho_j;
      auto Mass_Term_in =
          (*mdot_soln)(node_in_i) / Si / rho_i + (*mdot_soln)(node_in_j) / Sj / rho_j;
      auto Term_out = Sij * rho_star * (Lij / dz) * Mass_Term_out;
      auto Term_in = Sij * rho_star * (Lij / dz) * Mass_Term_in * Wij(i_gap, iz - 1);
      auto Pressure_Term = 2 * std::pow(Sij, 2.0) * DPij * rho_star;
      auto time_term =
          _TR * 2.0 * (Wij(i_gap, iz) - Wij_old(i_gap, iz)) * Lij * Sij * rho_star / _dt;

      Wij_residual_matrix(i_gap, iz - 1 - iblock * block_size) =
          time_term + Kij * Wij(i_gap, iz) * std::abs(Wij(i_gap, iz)) + Term_out * Wij(i_gap, iz) -
          Term_in - Pressure_Term;
    }
  }

  // Make the residual matrix into a residual vector
  for (unsigned int iz = 0; iz < block_size; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < n_gaps; i_gap++)
    {
      int i = n_gaps * iz + i_gap; // column wise transfer
      Wij_residual_vector(i) = Wij_residual_matrix(i_gap, iz);
    }
  }
  return Wij_residual_vector;
}

void
BetterSubChannel1PhaseProblemBase::externalSolve()
{
  _console << "Executing subchannel solver\n";

  auto P_error = 1.0;
  auto P_tol = 1e-6;
  unsigned int P_it = 0;
  unsigned int P_it_max = 2 * n_blocks;
  if (n_blocks == 1)
    P_it_max = 1;
  while (P_error > P_tol && P_it < P_it_max)
  {
    P_it += 1;
    _console << "Solving Outer Iteration : " << P_it << std::endl;
    auto mdot_L2norm_old_axial = mdot_soln->L2norm();
    auto P_L2norm_old_axial = P_soln->L2norm();
    auto DP_L2norm_old_axial = DP_soln->L2norm();
    auto T_L2norm_old_axial = T_soln->L2norm();

    for (unsigned int iblock = 0; iblock < n_blocks; iblock++)
    {
      int last_node = (iblock + 1) * block_size;
      int first_node = iblock * block_size + 1;
      auto T_block_error = 1.0;
      auto T_tol = 1e-6;
      auto T_it_max = 5;
      auto T_it = 0;
      _console << "Solving Block :" << iblock << " From first node :" << first_node
               << " to last node :" << last_node << std::endl;
      while (T_block_error > T_tol && T_it < T_it_max)
      {
        T_it += 1;
        auto T_L2norm_old_block = T_soln->L2norm();

        // Compute Crossflow
        computeWij(iblock);

        if (_Power)
        {
          // Energy conservation equation
          computeh(iblock);

          // calculate temperature (equation of state)
          computeT(iblock);
        }

        if (_Density)
          computeRho(iblock);

        if (_Viscosity)
          computeMu(iblock);

        auto T_L2norm_new = T_soln->L2norm();
        T_block_error =
            std::abs((T_L2norm_new - T_L2norm_old_block) / (T_L2norm_old_block + 1E-14));
        _console << "T_block_error : " << T_block_error << std::endl;
      }
    }
    auto T_L2norm_new_axial = T_soln->L2norm();
    auto P_L2norm_new_axial = P_soln->L2norm();
    auto DP_L2norm_new_axial = DP_soln->L2norm();
    auto mdot_L2norm_new_axial = mdot_soln->L2norm();

    auto T_error =
        std::abs((T_L2norm_new_axial - T_L2norm_old_axial) / (T_L2norm_old_axial + 1E-14));
    P_error = std::abs((P_L2norm_new_axial - P_L2norm_old_axial) / (P_L2norm_old_axial + 1E-14));
    auto DP_error =
        std::abs((DP_L2norm_new_axial - DP_L2norm_old_axial) / (DP_L2norm_old_axial + 1E-14));
    auto mdot_error =
        std::abs((mdot_L2norm_new_axial - mdot_L2norm_old_axial) / (mdot_L2norm_old_axial + 1E-14));

    _console << "P_error" << P_error << std::endl;
  }

  // update old crossflow matrix
  Wij_old = Wij;
  // Save Wij_global
  std::ofstream myfile1;
  myfile1.open("Wij_global", std::ofstream::trunc);
  myfile1 << std::setprecision(12) << std::scientific << Wij << "\n";
  myfile1.close();
  _console << "Finished executing subchannel solver\n";
  _aux->solution().close();
}

void BetterSubChannel1PhaseProblemBase::syncSolutions(Direction /*direction*/) {}
