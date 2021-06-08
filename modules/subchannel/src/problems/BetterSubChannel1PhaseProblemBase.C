#include "BetterSubChannel1PhaseProblemBase.h"
#include "SystemBase.h"
#include "libmesh/petsc_vector.h"
#include <petscdm.h>
#include <petscdmda.h>
#include <petscksp.h>
#include <petscsys.h>
#include <petscvec.h>
#include <petscsnes.h>
#include <iostream>
#include <Eigen/Dense>
#include <cmath>
#include "AuxiliarySystem.h"
registerMooseObject("SubChannelApp", BetterSubChannel1PhaseProblemBase);

struct Ctx
{
  int iblock;
  BetterSubChannel1PhaseProblemBase * schp;
};

PetscErrorCode
formFunction(SNES snes, Vec x, Vec f, void * ctx)
{
  PetscErrorCode ierr;
  const PetscScalar * xx;
  PetscScalar * ff;
  PetscInt size;
  Ctx * cc = static_cast<Ctx *>(ctx);
  ierr = VecGetSize(x, &size);
  CHKERRQ(ierr);

  /*
   Get pointers to vector data.
      - For default PETSc vectors, VecGetArray() returns a pointer to
        the data array.  Otherwise, the routine is implementation dependent.
      - You MUST call VecRestoreArray() when you no longer need access to
        the array.
   */

  /// Put x into eigen vector solution_seed
  Eigen::VectorXd solution_seed(size);
  ierr = VecGetArrayRead(x, &xx);
  CHKERRQ(ierr);
  for (unsigned int i = 0; i < size; i++)
  {
    solution_seed(i) = xx[i];
  }

  Eigen::VectorXd Wij_residual_vector = cc->schp->residualFunction(cc->iblock, solution_seed);

  ierr = VecGetArray(f, &ff);
  CHKERRQ(ierr);

  /* Compute function */
  for (unsigned int i = 0; i < size; i++)
  {
    ff[i] = Wij_residual_vector(i);
  }

  /* Restore vectors */
  ierr = VecRestoreArrayRead(x, &xx);
  CHKERRQ(ierr);
  ierr = VecRestoreArray(f, &ff);
  CHKERRQ(ierr);
  return 0;
}

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
  params.addRequiredParam<Real>("P_out", "Outlet Pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  return params;
}

BetterSubChannel1PhaseProblemBase::BetterSubChannel1PhaseProblemBase(const InputParameters & params)
  : ExternalProblem(params),
    _g_grav(9.87),
    _one(1.0),
    _TR(isTransient() ? 1. : 0.),
    _density(getParam<bool>("Density")),
    _viscosity(getParam<bool>("Viscosity")),
    _power(getParam<bool>("Power")),
    _dt(isTransient() ? dt() : _one),
    _P_out(getParam<Real>("P_out")),
    _subchannel_mesh(dynamic_cast<BetterSubChannelMeshBase &>(_mesh)),
    _beta(getParam<Real>("beta")),
    _CT(getParam<Real>("CT")),
    _enforce_uniform_pressure(getParam<bool>("enforce_uniform_pressure")),
    _fp(nullptr)
{
  _n_cells = _subchannel_mesh.getNumOfAxialCells();
  _n_blocks = _subchannel_mesh.getNumOfAxialBlocks();
  _n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();
  _n_channels = _subchannel_mesh.getNumOfChannels();
  _block_size = _n_cells / _n_blocks;
  // Turbulent crossflow (stuff that live on the gaps)
  _Wij.resize(_n_gaps, _n_cells + 1);
  _Wij_old.resize(_n_gaps, _n_cells + 1);
  _WijPrime.resize(_n_gaps, _n_cells + 1);
  _Wij.setZero();
  _Wij_old.setZero();
  _WijPrime.setZero();
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
          _Wij(row_index, column_index) = std::stof(tmp);
          _Wij_old(row_index, column_index) = std::stof(tmp);
          column_index += 1;
        }
      }
      row_index += 1;
    }
  }

  _fp = &getUserObject<SinglePhaseFluidProperties>(getParam<UserObjectName>("fp"));
  _mdot_soln = new SolutionHandle(getVariable(0, "mdot"));
  _SumWij_soln = new SolutionHandle(getVariable(0, "SumWij"));
  _P_soln = new SolutionHandle(getVariable(0, "P"));
  _DP_soln = new SolutionHandle(getVariable(0, "DP"));
  _h_soln = new SolutionHandle(getVariable(0, "h"));
  _T_soln = new SolutionHandle(getVariable(0, "T"));
  _rho_soln = new SolutionHandle(getVariable(0, "rho"));
  _Mu_soln = new SolutionHandle(getVariable(0, "Mu"));
  _S_flow_soln = new SolutionHandle(getVariable(0, "S"));
  _w_perim_soln = new SolutionHandle(getVariable(0, "w_perim"));
  _q_prime_soln = new SolutionHandle(getVariable(0, "q_prime"));
}

BetterSubChannel1PhaseProblemBase::~BetterSubChannel1PhaseProblemBase()
{
  delete _mdot_soln;
  delete _SumWij_soln;
  delete _P_soln;
  delete _DP_soln;
  delete _h_soln;
  delete _T_soln;
  delete _rho_soln;
  delete _Mu_soln;
  delete _S_flow_soln;
  delete _w_perim_soln;
  delete _q_prime_soln;
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
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  /// Initial guess, port crossflow in block (iblock) into a vector that will act as my initial guess
  Eigen::MatrixXd solution_seed_matrix = _Wij.block(0, first_node, _n_gaps, _block_size);

  Eigen::VectorXd solution_seed(_n_gaps * _block_size);
  for (unsigned int iz = 0; iz < _block_size; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      int i = _n_gaps * iz + i_gap; // column wise transfer
      solution_seed(i) = solution_seed_matrix(i_gap, iz);
    }
  }

  /// Solving the combined lateral momentum equation for Wij using a PETSc solver and returns a vector root
  Eigen::VectorXd root(_n_gaps * _block_size);
  petscSnesSolver(iblock, solution_seed, root);

  // Assign the solution to the cross-flow matrix
  int i = 0;
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      _Wij(i_gap, iz) = root(i);
      i++;
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeSumWij(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      double sumWij = 0.0;
      // Calculate sum of crossflow into channel i from channels j around i
      unsigned int counter = 0;
      for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
      {
        sumWij += _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz);
        counter++;
      }
      // The net crossflow coming out of cell i [kg/sec]
      _SumWij_soln->set(node_out, sumWij);
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeMdot(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // go through the channels of the level.
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      // Start with applying mass-conservation equation & energy - conservation equation
      // Find the nodes for the top and bottom of this element.
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto volume = dz * (*_S_flow_soln)(node_in);
      // mass damping
      double am = 1.0; // means no damping
      auto time_term = _TR * ((*_rho_soln)(node_out)-_rho_soln->old(node_out)) * volume / _dt;
      // Wij positive out of i into j;
      auto mdot_out = am * ((*_mdot_soln)(node_in) - (*_SumWij_soln)(node_out)-time_term) +
                      (1.0 - am) * (*_mdot_soln)(node_out);
      if (mdot_out < 0)
      {
        _console << "Wij = : " << _Wij << "\n";
        mooseError(name(),
                   " : Calculation of negative mass flow mdot_out = : ",
                   mdot_out,
                   " Axial Level= : ",
                   iz);
      }
      // Update solution vector
      _mdot_soln->set(node_out, mdot_out); // kg/sec
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeWijPrime(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int i_ch = chans.first;
      unsigned int j_ch = chans.second;
      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
      auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
      // area of channel i
      auto Si_in = (*_S_flow_soln)(node_in_i);
      // area of channel j
      auto Sj_in = (*_S_flow_soln)(node_in_j);
      // area of channel i
      auto Si_out = (*_S_flow_soln)(node_out_i);
      // area of channel j
      auto Sj_out = (*_S_flow_soln)(node_out_j);
      // crossflow area between channels i,j dz*gap_width
      auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
      // Calculation of Turbulent Crossflow
      _WijPrime(i_gap, iz) =
          _beta * 0.5 *
          (((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j)) / (Si_in + Sj_in) +
           ((*_mdot_soln)(node_out_i) + (*_mdot_soln)(node_out_j)) / (Si_out + Sj_out)) *
          Sij;
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeDP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // Sweep through the channels of level
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      // Find the nodes for the top and bottom of this element.
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto rho_in = (*_rho_soln)(node_in);
      auto rho_out = (*_rho_soln)(node_out);
      auto Mu_in = (*_Mu_soln)(node_in);
      auto S = (*_S_flow_soln)(node_in);
      auto w_perim = (*_w_perim_soln)(node_in);
      // hydraulic diameter in the i direction
      auto Dh_i = 4.0 * S / w_perim;
      auto time_term =
          _TR * ((*_mdot_soln)(node_out)-_mdot_soln->old(node_out)) * dz / _dt -
          dz * 2.0 * (*_mdot_soln)(node_out) * (rho_out - _rho_soln->old(node_out)) / rho_in / _dt;

      auto Mass_Term1 =
          std::pow((*_mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
      auto Mass_Term2 = -2.0 * (*_mdot_soln)(node_out) * (*_SumWij_soln)(node_out) / S / rho_in;

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
        auto rho_i = (*_rho_soln)(node_in_i);
        auto rho_j = (*_rho_soln)(node_in_j);
        auto Si = (*_S_flow_soln)(node_in_i);
        auto Sj = (*_S_flow_soln)(node_in_j);
        auto U_star = 0.0;
        // figure out donor axial velocity
        if (_Wij(i_gap, iz) > 0.0)
        {
          U_star = (*_mdot_soln)(node_out_i) / Si / rho_i;
        }
        else
        {
          U_star = (*_mdot_soln)(node_out_j) / Sj / rho_j;
        }

        CrossFlow_Term +=
            _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * U_star;

        Turbulent_Term += _WijPrime(i_gap, iz) * (2 * (*_mdot_soln)(node_out) / rho_in / S -
                                                  (*_mdot_soln)(node_out_j) / Sj / rho_j -
                                                  (*_mdot_soln)(node_out_i) / Si / rho_i);
        counter++;
      }
      Turbulent_Term *= _CT;

      auto Re = (((*_mdot_soln)(node_in) / S) * Dh_i / Mu_in);
      auto fi = computeFrictionFactor(Re);
      auto Friction_Term = (fi * dz / Dh_i) * 0.5 * (std::pow((*_mdot_soln)(node_out), 2.0)) /
                           (S * (*_rho_soln)(node_out));
      auto Gravity_Term = _g_grav * (*_rho_soln)(node_out)*dz * S;
      auto DP = std::pow(S, -1.0) * (time_term + Mass_Term1 + Mass_Term2 + CrossFlow_Term +
                                     Turbulent_Term + Friction_Term + Gravity_Term); // Pa
      // update solution
      _DP_soln->set(node_out, DP);
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = last_node; iz > first_node - 1; iz--)
  {
    // Calculate pressure in the inlet of the cell assuming known outlet
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      // update Pressure solution
      _P_soln->set(node_in, (*_P_soln)(node_out) + (*_DP_soln)(node_out));
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeh(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      _h_soln->set(node, _fp->h_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto z_grid = _subchannel_mesh.getZGrid();
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // go through the channels of the level.
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      // Start with applying mass-conservation equation & energy - conservation equation
      // Find the nodes for the top and bottom of this element.
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      // Copy the variables at the inlet (bottom) of this element.
      auto mdot_in = (*_mdot_soln)(node_in);
      auto h_in = (*_h_soln)(node_in); // J/kg
      auto volume = dz * (*_S_flow_soln)(node_in);
      auto mdot_out = (*_mdot_soln)(node_out);
      auto h_out = 0.0;
      double sumWijh = 0.0;
      double sumWijPrimeDhij = 0.0;
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
        if (_Wij(i_gap, iz) > 0.0)
        {
          h_star = (*_h_soln)(node_in_i);
        }
        else if (_Wij(i_gap, iz) < 0.0)
        {
          h_star = (*_h_soln)(node_in_j);
        }
        // take care of the sign by applying the map, use donor cell
        sumWijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * h_star;
        sumWijPrimeDhij += _WijPrime(i_gap, iz) * (2 * (*_h_soln)(node_in) - (*_h_soln)(node_in_j) -
                                                   (*_h_soln)(node_in_i));
        counter++;
      }

      h_out = (mdot_in * h_in - sumWijh - sumWijPrimeDhij +
               ((*_q_prime_soln)(node_out) + (*_q_prime_soln)(node_in)) * dz / 2.0 +
               _TR * _rho_soln->old(node_out) * _h_soln->old(node_out) * volume / _dt) /
              (mdot_out + _TR * (*_rho_soln)(node_out)*volume / _dt);

      if (h_out < 0)
      {
        _console << "Wij = : " << _Wij << "\n";
        mooseError(name(),
                   " : Calculation of negative Enthalpy h_out = : ",
                   h_out,
                   " Axial Level= : ",
                   iz);
      }
      // Update the solution vectors at the outlet of the cell
      _h_soln->set(node_out, h_out); // J/kg
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeT(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      _T_soln->set(node, _fp->T_from_p_h((*_P_soln)(node) + _P_out, (*_h_soln)(node)));
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeRho(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      _rho_soln->set(node, _fp->rho_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      // Find the node
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      _rho_soln->set(node, _fp->rho_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }
}

void
BetterSubChannel1PhaseProblemBase::computeMu(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      _Mu_soln->set(node, _fp->mu_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      // Find the node
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      _Mu_soln->set(node, _fp->mu_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }
}

Eigen::VectorXd
BetterSubChannel1PhaseProblemBase::residualFunction(int iblock, Eigen::VectorXd solution)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  Eigen::MatrixXd Wij_residual_matrix(_n_gaps, _block_size);
  Wij_residual_matrix.setZero();
  Eigen::VectorXd Wij_residual_vector(_n_gaps * _block_size);
  Wij_residual_vector.setZero();

  // Assign the solution to the cross-flow matrix (This may not be needed)
  int i = 0;
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      _Wij(i_gap, iz) = solution(i);
      i++;
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
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int i_ch = chans.first;
      unsigned int j_ch = chans.second;
      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
      auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
      auto rho_i = (*_rho_soln)(node_in_i);
      auto rho_j = (*_rho_soln)(node_in_j);
      // area of channel i
      auto Si = (*_S_flow_soln)(node_in_i);
      // area of channel j
      auto Sj = (*_S_flow_soln)(node_in_j);
      // crossflow area between channels i,j dz*gap_width
      auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
      // hydraulic diameter in the ij direction
      auto Lij = pitch;
      // total local form loss in the ij direction
      auto Kij = 0.5;

      // apply lateral pressure difference damping
      auto asp = 1.0; // means no damping
      auto DPi = (*_DP_soln)(node_out_i);
      auto DPj = (*_DP_soln)(node_out_j);
      auto DPij_out = (*_P_soln)(node_out_i) - (*_P_soln)(node_out_j);
      auto DPij_in = (*_P_soln)(node_in_i) - (*_P_soln)(node_in_j);
      auto DPij = (1 - asp) * (DPij_out + DPi - DPj) + asp * DPij_in;

      // Figure out donor cell density
      auto rho_star = 0.0;
      if (_Wij(i_gap, iz) > 0.0)
      {
        rho_star = rho_i;
      }
      else if (_Wij(i_gap, iz) < 0.0)
      {
        rho_star = rho_j;
      }
      else
      {
        rho_star = (rho_i + rho_j) / 2.0;
      }

      auto Mass_Term_out =
          (*_mdot_soln)(node_out_i) / Si / rho_i + (*_mdot_soln)(node_out_j) / Sj / rho_j;
      auto Mass_Term_in =
          (*_mdot_soln)(node_in_i) / Si / rho_i + (*_mdot_soln)(node_in_j) / Sj / rho_j;
      auto Term_out = Sij * rho_star * (Lij / dz) * Mass_Term_out;
      auto Term_in = Sij * rho_star * (Lij / dz) * Mass_Term_in * _Wij(i_gap, iz - 1);
      auto Pressure_Term = 2 * std::pow(Sij, 2.0) * DPij * rho_star;
      auto time_term =
          _TR * 2.0 * (_Wij(i_gap, iz) - _Wij_old(i_gap, iz)) * Lij * Sij * rho_star / _dt;

      Wij_residual_matrix(i_gap, iz - 1 - iblock * _block_size) =
          time_term + Kij * _Wij(i_gap, iz) * std::abs(_Wij(i_gap, iz)) +
          Term_out * _Wij(i_gap, iz) - Term_in - Pressure_Term;
    }
  }

  // Make the residual matrix into a residual vector
  for (unsigned int iz = 0; iz < _block_size; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      int i = _n_gaps * iz + i_gap; // column wise transfer
      Wij_residual_vector(i) = Wij_residual_matrix(i_gap, iz);
    }
  }
  return Wij_residual_vector;
}

PetscErrorCode
BetterSubChannel1PhaseProblemBase::petscSnesSolver(int iblock,
                                                   const Eigen::VectorXd & solution,
                                                   Eigen::VectorXd & root)
{
  SNES snes; /* nonlinear solver context */
  KSP ksp;   /* linear solver context */
  PC pc;     /* preconditioner context */
  Vec x, r;  /* solution, residual vectors */
  PetscErrorCode ierr;
  PetscMPIInt size;
  PetscScalar * xx;

  ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size);
  CHKERRMPI(ierr);
  if (size > 1)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_SUP, "Example is only for sequential runs");

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create nonlinear solver context
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = SNESCreate(PETSC_COMM_WORLD, &snes);
  CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Create matrix and vector data structures; set corresponding routines
     - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /*
     Create vectors for solution and nonlinear function
  */
  ierr = VecCreate(PETSC_COMM_WORLD, &x);
  CHKERRQ(ierr);
  ierr = VecSetSizes(x, PETSC_DECIDE, _block_size * _n_gaps);
  CHKERRQ(ierr);
  ierr = VecSetFromOptions(x);
  CHKERRQ(ierr);
  ierr = VecDuplicate(x, &r);
  CHKERRQ(ierr);

  ierr = SNESSetUseMatrixFree(snes, PETSC_FALSE, PETSC_TRUE);
  CHKERRQ(ierr);
  Ctx ctx;
  ctx.iblock = iblock;
  ctx.schp = this;
  ierr = SNESSetFunction(snes, r, formFunction, &ctx);
  CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Customize nonlinear solver; set runtime options
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /*
     Set linear solver defaults for this problem. By extracting the
     KSP and PC contexts from the SNES context, we can then
     directly call any KSP and PC routines to set various options.
  */
  ierr = SNESGetKSP(snes, &ksp);
  CHKERRQ(ierr);
  ierr = KSPGetPC(ksp, &pc);
  CHKERRQ(ierr);
  ierr = PCSetType(pc, PCNONE);
  CHKERRQ(ierr);
  ierr = KSPSetTolerances(ksp, 1.e-4, PETSC_DEFAULT, PETSC_DEFAULT, 20);
  CHKERRQ(ierr);

  /*
     Set SNES/KSP/KSP/PC runtime options, e.g.,
         -snes_view -snes_monitor -ksp_type <ksp> -pc_type <pc>
     These options will override those specified above as long as
     SNESSetFromOptions() is called _after_ any other customization
     routines.
  */
  ierr = SNESSetFromOptions(snes);
  CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Evaluate initial guess; then solve nonlinear system
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = VecGetArray(x, &xx);
  CHKERRQ(ierr);
  for (unsigned int i = 0; i < _block_size * _n_gaps; i++)
  {
    xx[i] = solution(i);
  }
  ierr = VecRestoreArray(x, &xx);
  CHKERRQ(ierr);

  /*
     Note: The user should initialize the vector, x, with the initial guess
     for the nonlinear solver prior to calling SNESSolve().  In particular,
     to employ an initial guess of zero, the user should explicitly set
     this vector to zero by calling VecSet().
  */

  ierr = SNESSolve(snes, NULL, x);
  CHKERRQ(ierr);
  //  Vec f;
  //  ierr = VecView(x, PETSC_VIEWER_STDOUT_WORLD);
  //  CHKERRQ(ierr);
  //  ierr = SNESGetFunction(snes, &f, 0, 0);
  //  CHKERRQ(ierr);
  //  ierr = VecView(r, PETSC_VIEWER_STDOUT_WORLD);
  //  CHKERRQ(ierr);

  /// put x into eigen vector root and return that vector
  ierr = VecGetArray(x, &xx);
  CHKERRQ(ierr);
  for (unsigned int i = 0; i < _block_size * _n_gaps; i++)
  {
    root(i) = xx[i];
  }
  /* Restore vectors */
  ierr = VecRestoreArray(x, &xx);
  CHKERRQ(ierr);

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     Free work space.  All PETSc objects should be destroyed when they
     are no longer needed.
   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  ierr = VecDestroy(&x);
  CHKERRQ(ierr);
  ierr = VecDestroy(&r);
  CHKERRQ(ierr);
  ierr = SNESDestroy(&snes);
  CHKERRQ(ierr);

  return ierr;
}

void
BetterSubChannel1PhaseProblemBase::externalSolve()
{
  _console << "Executing subchannel solver\n";
  auto P_error = 1.0;
  auto P_tol = 1e-6;
  unsigned int P_it = 0;
  unsigned int P_it_max = 2 * _n_blocks;
  if (_n_blocks == 1)
    P_it_max = 1;
  while (P_error > P_tol && P_it < P_it_max)
  {
    P_it += 1;
    _console << "Solving Outer Iteration : " << P_it << std::endl;
    auto mdot_L2norm_old_axial = _mdot_soln->L2norm();
    auto P_L2norm_old_axial = _P_soln->L2norm();
    auto DP_L2norm_old_axial = _DP_soln->L2norm();
    auto T_L2norm_old_axial = _T_soln->L2norm();

    for (unsigned int iblock = 0; iblock < _n_blocks; iblock++)
    {
      int last_node = (iblock + 1) * _block_size;
      int first_node = iblock * _block_size + 1;
      auto T_block_error = 1.0;
      auto T_tol = 1e-6;
      auto T_it_max = 5;
      auto T_it = 0;
      _console << "Solving Block :" << iblock << " From first node :" << first_node
               << " to last node :" << last_node << std::endl;
      while (T_block_error > T_tol && T_it < T_it_max)
      {
        T_it += 1;
        auto T_L2norm_old_block = _T_soln->L2norm();

        // Compute Crossflow
        computeWij(iblock);

        if (_power)
        {
          // Energy conservation equation
          computeh(iblock);

          // calculate temperature (equation of state)
          computeT(iblock);
        }

        if (_density)
          computeRho(iblock);

        if (_viscosity)
          computeMu(iblock);

        auto T_L2norm_new = _T_soln->L2norm();
        T_block_error =
            std::abs((T_L2norm_new - T_L2norm_old_block) / (T_L2norm_old_block + 1E-14));
        _console << "T_block_error : " << T_block_error << std::endl;
      }
    }
    auto T_L2norm_new_axial = _T_soln->L2norm();
    auto P_L2norm_new_axial = _P_soln->L2norm();
    auto DP_L2norm_new_axial = _DP_soln->L2norm();
    auto mdot_L2norm_new_axial = _mdot_soln->L2norm();

    auto T_error =
        std::abs((T_L2norm_new_axial - T_L2norm_old_axial) / (T_L2norm_old_axial + 1E-14));
    P_error =
        std::abs((P_L2norm_new_axial - P_L2norm_old_axial) / (P_L2norm_old_axial + _P_out + 1E-14));
    auto DP_error =
        std::abs((DP_L2norm_new_axial - DP_L2norm_old_axial) / (DP_L2norm_old_axial + 1E-14));
    auto mdot_error =
        std::abs((mdot_L2norm_new_axial - mdot_L2norm_old_axial) / (mdot_L2norm_old_axial + 1E-14));

    _console << "P_error :" << P_error << std::endl;
  }

  // update old crossflow matrix
  _Wij_old = _Wij;
  // Save Wij
  std::ofstream myfile1;
  myfile1.open("Wij", std::ofstream::trunc);
  myfile1 << std::setprecision(12) << std::scientific << _Wij << "\n";
  myfile1.close();
  _console << "Finished executing subchannel solver\n";
  _aux->solution().close();
}

void BetterSubChannel1PhaseProblemBase::syncSolutions(Direction /*direction*/) {}
