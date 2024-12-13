//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterWrapper1PhaseProblem.h"
#include "SystemBase.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include <iostream>
#include <cmath>
#include "AuxiliarySystem.h"

struct CtxIW
{
  int iblock;
  InterWrapper1PhaseProblem * schp;
};

PetscErrorCode
formFunctionIW(SNES, Vec x, Vec f, void * ctxIW)
{
  const PetscScalar * xx;
  PetscScalar * ff;
  PetscInt size;

  PetscFunctionBegin;
  CtxIW * cc = static_cast<CtxIW *>(ctxIW);
  LibmeshPetscCall(VecGetSize(x, &size));

  libMesh::DenseVector<Real> solution_seed(size, 0.0);
  LibmeshPetscCall(VecGetArrayRead(x, &xx));
  for (PetscInt i = 0; i < size; i++)
    solution_seed(i) = xx[i];

  LibmeshPetscCall(VecRestoreArrayRead(x, &xx));

  libMesh::DenseVector<Real> Wij_residual_vector =
      cc->schp->residualFunction(cc->iblock, solution_seed);

  LibmeshPetscCall(VecGetArray(f, &ff));
  for (int i = 0; i < size; i++)
    ff[i] = Wij_residual_vector(i);

  LibmeshPetscCall(VecRestoreArray(f, &ff));
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

InputParameters
InterWrapper1PhaseProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();
  params.addClassDescription("Base class of the interwrapper solvers");
  params.addRequiredParam<unsigned int>("n_blocks", "The number of blocks in the axial direction");
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
  params.addParam<std::string>(
      "interpolation_scheme", "central_difference", "Interpolation scheme used for the method.");
  params.addParam<bool>(
      "implicit", false, "Boolean to define the use of explicit or implicit solution.");
  params.addParam<bool>(
      "staggered_pressure", false, "Boolean to define the use of explicit or implicit solution.");
  params.addParam<bool>(
      "segregated", true, "Boolean to define whether to use a segregated solution.");
  params.addParam<bool>(
      "monolithic_thermal", false, "Boolean to define whether to use thermal monolithic solve.");
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

InterWrapper1PhaseProblem::InterWrapper1PhaseProblem(const InputParameters & params)
  : ExternalProblem(params),
    _subchannel_mesh(dynamic_cast<InterWrapperMesh &>(_mesh)),
    _n_blocks(getParam<unsigned int>("n_blocks")),
    _Wij(declareRestartableData<libMesh::DenseMatrix<Real>>("Wij")),
    _g_grav(9.81),
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
    _interpolation_scheme(getParam<std::string>("interpolation_scheme")),
    _implicit_bool(getParam<bool>("implicit")),
    _staggered_pressure_bool(getParam<bool>("staggered_pressure")),
    _segregated_bool(getParam<bool>("segregated")),
    _monolithic_thermal_bool(getParam<bool>("monolithic_thermal")),
    _fp(nullptr),
    _Tpin_soln(nullptr)
{
  _n_cells = _subchannel_mesh.getNumOfAxialCells();
  _n_gaps = _subchannel_mesh.getNumOfGapsPerLayer();
  _n_pins = _subchannel_mesh.getNumOfPins();
  _n_channels = _subchannel_mesh.getNumOfChannels();
  _z_grid = _subchannel_mesh.getZGrid();
  _block_size = _n_cells / _n_blocks;
  // Turbulent crossflow (stuff that live on the gaps)
  if (!_app.isRestarting() && !_app.isRecovering())
  {
    _Wij.resize(_n_gaps, _n_cells + 1);
    _Wij.zero();
  }
  _Wij_old.resize(_n_gaps, _n_cells + 1);
  _Wij_old.zero();
  _WijPrime.resize(_n_gaps, _n_cells + 1);
  _WijPrime.zero();
  _Wij_residual_matrix.resize(_n_gaps, _block_size);
  _Wij_residual_matrix.zero();
  _converged = true;

  // Mass conservation components
  LibmeshPetscCall(
      createPetscMatrix(_mc_sumWij_mat, _block_size * _n_channels, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscVector(_Wij_vec, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscVector(_prod, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_prodp, _block_size * _n_channels));
  LibmeshPetscCall(createPetscMatrix(
      _mc_axial_convection_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_mc_axial_convection_rhs, _block_size * _n_channels));

  // Axial momentum conservation components
  LibmeshPetscCall(createPetscMatrix(
      _amc_turbulent_cross_flows_mat, _block_size * _n_gaps, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_turbulent_cross_flows_rhs, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscMatrix(
      _amc_time_derivative_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_time_derivative_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscMatrix(
      _amc_advective_derivative_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_advective_derivative_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscMatrix(
      _amc_cross_derivative_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_cross_derivative_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscMatrix(
      _amc_friction_force_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_friction_force_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_gravity_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscMatrix(
      _amc_pressure_force_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_pressure_force_rhs, _block_size * _n_channels));
  LibmeshPetscCall(
      createPetscMatrix(_amc_sys_mdot_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_amc_sys_mdot_rhs, _block_size * _n_channels));

  // Lateral momentum conservation components
  LibmeshPetscCall(
      createPetscMatrix(_cmc_time_derivative_mat, _block_size * _n_gaps, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscVector(_cmc_time_derivative_rhs, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscMatrix(
      _cmc_advective_derivative_mat, _block_size * _n_gaps, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscVector(_cmc_advective_derivative_rhs, _block_size * _n_gaps));
  LibmeshPetscCall(
      createPetscMatrix(_cmc_friction_force_mat, _block_size * _n_gaps, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscVector(_cmc_friction_force_rhs, _block_size * _n_gaps));
  LibmeshPetscCall(
      createPetscMatrix(_cmc_pressure_force_mat, _block_size * _n_gaps, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_cmc_pressure_force_rhs, _block_size * _n_gaps));
  LibmeshPetscCall(
      createPetscMatrix(_cmc_sys_Wij_mat, _block_size * _n_gaps, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscVector(_cmc_sys_Wij_rhs, _block_size * _n_gaps));
  LibmeshPetscCall(createPetscVector(_cmc_Wij_channel_dummy, _block_size * _n_channels));

  // Energy conservation components
  LibmeshPetscCall(createPetscMatrix(
      _hc_time_derivative_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_hc_time_derivative_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscMatrix(
      _hc_advective_derivative_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_hc_advective_derivative_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscMatrix(
      _hc_cross_derivative_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_hc_cross_derivative_rhs, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_hc_added_heat_rhs, _block_size * _n_channels));
  LibmeshPetscCall(
      createPetscMatrix(_hc_sys_h_mat, _block_size * _n_channels, _block_size * _n_channels));
  LibmeshPetscCall(createPetscVector(_hc_sys_h_rhs, _block_size * _n_channels));

  if ((_n_blocks == _n_cells) && _implicit_bool)
  {
    mooseError(name(),
               ": When implicit number of blocks can't be equal to number of cells. This will "
               "cause problems with the subchannel interpolation scheme.");
  }
}

void
InterWrapper1PhaseProblem::initialSetup()
{
  ExternalProblem::initialSetup();
  _fp = &getUserObject<SinglePhaseFluidProperties>(getParam<UserObjectName>("fp"));
  _mdot_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::MASS_FLOW_RATE));
  _SumWij_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::SUM_CROSSFLOW));
  _P_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::PRESSURE));
  _DP_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::PRESSURE_DROP));
  _h_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::ENTHALPY));
  _T_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::TEMPERATURE));
  if (_pin_mesh_exist)
    _Tpin_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::PIN_TEMPERATURE));
  _rho_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::DENSITY));
  _mu_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::VISCOSITY));
  _S_flow_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::SURFACE_AREA));
  _w_perim_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::WETTED_PERIMETER));
  _q_prime_soln = std::make_unique<SolutionHandle>(getVariable(0, SubChannelApp::LINEAR_HEAT_RATE));
}

InterWrapper1PhaseProblem::~InterWrapper1PhaseProblem()
{
  PetscErrorCode ierr = cleanUp();
  if (ierr)
    mooseError(name(), ": Error in memory cleanup");
}

PetscErrorCode
InterWrapper1PhaseProblem::cleanUp()
{
  PetscFunctionBegin;
  // We need to clean up the petsc matrices/vectors
  // Mass conservation components
  LibmeshPetscCall(MatDestroy(&_mc_sumWij_mat));
  LibmeshPetscCall(VecDestroy(&_Wij_vec));
  LibmeshPetscCall(VecDestroy(&_prod));
  LibmeshPetscCall(VecDestroy(&_prodp));
  LibmeshPetscCall(MatDestroy(&_mc_axial_convection_mat));
  LibmeshPetscCall(VecDestroy(&_mc_axial_convection_rhs));

  // Axial momentum conservation components
  LibmeshPetscCall(MatDestroy(&_amc_turbulent_cross_flows_mat));
  LibmeshPetscCall(VecDestroy(&_amc_turbulent_cross_flows_rhs));
  LibmeshPetscCall(MatDestroy(&_amc_time_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_amc_time_derivative_rhs));
  LibmeshPetscCall(MatDestroy(&_amc_advective_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_amc_advective_derivative_rhs));
  LibmeshPetscCall(MatDestroy(&_amc_cross_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_amc_cross_derivative_rhs));
  LibmeshPetscCall(MatDestroy(&_amc_friction_force_mat));
  LibmeshPetscCall(VecDestroy(&_amc_friction_force_rhs));
  LibmeshPetscCall(VecDestroy(&_amc_gravity_rhs));
  LibmeshPetscCall(MatDestroy(&_amc_pressure_force_mat));
  LibmeshPetscCall(VecDestroy(&_amc_pressure_force_rhs));
  LibmeshPetscCall(MatDestroy(&_amc_sys_mdot_mat));
  LibmeshPetscCall(VecDestroy(&_amc_sys_mdot_rhs));

  // Lateral momentum conservation components
  LibmeshPetscCall(MatDestroy(&_cmc_time_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_cmc_time_derivative_rhs));
  LibmeshPetscCall(MatDestroy(&_cmc_advective_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_cmc_advective_derivative_rhs));
  LibmeshPetscCall(MatDestroy(&_cmc_friction_force_mat));
  LibmeshPetscCall(VecDestroy(&_cmc_friction_force_rhs));
  LibmeshPetscCall(MatDestroy(&_cmc_pressure_force_mat));
  LibmeshPetscCall(VecDestroy(&_cmc_pressure_force_rhs));
  LibmeshPetscCall(MatDestroy(&_cmc_sys_Wij_mat));
  LibmeshPetscCall(VecDestroy(&_cmc_sys_Wij_rhs));
  LibmeshPetscCall(VecDestroy(&_cmc_Wij_channel_dummy));

  // Energy conservation components
  LibmeshPetscCall(MatDestroy(&_hc_time_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_hc_time_derivative_rhs));
  LibmeshPetscCall(MatDestroy(&_hc_advective_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_hc_advective_derivative_rhs));
  LibmeshPetscCall(MatDestroy(&_hc_cross_derivative_mat));
  LibmeshPetscCall(VecDestroy(&_hc_cross_derivative_rhs));
  LibmeshPetscCall(VecDestroy(&_hc_added_heat_rhs));
  LibmeshPetscCall(MatDestroy(&_hc_sys_h_mat));
  LibmeshPetscCall(VecDestroy(&_hc_sys_h_rhs));

  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

bool
InterWrapper1PhaseProblem::solverSystemConverged(const unsigned int)
{
  return _converged;
}

PetscScalar
InterWrapper1PhaseProblem::computeInterpolationCoefficients(const std::string interp_type,
                                                            PetscScalar Peclet)
{
  if (interp_type.compare("upwind") != 0)
  {
    return 1.0;
  }
  else if (interp_type.compare("downwind") != 0)
  {
    return 0.0;
  }
  else if (interp_type.compare("central_difference") != 0)
  {
    return 0.5;
  }
  else
  {
    return ((Peclet - 1.0) * std::exp(Peclet) + 1) / (Peclet * (std::exp(Peclet) - 1.) + 1e-10);
  }
}

PetscScalar
InterWrapper1PhaseProblem::computeInterpolatedValue(PetscScalar topValue,
                                                    PetscScalar botValue,
                                                    const std::string interp_type,
                                                    PetscScalar Peclet = 0.5)
{
  PetscScalar alpha = computeInterpolationCoefficients(interp_type, Peclet);
  return alpha * botValue + (1.0 - alpha) * topValue;
}

PetscErrorCode
InterWrapper1PhaseProblem::createPetscVector(Vec & v, PetscInt n)
{
  PetscFunctionBegin;
  LibmeshPetscCall(VecCreate(PETSC_COMM_WORLD, &v));
  LibmeshPetscCall(PetscObjectSetName((PetscObject)v, "Solution"));
  LibmeshPetscCall(VecSetSizes(v, PETSC_DECIDE, n));
  LibmeshPetscCall(VecSetFromOptions(v));
  LibmeshPetscCall(VecZeroEntries(v));
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

PetscErrorCode
InterWrapper1PhaseProblem::createPetscMatrix(Mat & M, PetscInt n, PetscInt m)
{
  PetscFunctionBegin;
  LibmeshPetscCall(MatCreate(PETSC_COMM_WORLD, &M));
  LibmeshPetscCall(MatSetSizes(M, PETSC_DECIDE, PETSC_DECIDE, n, m));
  LibmeshPetscCall(MatSetFromOptions(M));
  LibmeshPetscCall(MatSetUp(M));
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

template <class T>
PetscErrorCode
InterWrapper1PhaseProblem::populateVectorFromDense(Vec & x,
                                                   const T & loc_solution,
                                                   const unsigned int first_axial_level,
                                                   const unsigned int last_axial_level,
                                                   const unsigned int cross_dimension)
{
  PetscScalar * xx;
  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int iz = first_axial_level; iz < last_axial_level; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      xx[iz_ind * cross_dimension + i_l] =
          loc_solution(i_l, iz); // loc_solution(iz_ind*cross_dimension + i_l);
    }
  }
  LibmeshPetscCall(VecRestoreArray(x, &xx));
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

template <class T>
PetscErrorCode
InterWrapper1PhaseProblem::populateVectorFromHandle(Vec & x,
                                                    const T & loc_solution,
                                                    const unsigned int first_axial_level,
                                                    const unsigned int last_axial_level,
                                                    const unsigned int cross_dimension)
{
  PetscScalar * xx;
  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      auto * loc_node = _subchannel_mesh.getChannelNode(i_l, iz);
      xx[iz_ind * cross_dimension + i_l] = loc_solution(loc_node);
    }
  }
  LibmeshPetscCall(VecRestoreArray(x, &xx));
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

template <class T>
PetscErrorCode
InterWrapper1PhaseProblem::populateSolutionChan(const Vec & x,
                                                T & loc_solution,
                                                const unsigned int first_axial_level,
                                                const unsigned int last_axial_level,
                                                const unsigned int cross_dimension)
{
  PetscScalar * xx;
  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  Node * loc_node;
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      loc_node = _subchannel_mesh.getChannelNode(i_l, iz);
      loc_solution.set(loc_node, xx[iz_ind * cross_dimension + i_l]);
    }
  }
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

template <class T>
PetscErrorCode
InterWrapper1PhaseProblem::populateSolutionGap(const Vec & x,
                                               T & loc_solution,
                                               const unsigned int first_axial_level,
                                               const unsigned int last_axial_level,
                                               const unsigned int cross_dimension)
{
  PetscScalar * xx;
  PetscFunctionBegin;
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      loc_solution(iz * cross_dimension + i_l) = xx[iz_ind * cross_dimension + i_l];
    }
  }
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

void
InterWrapper1PhaseProblem::computeWijFromSolve(int iblock)
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
  LibmeshPetscCall(petscSnesSolver(iblock, solution_seed, root));

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
InterWrapper1PhaseProblem::computeSumWij(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  // Add to solution vector if explicit
  if (!_implicit_bool)
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        Real sumWij = 0.0;
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
  // Add to matrix if explicit
  else
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      unsigned int iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        // Calculate sum of crossflow into channel i from channels j around i
        unsigned int counter = 0;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          PetscInt row = i_ch + _n_channels * iz_ind;
          PetscInt col = i_gap + _n_gaps * iz_ind;
          PetscScalar value = _subchannel_mesh.getCrossflowSign(i_ch, counter);
          LibmeshPetscCall(MatSetValues(_mc_sumWij_mat, 1, &row, 1, &col, &value, INSERT_VALUES));
          counter++;
        }
      }
    }
    LibmeshPetscCall(MatAssemblyBegin(_mc_sumWij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_mc_sumWij_mat, MAT_FINAL_ASSEMBLY));
    if (_segregated_bool)
    {
      Vec loc_prod;
      Vec loc_Wij;
      LibmeshPetscCall(VecDuplicate(_amc_sys_mdot_rhs, &loc_prod));
      LibmeshPetscCall(VecDuplicate(_Wij_vec, &loc_Wij));
      LibmeshPetscCall(populateVectorFromDense<libMesh::DenseMatrix<Real>>(
          loc_Wij, _Wij, first_node, last_node + 1, _n_gaps));
      LibmeshPetscCall(MatMult(_mc_sumWij_mat, loc_Wij, loc_prod));
      LibmeshPetscCall(populateSolutionChan<SolutionHandle>(
          loc_prod, *_SumWij_soln, first_node, last_node, _n_channels));
      LibmeshPetscCall(VecDestroy(&loc_prod));
      LibmeshPetscCall(VecDestroy(&loc_Wij));
    }
  }
}

void
InterWrapper1PhaseProblem::computeMdot(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  if (!_implicit_bool)
  {
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
                     iz,
                     " - Implicit solves are required for recirculating flow.");
        }
        _mdot_soln->set(node_out, mdot_out); // kg/sec
      }
    }
  }
  else
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto volume = dz * (*_S_flow_soln)(node_in);

        // Adding time derivative to the RHS
        auto time_term = _TR * ((*_rho_soln)(node_out)-_rho_soln->old(node_out)) * volume / _dt;
        PetscInt row_vec = i_ch + _n_channels * iz_ind;
        PetscScalar value_vec = -1.0 * time_term;
        LibmeshPetscCall(
            VecSetValues(_mc_axial_convection_rhs, 1, &row_vec, &value_vec, INSERT_VALUES));

        // Imposing bottom boundary condition or adding of diagonal elements
        if (iz == first_node)
        {
          PetscScalar value_vec = (*_mdot_soln)(node_in);
          PetscInt row_vec = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_mc_axial_convection_rhs, 1, &row_vec, &value_vec, ADD_VALUES));
        }
        else
        {
          PetscInt row = i_ch + _n_channels * iz_ind;
          PetscInt col = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value = -1.0;
          LibmeshPetscCall(
              MatSetValues(_mc_axial_convection_mat, 1, &row, 1, &col, &value, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row = i_ch + _n_channels * iz_ind;
        PetscInt col = i_ch + _n_channels * iz_ind;
        PetscScalar value = 1.0;
        LibmeshPetscCall(
            MatSetValues(_mc_axial_convection_mat, 1, &row, 1, &col, &value, INSERT_VALUES));

        // Adding cross flows RHS
        if (_segregated_bool)
        {
          PetscScalar value_vec_2 = -1.0 * (*_SumWij_soln)(node_out);
          PetscInt row_vec_2 = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_mc_axial_convection_rhs, 1, &row_vec_2, &value_vec_2, ADD_VALUES));
        }
      }
    }
    LibmeshPetscCall(MatAssemblyBegin(_mc_axial_convection_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_mc_axial_convection_mat, MAT_FINAL_ASSEMBLY));
    _console << "Block: " << iblock << " - Mass conservation matrix assembled" << std::endl;

    if (_segregated_bool)
    {
      KSP ksploc;
      PC pc;
      Vec sol;
      LibmeshPetscCall(VecDuplicate(_mc_axial_convection_rhs, &sol));
      LibmeshPetscCall(KSPCreate(PETSC_COMM_WORLD, &ksploc));
      LibmeshPetscCall(KSPSetOperators(ksploc, _mc_axial_convection_mat, _mc_axial_convection_mat));
      LibmeshPetscCall(KSPGetPC(ksploc, &pc));
      LibmeshPetscCall(PCSetType(pc, PCJACOBI));
      LibmeshPetscCall(KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit));
      LibmeshPetscCall(KSPSetFromOptions(ksploc));
      LibmeshPetscCall(KSPSolve(ksploc, _mc_axial_convection_rhs, sol));
      LibmeshPetscCall(populateSolutionChan<SolutionHandle>(
          sol, *_mdot_soln, first_node, last_node, _n_channels));
      LibmeshPetscCall(VecZeroEntries(_mc_axial_convection_rhs));
      LibmeshPetscCall(KSPDestroy(&ksploc));
      LibmeshPetscCall(VecDestroy(&sol));
    }
  }
}

void
InterWrapper1PhaseProblem::computeWijPrime(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (!_implicit_bool)
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto chans = _subchannel_mesh.getGapChannels(i_gap);
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
  else
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node;
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto chans = _subchannel_mesh.getGapChannels(i_gap);
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

        // Base value - I don't want to write it every time
        PetscScalar base_value = _beta * 0.5 * Sij;

        // Bottom values
        if (iz == first_node)
        {
          PetscScalar value_tl = -1.0 * base_value / (Si_in + Sj_in) *
                                 ((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j));
          PetscInt row = i_gap + _n_gaps * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_amc_turbulent_cross_flows_rhs, 1, &row, &value_tl, INSERT_VALUES));
        }
        else
        {
          PetscScalar value_tl = base_value / (Si_in + Sj_in);
          PetscInt row = i_gap + _n_gaps * iz_ind;

          PetscInt col_ich = i_ch + _n_channels * (iz_ind - 1);
          LibmeshPetscCall(MatSetValues(
              _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_ich, &value_tl, INSERT_VALUES));

          PetscInt col_jch = j_ch + _n_channels * (iz_ind - 1);
          LibmeshPetscCall(MatSetValues(
              _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_jch, &value_tl, INSERT_VALUES));
        }

        // Top values
        PetscScalar value_bl = base_value / (Si_out + Sj_out);
        PetscInt row = i_gap + _n_gaps * iz_ind;

        PetscInt col_ich = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(MatSetValues(
            _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_ich, &value_bl, INSERT_VALUES));

        PetscInt col_jch = j_ch + _n_channels * iz_ind;
        LibmeshPetscCall(MatSetValues(
            _amc_turbulent_cross_flows_mat, 1, &row, 1, &col_jch, &value_bl, INSERT_VALUES));
      }
    }
    LibmeshPetscCall(MatAssemblyBegin(_amc_turbulent_cross_flows_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_turbulent_cross_flows_mat, MAT_FINAL_ASSEMBLY));

    /// Update turbulent crossflow
    Vec loc_prod;
    Vec loc_Wij;
    LibmeshPetscCall(VecDuplicate(_amc_sys_mdot_rhs, &loc_prod));
    LibmeshPetscCall(VecDuplicate(_Wij_vec, &loc_Wij));
    LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
        loc_prod, *_mdot_soln, first_node, last_node, _n_channels));
    LibmeshPetscCall(MatMult(_amc_turbulent_cross_flows_mat, loc_prod, loc_Wij));
    LibmeshPetscCall(VecAXPY(loc_Wij, -1.0, _amc_turbulent_cross_flows_rhs));
    LibmeshPetscCall(populateDenseFromVector<libMesh::DenseMatrix<Real>>(
        loc_Wij, _WijPrime, first_node, last_node, _n_gaps));
    LibmeshPetscCall(VecDestroy(&loc_prod));
    LibmeshPetscCall(VecDestroy(&loc_Wij));
  }
}

void
InterWrapper1PhaseProblem::computeDP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (!_implicit_bool)
  {
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
        auto time_term = _TR * ((*_mdot_soln)(node_out)-_mdot_soln->old(node_out)) * dz / _dt -
                         dz * 2.0 * (*_mdot_soln)(node_out) * (rho_out - _rho_soln->old(node_out)) /
                             rho_in / _dt;
        auto mass_term1 =
            std::pow((*_mdot_soln)(node_out), 2.0) * (1.0 / S / rho_out - 1.0 / S / rho_in);
        auto mass_term2 = -2.0 * (*_mdot_soln)(node_out) * (*_SumWij_soln)(node_out) / S / rho_in;
        auto crossflow_term = 0.0;
        auto turbulent_term = 0.0;
        unsigned int counter = 0;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapChannels(i_gap);
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
          Real u_star = 0.0;
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
        auto ki = k_grid[i_ch][iz - 1];
        auto friction_term = (fi * dz / Dh_i + ki) * 0.5 *
                             (std::pow((*_mdot_soln)(node_out), 2.0)) /
                             (S * (*_rho_soln)(node_out));
        auto gravity_term = _g_grav * (*_rho_soln)(node_out)*dz * S;
        auto DP = std::pow(S, -1.0) * (time_term + mass_term1 + mass_term2 + crossflow_term +
                                       turbulent_term + friction_term + gravity_term); // Pa
        _DP_soln->set(node_out, DP);
      }
    }
  }
  else
  {
    LibmeshPetscCall(MatZeroEntries(_amc_time_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_amc_advective_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_amc_cross_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_amc_friction_force_mat));
    LibmeshPetscCall(VecZeroEntries(_amc_time_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_amc_advective_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_amc_cross_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_amc_friction_force_rhs));
    LibmeshPetscCall(VecZeroEntries(_amc_gravity_rhs));
    LibmeshPetscCall(MatZeroEntries(_amc_sys_mdot_mat));
    LibmeshPetscCall(VecZeroEntries(_amc_sys_mdot_rhs));
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto k_grid = _subchannel_mesh.getKGrid();
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        // inlet and outlet nodes
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

        // interpolation weight coefficient
        PetscScalar Pe = 0.5;
        auto alpha = computeInterpolationCoefficients("central_difference", Pe);

        // inlet, outlet, and interpolated density
        auto rho_in = (*_rho_soln)(node_in);
        auto rho_out = (*_rho_soln)(node_out);
        auto rho_interp = computeInterpolatedValue(rho_out, rho_in, "central_difference");

        // inlet, outlet, and interpolated viscosity
        auto mu_in = (*_mu_soln)(node_in);
        auto mu_out = (*_mu_soln)(node_out);
        auto mu_interp = computeInterpolatedValue(mu_out, mu_in, "central_difference");

        // inlet, outlet, and interpolated axial surface area
        auto S_in = (*_S_flow_soln)(node_in);
        auto S_out = (*_S_flow_soln)(node_out);
        auto S_interp = computeInterpolatedValue(S_out, S_in, "central_difference");

        // inlet, outlet, and interpolated wetted perimeter
        auto w_perim_in = (*_w_perim_soln)(node_in);
        auto w_perim_out = (*_w_perim_soln)(node_out);
        auto w_perim_interp =
            computeInterpolatedValue(w_perim_out, w_perim_in, "central_difference");

        // hydraulic diameter in the i direction
        auto Dh_i = 4.0 * S_interp / w_perim_interp;

        /// Time derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_tt = -1.0 * _TR * alpha * (*_mdot_soln)(node_in)*dz / _dt;
          PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_amc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES));
        }
        else
        {
          PetscInt row_tt = i_ch + _n_channels * iz_ind;
          PetscInt col_tt = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_tt = _TR * alpha * dz / _dt;
          LibmeshPetscCall(MatSetValues(
              _amc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row_tt = i_ch + _n_channels * iz_ind;
        PetscInt col_tt = i_ch + _n_channels * iz_ind;
        PetscScalar value_tt = _TR * (1.0 - alpha) * dz / _dt;
        LibmeshPetscCall(MatSetValues(
            _amc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES));

        // Adding RHS elements
        PetscScalar mdot_old_interp = computeInterpolatedValue(
            _mdot_soln->old(node_out), _mdot_soln->old(node_in), "central_difference", Pe);
        PetscScalar value_vec_tt = _TR * mdot_old_interp * dz / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(
            VecSetValues(_amc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES));

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_at = std::pow((*_mdot_soln)(node_in), 2.0) / (S_in * rho_in);
          PetscInt row_vec_at = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(VecSetValues(
              _amc_advective_derivative_rhs, 1, &row_vec_at, &value_vec_at, ADD_VALUES));
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_at = -1.0 * std::abs((*_mdot_soln)(node_in)) / (S_in * rho_in);
          LibmeshPetscCall(MatSetValues(
              _amc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row_at = i_ch + _n_channels * iz_ind;
        PetscInt col_at = i_ch + _n_channels * iz_ind;
        PetscScalar value_at = std::abs((*_mdot_soln)(node_out)) / (S_out * rho_out);
        LibmeshPetscCall(MatSetValues(
            _amc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES));

        /// Cross derivative term
        unsigned int counter = 0;
        unsigned int cross_index = iz; // iz-1;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapChannels(i_gap);
          unsigned int ii_ch = chans.first;
          unsigned int jj_ch = chans.second;
          auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
          auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);
          auto * node_out_i = _subchannel_mesh.getChannelNode(ii_ch, iz);
          auto * node_out_j = _subchannel_mesh.getChannelNode(jj_ch, iz);
          auto rho_i = computeInterpolatedValue(
              (*_rho_soln)(node_out_i), (*_rho_soln)(node_in_i), "central_difference", Pe);
          auto rho_j = computeInterpolatedValue(
              (*_rho_soln)(node_out_j), (*_rho_soln)(node_in_j), "central_difference", Pe);
          auto S_i = computeInterpolatedValue(
              (*_S_flow_soln)(node_out_i), (*_S_flow_soln)(node_in_i), "central_difference", Pe);
          auto S_j = computeInterpolatedValue(
              (*_S_flow_soln)(node_out_j), (*_S_flow_soln)(node_in_j), "central_difference", Pe);
          auto u_star = 0.0;
          // figure out donor axial velocity
          if (_Wij(i_gap, cross_index) > 0.0)
          {
            if (iz == first_node)
            {
              u_star = (*_mdot_soln)(node_in_i) / S_i / rho_i;
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * u_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              LibmeshPetscCall(VecSetValues(
                  _amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index) / S_i / rho_i;
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = ii_ch + _n_channels * (iz_ind - 1);
              LibmeshPetscCall(MatSetValues(
                  _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index) / S_i / rho_i;
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = ii_ch + _n_channels * iz_ind;
            LibmeshPetscCall(MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
          }
          else if (_Wij(i_gap, cross_index) < 0.0) // _Wij=0 operations not necessary
          {
            if (iz == first_node)
            {
              u_star = (*_mdot_soln)(node_in_j) / S_j / rho_j;
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * u_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              LibmeshPetscCall(VecSetValues(
                  _amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index) / S_j / rho_j;
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = jj_ch + _n_channels * (iz_ind - 1);
              LibmeshPetscCall(MatSetValues(
                  _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index) / S_j / rho_j;
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = jj_ch + _n_channels * iz_ind;
            LibmeshPetscCall(MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
          }

          if (iz == first_node)
          {
            PetscScalar value_vec_ct = -2.0 * alpha *
                                       (*_mdot_soln)(node_in)*_WijPrime(i_gap, cross_index) /
                                       (rho_interp * S_interp);
            value_vec_ct +=
                alpha * (*_mdot_soln)(node_in_j)*_WijPrime(i_gap, cross_index) / (rho_j * S_j);
            value_vec_ct +=
                alpha * (*_mdot_soln)(node_in_i)*_WijPrime(i_gap, cross_index) / (rho_i * S_i);
            PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
            LibmeshPetscCall(
                VecSetValues(_amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
          }
          else
          {
            PetscScalar value_center_ct =
                2.0 * alpha * _WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = i_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES));

            PetscScalar value_left_ct =
                -1.0 * alpha * _WijPrime(i_gap, cross_index) / (rho_j * S_j);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = jj_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES));

            PetscScalar value_right_ct =
                -1.0 * alpha * _WijPrime(i_gap, cross_index) / (rho_i * S_i);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = ii_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES));
          }

          PetscScalar value_center_ct =
              2.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
          PetscInt row_ct = i_ch + _n_channels * iz_ind;
          PetscInt col_ct = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES));

          PetscScalar value_left_ct =
              -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index) / (rho_j * S_j);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES));

          PetscScalar value_right_ct =
              -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index) / (rho_i * S_i);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES));

          counter++;
        }

        /// Friction term
        PetscScalar mdot_interp = computeInterpolatedValue(
            (*_mdot_soln)(node_out), (*_mdot_soln)(node_in), "central_difference", Pe);
        auto Re = ((mdot_interp / S_interp) * Dh_i / mu_interp);
        auto fi = computeFrictionFactor(Re);
        auto ki = computeInterpolatedValue(
            k_grid[i_ch][iz], k_grid[i_ch][iz - 1], "central_difference", Pe);
        auto coef = (fi * dz / Dh_i + ki) * 0.5 * std::abs((*_mdot_soln)(node_out)) /
                    (S_interp * rho_interp);
        if (iz == first_node)
        {
          PetscScalar value_vec = -1.0 * alpha * coef * (*_mdot_soln)(node_in);
          PetscInt row_vec = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_amc_friction_force_rhs, 1, &row_vec, &value_vec, ADD_VALUES));
        }
        else
        {
          PetscInt row = i_ch + _n_channels * iz_ind;
          PetscInt col = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value = alpha * coef;
          LibmeshPetscCall(
              MatSetValues(_amc_friction_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row = i_ch + _n_channels * iz_ind;
        PetscInt col = i_ch + _n_channels * iz_ind;
        PetscScalar value = (1.0 - alpha) * coef;
        LibmeshPetscCall(
            MatSetValues(_amc_friction_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES));

        /// Gravity force
        PetscScalar value_vec = -1.0 * _g_grav * rho_interp * dz * S_interp;
        PetscInt row_vec = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(VecSetValues(_amc_gravity_rhs, 1, &row_vec, &value_vec, ADD_VALUES));
      }
    }
    /// Assembling system
    LibmeshPetscCall(MatZeroEntries(_amc_sys_mdot_mat));
    LibmeshPetscCall(VecZeroEntries(_amc_sys_mdot_rhs));
    LibmeshPetscCall(MatAssemblyBegin(_amc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_amc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_amc_cross_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_cross_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_amc_friction_force_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_friction_force_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    // Matrix
#if !PETSC_VERSION_LESS_THAN(3, 15, 0)
    LibmeshPetscCall(
        MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_friction_force_mat, UNKNOWN_NONZERO_PATTERN));
#else
    LibmeshPetscCall(
        MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_time_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_advective_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_amc_sys_mdot_mat, 1.0, _amc_friction_force_mat, DIFFERENT_NONZERO_PATTERN));
#endif
    LibmeshPetscCall(MatAssemblyBegin(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_amc_sys_mdot_mat, MAT_FINAL_ASSEMBLY));
    _console << "Block: " << iblock << " - Linear momentum conservation matrix assembled"
             << std::endl;
    // RHS
    LibmeshPetscCall(VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_time_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_advective_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_friction_force_rhs));
    LibmeshPetscCall(VecAXPY(_amc_sys_mdot_rhs, 1.0, _amc_gravity_rhs));

    if (_segregated_bool)
    {
      // Assembly the matrix system
      LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
          _prod, *_mdot_soln, first_node, last_node, _n_channels));
      Vec ls;
      LibmeshPetscCall(VecDuplicate(_amc_sys_mdot_rhs, &ls));
      LibmeshPetscCall(MatMult(_amc_sys_mdot_mat, _prod, ls));
      LibmeshPetscCall(VecAXPY(ls, -1.0, _amc_sys_mdot_rhs));
      PetscScalar * xx;
      LibmeshPetscCall(VecGetArray(ls, &xx));
      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          // Setting nodes
          auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

          // inlet, outlet, and interpolated axial surface area
          auto S_in = (*_S_flow_soln)(node_in);
          auto S_out = (*_S_flow_soln)(node_out);
          auto S_interp = computeInterpolatedValue(S_out, S_in, "central_difference");

          // Setting solutions
          if (S_interp != 0)
          {
            auto DP = std::pow(S_interp, -1.0) * xx[iz_ind * _n_channels + i_ch];
            _DP_soln->set(node_out, DP);
          }
          else
          {
            auto DP = 0.0;
            _DP_soln->set(node_out, DP);
          }
        }
      }
      LibmeshPetscCall(VecDestroy(&ls));
    }
  }
}

void
InterWrapper1PhaseProblem::computeP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;
  if (!_implicit_bool)
  {
    if (!_staggered_pressure_bool)
    {
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
    else
    {
      for (unsigned int iz = last_node; iz > first_node - 1; iz--)
      {
        // Calculate pressure in the inlet of the cell assuming known outlet
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
          // update Pressure solution
          // Note: assuming uniform axial discretization in the curren code
          // We will need to update this later if we allow non-uniform refinements in the axial
          // direction
          PetscScalar Pe = 0.5;
          auto alpha = computeInterpolationCoefficients("central_difference", Pe);
          if (iz == last_node)
          {
            _P_soln->set(node_in, (*_P_soln)(node_out) + (*_DP_soln)(node_out) / 2.0);
          }
          else
          {
            _P_soln->set(node_in,
                         (*_P_soln)(node_out) + (1.0 - alpha) * (*_DP_soln)(node_out) +
                             alpha * (*_DP_soln)(node_in));
          }
        }
      }
    }
  }
  else
  {
    if (!_staggered_pressure_bool)
    {
      LibmeshPetscCall(VecZeroEntries(_amc_pressure_force_rhs));
      for (unsigned int iz = last_node; iz > first_node - 1; iz--)
      {
        auto iz_ind = iz - first_node;
        // Calculate pressure in the inlet of the cell assuming known outlet
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);

          // inlet, outlet, and interpolated axial surface area
          auto S_in = (*_S_flow_soln)(node_in);
          auto S_out = (*_S_flow_soln)(node_out);
          auto S_interp = computeInterpolatedValue(S_out, S_in, "central_difference");

          // Creating matrix of coefficients
          PetscInt row = i_ch + _n_channels * iz_ind;
          PetscInt col = i_ch + _n_channels * iz_ind;
          PetscScalar value = -1.0 * S_interp;
          LibmeshPetscCall(
              MatSetValues(_amc_pressure_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES));

          if (iz == last_node)
          {
            PetscScalar value = -1.0 * (*_P_soln)(node_out)*S_interp;
            PetscInt row = i_ch + _n_channels * iz_ind;
            LibmeshPetscCall(VecSetValues(_amc_pressure_force_rhs, 1, &row, &value, ADD_VALUES));
          }
          else
          {
            PetscInt row = i_ch + _n_channels * iz_ind;
            PetscInt col = i_ch + _n_channels * (iz_ind + 1);
            PetscScalar value = 1.0 * S_interp;
            LibmeshPetscCall(
                MatSetValues(_amc_pressure_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES));
          }

          if (_segregated_bool)
          {
            auto dp_out = (*_DP_soln)(node_out);
            PetscScalar value_v = -1.0 * dp_out * S_interp;
            PetscInt row_v = i_ch + _n_channels * iz_ind;
            LibmeshPetscCall(
                VecSetValues(_amc_pressure_force_rhs, 1, &row_v, &value_v, ADD_VALUES));
          }
        }
      }
      // Solving pressure problem
      LibmeshPetscCall(MatAssemblyBegin(_amc_pressure_force_mat, MAT_FINAL_ASSEMBLY));
      LibmeshPetscCall(MatAssemblyEnd(_amc_pressure_force_mat, MAT_FINAL_ASSEMBLY));
      if (_segregated_bool)
      {
        KSP ksploc;
        PC pc;
        Vec sol;
        LibmeshPetscCall(VecDuplicate(_amc_pressure_force_rhs, &sol));
        LibmeshPetscCall(KSPCreate(PETSC_COMM_WORLD, &ksploc));
        LibmeshPetscCall(KSPSetOperators(ksploc, _amc_pressure_force_mat, _amc_pressure_force_mat));
        LibmeshPetscCall(KSPGetPC(ksploc, &pc));
        LibmeshPetscCall(PCSetType(pc, PCJACOBI));
        LibmeshPetscCall(KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit));
        LibmeshPetscCall(KSPSetFromOptions(ksploc));
        LibmeshPetscCall(KSPSolve(ksploc, _amc_pressure_force_rhs, sol));
        PetscScalar * xx;
        LibmeshPetscCall(VecGetArray(sol, &xx));
        // update Pressure solution
        for (unsigned int iz = last_node; iz > first_node - 1; iz--)
        {
          auto iz_ind = iz - first_node;
          for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
          {
            auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
            PetscScalar value = xx[iz_ind * _n_channels + i_ch];
            _P_soln->set(node_in, value);
          }
        }
        LibmeshPetscCall(VecZeroEntries(_amc_pressure_force_rhs));
        LibmeshPetscCall(KSPDestroy(&ksploc));
        LibmeshPetscCall(VecDestroy(&sol));
      }
    }
    else
    {
      LibmeshPetscCall(VecZeroEntries(_amc_pressure_force_rhs));
      for (unsigned int iz = last_node; iz > first_node - 1; iz--)
      {
        auto iz_ind = iz - first_node;
        // Calculate pressure in the inlet of the cell assuming known outlet
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);

          // inlet, outlet, and interpolated axial surface area
          auto S_in = (*_S_flow_soln)(node_in);
          auto S_out = (*_S_flow_soln)(node_out);
          auto S_interp = computeInterpolatedValue(S_out, S_in, "central_difference");

          // Creating matrix of coefficients
          PetscInt row = i_ch + _n_channels * iz_ind;
          PetscInt col = i_ch + _n_channels * iz_ind;
          PetscScalar value = -1.0 * S_interp;
          LibmeshPetscCall(
              MatSetValues(_amc_pressure_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES));

          if (iz == last_node)
          {
            PetscScalar value = -1.0 * (*_P_soln)(node_out)*S_interp;
            PetscInt row = i_ch + _n_channels * iz_ind;
            LibmeshPetscCall(VecSetValues(_amc_pressure_force_rhs, 1, &row, &value, ADD_VALUES));

            auto dp_out = (*_DP_soln)(node_out);
            PetscScalar value_v = -1.0 * dp_out / 2.0 * S_interp;
            PetscInt row_v = i_ch + _n_channels * iz_ind;
            LibmeshPetscCall(
                VecSetValues(_amc_pressure_force_rhs, 1, &row_v, &value_v, ADD_VALUES));
          }
          else
          {
            PetscInt row = i_ch + _n_channels * iz_ind;
            PetscInt col = i_ch + _n_channels * (iz_ind + 1);
            PetscScalar value = 1.0 * S_interp;
            LibmeshPetscCall(
                MatSetValues(_amc_pressure_force_mat, 1, &row, 1, &col, &value, INSERT_VALUES));

            if (_segregated_bool)
            {
              auto dp_in = (*_DP_soln)(node_in);
              auto dp_out = (*_DP_soln)(node_out);
              auto dp_interp = computeInterpolatedValue(dp_out, dp_in, "central_difference");
              PetscScalar value_v = -1.0 * dp_interp * S_interp;
              PetscInt row_v = i_ch + _n_channels * iz_ind;
              LibmeshPetscCall(
                  VecSetValues(_amc_pressure_force_rhs, 1, &row_v, &value_v, ADD_VALUES));
            }
          }
        }
      }
      // Solving pressure problem
      LibmeshPetscCall(MatAssemblyBegin(_amc_pressure_force_mat, MAT_FINAL_ASSEMBLY));
      LibmeshPetscCall(MatAssemblyEnd(_amc_pressure_force_mat, MAT_FINAL_ASSEMBLY));
      _console << "Block: " << iblock << " - Axial momentum pressure force matrix assembled"
               << std::endl;

      if (_segregated_bool)
      {
        KSP ksploc;
        PC pc;
        Vec sol;
        LibmeshPetscCall(VecDuplicate(_amc_pressure_force_rhs, &sol));
        LibmeshPetscCall(KSPCreate(PETSC_COMM_WORLD, &ksploc));
        LibmeshPetscCall(KSPSetOperators(ksploc, _amc_pressure_force_mat, _amc_pressure_force_mat));
        LibmeshPetscCall(KSPGetPC(ksploc, &pc));
        LibmeshPetscCall(PCSetType(pc, PCJACOBI));
        LibmeshPetscCall(KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit));
        LibmeshPetscCall(KSPSetFromOptions(ksploc));
        LibmeshPetscCall(KSPSolve(ksploc, _amc_pressure_force_rhs, sol));
        PetscScalar * xx;
        LibmeshPetscCall(VecGetArray(sol, &xx));
        // update Pressure solution
        for (unsigned int iz = last_node; iz > first_node - 1; iz--)
        {
          auto iz_ind = iz - first_node;
          for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
          {
            auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
            PetscScalar value = xx[iz_ind * _n_channels + i_ch];
            _P_soln->set(node_in, value);
          }
        }
        LibmeshPetscCall(VecZeroEntries(_amc_pressure_force_rhs));
        LibmeshPetscCall(KSPDestroy(&ksploc));
        LibmeshPetscCall(VecDestroy(&sol));
      }
    }
  }
}

Real
InterWrapper1PhaseProblem::computeAddedHeat(unsigned int i_ch, unsigned int iz)
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
InterWrapper1PhaseProblem::computeh(int iblock)
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

  if (!_implicit_bool)
  {
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
        Real sumWijh = 0.0;
        Real sumWijPrimeDhij = 0.0;
        Real added_enthalpy;
        if (_z_grid[iz] > unheated_length_entry &&
            _z_grid[iz] <= unheated_length_entry + heated_length)
          added_enthalpy = computeAddedHeat(i_ch, iz);
        else
          added_enthalpy = 0.0;

        // Calculate sum of crossflow into channel i from channels j around i
        unsigned int counter = 0;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapChannels(i_gap);
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
          sumWijPrimeDhij += _WijPrime(i_gap, iz) * (2 * (*_h_soln)(node_in) -
                                                     (*_h_soln)(node_in_j) - (*_h_soln)(node_in_i));
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
  else
  {
    _console << "Setting matrices" << std::endl;
    LibmeshPetscCall(MatZeroEntries(_hc_time_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_hc_advective_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_hc_cross_derivative_mat));
    LibmeshPetscCall(VecZeroEntries(_hc_time_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_hc_advective_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_hc_cross_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_hc_added_heat_rhs));
    LibmeshPetscCall(MatZeroEntries(_hc_sys_h_mat));
    LibmeshPetscCall(VecZeroEntries(_hc_sys_h_rhs));

    _console << "Starting enthalpy assembly" << std::endl;
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto heated_length = _subchannel_mesh.getHeatedLength();
      auto unheated_length_entry = _subchannel_mesh.getHeatedLengthEntry();
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto volume = dz * (*_S_flow_soln)(node_in);

        // interpolation weight coefficient
        PetscScalar Pe = 0.5;
        auto alpha = computeInterpolationCoefficients("central_difference", Pe);

        /// Time derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_tt =
              -1.0 * _TR * alpha * (*_rho_soln)(node_in) * (*_h_soln)(node_in)*volume / _dt;
          PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(
              VecSetValues(_hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES));
        }
        else
        {
          PetscInt row_tt = i_ch + _n_channels * iz_ind;
          PetscInt col_tt = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_tt = _TR * alpha * (*_rho_soln)(node_in)*volume / _dt;
          LibmeshPetscCall(MatSetValues(
              _hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row_tt = i_ch + _n_channels * iz_ind;
        PetscInt col_tt = i_ch + _n_channels * iz_ind;
        PetscScalar value_tt = _TR * (1.0 - alpha) * (*_rho_soln)(node_out)*volume / _dt;
        LibmeshPetscCall(MatSetValues(
            _hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES));

        // Adding RHS elements
        PetscScalar rho_old_interp = computeInterpolatedValue(
            _rho_soln->old(node_out), _rho_soln->old(node_in), "central_difference", Pe);
        PetscScalar h_old_interp = computeInterpolatedValue(
            _h_soln->old(node_out), _h_soln->old(node_in), "central_difference", Pe);

        PetscScalar value_vec_tt = _TR * rho_old_interp * h_old_interp * volume / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(
            VecSetValues(_hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES));

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_at = (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
          PetscInt row_vec_at = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(VecSetValues(
              _hc_advective_derivative_rhs, 1, &row_vec_at, &value_vec_at, ADD_VALUES));
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_at = -1.0 * (*_mdot_soln)(node_in);
          LibmeshPetscCall(MatSetValues(
              _hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES));
        }

        // Adding diagonal elements
        PetscInt row_at = i_ch + _n_channels * iz_ind;
        PetscInt col_at = i_ch + _n_channels * iz_ind;
        PetscScalar value_at = (*_mdot_soln)(node_out);
        LibmeshPetscCall(MatSetValues(
            _hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES));

        /// Cross derivative term
        unsigned int counter = 0;
        unsigned int cross_index = iz; // iz-1;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapChannels(i_gap);
          unsigned int ii_ch = chans.first;
          unsigned int jj_ch = chans.second;
          auto * node_in_i = _subchannel_mesh.getChannelNode(ii_ch, iz - 1);
          auto * node_in_j = _subchannel_mesh.getChannelNode(jj_ch, iz - 1);

          PetscScalar h_star;
          // figure out donor axial velocity
          if (_Wij(i_gap, cross_index) > 0.0)
          {
            if (iz == first_node)
            {
              h_star = (*_h_soln)(node_in_i);
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * h_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              LibmeshPetscCall(VecSetValues(
                  _hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = ii_ch + _n_channels * (iz_ind - 1);
              LibmeshPetscCall(MatSetValues(
                  _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = ii_ch + _n_channels * iz_ind;
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
          }
          else if (_Wij(i_gap, cross_index) < 0.0) // _Wij=0 operations not necessary
          {
            if (iz == first_node)
            {
              h_star = (*_h_soln)(node_in_j);
              PetscScalar value_vec_ct = -1.0 * alpha *
                                         _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                         _Wij(i_gap, cross_index) * h_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              LibmeshPetscCall(VecSetValues(
                  _hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = jj_ch + _n_channels * (iz_ind - 1);
              LibmeshPetscCall(MatSetValues(
                  _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = jj_ch + _n_channels * iz_ind;
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES));
          }

          // Turbulent cross flows
          if (iz == first_node)
          {
            PetscScalar value_vec_ct =
                -2.0 * alpha * (*_mdot_soln)(node_in)*_WijPrime(i_gap, cross_index);
            value_vec_ct = alpha * (*_mdot_soln)(node_in_j)*_WijPrime(i_gap, cross_index);
            value_vec_ct += alpha * (*_mdot_soln)(node_in_i)*_WijPrime(i_gap, cross_index);
            PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
            LibmeshPetscCall(
                VecSetValues(_hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES));
          }
          else
          {
            PetscScalar value_center_ct = 2.0 * alpha * _WijPrime(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = i_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES));

            PetscScalar value_left_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = jj_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES));

            PetscScalar value_right_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = ii_ch + _n_channels * (iz_ind - 1);
            LibmeshPetscCall(MatSetValues(
                _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES));
          }

          PetscScalar value_center_ct = 2.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          PetscInt row_ct = i_ch + _n_channels * iz_ind;
          PetscInt col_ct = i_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES));

          PetscScalar value_left_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES));

          PetscScalar value_right_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          LibmeshPetscCall(MatSetValues(
              _hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES));

          counter++;
        }

        /// Added heat enthalpy
        PetscScalar added_enthalpy;
        if (_z_grid[iz] > unheated_length_entry &&
            _z_grid[iz] <= unheated_length_entry + heated_length)
          added_enthalpy = computeAddedHeat(i_ch, iz);
        else
          added_enthalpy = 0.0;

        PetscInt row_vec_ht = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(
            VecSetValues(_hc_added_heat_rhs, 1, &row_vec_ht, &added_enthalpy, ADD_VALUES));
      }
    }
    _console << "Done with enthalpy assembly" << std::endl;

    /// Assembling system
    LibmeshPetscCall(MatAssemblyBegin(_hc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_hc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_hc_cross_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_cross_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    // Matrix
#if !PETSC_VERSION_LESS_THAN(3, 15, 0)
    LibmeshPetscCall(MatAXPY(_hc_sys_h_mat, 1.0, _hc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_cross_derivative_mat, UNKNOWN_NONZERO_PATTERN));
#else
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_time_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_advective_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_hc_sys_h_mat, 1.0, _hc_cross_derivative_mat, DIFFERENT_NONZERO_PATTERN));
#endif
    LibmeshPetscCall(MatAssemblyBegin(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_hc_sys_h_mat, MAT_FINAL_ASSEMBLY));
    _console << "Block: " << iblock << " - Enthalpy conservation matrix assembled" << std::endl;
    // RHS
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_time_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_advective_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_cross_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_hc_sys_h_rhs, 1.0, _hc_added_heat_rhs));

    if (_segregated_bool)
    {
      // Assembly the matrix system
      KSP ksploc;
      PC pc;
      Vec sol;
      LibmeshPetscCall(VecDuplicate(_hc_sys_h_rhs, &sol));
      LibmeshPetscCall(KSPCreate(PETSC_COMM_WORLD, &ksploc));
      LibmeshPetscCall(KSPSetOperators(ksploc, _hc_sys_h_mat, _hc_sys_h_mat));
      LibmeshPetscCall(KSPGetPC(ksploc, &pc));
      LibmeshPetscCall(PCSetType(pc, PCJACOBI));
      LibmeshPetscCall(KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit));
      LibmeshPetscCall(KSPSetFromOptions(ksploc));
      LibmeshPetscCall(KSPSolve(ksploc, _hc_sys_h_rhs, sol));
      PetscScalar * xx;
      LibmeshPetscCall(VecGetArray(sol, &xx));

      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

          if (xx[iz_ind * _n_channels + i_ch] < 0)
          {
            _console << "Wij = : " << _Wij << "\n";
            mooseError(name(),
                       " : Calculation of negative Enthalpy h_out = : ",
                       xx[iz_ind * _n_channels + i_ch],
                       " Axial Level= : ",
                       iz);
          }
          _h_soln->set(node_out, xx[iz_ind * _n_channels + i_ch]);
        }
      }
      LibmeshPetscCall(KSPDestroy(&ksploc));
      LibmeshPetscCall(VecDestroy(&sol));
    }
  }
}

void
InterWrapper1PhaseProblem::computeT(int iblock)
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
InterWrapper1PhaseProblem::computeRho(int iblock)
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
InterWrapper1PhaseProblem::computeMu(int iblock)
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

void
InterWrapper1PhaseProblem::computeWij(int iblock)
{
  // Cross flow residual
  if (!_implicit_bool)
  {
    unsigned int last_node = (iblock + 1) * _block_size;
    unsigned int first_node = iblock * _block_size;

    const Real & pitch = _subchannel_mesh.getPitch();
    for (unsigned int iz = first_node + 1; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto chans = _subchannel_mesh.getGapChannels(i_gap);
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

        _Wij_residual_matrix(i_gap, iz - 1 - iblock * _block_size) =
            time_term + friction_term + inertia_term - pressure_term;
      }
    }
  }
  else
  {

    // Initializing to zero the elements of the lateral momentum assembly
    LibmeshPetscCall(MatZeroEntries(_cmc_time_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_cmc_advective_derivative_mat));
    LibmeshPetscCall(MatZeroEntries(_cmc_friction_force_mat));
    LibmeshPetscCall(MatZeroEntries(_cmc_pressure_force_mat));
    LibmeshPetscCall(VecZeroEntries(_cmc_time_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_cmc_advective_derivative_rhs));
    LibmeshPetscCall(VecZeroEntries(_cmc_friction_force_rhs));
    LibmeshPetscCall(VecZeroEntries(_cmc_pressure_force_rhs));
    LibmeshPetscCall(MatZeroEntries(_cmc_sys_Wij_mat));
    LibmeshPetscCall(VecZeroEntries(_cmc_sys_Wij_rhs));

    unsigned int last_node = (iblock + 1) * _block_size;
    unsigned int first_node = iblock * _block_size;

    const Real & pitch = _subchannel_mesh.getPitch();
    for (unsigned int iz = first_node + 1; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node - 1;
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto chans = _subchannel_mesh.getGapChannels(i_gap);
        unsigned int i_ch = chans.first;
        unsigned int j_ch = chans.second;
        auto * node_in_i = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
        auto * node_out_i = _subchannel_mesh.getChannelNode(i_ch, iz);
        auto * node_in_j = _subchannel_mesh.getChannelNode(j_ch, iz - 1);
        auto * node_out_j = _subchannel_mesh.getChannelNode(j_ch, iz);

        // inlet, outlet, and interpolated densities
        auto rho_i_in = (*_rho_soln)(node_in_i);
        auto rho_i_out = (*_rho_soln)(node_out_i);
        auto rho_i_interp = computeInterpolatedValue(rho_i_out, rho_i_in, "central_difference");
        auto rho_j_in = (*_rho_soln)(node_in_j);
        auto rho_j_out = (*_rho_soln)(node_out_j);
        auto rho_j_interp = computeInterpolatedValue(rho_j_out, rho_j_in, "central_difference");

        // inlet, outlet, and interpolated areas
        auto S_i_in = (*_S_flow_soln)(node_in_i);
        auto S_i_out = (*_S_flow_soln)(node_out_i);
        auto S_j_in = (*_S_flow_soln)(node_in_j);
        auto S_j_out = (*_S_flow_soln)(node_out_j);

        // Cross-sectional gap area
        auto Sij = dz * _subchannel_mesh.getGapWidth(i_gap);
        auto Lij = pitch;

        // Figure out donor cell density
        auto rho_star = 0.0;
        if (_Wij(i_gap, iz) > 0.0)
          rho_star = rho_i_interp;
        else if (_Wij(i_gap, iz) < 0.0)
          rho_star = rho_j_interp;
        else
          rho_star = (rho_i_interp + rho_j_interp) / 2.0;

        // Assembling time derivative
        PetscScalar time_factor = _TR * Lij * Sij * rho_star / _dt;
        PetscInt row_td = i_gap + _n_gaps * iz_ind;
        PetscInt col_td = i_gap + _n_gaps * iz_ind;
        PetscScalar value_td = time_factor;
        LibmeshPetscCall(MatSetValues(
            _cmc_time_derivative_mat, 1, &row_td, 1, &col_td, &value_td, INSERT_VALUES));
        PetscScalar value_td_rhs = time_factor * _Wij_old(i_gap, iz);
        LibmeshPetscCall(
            VecSetValues(_cmc_time_derivative_rhs, 1, &row_td, &value_td_rhs, INSERT_VALUES));

        // Assembling inertial term
        PetscScalar Pe = 0.5;
        auto alpha = computeInterpolationCoefficients("upwind", Pe);
        auto mass_term_out = (*_mdot_soln)(node_out_i) / S_i_out / rho_i_out +
                             (*_mdot_soln)(node_out_j) / S_j_out / rho_j_out;
        auto mass_term_in = (*_mdot_soln)(node_in_i) / S_i_in / rho_i_in +
                            (*_mdot_soln)(node_in_j) / S_j_in / rho_j_in;
        auto term_out = Sij * rho_star * (Lij / dz) * mass_term_out / 2.0;
        auto term_in = Sij * rho_star * (Lij / dz) * mass_term_in / 2.0;
        if (iz == first_node + 1)
        {
          PetscInt row_ad = i_gap + _n_gaps * iz_ind;
          PetscScalar value_ad = term_in * alpha * _Wij(i_gap, iz - 1);
          LibmeshPetscCall(
              VecSetValues(_cmc_advective_derivative_rhs, 1, &row_ad, &value_ad, ADD_VALUES));

          PetscInt col_ad = i_gap + _n_gaps * iz_ind;
          value_ad = -1.0 * term_in * (1.0 - alpha) + term_out * alpha;
          LibmeshPetscCall(MatSetValues(
              _cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES));

          col_ad = i_gap + _n_gaps * (iz_ind + 1);
          value_ad = term_out * (1.0 - alpha);
          LibmeshPetscCall(MatSetValues(
              _cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES));
        }
        else if (iz == last_node)
        {
          PetscInt row_ad = i_gap + _n_gaps * iz_ind;
          PetscInt col_ad = i_gap + _n_gaps * (iz_ind - 1);
          PetscScalar value_ad = -1.0 * term_in * alpha;
          LibmeshPetscCall(MatSetValues(
              _cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES));

          col_ad = i_gap + _n_gaps * iz_ind;
          value_ad = -1.0 * term_in * (1.0 - alpha) + term_out * alpha;
          LibmeshPetscCall(MatSetValues(
              _cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES));

          value_ad = -1.0 * term_out * (1.0 - alpha) * _Wij(i_gap, iz);
          LibmeshPetscCall(
              VecSetValues(_cmc_advective_derivative_rhs, 1, &row_ad, &value_ad, ADD_VALUES));
        }
        else
        {
          PetscInt row_ad = i_gap + _n_gaps * iz_ind;
          PetscInt col_ad = i_gap + _n_gaps * (iz_ind - 1);
          PetscScalar value_ad = -1.0 * term_in * alpha;
          LibmeshPetscCall(MatSetValues(
              _cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES));

          col_ad = i_gap + _n_gaps * iz_ind;
          value_ad = -1.0 * term_in * (1.0 - alpha) + term_out * alpha;
          LibmeshPetscCall(MatSetValues(
              _cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES));

          col_ad = i_gap + _n_gaps * (iz_ind + 1);
          value_ad = term_out * (1.0 - alpha);
          LibmeshPetscCall(MatSetValues(
              _cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES));
        }
        // Assembling friction force
        PetscInt row_ff = i_gap + _n_gaps * iz_ind;
        PetscInt col_ff = i_gap + _n_gaps * iz_ind;
        PetscScalar value_ff = _kij * std::abs(_Wij(i_gap, iz)) / 2.0;
        LibmeshPetscCall(MatSetValues(
            _cmc_friction_force_mat, 1, &row_ff, 1, &col_ff, &value_ff, INSERT_VALUES));

        // Assembling pressure force
        alpha = computeInterpolationCoefficients("central_difference", Pe);

        if (!_staggered_pressure_bool)
        {
          PetscScalar pressure_factor = std::pow(Sij, 2.0) * rho_star;
          PetscInt row_pf = i_gap + _n_gaps * iz_ind;
          PetscInt col_pf = i_ch + _n_channels * iz_ind;
          PetscScalar value_pf = -1.0 * alpha * pressure_factor;
          LibmeshPetscCall(
              MatSetValues(_cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES));
          col_pf = j_ch + _n_channels * iz_ind;
          value_pf = alpha * pressure_factor;
          LibmeshPetscCall(
              MatSetValues(_cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES));

          if (iz == last_node)
          {
            PetscInt row_pf = i_gap + _n_gaps * iz_ind;
            PetscScalar value_pf = (1.0 - alpha) * pressure_factor * (*_P_soln)(node_out_i);
            LibmeshPetscCall(
                VecSetValues(_cmc_pressure_force_rhs, 1, &row_pf, &value_pf, ADD_VALUES));
            value_pf = -1.0 * (1.0 - alpha) * pressure_factor * (*_P_soln)(node_out_j);
            LibmeshPetscCall(
                VecSetValues(_cmc_pressure_force_rhs, 1, &row_pf, &value_pf, ADD_VALUES));
          }
          else
          {
            row_pf = i_gap + _n_gaps * iz_ind;
            col_pf = i_ch + _n_channels * (iz_ind + 1);
            value_pf = -1.0 * (1.0 - alpha) * pressure_factor;
            LibmeshPetscCall(MatSetValues(
                _cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES));
            col_pf = j_ch + _n_channels * (iz_ind + 1);
            value_pf = (1.0 - alpha) * pressure_factor;
            LibmeshPetscCall(MatSetValues(
                _cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES));
          }
        }
        else
        {
          PetscScalar pressure_factor = std::pow(Sij, 2.0) * rho_star;
          PetscInt row_pf = i_gap + _n_gaps * iz_ind;
          PetscInt col_pf = i_ch + _n_channels * iz_ind;
          PetscScalar value_pf = -1.0 * pressure_factor;
          LibmeshPetscCall(
              MatSetValues(_cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES));
          col_pf = j_ch + _n_channels * iz_ind;
          value_pf = pressure_factor;
          LibmeshPetscCall(
              MatSetValues(_cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES));
        }
      }
    }
    /// Assembling system
    LibmeshPetscCall(MatZeroEntries(_cmc_sys_Wij_mat));
    LibmeshPetscCall(VecZeroEntries(_cmc_sys_Wij_rhs));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_time_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_advective_derivative_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_friction_force_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_friction_force_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_pressure_force_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_pressure_force_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    // Matrix
#if !PETSC_VERSION_LESS_THAN(3, 15, 0)
    LibmeshPetscCall(
        MatAXPY(_cmc_sys_Wij_mat, 1.0, _cmc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_cmc_sys_Wij_mat, 1.0, _cmc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_cmc_sys_Wij_mat, 1.0, _cmc_friction_force_mat, UNKNOWN_NONZERO_PATTERN));
#else
    LibmeshPetscCall(
        MatAXPY(_cmc_sys_Wij_mat, 1.0, _cmc_time_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_cmc_sys_Wij_mat, 1.0, _cmc_advective_derivative_mat, DIFFERENT_NONZERO_PATTERN));
    LibmeshPetscCall(MatAssemblyBegin(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(
        MatAXPY(_cmc_sys_Wij_mat, 1.0, _cmc_friction_force_mat, DIFFERENT_NONZERO_PATTERN));
#endif
    LibmeshPetscCall(MatAssemblyBegin(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(_cmc_sys_Wij_mat, MAT_FINAL_ASSEMBLY));
    _console << "Block: " << iblock << " - Cross flow system matrix assembled" << std::endl;
    _console << "Block: " << iblock << " - Cross flow pressure force matrix assembled" << std::endl;
    // RHS
    LibmeshPetscCall(VecAXPY(_cmc_sys_Wij_rhs, 1.0, _cmc_time_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_cmc_sys_Wij_rhs, 1.0, _cmc_advective_derivative_rhs));
    LibmeshPetscCall(VecAXPY(_cmc_sys_Wij_rhs, 1.0, _cmc_friction_force_rhs));

    if (_segregated_bool)
    {
      // Assembly the matrix system
      Vec sol_holder_P;
      LibmeshPetscCall(createPetscVector(sol_holder_P, _block_size * _n_gaps));
      Vec sol_holder_W;
      LibmeshPetscCall(createPetscVector(sol_holder_W, _block_size * _n_gaps));
      Vec loc_holder_Wij;
      LibmeshPetscCall(createPetscVector(loc_holder_Wij, _block_size * _n_gaps));
      LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
          _prodp, *_P_soln, iblock * _block_size, (iblock + 1) * _block_size - 1, _n_channels));
      LibmeshPetscCall(populateVectorFromDense<libMesh::DenseMatrix<Real>>(
          loc_holder_Wij, _Wij, first_node, last_node, _n_gaps));
      LibmeshPetscCall(MatMult(_cmc_sys_Wij_mat, _Wij_vec, sol_holder_W));
      LibmeshPetscCall(VecAXPY(sol_holder_W, -1.0, _cmc_sys_Wij_rhs));
      LibmeshPetscCall(MatMult(_cmc_pressure_force_mat, _prodp, sol_holder_P));
      LibmeshPetscCall(VecAXPY(sol_holder_P, -1.0, _cmc_pressure_force_rhs));
      LibmeshPetscCall(VecAXPY(sol_holder_W, 1.0, sol_holder_P));
      PetscScalar * xx;
      LibmeshPetscCall(VecGetArray(sol_holder_W, &xx));
      for (unsigned int iz = first_node + 1; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node - 1;
        for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
        {
          _Wij_residual_matrix(i_gap, iz - 1 - iblock * _block_size) = xx[iz_ind * _n_gaps + i_gap];
        }
      }
      LibmeshPetscCall(VecDestroy(&sol_holder_P));
      LibmeshPetscCall(VecDestroy(&sol_holder_W));
      LibmeshPetscCall(VecDestroy(&loc_holder_Wij));
    }
  }
}

libMesh::DenseVector<Real>
InterWrapper1PhaseProblem::residualFunction(int iblock, libMesh::DenseVector<Real> solution)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size;
  libMesh::DenseVector<Real> Wij_residual_vector(_n_gaps * _block_size, 0.0);
  // Assign the solution to the cross-flow matrix
  int i = 0;
  for (unsigned int iz = first_node + 1; iz < last_node + 1; iz++)
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
  // Solving cross fluxes
  computeWij(iblock);

  // Turn the residual matrix into a residual vector
  for (unsigned int iz = 0; iz < _block_size; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      int i = _n_gaps * iz + i_gap; // column wise transfer
      Wij_residual_vector(i) = _Wij_residual_matrix(i_gap, iz);
    }
  }
  return Wij_residual_vector;
}

PetscErrorCode
InterWrapper1PhaseProblem::petscSnesSolver(int iblock,
                                           const libMesh::DenseVector<Real> & solution,
                                           libMesh::DenseVector<Real> & root)
{
  SNES snes;
  KSP ksp;
  PC pc;
  Vec x, r;
  PetscMPIInt size;
  PetscScalar * xx;

  PetscFunctionBegin;
  PetscCallMPI(MPI_Comm_size(PETSC_COMM_WORLD, &size));
  if (size > 1)
    SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_SUP, "Example is only for sequential runs");
  LibmeshPetscCall(SNESCreate(PETSC_COMM_WORLD, &snes));
  LibmeshPetscCall(VecCreate(PETSC_COMM_WORLD, &x));
  LibmeshPetscCall(VecSetSizes(x, PETSC_DECIDE, _block_size * _n_gaps));
  LibmeshPetscCall(VecSetFromOptions(x));
  LibmeshPetscCall(VecDuplicate(x, &r));

#if PETSC_VERSION_LESS_THAN(3, 13, 0)
  LibmeshPetscCall(PetscOptionsSetValue(PETSC_NULL, "-snes_mf", PETSC_NULL));
#else
  LibmeshPetscCall(SNESSetUseMatrixFree(snes, PETSC_FALSE, PETSC_TRUE));
#endif
  CtxIW ctxIW;
  ctxIW.iblock = iblock;
  ctxIW.schp = this;
  LibmeshPetscCall(SNESSetFunction(snes, r, formFunctionIW, &ctxIW));

  LibmeshPetscCall(SNESGetKSP(snes, &ksp));
  LibmeshPetscCall(KSPGetPC(ksp, &pc));
  LibmeshPetscCall(PCSetType(pc, PCNONE));
  LibmeshPetscCall(KSPSetTolerances(ksp, _rtol, _atol, _dtol, _maxit));
  LibmeshPetscCall(SNESSetFromOptions(snes));
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int i = 0; i < _block_size * _n_gaps; i++)
  {
    xx[i] = solution(i);
  }
  LibmeshPetscCall(VecRestoreArray(x, &xx));

  LibmeshPetscCall(SNESSolve(snes, NULL, x));
  LibmeshPetscCall(VecGetArray(x, &xx));
  for (unsigned int i = 0; i < _block_size * _n_gaps; i++)
    root(i) = xx[i];

  LibmeshPetscCall(VecRestoreArray(x, &xx));
  LibmeshPetscCall(VecDestroy(&x));
  LibmeshPetscCall(VecDestroy(&r));
  LibmeshPetscCall(SNESDestroy(&snes));
  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

PetscErrorCode
InterWrapper1PhaseProblem::implicitPetscSolve(int iblock)
{
  Vec b_nest, x_nest; /* approx solution, RHS, exact solution */
  Mat A_nest;         /* linear system matrix */
  KSP ksp;            /* linear solver context */
  PC pc;              /* preconditioner context */

  PetscFunctionBegin;
  PetscInt Q = _monolithic_thermal_bool ? 4 : 3;
  std::vector<Mat> mat_array(Q * Q);
  std::vector<Vec> vec_array(Q);

  /// Initializing flags
  bool _axial_mass_flow_tight_coupling = true;
  bool _pressure_axial_momentum_tight_coupling = true;
  bool _pressure_cross_momentum_tight_coupling = true;
  unsigned int first_node = iblock * _block_size + 1;
  unsigned int last_node = (iblock + 1) * _block_size;

  /// Assembling matrices
  // Computing sum of crossflows with previous iteration
  computeSumWij(iblock);
  // Assembling axial flux matrix
  computeMdot(iblock);
  // Computing turbulent crossflow with previous step axial mass flows
  computeWijPrime(iblock);
  // Assembling for Pressure Drop matrix
  computeDP(iblock);
  // Assembling pressure matrix
  computeP(iblock);
  // Assembling cross fluxes matrix
  computeWij(iblock);
  // If monolithic solve - Assembling enthalpy matrix
  if (_monolithic_thermal_bool)
    computeh(iblock);

  _console << "Starting nested system." << std::endl;
  _console << "Number of simultaneous variables: " << Q << std::endl;
  // Mass conservation
  PetscInt field_num = 0;
  LibmeshPetscCall(
      MatDuplicate(_mc_axial_convection_mat, MAT_COPY_VALUES, &mat_array[Q * field_num + 0]));
  LibmeshPetscCall(MatAssemblyBegin(mat_array[Q * field_num + 0], MAT_FINAL_ASSEMBLY));
  LibmeshPetscCall(MatAssemblyEnd(mat_array[Q * field_num + 0], MAT_FINAL_ASSEMBLY));
  mat_array[Q * field_num + 1] = NULL;
  if (_axial_mass_flow_tight_coupling)
  {
    LibmeshPetscCall(MatDuplicate(_mc_sumWij_mat, MAT_COPY_VALUES, &mat_array[Q * field_num + 2]));
    LibmeshPetscCall(MatAssemblyBegin(mat_array[Q * field_num + 2], MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(mat_array[Q * field_num + 2], MAT_FINAL_ASSEMBLY));
  }
  else
  {
    mat_array[Q * field_num + 2] = NULL;
  }
  if (_monolithic_thermal_bool)
  {
    mat_array[Q * field_num + 3] = NULL;
  }
  LibmeshPetscCall(VecDuplicate(_mc_axial_convection_rhs, &vec_array[field_num]));
  LibmeshPetscCall(VecCopy(_mc_axial_convection_rhs, vec_array[field_num]));
  if (!_axial_mass_flow_tight_coupling)
  {
    Vec sumWij_loc;
    LibmeshPetscCall(VecDuplicate(_mc_axial_convection_rhs, &sumWij_loc));
    LibmeshPetscCall(VecSet(sumWij_loc, 0.0));
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

        PetscScalar value_vec_2 = -1.0 * (*_SumWij_soln)(node_out);
        PetscInt row_vec_2 = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(VecSetValues(sumWij_loc, 1, &row_vec_2, &value_vec_2, ADD_VALUES));
      }
    }
    LibmeshPetscCall(VecAXPY(vec_array[field_num], 1.0, sumWij_loc));
    LibmeshPetscCall(VecDestroy(&sumWij_loc));
  }

  _console << "Mass ok." << std::endl;
  // Axial momentum conservation
  field_num = 1;
  if (_pressure_axial_momentum_tight_coupling)
  {
    LibmeshPetscCall(
        MatDuplicate(_amc_sys_mdot_mat, MAT_COPY_VALUES, &mat_array[Q * field_num + 0]));
    LibmeshPetscCall(MatAssemblyBegin(mat_array[Q * field_num + 0], MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(mat_array[Q * field_num + 0], MAT_FINAL_ASSEMBLY));
  }
  else
  {
    mat_array[Q * field_num + 0] = NULL;
  }
  LibmeshPetscCall(
      MatDuplicate(_amc_pressure_force_mat, MAT_COPY_VALUES, &mat_array[Q * field_num + 1]));
  LibmeshPetscCall(MatAssemblyBegin(mat_array[Q * field_num + 1], MAT_FINAL_ASSEMBLY));
  LibmeshPetscCall(MatAssemblyEnd(mat_array[Q * field_num + 1], MAT_FINAL_ASSEMBLY));
  mat_array[Q * field_num + 2] = NULL;
  if (_monolithic_thermal_bool)
  {
    mat_array[Q * field_num + 3] = NULL;
  }
  LibmeshPetscCall(VecDuplicate(_amc_pressure_force_rhs, &vec_array[field_num]));
  LibmeshPetscCall(VecCopy(_amc_pressure_force_rhs, vec_array[field_num]));
  if (_pressure_axial_momentum_tight_coupling)
  {
    LibmeshPetscCall(VecAXPY(vec_array[field_num], 1.0, _amc_sys_mdot_rhs));
  }
  else
  {
    unsigned int last_node = (iblock + 1) * _block_size;
    unsigned int first_node = iblock * _block_size + 1;
    LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
        _prod, *_mdot_soln, first_node, last_node, _n_channels));
    Vec ls;
    LibmeshPetscCall(VecDuplicate(_amc_sys_mdot_rhs, &ls));
    LibmeshPetscCall(MatMult(_amc_sys_mdot_mat, _prod, ls));
    LibmeshPetscCall(VecAXPY(ls, -1.0, _amc_sys_mdot_rhs));
    LibmeshPetscCall(VecAXPY(vec_array[field_num], -1.0, ls));
    LibmeshPetscCall(VecDestroy(&ls));
  }

  _console << "Lin mom OK." << std::endl;

  // Cross momentum conservation
  field_num = 2;
  mat_array[Q * field_num + 0] = NULL;
  if (_pressure_cross_momentum_tight_coupling)
  {
    LibmeshPetscCall(
        MatDuplicate(_cmc_pressure_force_mat, MAT_COPY_VALUES, &mat_array[Q * field_num + 1]));
    LibmeshPetscCall(MatAssemblyBegin(mat_array[Q * field_num + 1], MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(mat_array[Q * field_num + 1], MAT_FINAL_ASSEMBLY));
  }
  else
  {
    mat_array[Q * field_num + 1] = NULL;
  }
  //  mat_array[Q*field_num+1] = NULL;

  LibmeshPetscCall(MatDuplicate(_cmc_sys_Wij_mat, MAT_COPY_VALUES, &mat_array[Q * field_num + 2]));
  LibmeshPetscCall(MatAssemblyBegin(mat_array[Q * field_num + 2], MAT_FINAL_ASSEMBLY));
  LibmeshPetscCall(MatAssemblyEnd(mat_array[Q * field_num + 2], MAT_FINAL_ASSEMBLY));
  if (_monolithic_thermal_bool)
  {
    mat_array[Q * field_num + 3] = NULL;
  }

  LibmeshPetscCall(VecDuplicate(_cmc_sys_Wij_rhs, &vec_array[field_num]));
  LibmeshPetscCall(VecCopy(_cmc_sys_Wij_rhs, vec_array[field_num]));
  if (_pressure_cross_momentum_tight_coupling)
  {
    LibmeshPetscCall(VecAXPY(vec_array[field_num], 1.0, _cmc_pressure_force_rhs));
  }
  else
  {
    Vec sol_holder_P;
    LibmeshPetscCall(createPetscVector(sol_holder_P, _block_size * _n_gaps));
    LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
        _prodp, *_P_soln, iblock * _block_size, (iblock + 1) * _block_size - 1, _n_channels));

    LibmeshPetscCall(MatMult(_cmc_pressure_force_mat, _prodp, sol_holder_P));
    LibmeshPetscCall(VecAXPY(sol_holder_P, -1.0, _cmc_pressure_force_rhs));
    LibmeshPetscCall(VecScale(sol_holder_P, 1.0));
    LibmeshPetscCall(VecAXPY(vec_array[field_num], 1.0, sol_holder_P));
    LibmeshPetscCall(VecDestroy(&sol_holder_P));
  }

  _console << "Cross mom ok." << std::endl;

  // Energy conservation
  if (_monolithic_thermal_bool)
  {
    field_num = 3;
    mat_array[Q * field_num + 0] = NULL;
    mat_array[Q * field_num + 1] = NULL;
    mat_array[Q * field_num + 2] = NULL;
    LibmeshPetscCall(MatDuplicate(_hc_sys_h_mat, MAT_COPY_VALUES, &mat_array[Q * field_num + 3]));
    LibmeshPetscCall(MatAssemblyBegin(mat_array[Q * field_num + 3], MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(MatAssemblyEnd(mat_array[Q * field_num + 3], MAT_FINAL_ASSEMBLY));
    LibmeshPetscCall(VecDuplicate(_hc_sys_h_rhs, &vec_array[field_num]));
    LibmeshPetscCall(VecCopy(_hc_sys_h_rhs, vec_array[field_num]));
  }
  _console << "Energy ok." << std::endl;

  // Relaxing linear system
  // Weaker relaxation
  if (true)
  {
    // Estimating cross-flow resistances to achieve realizable solves
    LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
        _prod, *_mdot_soln, first_node, last_node, _n_channels));
    Vec mdot_estimate;
    LibmeshPetscCall(createPetscVector(mdot_estimate, _block_size * _n_channels));
    Vec pmat_diag;
    LibmeshPetscCall(createPetscVector(pmat_diag, _block_size * _n_channels));
    Vec p_estimate;
    LibmeshPetscCall(createPetscVector(p_estimate, _block_size * _n_channels));
    Vec unity_vec;
    LibmeshPetscCall(createPetscVector(unity_vec, _block_size * _n_channels));
    LibmeshPetscCall(VecSet(unity_vec, 1.0));
    Vec sol_holder_P;
    LibmeshPetscCall(createPetscVector(sol_holder_P, _block_size * _n_gaps));
    Vec diag_Wij_loc;
    LibmeshPetscCall(createPetscVector(diag_Wij_loc, _block_size * _n_gaps));
    Vec Wij_estimate;
    LibmeshPetscCall(createPetscVector(Wij_estimate, _block_size * _n_gaps));
    Vec unity_vec_Wij;
    LibmeshPetscCall(createPetscVector(unity_vec_Wij, _block_size * _n_gaps));
    LibmeshPetscCall(VecSet(unity_vec_Wij, 1.0));
    Vec _Wij_loc_vec;
    LibmeshPetscCall(createPetscVector(_Wij_loc_vec, _block_size * _n_gaps));
    Vec _Wij_old_loc_vec;
    LibmeshPetscCall(createPetscVector(_Wij_old_loc_vec, _block_size * _n_gaps));
    LibmeshPetscCall(MatMult(mat_array[Q], _prod, mdot_estimate));
    LibmeshPetscCall(MatGetDiagonal(mat_array[Q + 1], pmat_diag));
    LibmeshPetscCall(VecAXPY(pmat_diag, 1e-10, unity_vec));
    LibmeshPetscCall(VecPointwiseDivide(p_estimate, mdot_estimate, pmat_diag));
    LibmeshPetscCall(MatMult(mat_array[2 * Q + 1], p_estimate, sol_holder_P));
    LibmeshPetscCall(VecAXPY(sol_holder_P, -1.0, _cmc_pressure_force_rhs));
    LibmeshPetscCall(MatGetDiagonal(mat_array[2 * Q + 2], diag_Wij_loc));
    LibmeshPetscCall(VecAXPY(diag_Wij_loc, 1e-10, unity_vec_Wij));
    LibmeshPetscCall(VecPointwiseDivide(Wij_estimate, sol_holder_P, diag_Wij_loc));
    Vec sumWij_loc;
    LibmeshPetscCall(createPetscVector(sumWij_loc, _block_size * _n_channels));
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        PetscScalar sumWij = 0.0;
        unsigned int counter = 0;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapChannels(i_gap);
          unsigned int i_ch_loc = chans.first;
          PetscInt row_vec = i_ch_loc + _n_channels * iz_ind;
          PetscScalar loc_Wij_value;
          LibmeshPetscCall(VecGetValues(sol_holder_P, 1, &row_vec, &loc_Wij_value));
          sumWij += _subchannel_mesh.getCrossflowSign(i_ch, counter) * loc_Wij_value;
          counter++;
        }
        PetscInt row_vec = i_ch + _n_channels * iz_ind;
        LibmeshPetscCall(VecSetValues(sumWij_loc, 1, &row_vec, &sumWij, INSERT_VALUES));
      }
    }

    PetscScalar min_mdot;
    LibmeshPetscCall(VecAbs(_prod));
    LibmeshPetscCall(VecMin(_prod, NULL, &min_mdot));
    _console << "Minimum estimated mdot: " << min_mdot << std::endl;

    LibmeshPetscCall(VecAbs(sumWij_loc));
    LibmeshPetscCall(VecMax(sumWij_loc, NULL, &_max_sumWij));
    _max_sumWij = std::max(1e-10, _max_sumWij);
    _console << "Maximum estimated Wij: " << _max_sumWij << std::endl;

    LibmeshPetscCall(populateVectorFromDense<libMesh::DenseMatrix<Real>>(
        _Wij_loc_vec, _Wij, first_node, last_node, _n_gaps));
    LibmeshPetscCall(VecAbs(_Wij_loc_vec));
    LibmeshPetscCall(populateVectorFromDense<libMesh::DenseMatrix<Real>>(
        _Wij_old_loc_vec, _Wij_old, first_node, last_node, _n_gaps));
    LibmeshPetscCall(VecAbs(_Wij_old_loc_vec));
    LibmeshPetscCall(VecAXPY(_Wij_loc_vec, -1.0, _Wij_old_loc_vec));
    PetscScalar relax_factor;
    LibmeshPetscCall(VecAbs(_Wij_loc_vec));
#if !PETSC_VERSION_LESS_THAN(3, 16, 0)
    LibmeshPetscCall(VecMean(_Wij_loc_vec, &relax_factor));
#else
    LibmeshPetscCall(VecSum(_Wij_loc_vec, &relax_factor));
    relax_factor /= _block_size * _n_gaps;
#endif
    relax_factor = relax_factor / _max_sumWij + 0.5;
    _console << "Relax base value: " << relax_factor << std::endl;

    PetscScalar resistance_relaxation = 0.9;
    _added_K = _max_sumWij / min_mdot;
    _console << "New cross resistance: " << _added_K << std::endl;
    _added_K = (_added_K * resistance_relaxation + (1.0 - resistance_relaxation) * _added_K_old) *
               relax_factor;
    _console << "Relaxed cross resistance: " << _added_K << std::endl;
    if (_added_K < 10 && _added_K >= 1.0)
      _added_K = 1.0; //(1.0 - resistance_relaxation);
    if (_added_K < 1.0 && _added_K >= 0.1)
      _added_K = 0.5;
    if (_added_K < 0.1 && _added_K >= 0.01)
      _added_K = 1. / 3.;
    if (_added_K < 1e-2 && _added_K >= 1e-3)
      _added_K = 0.1;
    if (_added_K < 1e-3)
      _added_K = 1.0 * _added_K;
    _console << "Actual added cross resistance: " << _added_K << std::endl;
    LibmeshPetscCall(VecScale(unity_vec_Wij, _added_K));
    _added_K_old = _added_K;

    // Adding cross resistances
    LibmeshPetscCall(MatDiagonalSet(mat_array[2 * Q + 2], unity_vec_Wij, ADD_VALUES));
    LibmeshPetscCall(VecDestroy(&mdot_estimate));
    LibmeshPetscCall(VecDestroy(&pmat_diag));
    LibmeshPetscCall(VecDestroy(&unity_vec));
    LibmeshPetscCall(VecDestroy(&p_estimate));
    LibmeshPetscCall(VecDestroy(&sol_holder_P));
    LibmeshPetscCall(VecDestroy(&diag_Wij_loc));
    LibmeshPetscCall(VecDestroy(&unity_vec_Wij));
    LibmeshPetscCall(VecDestroy(&Wij_estimate));
    LibmeshPetscCall(VecDestroy(&sumWij_loc));
    LibmeshPetscCall(VecDestroy(&_Wij_loc_vec));
    LibmeshPetscCall(VecDestroy(&_Wij_old_loc_vec));

    // Auto-computing relaxation factors
    PetscScalar relaxation_factor_mdot, relaxation_factor_P, relaxation_factor_Wij;
    relaxation_factor_mdot = 1.0;
    relaxation_factor_P = 1.0; // std::exp(-5.0);
    relaxation_factor_Wij = 0.1;

    _console << "Relax mdot: " << relaxation_factor_mdot << std::endl;
    _console << "Relax P: " << relaxation_factor_P << std::endl;
    _console << "Relax Wij: " << relaxation_factor_Wij << std::endl;

    PetscInt field_num = 0;
    Vec diag_mdot;
    LibmeshPetscCall(VecDuplicate(vec_array[field_num], &diag_mdot));
    LibmeshPetscCall(MatGetDiagonal(mat_array[Q * field_num + field_num], diag_mdot));
    LibmeshPetscCall(VecScale(diag_mdot, 1.0 / relaxation_factor_mdot));
    LibmeshPetscCall(
        MatDiagonalSet(mat_array[Q * field_num + field_num], diag_mdot, INSERT_VALUES));
    LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
        _prod, *_mdot_soln, first_node, last_node, _n_channels));
    LibmeshPetscCall(VecScale(diag_mdot, (1.0 - relaxation_factor_mdot)));
    LibmeshPetscCall(VecPointwiseMult(_prod, _prod, diag_mdot));
    LibmeshPetscCall(VecAXPY(vec_array[field_num], 1.0, _prod));
    LibmeshPetscCall(VecDestroy(&diag_mdot));

    _console << "mdot relaxed" << std::endl;

    field_num = 1;
    Vec diag_P;
    LibmeshPetscCall(VecDuplicate(vec_array[field_num], &diag_P));
    LibmeshPetscCall(MatGetDiagonal(mat_array[Q * field_num + field_num], diag_P));
    LibmeshPetscCall(VecScale(diag_P, 1.0 / relaxation_factor_P));
    LibmeshPetscCall(MatDiagonalSet(mat_array[Q * field_num + field_num], diag_P, INSERT_VALUES));
    _console << "Mat assembled" << std::endl;
    LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
        _prod, *_P_soln, first_node, last_node, _n_channels));
    LibmeshPetscCall(VecScale(diag_P, (1.0 - relaxation_factor_P)));
    LibmeshPetscCall(VecPointwiseMult(_prod, _prod, diag_P));
    LibmeshPetscCall(VecAXPY(vec_array[field_num], 1.0, _prod));
    LibmeshPetscCall(VecDestroy(&diag_P));

    _console << "P relaxed" << std::endl;

    field_num = 2;
    Vec diag_Wij;
    LibmeshPetscCall(VecDuplicate(vec_array[field_num], &diag_Wij));
    LibmeshPetscCall(MatGetDiagonal(mat_array[Q * field_num + field_num], diag_Wij));
    LibmeshPetscCall(VecScale(diag_Wij, 1.0 / relaxation_factor_Wij));
    LibmeshPetscCall(MatDiagonalSet(mat_array[Q * field_num + field_num], diag_Wij, INSERT_VALUES));
    LibmeshPetscCall(populateVectorFromDense<libMesh::DenseMatrix<Real>>(
        _Wij_vec, _Wij, first_node, last_node, _n_gaps));
    LibmeshPetscCall(VecScale(diag_Wij, (1.0 - relaxation_factor_Wij)));
    LibmeshPetscCall(VecPointwiseMult(_Wij_vec, _Wij_vec, diag_Wij));
    LibmeshPetscCall(VecAXPY(vec_array[field_num], 1.0, _Wij_vec));
    LibmeshPetscCall(VecDestroy(&diag_Wij));

    _console << "Wij relaxed" << std::endl;
  }
  _console << "Linear solver relaxed." << std::endl;

  // Creating nested matrices
  LibmeshPetscCall(MatCreateNest(PETSC_COMM_WORLD, Q, NULL, Q, NULL, mat_array.data(), &A_nest));
  LibmeshPetscCall(VecCreateNest(PETSC_COMM_WORLD, Q, NULL, vec_array.data(), &b_nest));
  _console << "Nested system created." << std::endl;

  /// Setting up linear solver
  // Creating linear solver
  LibmeshPetscCall(KSPCreate(PETSC_COMM_WORLD, &ksp));
  LibmeshPetscCall(KSPSetType(ksp, KSPFGMRES));
  // Setting KSP operators
  LibmeshPetscCall(KSPSetOperators(ksp, A_nest, A_nest));
  // Set KSP and PC options
  LibmeshPetscCall(KSPGetPC(ksp, &pc));
  LibmeshPetscCall(PCSetType(pc, PCFIELDSPLIT));
  LibmeshPetscCall(KSPSetTolerances(ksp, _rtol, _atol, _dtol, _maxit));
  // Splitting fields
  std::vector<IS> rows(Q);
  // IS rows[Q];
  PetscInt M = 0;
  LibmeshPetscCall(MatNestGetISs(A_nest, rows.data(), NULL));
  for (PetscInt j = 0; j < Q; ++j)
  {
    IS expand1;
    LibmeshPetscCall(ISDuplicate(rows[M], &expand1));
    M += 1;
    LibmeshPetscCall(PCFieldSplitSetIS(pc, NULL, expand1));
    LibmeshPetscCall(ISDestroy(&expand1));
  }
  _console << "Linear solver assembled." << std::endl;

  /// Solving
  LibmeshPetscCall(VecDuplicate(b_nest, &x_nest));
  LibmeshPetscCall(VecSet(x_nest, 0.0));
  LibmeshPetscCall(KSPSolve(ksp, b_nest, x_nest));

  /// Destroying solver elements
  LibmeshPetscCall(VecDestroy(&b_nest));
  LibmeshPetscCall(MatDestroy(&A_nest));
  LibmeshPetscCall(KSPDestroy(&ksp));
  for (PetscInt i = 0; i < Q * Q; i++)
  {
    LibmeshPetscCall(MatDestroy(&mat_array[i]));
  }
  for (PetscInt i = 0; i < Q; i++)
  {
    LibmeshPetscCall(VecDestroy(&vec_array[i]));
  }
  _console << "Solver elements destroyed." << std::endl;

  /// Recovering the solutions
  Vec sol_mdot, sol_p, sol_Wij;
  _console << "Vectors created." << std::endl;
  PetscInt num_vecs;
  Vec * loc_vecs;
  LibmeshPetscCall(VecNestGetSubVecs(x_nest, &num_vecs, &loc_vecs));
  _console << "Starting extraction." << std::endl;
  LibmeshPetscCall(VecDuplicate(_mc_axial_convection_rhs, &sol_mdot));
  LibmeshPetscCall(VecCopy(loc_vecs[0], sol_mdot));
  LibmeshPetscCall(VecDuplicate(_amc_sys_mdot_rhs, &sol_p));
  LibmeshPetscCall(VecCopy(loc_vecs[1], sol_p));
  LibmeshPetscCall(VecDuplicate(_cmc_sys_Wij_rhs, &sol_Wij));
  LibmeshPetscCall(VecCopy(loc_vecs[2], sol_Wij));
  _console << "Getting individual field solutions from coupled solver." << std::endl;

  /// Assigning the solutions to arrays
  PetscScalar * sol_p_array;
  LibmeshPetscCall(VecGetArray(sol_p, &sol_p_array));
  PetscScalar * sol_Wij_array;
  LibmeshPetscCall(VecGetArray(sol_Wij, &sol_Wij_array));

  /// Populating Mass flow
  LibmeshPetscCall(populateSolutionChan<SolutionHandle>(
      sol_mdot, *_mdot_soln, first_node, last_node, _n_channels));

  /// Populating Pressure
  for (unsigned int iz = last_node; iz > first_node - 1; iz--)
  {
    auto iz_ind = iz - first_node;
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      PetscScalar value = sol_p_array[iz_ind * _n_channels + i_ch];
      _P_soln->set(node_in, value);
    }
  }

  /// Populating Crossflow
  LibmeshPetscCall(populateDenseFromVector<libMesh::DenseMatrix<Real>>(
      sol_Wij, _Wij, first_node, last_node - 1, _n_gaps));

  /// Populating Enthalpy
  if (_monolithic_thermal_bool)
  {
    Vec sol_h;
    LibmeshPetscCall(VecDuplicate(_hc_sys_h_rhs, &sol_h));
    LibmeshPetscCall(VecCopy(loc_vecs[3], sol_h));
    PetscScalar * sol_h_array;
    LibmeshPetscCall(VecGetArray(sol_h, &sol_h_array));
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
        PetscScalar value = sol_h_array[iz_ind * _n_channels + i_ch];
        _h_soln->set(node_out, value);

        if (value < 0)
        {
          mooseError(name(),
                     " : Calculation of negative Enthalpy h_out = : ",
                     value,
                     " Axial Level= : ",
                     iz);
        }
      }
    }
    LibmeshPetscCall(VecDestroy(&sol_h));
  }

  /// Populating sum_Wij
  LibmeshPetscCall(MatMult(_mc_sumWij_mat, sol_Wij, _prod));
  LibmeshPetscCall(populateSolutionChan<SolutionHandle>(
      _prod, *_SumWij_soln, first_node, last_node, _n_channels));

  Vec sumWij_loc;
  LibmeshPetscCall(createPetscVector(sumWij_loc, _block_size * _n_channels));
  LibmeshPetscCall(populateVectorFromHandle<SolutionHandle>(
      _prod, *_SumWij_soln, first_node, last_node, _n_channels));

  LibmeshPetscCall(VecAbs(_prod));
  LibmeshPetscCall(VecMax(_prod, NULL, &_max_sumWij_new));
  _console << "Maximum estimated Wij new: " << _max_sumWij_new << std::endl;
  _correction_factor = _max_sumWij_new / _max_sumWij;
  _console << "Correction factor: " << _correction_factor << std::endl;
  _console << "Solutions assigned to MOOSE variables." << std::endl;

  /// Destroying arrays
  LibmeshPetscCall(VecDestroy(&x_nest));
  LibmeshPetscCall(VecDestroy(&sol_mdot));
  LibmeshPetscCall(VecDestroy(&sol_p));
  LibmeshPetscCall(VecDestroy(&sol_Wij));
  LibmeshPetscCall(VecDestroy(&sumWij_loc));
  _console << "Solutions destroyed." << std::endl;

  PetscFunctionReturn(LIBMESH_PETSC_SUCCESS);
}

void
InterWrapper1PhaseProblem::initializeSolution()
{
  unsigned int last_node = _n_cells;
  unsigned int first_node = 1;
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz - 1);
      _mdot_soln->set(node_out, (*_mdot_soln)(node_in));
    }
  }
  _mdot_soln->close();
}

void
InterWrapper1PhaseProblem::externalSolve()
{
  _console << "Executing subchannel solver\n";
  initializeSolution();
  _console << "Solution initialized" << std::endl;
  Real P_error = 1.0;
  unsigned int P_it = 0;
  unsigned int P_it_max;

  if (_segregated_bool)
    P_it_max = 20 * _n_blocks;
  else
    P_it_max = 100;

  if ((_n_blocks == 1) && (_segregated_bool))
    P_it_max = 5;

  while ((P_error > _P_tol && P_it < P_it_max))
  {
    P_it += 1;
    if (P_it == P_it_max && _n_blocks != 1)
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
      Real T_block_error = 1.0;
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

        if (_segregated_bool)
        {
          computeWijFromSolve(iblock);

          if (_compute_power)
          {
            computeh(iblock);
            _console << "Done with h solve" << std::endl;
            computeT(iblock);
            _console << "Done with T solve" << std::endl;
          }
        }
        else
        {
          LibmeshPetscCall(implicitPetscSolve(iblock));
          computeWijPrime(iblock);

          _console << "Done with main solve." << std::endl;
          if (_monolithic_thermal_bool)
          {
            // Enthalpy is already solved from the monolithic solve
            computeT(iblock);
          }
          else
          {
            _console << "No llores mas aqui estoy 2." << std::endl;
            if (_compute_power)
            {
              computeh(iblock);
              computeT(iblock);
            }
            _console << "Done with thermal solve." << std::endl;
          }
        }

        if (_compute_density)
          computeRho(iblock);

        if (_compute_viscosity)
          computeMu(iblock);

        _console << "Done updating thermophysical properties." << std::endl;

        // We must do a global assembly to make sure data is parallel consistent before we do things
        // like compute L2 norms
        _aux->solution().close();

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
        Real sumTemp = 0.0;
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

          auto perimeter = 2.0 * (_subchannel_mesh.getSideX() + _subchannel_mesh.getSideY());
          sumTemp += (*_q_prime_soln)(pin_node) / (perimeter * hw) + (*_T_soln)(node);
        }
        _Tpin_soln->set(pin_node, 0.25 * sumTemp);
      }
    }
  }
  _aux->solution().close();
  _aux->update();
}

void
InterWrapper1PhaseProblem::syncSolutions(Direction /*direction*/)
{
}
