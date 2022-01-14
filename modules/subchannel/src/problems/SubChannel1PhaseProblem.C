#include "SubChannel1PhaseProblem.h"
#include "SystemBase.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include <petscdm.h>
#include <petscdmda.h>
#include <petscksp.h>
#include <petscsys.h>
#include <petscvec.h>
#include <petscsnes.h>
#include <iostream>
#include <cmath>
#include "AuxiliarySystem.h"

struct Ctx
{
  int iblock;
  SubChannel1PhaseProblem * schp;
};

PetscErrorCode
formFunction(SNES, Vec x, Vec f, void * ctx)
{
  PetscErrorCode ierr;
  const PetscScalar * xx;
  PetscScalar * ff;
  PetscInt size;
  Ctx * cc = static_cast<Ctx *>(ctx);
  ierr = VecGetSize(x, &size);
  CHKERRQ(ierr);

  libMesh::DenseVector<Real> solution_seed(size, 0.0);
  ierr = VecGetArrayRead(x, &xx);
  CHKERRQ(ierr);
  for (PetscInt i = 0; i < size; i++)
    solution_seed(i) = xx[i];

  libMesh::DenseVector<Real> Wij_residual_vector =
      cc->schp->residualFunction(cc->iblock, solution_seed);

  ierr = VecGetArray(f, &ff);
  CHKERRQ(ierr);
  for (PetscInt i = 0; i < size; i++)
    ff[i] = Wij_residual_vector(i);

  ierr = VecRestoreArrayRead(x, &xx);
  CHKERRQ(ierr);
  ierr = VecRestoreArray(f, &ff);
  CHKERRQ(ierr);
  return 0;
}

InputParameters
SubChannel1PhaseProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addRequiredParam<Real>("beta",
                                "Thermal diffusion coefficient used in turbulent crossflow");
  params.addRequiredParam<Real>("CT", "Turbulent modeling parameter");
  params.addParam<Real>("P_tol", 1e-6, "Pressure tolerance");
  params.addParam<Real>("T_tol", 1e-6, "Temperature tolerance");
  params.addParam<int>("T_maxit", 10, "Maximum number of iterations for inner temperature loop");
  params.addParam<PetscReal>("rtol", 1e-6, "Relative tolerance for ksp solver");
  params.addParam<PetscReal>("atol", 1e-6, "Absolute tolerance for ksp solver");
  params.addParam<PetscReal>("dtol", 1e5, "Divergence tolerance or ksp solver");
  params.addParam<PetscInt>("maxit", 1e4, "Maximum number of iterations for ksp solver");
  params.addRequiredParam<bool>("compute_density", "Flag that enables the calculation of density");
  params.addRequiredParam<bool>("compute_viscosity",
                                "Flag that enables the calculation of viscosity");
  params.addRequiredParam<bool>(
      "compute_power",
      "Flag that informs whether we solve the Enthalpy/Temperature equations or not");
  params.addRequiredParam<Real>("P_out", "Outlet Pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "Fluid properties user object name");
  return params;
}

SubChannel1PhaseProblem::SubChannel1PhaseProblem(const InputParameters & params)
  : ExternalProblem(params),
    _subchannel_mesh(dynamic_cast<QuadSubChannelMesh &>(_mesh)),
    _Wij(declareRestartableData<libMesh::DenseMatrix<Real>>("Wij")),
    _g_grav(9.87),
    _kij(_subchannel_mesh.getKij()),
    _one(1.0),
    _TR(isTransient() ? 1. : 0.),
    _compute_density(getParam<bool>("compute_density")),
    _compute_viscosity(getParam<bool>("compute_viscosity")),
    _compute_power(getParam<bool>("compute_power")),
    _pin_mesh_exist(_subchannel_mesh.pinMeshExist()),
    _dt(isTransient() ? dt() : _one),
    _P_out(getParam<Real>("P_out")),
    _beta(getParam<Real>("beta")),
    _CT(getParam<Real>("CT")),
    _P_tol(getParam<Real>("P_tol")),
    _T_tol(getParam<Real>("T_tol")),
    _T_maxit(getParam<int>("T_maxit")),
    _rtol(getParam<PetscReal>("rtol")),
    _atol(getParam<PetscReal>("atol")),
    _dtol(getParam<PetscReal>("dtol")),
    _maxit(getParam<PetscInt>("maxit")),
    _fp(nullptr)
{
  _n_cells = _subchannel_mesh.getNumOfAxialCells();
  _n_blocks = _subchannel_mesh.getNumOfAxialBlocks();
  _n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();
  _n_pins = _subchannel_mesh.getNumOfPins();
  _n_channels = _subchannel_mesh.getNumOfChannels();
  _nx = _subchannel_mesh.getNx();
  _ny = _subchannel_mesh.getNy();
  _z_grid = _subchannel_mesh.getZGrid();
  _block_size = _n_cells / _n_blocks;
  // Turbulent crossflow (stuff that live on the gaps)
  if (!_app.isRestarting() && !_app.isRecovering())
  {
    _Wij.resize(_n_gaps, _n_cells + 1);
    _Wij.zero();
  }
  _Wij_old.resize(_n_gaps, _n_cells + 1);
  _WijPrime.resize(_n_gaps, _n_cells + 1);
  _Wij_old.zero();
  _WijPrime.zero();
  _converged = true;
}

void
SubChannel1PhaseProblem::initialSetup()
{
  ExternalProblem::initialSetup();

  _fp = &getUserObject<SinglePhaseFluidProperties>(getParam<UserObjectName>("fp"));
  _mdot_soln = new SolutionHandle(getVariable(0, SubChannelApp::MASS_FLOW_RATE));
  _SumWij_soln = new SolutionHandle(getVariable(0, SubChannelApp::SUM_CROSSFLOW));
  _P_soln = new SolutionHandle(getVariable(0, SubChannelApp::PRESSURE));
  _DP_soln = new SolutionHandle(getVariable(0, SubChannelApp::PRESSURE_DROP));
  _h_soln = new SolutionHandle(getVariable(0, SubChannelApp::ENTHALPY));
  _T_soln = new SolutionHandle(getVariable(0, SubChannelApp::TEMPERATURE));
  _Tpin_soln = new SolutionHandle(getVariable(0, SubChannelApp::PIN_TEMPERATURE));
  _rho_soln = new SolutionHandle(getVariable(0, SubChannelApp::DENSITY));
  _mu_soln = new SolutionHandle(getVariable(0, SubChannelApp::VISCOSITY));
  _S_flow_soln = new SolutionHandle(getVariable(0, SubChannelApp::SURFACE_AREA));
  _w_perim_soln = new SolutionHandle(getVariable(0, SubChannelApp::WETTED_PERIMETER));
  _q_prime_soln = new SolutionHandle(getVariable(0, SubChannelApp::LINEAR_HEAT_RATE));
}

SubChannel1PhaseProblem::~SubChannel1PhaseProblem()
{
  delete _mdot_soln;
  delete _SumWij_soln;
  delete _P_soln;
  delete _DP_soln;
  delete _h_soln;
  delete _T_soln;
  delete _Tpin_soln;
  delete _rho_soln;
  delete _mu_soln;
  delete _S_flow_soln;
  delete _w_perim_soln;
  delete _q_prime_soln;
}

bool
SubChannel1PhaseProblem::converged()
{
  return _converged;
}

void
SubChannel1PhaseProblem::computeWij(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  // Initial guess, port crossflow of block (iblock) into a vector that will act as my initial guess
  libMesh::DenseVector<Real> solution_seed(_n_gaps * _block_size, 0.0);
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      int i = _n_gaps * (iz - first_node) + i_gap; // column wise transfer
      solution_seed(i) = _Wij(i_gap, iz);
    }
  }

  // Solving the combined lateral momentum equation for Wij using a PETSc solver and update vector
  // root
  libMesh::DenseVector<Real> root(_n_gaps * _block_size, 0.0);
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
SubChannel1PhaseProblem::computeSumWij(int iblock)
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
SubChannel1PhaseProblem::computeMdot(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto dz = _z_grid[iz] - _z_grid[iz - 1];
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto volume = dz * (*_S_flow_soln)(node_in);
      auto time_term = _TR * ((*_rho_soln)(node_out)-_rho_soln->old(node_out)) * volume / _dt;
      // Wij positive out of i into j;
      auto mdot_out = (*_mdot_soln)(node_in) - (*_SumWij_soln)(node_out)-time_term;
      if (mdot_out < 0)
      {
        _console << "Wij = : " << _Wij << "\n";
        mooseError(name(),
                   " : Calculation of negative mass flow mdot_out = : ",
                   mdot_out,
                   " Axial Level= : ",
                   iz);
      }
      _mdot_soln->set(node_out, mdot_out); // kg/sec
    }
  }
}

void
SubChannel1PhaseProblem::computeWijPrime(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto dz = _z_grid[iz] - _z_grid[iz - 1];
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
      unsigned int i_ch = chans.first;
      unsigned int j_ch = chans.second;
      auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
      auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);
      auto Si_in = (*_S_flow_soln)(node_in_i);
      auto Sj_in = (*_S_flow_soln)(node_in_j);
      auto Si_out = (*_S_flow_soln)(node_out_i);
      auto Sj_out = (*_S_flow_soln)(node_out_j);
      // crossflow area between channels i,j (dz*gap_width)
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
SubChannel1PhaseProblem::computeDP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto k_grid = _subchannel_mesh.getKGrid();
    auto dz = _z_grid[iz] - _z_grid[iz - 1];
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto rho_in = (*_rho_soln)(node_in);
      auto rho_out = (*_rho_soln)(node_out);
      auto mu_in = (*_mu_soln)(node_in);
      auto S = (*_S_flow_soln)(node_in);
      auto w_perim = (*_w_perim_soln)(node_in);
      // hydraulic diameter in the i direction
      auto Dh_i = 4.0 * S / w_perim;
      auto time_term =
          _TR * ((*_mdot_soln)(node_out)-_mdot_soln->old(node_out)) * dz / _dt -
          dz * 2.0 * (*_mdot_soln)(node_out) * (rho_out - _rho_soln->old(node_out)) / rho_in / _dt;

      auto mass_term1 =
          std::pow((*_mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
      auto mass_term2 = -2.0 * (*_mdot_soln)(node_out) * (*_SumWij_soln)(node_out) / S / rho_in;

      auto crossflow_term = 0.0;
      auto turbulent_term = 0.0;

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
        auto u_star = 0.0;
        // figure out donor axial velocity
        if (_Wij(i_gap, iz) > 0.0)
          u_star = (*_mdot_soln)(node_out_i) / Si / rho_i;
        else
          u_star = (*_mdot_soln)(node_out_j) / Sj / rho_j;

        crossflow_term +=
            _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * u_star;

        turbulent_term += _WijPrime(i_gap, iz) * (2 * (*_mdot_soln)(node_out) / rho_in / S -
                                                  (*_mdot_soln)(node_out_j) / Sj / rho_j -
                                                  (*_mdot_soln)(node_out_i) / Si / rho_i);
        counter++;
      }
      turbulent_term *= _CT;

      auto Re = (((*_mdot_soln)(node_in) / S) * Dh_i / mu_in);
      auto fi = computeFrictionFactor(Re);
      auto ki = k_grid[iz - 1];
      auto friction_term = (fi * dz / Dh_i + ki) * 0.5 * (std::pow((*_mdot_soln)(node_out), 2.0)) /
                           (S * (*_rho_soln)(node_out));
      auto gravity_term = _g_grav * (*_rho_soln)(node_out)*dz * S;
      auto DP = std::pow(S, -1.0) * (time_term + mass_term1 + mass_term2 + crossflow_term +
                                     turbulent_term + friction_term + gravity_term); // Pa
      _DP_soln->set(node_out, DP);
    }
  }
}

void
SubChannel1PhaseProblem::computeP(int iblock)
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

Real
SubChannel1PhaseProblem::computeAddedHeat(unsigned int i_ch, unsigned int iz)
{
  auto dz = _z_grid[iz] - _z_grid[iz - 1];
  if (_pin_mesh_exist)
  {
    auto heat_rate_in = 0.0;
    auto heat_rate_out = 0.0;
    for (auto i_pin : _subchannel_mesh.getChannelPins(i_ch))
    {
      auto * node_in = _subchannel_mesh.getPinNode(i_pin, iz - 1);
      auto * node_out = _subchannel_mesh.getPinNode(i_pin, iz);
      heat_rate_out += 0.25 * (*_q_prime_soln)(node_out);
      heat_rate_in += 0.25 * (*_q_prime_soln)(node_in);
    }
    return (heat_rate_in + heat_rate_out) * dz / 2.0;
  }
  else
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
    return ((*_q_prime_soln)(node_out) + (*_q_prime_soln)(node_in)) * dz / 2.0;
  }
}

void
SubChannel1PhaseProblem::computeh(int iblock)
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
    auto dz = _z_grid[iz] - _z_grid[iz - 1];
    auto heated_length = _subchannel_mesh.getHeatedLength();
    auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto mdot_in = (*_mdot_soln)(node_in);
      auto h_in = (*_h_soln)(node_in); // J/kg
      auto volume = dz * (*_S_flow_soln)(node_in);
      auto mdot_out = (*_mdot_soln)(node_out);
      auto h_out = 0.0;
      double sumWijh = 0.0;
      double sumWijPrimeDhij = 0.0;
      double added_enthalpy;
      if (_z_grid[iz] > unheated_length_entry &&
          _z_grid[iz] <= unheated_length_entry + heated_length)
        added_enthalpy = computeAddedHeat(i_ch, iz);
      else
        added_enthalpy = 0.0;

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
          h_star = (*_h_soln)(node_in_i);
        else if (_Wij(i_gap, iz) < 0.0)
          h_star = (*_h_soln)(node_in_j);
        // take care of the sign by applying the map, use donor cell
        sumWijh += _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, iz) * h_star;
        sumWijPrimeDhij += _WijPrime(i_gap, iz) * (2 * (*_h_soln)(node_in) - (*_h_soln)(node_in_j) -
                                                   (*_h_soln)(node_in_i));
        counter++;
      }

      h_out = (mdot_in * h_in - sumWijh - sumWijPrimeDhij + added_enthalpy +
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
      _h_soln->set(node_out, h_out); // J/kg
    }
  }
}

void
SubChannel1PhaseProblem::computeT(int iblock)
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
SubChannel1PhaseProblem::computeRho(int iblock)
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
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      _rho_soln->set(node, _fp->rho_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }
}

void
SubChannel1PhaseProblem::computeMu(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (iblock == 0)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, 0);
      _mu_soln->set(node, _fp->mu_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);
      _mu_soln->set(node, _fp->mu_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node)));
    }
  }
}

libMesh::DenseVector<Real>
SubChannel1PhaseProblem::residualFunction(int iblock, libMesh::DenseVector<Real> solution)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  libMesh::DenseMatrix<Real> Wij_residual_matrix(_n_gaps, _block_size);
  Wij_residual_matrix.zero();
  libMesh::DenseVector<Real> Wij_residual_vector(_n_gaps * _block_size, 0.0);

  // Assign the solution to the cross-flow matrix
  int i = 0;
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      _Wij(i_gap, iz) = solution(i);
      i++;
    }
  }

  // Calculating sum of crossflows
  computeSumWij(iblock);

  // Solving axial flux
  computeMdot(iblock);

  // Calculation of turbulent Crossflow
  computeWijPrime(iblock);

  // Solving for Pressure Drop
  computeDP(iblock);

  // Solving for pressure
  computeP(iblock);

  // Cross flow residual
  const Real & pitch = _subchannel_mesh.getPitch();
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto dz = _z_grid[iz] - _z_grid[iz - 1];
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
      auto Si = (*_S_flow_soln)(node_in_i);
      auto Sj = (*_S_flow_soln)(node_in_j);
      auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
      auto Lij = pitch;
      // total local form loss in the ij direction
      auto friction_term = _kij * _Wij(i_gap, iz) * std::abs(_Wij(i_gap, iz));
      auto DPij = (*_P_soln)(node_in_i) - (*_P_soln)(node_in_j);
      // Figure out donor cell density
      auto rho_star = 0.0;
      if (_Wij(i_gap, iz) > 0.0)
        rho_star = rho_i;
      else if (_Wij(i_gap, iz) < 0.0)
        rho_star = rho_j;
      else
        rho_star = (rho_i + rho_j) / 2.0;

      auto mass_term_out =
          (*_mdot_soln)(node_out_i) / Si / rho_i + (*_mdot_soln)(node_out_j) / Sj / rho_j;
      auto mass_term_in =
          (*_mdot_soln)(node_in_i) / Si / rho_i + (*_mdot_soln)(node_in_j) / Sj / rho_j;
      auto term_out = Sij * rho_star * (Lij / dz) * mass_term_out * _Wij(i_gap, iz);
      auto term_in = Sij * rho_star * (Lij / dz) * mass_term_in * _Wij(i_gap, iz - 1);
      auto inertia_term = term_out - term_in;
      auto pressure_term = 2 * std::pow(Sij, 2.0) * DPij * rho_star;
      auto time_term =
          _TR * 2.0 * (_Wij(i_gap, iz) - _Wij_old(i_gap, iz)) * Lij * Sij * rho_star / _dt;

      Wij_residual_matrix(i_gap, iz - 1 - iblock * _block_size) =
          time_term + friction_term + inertia_term - pressure_term;
    }
  }

  // Turn the residual matrix into a residual vector
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
SubChannel1PhaseProblem::petscSnesSolver(int iblock,
                                         const libMesh::DenseVector<Real> & solution,
                                         libMesh::DenseVector<Real> & root)
{
  SNES snes;
  KSP ksp;
  PC pc;
  Vec x, r;
  PetscErrorCode ierr;
  PetscMPIInt size;
  PetscScalar * xx;

  ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size);
  CHKERRMPI(ierr);
  if (size > 1)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_SUP, "Example is only for sequential runs");

  ierr = SNESCreate(PETSC_COMM_WORLD, &snes);
  CHKERRQ(ierr);

  ierr = VecCreate(PETSC_COMM_WORLD, &x);
  CHKERRQ(ierr);
  ierr = VecSetSizes(x, PETSC_DECIDE, _block_size * _n_gaps);
  CHKERRQ(ierr);
  ierr = VecSetFromOptions(x);
  CHKERRQ(ierr);
  ierr = VecDuplicate(x, &r);
  CHKERRQ(ierr);

#if PETSC_VERSION_LESS_THAN(3, 13, 0)
  PetscOptionsSetValue(PETSC_NULL, "-snes_mf", PETSC_NULL);
#else
  ierr = SNESSetUseMatrixFree(snes, PETSC_FALSE, PETSC_TRUE);
#endif
  CHKERRQ(ierr);
  Ctx ctx;
  ctx.iblock = iblock;
  ctx.schp = this;
  ierr = SNESSetFunction(snes, r, formFunction, &ctx);
  CHKERRQ(ierr);

  ierr = SNESGetKSP(snes, &ksp);
  CHKERRQ(ierr);
  ierr = KSPGetPC(ksp, &pc);
  CHKERRQ(ierr);
  ierr = PCSetType(pc, PCNONE);
  CHKERRQ(ierr);
  ierr = KSPSetTolerances(ksp, _rtol, _atol, _dtol, _maxit);
  CHKERRQ(ierr);

  ierr = SNESSetFromOptions(snes);
  CHKERRQ(ierr);

  ierr = VecGetArray(x, &xx);
  CHKERRQ(ierr);
  for (unsigned int i = 0; i < _block_size * _n_gaps; i++)
  {
    xx[i] = solution(i);
  }
  ierr = VecRestoreArray(x, &xx);
  CHKERRQ(ierr);

  ierr = SNESSolve(snes, NULL, x);
  CHKERRQ(ierr);

  ierr = VecGetArray(x, &xx);
  CHKERRQ(ierr);
  for (unsigned int i = 0; i < _block_size * _n_gaps; i++)
    root(i) = xx[i];

  ierr = VecRestoreArray(x, &xx);
  CHKERRQ(ierr);

  ierr = VecDestroy(&x);
  CHKERRQ(ierr);
  ierr = VecDestroy(&r);
  CHKERRQ(ierr);
  ierr = SNESDestroy(&snes);
  CHKERRQ(ierr);

  return ierr;
}

void
SubChannel1PhaseProblem::externalSolve()
{
  _console << "Executing subchannel solver\n";
  auto P_error = 1.0;
  unsigned int P_it = 0;
  unsigned int P_it_max = 2 * _n_blocks;
  if (_n_blocks == 1)
    P_it_max = 1;
  while (P_error > _P_tol && P_it < P_it_max)
  {
    P_it += 1;
    if (P_it == P_it_max and _n_blocks != 1)
    {
      _console << "Reached maximum number of axial pressure iterations" << std::endl;
      _converged = false;
    }
    _console << "Solving Outer Iteration : " << P_it << std::endl;
    auto P_L2norm_old_axial = _P_soln->L2norm();
    for (unsigned int iblock = 0; iblock < _n_blocks; iblock++)
    {
      int last_level = (iblock + 1) * _block_size;
      int first_level = iblock * _block_size + 1;
      auto T_block_error = 1.0;
      auto T_it = 0;
      _console << "Solving Block: " << iblock << " From first level: " << first_level
               << " to last level: " << last_level << std::endl;
      while (T_block_error > _T_tol && T_it < _T_maxit)
      {
        T_it += 1;
        if (T_it == _T_maxit)
        {
          _console << "Reached maximum number of temperature iterations for block: " << iblock
                   << std::endl;
          _converged = false;
        }
        auto T_L2norm_old_block = _T_soln->L2norm();

        computeWij(iblock);

        if (_compute_power)
        {
          computeh(iblock);

          computeT(iblock);
        }

        if (_compute_density)
          computeRho(iblock);

        if (_compute_viscosity)
          computeMu(iblock);

        auto T_L2norm_new = _T_soln->L2norm();
        T_block_error =
            std::abs((T_L2norm_new - T_L2norm_old_block) / (T_L2norm_old_block + 1E-14));
        _console << "T_block_error: " << T_block_error << std::endl;
      }
    }
    auto P_L2norm_new_axial = _P_soln->L2norm();
    P_error =
        std::abs((P_L2norm_new_axial - P_L2norm_old_axial) / (P_L2norm_old_axial + _P_out + 1E-14));
    _console << "P_error :" << P_error << std::endl;
  }
  // update old crossflow matrix
  _Wij_old = _Wij;

  auto power_in = 0.0;
  auto power_out = 0.0;
  for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
  {
    auto * node_in = _subchannel_mesh.getChannelNode(i_ch, 0);
    auto * node_out = _subchannel_mesh.getChannelNode(i_ch, _n_cells);
    power_in += (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
    power_out += (*_mdot_soln)(node_out) * (*_h_soln)(node_out);
  }
  _console << "Power added to coolant is: " << power_out - power_in << " Watt" << std::endl;
  if (_pin_mesh_exist)
  {
    _console << "Commencing calculation of Pin surface temperature \n";
    for (unsigned int i_pin = 0; i_pin < _n_pins; i_pin++)
    {
      for (unsigned int iz = 0; iz < _n_cells + 1; ++iz)
      {
        auto * pin_node = _subchannel_mesh.getPinNode(i_pin, iz);
        double sumTemp = 0.0;
        // Calculate sum of pin surface temperatures that the channels around the pin see
        for (auto i_ch : _subchannel_mesh.getPinChannels(i_pin))
        {
          auto * node = _subchannel_mesh.getChannelNode(i_ch, iz);

          auto mu = (*_mu_soln)(node);
          auto S = (*_S_flow_soln)(node);
          auto w_perim = (*_w_perim_soln)(node);
          auto Dh_i = 4.0 * S / w_perim;
          auto Re = (((*_mdot_soln)(node) / S) * Dh_i / mu);

          auto k = _fp->k_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node));
          auto cp = _fp->cp_from_p_T((*_P_soln)(node) + _P_out, (*_T_soln)(node));
          auto Pr = (*_mu_soln)(node)*cp / k;

          auto Nu = 0.023 * std::pow(Re, 0.8) * std::pow(Pr, 0.4);
          auto hw = Nu * k / Dh_i;

          sumTemp += (*_q_prime_soln)(pin_node) / (_subchannel_mesh.getRodDiameter() * M_PI * hw) +
                     (*_T_soln)(node);
        }
        _Tpin_soln->set(pin_node, 0.25 * sumTemp);
      }
    }
  }
  _aux->solution().close();
}

void SubChannel1PhaseProblem::syncSolutions(Direction /*direction*/) {}
