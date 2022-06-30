#include "SubChannel1PhaseProblem.h"
#include "SystemBase.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
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
  for (int i = 0; i < size; i++)
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
  params.addRequiredParam<unsigned int>("n_blocks", "The number of blocks in the axial direction");
  params.addRequiredParam<Real>("beta",
                                "Thermal diffusion coefficient used in turbulent crossflow");
  params.addRequiredParam<Real>("CT", "Turbulent modeling parameter");
  params.addParam<Real>("P_tol", 1e-6, "Pressure tolerance");
  params.addParam<Real>("T_tol", 1e-6, "Temperature tolerance");
  params.addParam<int>("T_maxit", 1000, "Maximum number of iterations for inner temperature loop");
  params.addParam<PetscReal>("rtol", 1e-6, "Relative tolerance for ksp solver");
  params.addParam<PetscReal>("atol", 1e-6, "Absolute tolerance for ksp solver");
  params.addParam<PetscReal>("dtol", 1e5, "Divergence tolerance or ksp solver");
  params.addParam<PetscInt>("maxit", 1e4, "Maximum number of iterations for ksp solver");
  params.addParam<std::string>("interpolation_scheme", "central_difference", "Interpolation scheme used for the method.");
  params.addParam<bool>("implicit", false, "Boolean to define the use of explicit or implicit solution.");
  params.addParam<bool>("staggered_pressure", false, "Boolean to define the use of explicit or implicit solution.");
  params.addParam<bool>("segregated", true, "Boolean to define whether to use a segregated solution.");
  params.addParam<bool>("monolithic_thermal", false, "Boolean to define whether to use thermal monolithic solve.");
  params.addParam<bool>("verbose_subchannel", false, "Boolean to print out information related to subchannel solve.");
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
    _subchannel_mesh(dynamic_cast<SubChannelMesh &>(_mesh)),
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
    _duct_mesh_exist(_subchannel_mesh.ductMeshExist()),
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
    _verbose_subchannel(getParam<bool>("verbose_subchannel")),
    _fp(nullptr),
    _Tpin_soln(nullptr),
    _q_prime_duct_soln(nullptr),
    _Tduct_soln(nullptr)
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
  _Wij_old.resize(_n_gaps, _n_cells + 1); _Wij_old.zero();
  _WijPrime.resize(_n_gaps, _n_cells + 1); _WijPrime.zero();
  _Wij_residual_matrix.resize(_n_gaps, _block_size); _Wij_residual_matrix.zero();
  _converged = true;

  // Mass conservation components
  createPetscMatrix(mc_sumWij_mat, _block_size * _n_channels, _block_size * _n_gaps);
  createPetscVector(Wij_vec, _block_size * _n_gaps);
  createPetscVector(prod, _block_size * _n_channels);
  createPetscVector(prodp, _block_size * _n_channels);
  createPetscMatrix(mc_axial_convection_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(mc_axial_convection_rhs, _block_size * _n_channels);

  // Axial momentum conservation components
  createPetscMatrix(amc_turbulent_cross_flows_mat, _block_size * _n_gaps, _block_size * _n_channels);
  createPetscVector(amc_turbulent_cross_flows_rhs, _block_size * _n_gaps);
  createPetscMatrix(amc_time_derivative_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(amc_time_derivative_rhs, _block_size * _n_channels);
  createPetscMatrix(amc_advective_derivative_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(amc_advective_derivative_rhs, _block_size * _n_channels);
  createPetscMatrix(amc_cross_derivative_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(amc_cross_derivative_rhs, _block_size * _n_channels);
  createPetscMatrix(amc_friction_force_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(amc_friction_force_rhs, _block_size * _n_channels);
  createPetscVector(amc_gravity_rhs, _block_size * _n_channels);
  createPetscMatrix(amc_pressure_force_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(amc_pressure_force_rhs, _block_size * _n_channels);
  createPetscMatrix(amc_sys_mdot_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(amc_sys_mdot_rhs, _block_size * _n_channels);

  // Lateral momentum conservation components
  createPetscMatrix(cmc_time_derivative_mat, _block_size * _n_gaps, _block_size * _n_gaps);
  createPetscVector(cmc_time_derivative_rhs, _block_size * _n_gaps);
  createPetscMatrix(cmc_advective_derivative_mat, _block_size * _n_gaps, _block_size * _n_gaps);
  createPetscVector(cmc_advective_derivative_rhs, _block_size * _n_gaps);
  createPetscMatrix(cmc_friction_force_mat, _block_size * _n_gaps, _block_size * _n_gaps);
  createPetscVector(cmc_friction_force_rhs, _block_size * _n_gaps);
  createPetscMatrix(cmc_pressure_force_mat, _block_size * _n_gaps, _block_size * _n_channels);
  createPetscVector(cmc_pressure_force_rhs, _block_size * _n_gaps);
  createPetscMatrix(cmc_sys_Wij_mat, _block_size * _n_gaps, _block_size * _n_gaps);
  createPetscVector(cmc_sys_Wij_rhs, _block_size * _n_gaps);
  createPetscVector(cmc_Wij_channel_dummy, _block_size * _n_channels);

  // Energy conservation components
  createPetscMatrix(hc_time_derivative_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(hc_time_derivative_rhs, _block_size * _n_channels);
  createPetscMatrix(hc_advective_derivative_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(hc_advective_derivative_rhs, _block_size * _n_channels);
  createPetscMatrix(hc_cross_derivative_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(hc_cross_derivative_rhs, _block_size * _n_channels);
  createPetscVector(hc_added_heat_rhs, _block_size * _n_channels);
  createPetscMatrix(hc_sys_h_mat, _block_size * _n_channels, _block_size * _n_channels);
  createPetscVector(hc_sys_h_rhs, _block_size * _n_channels);
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
  if (_pin_mesh_exist)
    _Tpin_soln = new SolutionHandle(getVariable(0, SubChannelApp::PIN_TEMPERATURE));
  _rho_soln = new SolutionHandle(getVariable(0, SubChannelApp::DENSITY));
  _mu_soln = new SolutionHandle(getVariable(0, SubChannelApp::VISCOSITY));
  _S_flow_soln = new SolutionHandle(getVariable(0, SubChannelApp::SURFACE_AREA));
  _w_perim_soln = new SolutionHandle(getVariable(0, SubChannelApp::WETTED_PERIMETER));
  _q_prime_soln = new SolutionHandle(getVariable(0, SubChannelApp::LINEAR_HEAT_RATE));
  if (_duct_mesh_exist)
  {
    _q_prime_duct_soln = new SolutionHandle(getVariable(0, SubChannelApp::DUCT_LINEAR_HEAT_RATE));
    _Tduct_soln = new SolutionHandle(getVariable(0, SubChannelApp::DUCT_TEMPERATURE));
  }
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
  if (_duct_mesh_exist)
  {
    delete _q_prime_duct_soln;
    delete _Tduct_soln;
  }
  //  MatDestroy(&mc_sumWij_mat);
  //  VecDestroy(&Wij_vec);
  //  VecDestroy(&prod);
  //  KSPDestroy(&mc_axial_convection_ksp);
}

bool
SubChannel1PhaseProblem::converged()
{
  return _converged;
}

PetscScalar
SubChannel1PhaseProblem::computeInterpolationCoefficients(const std::string interp_type,
                                                          PetscScalar Peclet){
  if(interp_type.compare("upwind") != 0){
    return 1.0;
  } else if(interp_type.compare("downwind") != 0){
    return 0.0;
  } else if (interp_type.compare("central_difference") != 0){
    return 0.5;
  }
  else {
    return ((Peclet-1.0)*std::exp(Peclet) + 1) / (Peclet*(std::exp(Peclet)-1.)+1e-10);
  }
}

PetscScalar
SubChannel1PhaseProblem::computeInterpolatedValue(PetscScalar topValue,
                                                  PetscScalar botValue,
                                                  const std::string interp_type,
                                                  PetscScalar Peclet=0.5){
  PetscScalar alpha = computeInterpolationCoefficients(interp_type, Peclet);
  return alpha * botValue + (1.0 - alpha) * topValue;
}

PetscErrorCode
SubChannel1PhaseProblem::createPetscVector(Vec & v, PetscInt n)
{
  PetscErrorCode ierr;
  ierr = VecCreate(PETSC_COMM_WORLD,&v);CHKERRQ(ierr);
  ierr = PetscObjectSetName((PetscObject) v, "Solution");CHKERRQ(ierr);
  ierr = VecSetSizes(v,PETSC_DECIDE,n);CHKERRQ(ierr);
  ierr = VecSetFromOptions(v);CHKERRQ(ierr);
  ierr = VecZeroEntries(v);CHKERRQ(ierr);
  return 0;
}

PetscErrorCode
SubChannel1PhaseProblem::createPetscMatrix(Mat & M, PetscInt n, PetscInt m)
{
  PetscErrorCode ierr;
  ierr = MatCreate(PETSC_COMM_WORLD,&M);CHKERRQ(ierr);
  ierr = MatSetSizes(M,PETSC_DECIDE,PETSC_DECIDE,n,m);CHKERRQ(ierr);
  ierr = MatSetFromOptions(M);CHKERRQ(ierr);
  ierr = MatSetUp(M);CHKERRQ(ierr);
  return 0;
}

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateVectorFromDense(Vec & x,
                                                 const T & loc_solution,
                                                 const unsigned int first_axial_level,
                                                 const unsigned int last_axial_level,
                                                 const unsigned int cross_dimension)
{
  PetscErrorCode ierr;
  PetscScalar * xx;
  ierr = VecGetArray(x, &xx); CHKERRQ(ierr);
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      xx[iz_ind*cross_dimension + i_l] = loc_solution(i_l, iz); //loc_solution(iz_ind*cross_dimension + i_l);
    }
  }
  ierr = VecRestoreArray(x, &xx); CHKERRQ(ierr);
  return 0;
}

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateVectorFromHandle(Vec & x,
                                                  const T & loc_solution,
                                                  const unsigned int first_axial_level,
                                                  const unsigned int last_axial_level,
                                                  const unsigned int cross_dimension)
{
  PetscErrorCode ierr;
  PetscScalar * xx;
  ierr = VecGetArray(x, &xx); CHKERRQ(ierr);
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      auto * loc_node = _subchannel_mesh.getChannelNode(i_l, iz);
      xx[iz_ind*cross_dimension + i_l] = (*loc_solution)(loc_node);
    }
  }
  ierr = VecRestoreArray(x, &xx); CHKERRQ(ierr);
  return 0;
}

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateSolutionChan(const Vec & x,
                                              T & loc_solution,
                                              const unsigned int first_axial_level,
                                              const unsigned int last_axial_level,
                                              const unsigned int cross_dimension)
{
  PetscErrorCode ierr;
  PetscScalar * xx;
  ierr = VecGetArray(x, &xx); CHKERRQ(ierr);
  Node * loc_node;
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      loc_node = _subchannel_mesh.getChannelNode(i_l, iz);
      loc_solution->set(loc_node, xx[iz_ind*cross_dimension + i_l]);
    }
  }
  return 0;
}

template <class T>
PetscErrorCode
SubChannel1PhaseProblem::populateSolutionGap(const Vec & x,
                                              T & loc_solution,
                                              const unsigned int first_axial_level,
                                              const unsigned int last_axial_level,
                                              const unsigned int cross_dimension)
{
  PetscErrorCode ierr;
  PetscScalar * xx;
  ierr = VecGetArray(x, &xx); CHKERRQ(ierr);
  for (unsigned int iz = first_axial_level; iz < last_axial_level + 1; iz++)
  {
    unsigned int iz_ind = iz - first_axial_level;
    for (unsigned int i_l = 0; i_l < cross_dimension; i_l++)
    {
      loc_solution(iz*cross_dimension + i_l) = xx[iz_ind*cross_dimension + i_l];
    }
  }
  return 0;
}

void
SubChannel1PhaseProblem::computeWijFromSolve(int iblock)
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

  // Add to solution vector if explicit
  if (! _implicit_bool)
  {
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
          PetscInt row = i_ch + _n_channels*iz_ind;
          PetscInt col = i_gap + _n_gaps*iz_ind;
          PetscScalar value = 1.0 * _subchannel_mesh.getCrossflowSign(i_ch, counter);
          MatSetValues(mc_sumWij_mat,1,&row,1,&col,&value,INSERT_VALUES);
          counter++;
        }
      }
    }
    MatAssemblyBegin(mc_sumWij_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(mc_sumWij_mat,MAT_FINAL_ASSEMBLY);
    if(_segregated_bool)
    {
      Vec loc_prod; VecDuplicate(amc_sys_mdot_rhs,&loc_prod);
      populateVectorFromDense<libMesh::DenseMatrix<Real>>(Wij_vec, _Wij, first_node, last_node, _n_gaps);
      PetscInt p, q;
      MatGetSize(mc_sumWij_mat, &p, &q);
      MatMult(mc_sumWij_mat, Wij_vec, loc_prod);
      PetscScalar * xx;
      VecGetArray(loc_prod, &xx);
      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        unsigned int iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          PetscScalar value = xx[iz_ind*_n_channels + i_ch];
          _SumWij_soln->set(node_out, value);
        }
      }
      VecDestroy(&loc_prod);
    }
  }
}

void
SubChannel1PhaseProblem::computeMdot(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (! _implicit_bool)
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
        auto mdot_out = (*_mdot_soln)(node_in) - (*_SumWij_soln)(node_out) - time_term;
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
        PetscInt row_vec = i_ch + _n_channels*iz_ind;
        PetscScalar value_vec = -1.0 * time_term;
        VecSetValues(mc_axial_convection_rhs,1,&row_vec,&value_vec,INSERT_VALUES);

        // Imposing bottom boundary condition or adding of diagonal elements
        if (iz == first_node)
        {
          PetscScalar value_vec = (*_mdot_soln)(node_in);
          PetscInt row_vec = i_ch + _n_channels*iz_ind;
          VecSetValues(mc_axial_convection_rhs,1,&row_vec,&value_vec,ADD_VALUES);
        }
        else
        {
          PetscInt row = i_ch + _n_channels*iz_ind;
          PetscInt col = i_ch + _n_channels*(iz_ind-1);
          PetscScalar value = -1.0;
          MatSetValues(mc_axial_convection_mat,1,&row,1,&col,&value,INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row = i_ch + _n_channels*iz_ind;
        PetscInt col = i_ch + _n_channels*iz_ind;
        PetscScalar value = 1.0;
        MatSetValues(mc_axial_convection_mat,1,&row,1,&col,&value,INSERT_VALUES);

        // Adding cross flows RHS
        if (_segregated_bool)
        {
          PetscScalar value_vec_2 = -1.0 * (*_SumWij_soln)(node_out);
          //PetscScalar value_vec_2 = -1.0 * (*_SumWij_soln)(node_in);
          PetscInt row_vec_2 = i_ch + _n_channels*iz_ind;
          VecSetValues(mc_axial_convection_rhs,1,&row_vec_2,&value_vec_2,ADD_VALUES);
        }
      }
    }
    MatAssemblyBegin(mc_axial_convection_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(mc_axial_convection_mat,MAT_FINAL_ASSEMBLY);
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Mass conservation matrix assembled" << std::endl;

    if(_segregated_bool)
    {
      KSP ksploc; PC  pc;
      Vec sol; VecDuplicate(mc_axial_convection_rhs, &sol);
      KSPCreate(PETSC_COMM_WORLD,&ksploc);
      KSPSetOperators(ksploc,mc_axial_convection_mat,mc_axial_convection_mat);
      KSPGetPC(ksploc,&pc); PCSetType(pc,PCJACOBI);
      KSPSetTolerances(ksploc,_rtol, _atol, _dtol, _maxit);
      KSPSetFromOptions(ksploc);
      KSPSolve(ksploc,mc_axial_convection_rhs,sol);
      PetscScalar * xx;
      VecGetArray(sol, &xx);
      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
          PetscScalar value = xx[iz_ind*_n_channels + i_ch];
          _mdot_soln->set(node_out, value);
        }
      }
      VecZeroEntries(mc_axial_convection_rhs);
      KSPDestroy(&ksploc); VecDestroy(&sol);
    }
  }
}

void
SubChannel1PhaseProblem::computeWijPrime(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (! _implicit_bool)
  {
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
  else
  {
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node;
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

        // Base value - I don't want to write it every time
        PetscScalar base_value = _beta * 0.5 * Sij;

        // Bottom values
        if(iz == first_node)
        {
          PetscScalar value_tl = -1.0 * base_value/(Si_in + Sj_in) * ((*_mdot_soln)(node_in_i) + (*_mdot_soln)(node_in_j));
          PetscInt row = i_gap + _n_gaps * iz_ind;
          VecSetValues(amc_turbulent_cross_flows_rhs,1,&row,&value_tl,INSERT_VALUES);
        }
        else
        {
          PetscScalar value_tl = base_value/(Si_in + Sj_in);
          PetscInt row = i_gap + _n_gaps * iz_ind;

          PetscInt col_ich = i_ch + _n_channels * iz_ind;
          MatSetValues(amc_turbulent_cross_flows_mat,1,&row,1,&col_ich,&value_tl,INSERT_VALUES);

          PetscInt col_jch = j_ch + _n_channels * iz_ind;
          MatSetValues(amc_turbulent_cross_flows_mat,1,&row,1,&col_jch,&value_tl,INSERT_VALUES);
        }

        // Top values
        PetscScalar value_bl = base_value/(Si_out + Sj_out);
        PetscInt row = i_gap + _n_gaps * iz_ind;

        PetscInt col_ich = i_ch + _n_channels * (iz_ind+1);
        MatSetValues(amc_turbulent_cross_flows_mat,1,&row,1,&col_ich,&value_bl,INSERT_VALUES);

        PetscInt col_jch = j_ch + _n_channels * (iz_ind+1);
        MatSetValues(amc_turbulent_cross_flows_mat,1,&row,1,&col_jch,&value_bl,INSERT_VALUES);
      }
    }
    MatAssemblyBegin(amc_turbulent_cross_flows_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(amc_turbulent_cross_flows_mat,MAT_FINAL_ASSEMBLY);
    if(_segregated_bool)
    {
      populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
      MatMult(amc_turbulent_cross_flows_mat, prod, Wij_vec);
      PetscScalar * xx;
      VecGetArray(Wij_vec, &xx);
      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
        {
          _WijPrime(i_gap, iz) = xx[iz_ind*_n_gaps + i_gap];
        }
      }
    }
  }
}

void
SubChannel1PhaseProblem::computeDP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (! _implicit_bool)
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
  else
  {
    MatZeroEntries(amc_time_derivative_mat); MatZeroEntries(amc_advective_derivative_mat);
    MatZeroEntries(amc_cross_derivative_mat); MatZeroEntries(amc_friction_force_mat);
    VecZeroEntries(amc_time_derivative_rhs); VecZeroEntries(amc_advective_derivative_rhs);
    VecZeroEntries(amc_cross_derivative_rhs); VecZeroEntries(amc_friction_force_rhs);
    VecZeroEntries(amc_gravity_rhs);
    MatZeroEntries(amc_sys_mdot_mat); VecZeroEntries(amc_sys_mdot_rhs);

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
        auto w_perim_interp = computeInterpolatedValue(w_perim_out, w_perim_in, "central_difference");

        // interpolation weight coefficient
        PetscScalar Pe = 0.5;
        auto alpha = computeInterpolationCoefficients("central_difference", Pe);

        // hydraulic diameter in the i direction
        auto Dh_i = 4.0 * S_interp / w_perim_interp;

        /// Time derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_tt = -1.0 * _TR * alpha * (*_mdot_soln)(node_in) * dz / _dt;
          PetscInt row_vec_tt = i_ch + _n_channels*iz_ind;
          VecSetValues(amc_time_derivative_rhs,1,&row_vec_tt,&value_vec_tt,ADD_VALUES);
        }
        else
        {
          PetscInt row_tt = i_ch + _n_channels*iz_ind;
          PetscInt col_tt = i_ch + _n_channels*(iz_ind-1);
          PetscScalar value_tt = _TR * alpha  * dz / _dt;
          MatSetValues(amc_time_derivative_mat,1,&row_tt,1,&col_tt,&value_tt,INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_tt = i_ch + _n_channels*iz_ind;
        PetscInt col_tt = i_ch + _n_channels*iz_ind;
        PetscScalar value_tt = _TR * (1.0  - alpha)  * dz / _dt;
        MatSetValues(amc_time_derivative_mat,1,&row_tt,1,&col_tt,&value_tt,INSERT_VALUES);

        // Adding RHS elements
        PetscScalar mdot_old_interp = computeInterpolatedValue(_mdot_soln->old(node_out), _mdot_soln->old(node_in),
                                                           "central_difference", Pe);
        PetscScalar value_vec_tt = _TR * mdot_old_interp * dz / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels*iz_ind;
        VecSetValues(amc_time_derivative_rhs,1,&row_vec_tt,&value_vec_tt,ADD_VALUES);

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_at = std::pow((*_mdot_soln)(node_in), 2.0) / (S_in * rho_in);
          PetscInt row_vec_at = i_ch + _n_channels*iz_ind;
          VecSetValues(amc_advective_derivative_rhs,1,&row_vec_at,&value_vec_at,ADD_VALUES);
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels*iz_ind;
          PetscInt col_at = i_ch + _n_channels*(iz_ind-1);
          PetscScalar value_at = -1.0 * (*_mdot_soln)(node_in) / (S_in * rho_in);
          MatSetValues(amc_advective_derivative_mat,1,&row_at,1,&col_at,&value_at,INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_at = i_ch + _n_channels*iz_ind;
        PetscInt col_at = i_ch + _n_channels*iz_ind;
        PetscScalar value_at = (*_mdot_soln)(node_out) / (S_out * rho_out);
        MatSetValues(amc_advective_derivative_mat,1,&row_at,1,&col_at,&value_at,INSERT_VALUES);

        /// Cross derivative term
        unsigned int counter = 0;
        unsigned int cross_index = iz; //iz-1;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
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
              PetscScalar value_vec_ct =
                  -1.0 * alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, cross_index) * u_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              VecSetValues(amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct =  alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, cross_index)
                                     / S_i / rho_i;
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = ii_ch + _n_channels * (iz_ind - 1);
              MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) * _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, cross_index)
                                   / S_i / rho_i;
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = ii_ch + _n_channels * iz_ind;
            MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
          }
          else if (_Wij(i_gap, cross_index) < 0.0) // _Wij=0 operations not necessary
          {
            if (iz == first_node)
            {
              u_star = (*_mdot_soln)(node_in_j) / S_j / rho_j;
              PetscScalar value_vec_ct =
                  -1.0 * alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, cross_index) * u_star;
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              VecSetValues(amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct =  alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, cross_index)
                                     / S_j / rho_j;
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = jj_ch + _n_channels * (iz_ind - 1);
              MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) * _subchannel_mesh.getCrossflowSign(i_ch, counter) * _Wij(i_gap, cross_index)
                                   / S_j / rho_j;
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = jj_ch + _n_channels * iz_ind;
            MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
          }

            if (iz == first_node)
            {
              PetscScalar value_vec_ct =
                  -2.0 * alpha * (*_mdot_soln)(node_in)*_WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
              value_vec_ct =
                  alpha * (*_mdot_soln)(node_in_j)*_WijPrime(i_gap, cross_index) / (rho_j * S_j);
              value_vec_ct +=
                  alpha * (*_mdot_soln)(node_in_i)*_WijPrime(i_gap, cross_index) / (rho_i * S_i);
              PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
              VecSetValues(amc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_center_ct = 2.0 * alpha * _WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = i_ch + _n_channels * (iz_ind - 1);
              MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

              PetscScalar value_left_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index) / (rho_j * S_j);
              row_ct = i_ch + _n_channels * iz_ind;
              col_ct = jj_ch + _n_channels * (iz_ind - 1);
              MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

              PetscScalar value_right_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index) / (rho_i * S_i);
              row_ct = i_ch + _n_channels * iz_ind;
              col_ct = ii_ch + _n_channels * (iz_ind - 1);
              MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);
            }

            PetscScalar value_center_ct = 2.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index) / (rho_interp * S_interp);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = i_ch + _n_channels * iz_ind;
            MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

            PetscScalar value_left_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index) / (rho_j * S_j);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = jj_ch + _n_channels * iz_ind;
            MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

            PetscScalar value_right_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index) / (rho_i * S_i);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = ii_ch + _n_channels * iz_ind;
            MatSetValues(amc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);

          counter++;
        }

        /// Friction term
        PetscScalar mdot_interp = computeInterpolatedValue((*_mdot_soln)(node_out), (*_mdot_soln)(node_in),
                                                           "central_difference", Pe);
        auto Re = ((mdot_interp / S_interp) * Dh_i / mu_interp);
        auto fi = computeFrictionFactor(Re);
        auto ki = computeInterpolatedValue(k_grid[iz ], k_grid[iz - 1], "central_difference", Pe);
        auto coef = (fi * dz / Dh_i + ki) * 0.5 * std::abs((*_mdot_soln)(node_out)) / (S_interp * rho_interp);
        if (iz == first_node)
        {
          PetscScalar value_vec = -1.0 * alpha * coef * (*_mdot_soln)(node_in);
          PetscInt row_vec = i_ch + _n_channels*iz_ind;
          VecSetValues(amc_friction_force_rhs,1,&row_vec,&value_vec,ADD_VALUES);
        }
        else
        {
          PetscInt row = i_ch + _n_channels*iz_ind;
          PetscInt col = i_ch + _n_channels*(iz_ind-1);
          PetscScalar value = alpha * coef;
          MatSetValues(amc_friction_force_mat,1,&row,1,&col,&value,INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row = i_ch + _n_channels*iz_ind;
        PetscInt col = i_ch + _n_channels*iz_ind;
        PetscScalar value = (1.0 - alpha) * coef;
        MatSetValues(amc_friction_force_mat,1,&row,1,&col,&value,INSERT_VALUES);

        /// Gravity force
        PetscScalar value_vec = -1.0 * _g_grav * rho_interp * dz * S_interp;
        PetscInt row_vec = i_ch + _n_channels*iz_ind;
        VecSetValues(amc_gravity_rhs,1,&row_vec,&value_vec,ADD_VALUES);
      }
    }
    /// Assembling system
    MatZeroEntries(amc_sys_mdot_mat);  VecZeroEntries(amc_sys_mdot_rhs);
    MatAssemblyBegin(amc_time_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_time_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(amc_advective_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_advective_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(amc_cross_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_cross_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(amc_friction_force_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_friction_force_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY);
    // Matrix
    MatAXPY(amc_sys_mdot_mat, 1.0, amc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY);
    MatAXPY(amc_sys_mdot_mat, 1.0, amc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY);
    MatAXPY(amc_sys_mdot_mat, 1.0, amc_cross_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY);
    MatAXPY(amc_sys_mdot_mat, 1.0, amc_friction_force_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY);
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Linear momentum conservation matrix assembled" << std::endl;
    // RHS
    VecAXPY(amc_sys_mdot_rhs, 1.0, amc_time_derivative_rhs);
    VecAXPY(amc_sys_mdot_rhs, 1.0, amc_advective_derivative_rhs);
    VecAXPY(amc_sys_mdot_rhs, 1.0, amc_cross_derivative_rhs);
    VecAXPY(amc_sys_mdot_rhs, 1.0, amc_friction_force_rhs);
    VecAXPY(amc_sys_mdot_rhs, 1.0, amc_gravity_rhs);

    if(_segregated_bool)
    {
      // Assembly the matrix system
      populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
      Vec ls; VecDuplicate(amc_sys_mdot_rhs,&ls);
      MatMult(amc_sys_mdot_mat, prod, ls);
      VecAXPY(ls, -1.0, amc_sys_mdot_rhs);
      PetscScalar * xx;
      VecGetArray(ls, &xx);
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
            auto DP =  std::pow(S_interp, 0.0) * xx[iz_ind*_n_channels + i_ch];
            _DP_soln->set(node_out, DP);
          }
          else
          {
            auto DP = 0.0;
            _DP_soln->set(node_out, DP);
          }
        }
      }
      VecDestroy(&ls);
    }
  }
}

void
SubChannel1PhaseProblem::computeP(int iblock)
{
  unsigned int last_node = (iblock + 1) * _block_size;
  unsigned int first_node = iblock * _block_size + 1;

  if (! _implicit_bool)
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
          // We will need to update this later if we allow non-uniform refinements in the axial direction
          PetscScalar Pe = 0.5;
          auto alpha = computeInterpolationCoefficients("central_difference", Pe);
          if(iz == last_node)
          {
            _P_soln->set(node_in, (*_P_soln)(node_out) +(*_DP_soln)(node_out)/2.0);
          }
          else
          {
            _P_soln->set(node_in, (*_P_soln)(node_out) +
                                      (1.0 - alpha)*(*_DP_soln)(node_out) + alpha*(*_DP_soln)(node_in));
          }
        }
      }
    }
  }
  else
  {
    if (!_staggered_pressure_bool)
    {
      VecZeroEntries(amc_pressure_force_rhs);
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
          PetscInt row = i_ch + _n_channels*iz_ind;
          PetscInt col = i_ch + _n_channels*iz_ind;
          PetscScalar value = -1.0*S_interp;
          MatSetValues(amc_pressure_force_mat,1,&row,1,&col,&value,INSERT_VALUES);

          if (iz == last_node)
          {
            PetscScalar value = -1.0 * (*_P_soln)(node_out) * S_interp;
            PetscInt row = i_ch + _n_channels*iz_ind;
            VecSetValues(amc_pressure_force_rhs,1,&row,&value,ADD_VALUES);
          }
          else
          {
            PetscInt row = i_ch + _n_channels*iz_ind;
            PetscInt col = i_ch + _n_channels*(iz_ind+1);
            PetscScalar value = 1.0 * S_interp;
            MatSetValues(amc_pressure_force_mat,1,&row,1,&col,&value,INSERT_VALUES);
          }

          if(_segregated_bool)
          {
            auto dp_out = (*_DP_soln)(node_out);
            PetscScalar value_v = -1.0 * dp_out;
            PetscInt row_v = i_ch + _n_channels*iz_ind;
            VecSetValues(amc_pressure_force_rhs,1,&row_v,&value_v,ADD_VALUES);
          }

        }
      }
      // Solving pressure problem
      MatAssemblyBegin(amc_pressure_force_mat,MAT_FINAL_ASSEMBLY);
      MatAssemblyEnd(amc_pressure_force_mat,MAT_FINAL_ASSEMBLY);

      if(_segregated_bool)
      {
        KSP ksploc; PC  pc;
        Vec sol; VecDuplicate(amc_pressure_force_rhs, &sol);
        KSPCreate(PETSC_COMM_WORLD,&ksploc);
        KSPSetOperators(ksploc,amc_pressure_force_mat,amc_pressure_force_mat);
        KSPGetPC(ksploc,&pc); PCSetType(pc,PCJACOBI);
        KSPSetTolerances(ksploc,_rtol, _atol, _dtol, _maxit);
        KSPSetFromOptions(ksploc);
        KSPSolve(ksploc,amc_pressure_force_rhs,sol);
        PetscScalar * xx;
        VecGetArray(sol, &xx);
        // update Pressure solution
        for (unsigned int iz = last_node; iz > first_node - 1; iz--)
        {
          auto iz_ind = iz - first_node;
          for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
          {
            auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz-1);
            PetscScalar value = xx[iz_ind*_n_channels + i_ch];
            _P_soln->set(node_in, value);
          }
        }
        VecZeroEntries(amc_pressure_force_rhs);
        KSPDestroy(&ksploc); VecDestroy(&sol);
      }
    }
    else
    {
      VecZeroEntries(amc_pressure_force_rhs);
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
          PetscInt row = i_ch + _n_channels*iz_ind;
          PetscInt col = i_ch + _n_channels*iz_ind;
          PetscScalar value = -1.0*S_interp;
          MatSetValues(amc_pressure_force_mat,1,&row,1,&col,&value,INSERT_VALUES);

          if (iz == last_node)
          {
            PetscScalar value = -1.0 * (*_P_soln)(node_out)*S_interp;
            PetscInt row = i_ch + _n_channels*iz_ind;
            VecSetValues(amc_pressure_force_rhs,1,&row,&value,ADD_VALUES);

            auto dp_out = (*_DP_soln)(node_out);
            PetscScalar value_v = -1.0 * dp_out / 2.0 * S_interp;
            PetscInt row_v = i_ch + _n_channels*iz_ind;
            VecSetValues(amc_pressure_force_rhs,1,&row_v,&value_v,ADD_VALUES);
          }
          else
          {
            PetscInt row = i_ch + _n_channels*iz_ind;
            PetscInt col = i_ch + _n_channels*(iz_ind+1);
            PetscScalar value = 1.0 * S_interp;
            MatSetValues(amc_pressure_force_mat,1,&row,1,&col,&value,INSERT_VALUES);

            if(_segregated_bool)
            {
              auto dp_in = (*_DP_soln)(node_in);
              auto dp_out = (*_DP_soln)(node_out);
              auto dp_interp = computeInterpolatedValue(dp_out, dp_in, "central_difference");
              PetscScalar value_v = -1.0 * dp_interp;
              PetscInt row_v = i_ch + _n_channels * iz_ind;
              VecSetValues(amc_pressure_force_rhs, 1, &row_v, &value_v, ADD_VALUES);
            }
          }

        }
      }
      // Solving pressure problem
      MatAssemblyBegin(amc_pressure_force_mat,MAT_FINAL_ASSEMBLY);
      MatAssemblyEnd(amc_pressure_force_mat,MAT_FINAL_ASSEMBLY);
      if (_verbose_subchannel)
        _console << "Block: " << iblock << " - Axial momentum pressure force matrix assembled" << std::endl;

      if(_segregated_bool)
      {
        KSP ksploc; PC  pc;
        Vec sol; VecDuplicate(amc_pressure_force_rhs, &sol);
        KSPCreate(PETSC_COMM_WORLD,&ksploc);
        KSPSetOperators(ksploc,amc_pressure_force_mat,amc_pressure_force_mat);
        KSPGetPC(ksploc,&pc); PCSetType(pc,PCJACOBI);
        KSPSetTolerances(ksploc,_rtol, _atol, _dtol, _maxit);
        KSPSetFromOptions(ksploc);
        KSPSolve(ksploc,amc_pressure_force_rhs,sol);
        PetscScalar * xx;
        VecGetArray(sol, &xx);
        // update Pressure solution
        for (unsigned int iz = last_node; iz > first_node - 1; iz--)
        {
          auto iz_ind = iz - first_node;
          for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
          {
            auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz-1);
            PetscScalar value = xx[iz_ind*_n_channels + i_ch];
            _P_soln->set(node_in, value);
          }
        }
        VecZeroEntries(amc_pressure_force_rhs);
        KSPDestroy(&ksploc); VecDestroy(&sol);
      }
    }
  }
}

Real
SubChannel1PhaseProblem::computeAddedHeatPin(unsigned int i_ch, unsigned int iz)
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

  if (! _implicit_bool)
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
        double sumWijh = 0.0;
        double sumWijPrimeDhij = 0.0;
        double added_enthalpy;
        if (_z_grid[iz] > unheated_length_entry &&
            _z_grid[iz] <= unheated_length_entry + heated_length)
          added_enthalpy = computeAddedHeatPin(i_ch, iz);
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

    MatZeroEntries(hc_time_derivative_mat); MatZeroEntries(hc_advective_derivative_mat);
    MatZeroEntries(hc_cross_derivative_mat);
    VecZeroEntries(hc_time_derivative_rhs); VecZeroEntries(hc_advective_derivative_rhs);
    VecZeroEntries(hc_cross_derivative_rhs); VecZeroEntries(hc_added_heat_rhs);
    MatZeroEntries(hc_sys_h_mat); VecZeroEntries(hc_sys_h_rhs);

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
          VecSetValues(hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES);
        }
        else
        {
          PetscInt row_tt = i_ch + _n_channels * iz_ind;
          PetscInt col_tt = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_tt = _TR * alpha * (*_rho_soln)(node_in)*volume / _dt;
          MatSetValues(hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_tt = i_ch + _n_channels * iz_ind;
        PetscInt col_tt = i_ch + _n_channels * iz_ind;
        PetscScalar value_tt = _TR * (1.0 - alpha) * (*_rho_soln)(node_out)*volume / _dt;
        MatSetValues(hc_time_derivative_mat, 1, &row_tt, 1, &col_tt, &value_tt, INSERT_VALUES);

        // Adding RHS elements
        PetscScalar rho_old_interp = computeInterpolatedValue(
            _rho_soln->old(node_out), _rho_soln->old(node_in), "central_difference", Pe);
        PetscScalar h_old_interp = computeInterpolatedValue(
            _h_soln->old(node_out), _h_soln->old(node_in), "central_difference", Pe);

        PetscScalar value_vec_tt = _TR * rho_old_interp * h_old_interp * volume / _dt;
        PetscInt row_vec_tt = i_ch + _n_channels * iz_ind;
        VecSetValues(hc_time_derivative_rhs, 1, &row_vec_tt, &value_vec_tt, ADD_VALUES);

        /// Advective derivative term
        if (iz == first_node)
        {
          PetscScalar value_vec_at = (*_mdot_soln)(node_in) * (*_h_soln)(node_in);
          PetscInt row_vec_at = i_ch + _n_channels * iz_ind;
          VecSetValues(hc_advective_derivative_rhs, 1, &row_vec_at, &value_vec_at, ADD_VALUES);
        }
        else
        {
          PetscInt row_at = i_ch + _n_channels * iz_ind;
          PetscInt col_at = i_ch + _n_channels * (iz_ind - 1);
          PetscScalar value_at = -1.0 * (*_mdot_soln)(node_in);
          MatSetValues(
              hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);
        }

        // Adding diagonal elements
        PetscInt row_at = i_ch + _n_channels * iz_ind;
        PetscInt col_at = i_ch + _n_channels * iz_ind;
        PetscScalar value_at = (*_mdot_soln)(node_out);
        MatSetValues(hc_advective_derivative_mat, 1, &row_at, 1, &col_at, &value_at, INSERT_VALUES);

        /// Cross derivative term
        unsigned int counter = 0;
        unsigned int cross_index = iz; // iz-1;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
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
              VecSetValues(hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = ii_ch + _n_channels * (iz_ind - 1);
              MatSetValues(hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = ii_ch + _n_channels * iz_ind;
            MatSetValues(hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
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
              VecSetValues(hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
            }
            else
            {
              PetscScalar value_ct = alpha * _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                     _Wij(i_gap, cross_index);
              PetscInt row_ct = i_ch + _n_channels * iz_ind;
              PetscInt col_ct = jj_ch + _n_channels * (iz_ind - 1);
              MatSetValues(hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
            }
            PetscScalar value_ct = (1.0 - alpha) *
                                   _subchannel_mesh.getCrossflowSign(i_ch, counter) *
                                   _Wij(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = jj_ch + _n_channels * iz_ind;
            MatSetValues(hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_ct, ADD_VALUES);
          }

          // Turbulent cross flows
          if (iz == first_node)
          {
            PetscScalar value_vec_ct =
                -2.0 * alpha * (*_mdot_soln)(node_in)*_WijPrime(i_gap, cross_index);
            value_vec_ct = alpha * (*_mdot_soln)(node_in_j)*_WijPrime(i_gap, cross_index);
            value_vec_ct += alpha * (*_mdot_soln)(node_in_i)*_WijPrime(i_gap, cross_index);
            PetscInt row_vec_ct = i_ch + _n_channels * iz_ind;
            VecSetValues(hc_cross_derivative_rhs, 1, &row_vec_ct, &value_vec_ct, ADD_VALUES);
          }
          else
          {
            PetscScalar value_center_ct = 2.0 * alpha * _WijPrime(i_gap, cross_index);
            PetscInt row_ct = i_ch + _n_channels * iz_ind;
            PetscInt col_ct = i_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

            PetscScalar value_left_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = jj_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

            PetscScalar value_right_ct = -1.0 * alpha * _WijPrime(i_gap, cross_index);
            row_ct = i_ch + _n_channels * iz_ind;
            col_ct = ii_ch + _n_channels * (iz_ind - 1);
            MatSetValues(
                hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);
          }

          PetscScalar value_center_ct = 2.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          PetscInt row_ct = i_ch + _n_channels * iz_ind;
          PetscInt col_ct = i_ch + _n_channels * iz_ind;
          MatSetValues(
              hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_center_ct, ADD_VALUES);

          PetscScalar value_left_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = jj_ch + _n_channels * iz_ind;
          MatSetValues(hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_left_ct, ADD_VALUES);

          PetscScalar value_right_ct = -1.0 * (1.0 - alpha) * _WijPrime(i_gap, cross_index);
          row_ct = i_ch + _n_channels * iz_ind;
          col_ct = ii_ch + _n_channels * iz_ind;
          MatSetValues(
              hc_cross_derivative_mat, 1, &row_ct, 1, &col_ct, &value_right_ct, ADD_VALUES);

          counter++;
        }

        /// Added heat enthalpy
        PetscScalar added_enthalpy;
        if (_z_grid[iz] > unheated_length_entry &&
            _z_grid[iz] <= unheated_length_entry + heated_length)
          added_enthalpy = computeAddedHeatPin(i_ch, iz);
        else
          added_enthalpy = 0.0;

        PetscInt row_vec_ht = i_ch + _n_channels * iz_ind;
        VecSetValues(hc_added_heat_rhs, 1, &row_vec_ht, &added_enthalpy, ADD_VALUES);
      }
    }

    /// Assembling system
    MatAssemblyBegin(hc_time_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(hc_time_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(hc_advective_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(hc_advective_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(hc_cross_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(hc_cross_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(hc_sys_h_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(hc_sys_h_mat,MAT_FINAL_ASSEMBLY);
    // Matrix
    MatAXPY(hc_sys_h_mat, 1.0, hc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(hc_sys_h_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(hc_sys_h_mat,MAT_FINAL_ASSEMBLY);
    MatAXPY(hc_sys_h_mat, 1.0, hc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(hc_sys_h_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(hc_sys_h_mat,MAT_FINAL_ASSEMBLY);
    MatAXPY(hc_sys_h_mat, 1.0, hc_cross_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(hc_sys_h_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(hc_sys_h_mat,MAT_FINAL_ASSEMBLY);
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Enthalpy conservation matrix assembled" << std::endl;
    // RHS
    VecAXPY(hc_sys_h_rhs, 1.0, hc_time_derivative_rhs);
    VecAXPY(hc_sys_h_rhs, 1.0, hc_advective_derivative_rhs);
    VecAXPY(hc_sys_h_rhs, 1.0, hc_cross_derivative_rhs);
    VecAXPY(hc_sys_h_rhs, 1.0, hc_added_heat_rhs);

    if(_segregated_bool || (! _monolithic_thermal_bool))
    {
      // Assembly the matrix system
      KSP ksploc; PC  pc;
      Vec sol; VecDuplicate(hc_sys_h_rhs, &sol);
      KSPCreate(PETSC_COMM_WORLD,&ksploc);
      KSPSetOperators(ksploc,hc_sys_h_mat,hc_sys_h_mat);
      KSPGetPC(ksploc,&pc); PCSetType(pc,PCJACOBI);
      KSPSetTolerances(ksploc,_rtol, _atol, _dtol, _maxit);
      KSPSetFromOptions(ksploc);
      KSPSolve(ksploc,hc_sys_h_rhs,sol);
      PetscScalar * xx;
      VecGetArray(sol, &xx);

      for (unsigned int iz = first_node; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node;
        for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
        {
          auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

          if (xx[iz_ind*_n_channels + i_ch] < 0)
          {
            _console << "Wij = : " << _Wij << "\n";
            mooseError(name(),
                       " : Calculation of negative Enthalpy h_out = : ",
                       xx[iz_ind*_n_channels + i_ch],
                       " Axial Level= : ",
                       iz);
          }
          _h_soln->set(node_out, xx[iz_ind*_n_channels + i_ch]);
        }
      }
      KSPDestroy(&ksploc); VecDestroy(&sol);
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

void
SubChannel1PhaseProblem::computeWij(int iblock)
{
  // Cross flow residual
  if (! _implicit_bool)
  {
    unsigned int last_node = (iblock + 1) * _block_size;
    unsigned int first_node = iblock * _block_size;

    const Real & pitch = _subchannel_mesh.getPitch();
    for (unsigned int iz = first_node+1; iz < last_node + 1; iz++)
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

        _Wij_residual_matrix(i_gap, iz - 1 - iblock * _block_size) =
            time_term + friction_term + inertia_term - pressure_term;
      }
    }
  }
  else
  {

    // Initializing to zero the elements of the lateral momentum assembly
    MatZeroEntries(cmc_time_derivative_mat); MatZeroEntries(cmc_advective_derivative_mat);
    MatZeroEntries(cmc_friction_force_mat); MatZeroEntries(cmc_pressure_force_mat);
    VecZeroEntries(cmc_time_derivative_rhs); VecZeroEntries(cmc_advective_derivative_rhs);
    VecZeroEntries(cmc_friction_force_rhs); VecZeroEntries(cmc_pressure_force_rhs);
    MatZeroEntries(cmc_sys_Wij_mat); VecZeroEntries(cmc_sys_Wij_rhs);

    unsigned int last_node = (iblock + 1) * _block_size;
    unsigned int first_node = iblock * _block_size;

    const Real & pitch = _subchannel_mesh.getPitch();
    for (unsigned int iz = first_node+1; iz < last_node + 1; iz++)
    {
      auto dz = _z_grid[iz] - _z_grid[iz - 1];
      auto iz_ind = iz - first_node - 1;
      for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
      {
        auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
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
        MatSetValues(cmc_time_derivative_mat, 1, &row_td, 1, &col_td, &value_td, INSERT_VALUES);
        PetscScalar value_td_rhs = time_factor * _Wij_old(i_gap, iz);
        VecSetValues(cmc_time_derivative_rhs, 1, &row_td, &value_td_rhs, INSERT_VALUES);

        // Assembling inertial term
        PetscScalar Pe = 0.5;
        auto alpha = computeInterpolationCoefficients("upwind", Pe);
        auto mass_term_out =
            (*_mdot_soln)(node_out_i) / S_i_out / rho_i_out + (*_mdot_soln)(node_out_j) / S_j_out / rho_j_out;
        auto mass_term_in =
            (*_mdot_soln)(node_in_i) / S_i_in / rho_i_in + (*_mdot_soln)(node_in_j) / S_j_in / rho_j_in;
        auto term_out = Sij * rho_star * (Lij / dz) * mass_term_out / 2.0;
        auto term_in = Sij * rho_star * (Lij / dz) * mass_term_in / 2.0;
        if (iz == first_node+1)
        {
          PetscInt row_ad = i_gap + _n_gaps * iz_ind;
          PetscScalar value_ad = term_in * alpha * _Wij(i_gap, iz - 1);
          VecSetValues(cmc_advective_derivative_rhs, 1, &row_ad, &value_ad, ADD_VALUES);

          PetscInt col_ad = i_gap + _n_gaps * iz_ind;
          value_ad = - 1.0 * term_in * (1.0 - alpha) + term_out * alpha;
          MatSetValues(cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES);

          col_ad = i_gap + _n_gaps * (iz_ind + 1);
          value_ad = term_out * (1.0 - alpha);
          MatSetValues(cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES);
        }
        else if (iz == last_node)
        {
          PetscInt row_ad = i_gap + _n_gaps * iz_ind;
          PetscInt col_ad = i_gap + _n_gaps * (iz_ind - 1);
          PetscScalar value_ad = - 1.0 * term_in * alpha;
          MatSetValues(cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES);

          col_ad = i_gap + _n_gaps * iz_ind;
          value_ad = -1.0 * term_in * (1.0 - alpha) + term_out * alpha;
          MatSetValues(cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES);

          value_ad = -1.0 * term_out * (1.0 - alpha) * _Wij(i_gap, iz);
          VecSetValues(cmc_advective_derivative_rhs, 1, &row_ad, &value_ad, ADD_VALUES);
        }
        else
        {
          PetscInt row_ad = i_gap + _n_gaps * iz_ind;
          PetscInt col_ad = i_gap + _n_gaps * (iz_ind - 1);
          PetscScalar value_ad = - 1.0 * term_in * alpha;
          MatSetValues(cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES);

          col_ad = i_gap + _n_gaps * iz_ind;
          value_ad = - 1.0 * term_in * (1.0 - alpha) + term_out * alpha;
          MatSetValues(cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES);

          col_ad = i_gap + _n_gaps * (iz_ind + 1);
          value_ad = term_out * (1.0 - alpha);
          MatSetValues(cmc_advective_derivative_mat, 1, &row_ad, 1, &col_ad, &value_ad, INSERT_VALUES);
        }

        // Assembling friction force
        PetscInt row_ff = i_gap + _n_gaps * iz_ind;
        PetscInt col_ff = i_gap + _n_gaps * iz_ind;
        PetscScalar value_ff = _kij * std::abs(_Wij(i_gap, iz)) / 2.0;
        MatSetValues(cmc_friction_force_mat, 1, &row_ff, 1, &col_ff, &value_ff, INSERT_VALUES);

        // Assembling pressure force
        alpha = computeInterpolationCoefficients("central_difference", Pe);

        if (!_staggered_pressure_bool)
        {
          PetscScalar pressure_factor = std::pow(Sij, 2.0) * rho_star;
          PetscInt row_pf = i_gap + _n_gaps * iz_ind;
          PetscInt col_pf = i_ch + _n_channels * iz_ind;
          PetscScalar value_pf = -1.0 * alpha * pressure_factor;
          MatSetValues(cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES);
          col_pf = j_ch + _n_channels * iz_ind;
          value_pf = alpha * pressure_factor;
          MatSetValues(cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES);

          if (iz == last_node)
          {
            PetscInt row_pf = i_gap + _n_gaps * iz_ind;
            PetscScalar value_pf = (1.0 - alpha) * pressure_factor * (*_P_soln)(node_out_i);
            VecSetValues(cmc_pressure_force_rhs, 1, &row_pf, &value_pf, ADD_VALUES);
            value_pf = -1.0 * (1.0 - alpha) * pressure_factor * (*_P_soln)(node_out_j);
            VecSetValues(cmc_pressure_force_rhs, 1, &row_pf, &value_pf, ADD_VALUES);
          }
          else
          {
            row_pf = i_gap + _n_gaps * iz_ind;
            col_pf = i_ch + _n_channels * (iz_ind+1);
            value_pf = -1.0 * (1.0 - alpha) * pressure_factor;
            MatSetValues(cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES);
            col_pf = j_ch + _n_channels * (iz_ind+1);
            value_pf = (1.0 - alpha) * pressure_factor;
            MatSetValues(cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES);
          }
        }
        else
        {
          PetscScalar pressure_factor = std::pow(Sij, 2.0) * rho_star;
          PetscInt row_pf = i_gap + _n_gaps * iz_ind;
          PetscInt col_pf = i_ch + _n_channels * iz_ind;
          PetscScalar value_pf = -1.0 * pressure_factor;
          MatSetValues(cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES);
          col_pf = j_ch + _n_channels * iz_ind;
          value_pf = pressure_factor;
          MatSetValues(cmc_pressure_force_mat, 1, &row_pf, 1, &col_pf, &value_pf, ADD_VALUES);
        }
      }
    }
    /// Assembling system
    MatZeroEntries(cmc_sys_Wij_mat);  VecZeroEntries(cmc_sys_Wij_rhs);
    MatAssemblyBegin(cmc_time_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_time_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(cmc_advective_derivative_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_advective_derivative_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(cmc_friction_force_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_friction_force_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(cmc_pressure_force_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_pressure_force_mat,MAT_FINAL_ASSEMBLY);
    MatAssemblyBegin(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY);
    // Matrix
    MatAXPY(cmc_sys_Wij_mat, 1.0, cmc_time_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY);
    MatAXPY(cmc_sys_Wij_mat, 1.0, cmc_advective_derivative_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY);
    MatAXPY(cmc_sys_Wij_mat, 1.0, cmc_friction_force_mat, UNKNOWN_NONZERO_PATTERN);
    MatAssemblyBegin(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY);
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Cross flow system matrix assembled" << std::endl;
    if (_verbose_subchannel)
      _console << "Block: " << iblock << " - Cross flow pressure force matrix assembled" << std::endl;
    // RHS
    VecAXPY(cmc_sys_Wij_rhs, 1.0, cmc_time_derivative_rhs);
    VecAXPY(cmc_sys_Wij_rhs, 1.0, cmc_advective_derivative_rhs);
    VecAXPY(cmc_sys_Wij_rhs, 1.0, cmc_friction_force_rhs);

    if(_segregated_bool)
    {
      // Assembly the matrix system
      Vec sol_holder_P; createPetscVector(sol_holder_P, _block_size * _n_gaps);
      Vec sol_holder_W; createPetscVector(sol_holder_W, _block_size * _n_gaps);
      populateVectorFromHandle<SolutionHandle *>(prodp, _P_soln, iblock * _block_size, (iblock + 1) * _block_size - 1, _n_channels);
      populateVectorFromDense<libMesh::DenseMatrix<Real>>(Wij_vec, _Wij, first_node, last_node, _n_gaps);

      MatMult(cmc_sys_Wij_mat, Wij_vec, sol_holder_W);
      VecAXPY(sol_holder_W, -1.0, cmc_sys_Wij_rhs);
      MatMult(cmc_pressure_force_mat, prodp, sol_holder_P);
      VecAXPY(sol_holder_P, -1.0, cmc_pressure_force_rhs);
      VecAXPY(sol_holder_W, 1.0, sol_holder_P);
      PetscScalar * xx;
      VecGetArray(sol_holder_W, &xx);

      for (unsigned int iz = first_node+1; iz < last_node + 1; iz++)
      {
        auto iz_ind = iz - first_node - 1;
        for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
        {
          _Wij_residual_matrix(i_gap, iz - 1 - iblock * _block_size) =
              xx[iz_ind*_n_gaps + i_gap];
        }
      }

      VecDestroy(&sol_holder_P);
      VecDestroy(&sol_holder_W);
    }

  }
}

libMesh::DenseVector<Real>
SubChannel1PhaseProblem::residualFunction(int iblock, libMesh::DenseVector<Real> solution)
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

  //Solving axial flux
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

PetscErrorCode
SubChannel1PhaseProblem::implicitPetscSolve(int iblock)
{
  Vec            b_nest, x_nest;      /* approx solution, RHS, exact solution */
  Mat            A_nest;       /* linear system matrix */
  KSP            ksp;          /* linear solver context */
  PC             pc;           /* preconditioner context */
  PetscErrorCode ierr;
  PetscInt       Q = _monolithic_thermal_bool ? 4 : 3;
  std::vector<Mat> mat_array(Q*Q);
  std::vector<Vec> vec_array(Q);
//  Mat            mat_array[Q*Q];
//  Vec            vec_array[Q];

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
  if(_monolithic_thermal_bool) computeh(iblock);

  if (_verbose_subchannel)
  {
    _console << "Starting nested system." << std::endl;
    _console << Q << std::endl;
  }
  // Mass conservation
  PetscInt field_num = 0;
  ierr = MatAssemblyBegin(mc_axial_convection_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(mc_axial_convection_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatDuplicate(mc_axial_convection_mat,MAT_COPY_VALUES,&mat_array[Q*field_num+0]); CHKERRQ(ierr);
  ierr = MatAssemblyBegin(mat_array[Q*field_num+0],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(mat_array[Q*field_num+0],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  mat_array[Q*field_num+1] = NULL;
  if (_axial_mass_flow_tight_coupling)
  {
    ierr = MatAssemblyBegin(mc_sumWij_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(mc_sumWij_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatDuplicate(mc_sumWij_mat,MAT_COPY_VALUES,&mat_array[Q*field_num+2]); CHKERRQ(ierr);
    ierr = MatAssemblyBegin(mat_array[Q*field_num+2],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(mat_array[Q*field_num+2],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  }
  else
  {
    mat_array[Q*field_num+2] = NULL;
  }
  _console << "Term 3" << std::endl;
  if(_monolithic_thermal_bool)
  {
    mat_array[Q*field_num+3] = NULL;
  }
  ierr = VecDuplicate(mc_axial_convection_rhs,&vec_array[field_num]); CHKERRQ(ierr);
  ierr = VecCopy(mc_axial_convection_rhs,vec_array[field_num]); CHKERRQ(ierr);
  if (! _axial_mass_flow_tight_coupling)
  {
    Vec sumWij_loc;
    ierr = VecDuplicate(mc_axial_convection_rhs,&sumWij_loc); CHKERRQ(ierr);
    ierr = VecSet(sumWij_loc, 0.0); CHKERRQ(ierr);
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

          PetscScalar value_vec_2 = -1.0 * (*_SumWij_soln)(node_out);
          PetscInt row_vec_2 = i_ch + _n_channels*iz_ind;
          VecSetValues(sumWij_loc,1,&row_vec_2,&value_vec_2,ADD_VALUES);
      }
    }
    //VecView(sumWij_loc, PETSC_VIEWER_STDOUT_WORLD);
    ierr = VecAXPY(vec_array[field_num], 1.0, sumWij_loc); CHKERRQ(ierr);
    VecDestroy(&sumWij_loc);
  }

//  MatGetRowMaxAbs(mc_axial_convection_mat, mc_axial_convection_rhs, NULL);
//  VecView(mc_axial_convection_rhs, PETSC_VIEWER_STDOUT_WORLD);

  if (_verbose_subchannel)
    _console << "Mass ok." << std::endl;
  // Axial momentum conservation
  field_num = 1;
  if (_pressure_axial_momentum_tight_coupling)
  {
    ierr = MatAssemblyBegin(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(amc_sys_mdot_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatDuplicate(amc_sys_mdot_mat,MAT_COPY_VALUES,&mat_array[Q*field_num+0]); CHKERRQ(ierr);
    ierr = MatAssemblyBegin(mat_array[Q*field_num+0],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(mat_array[Q*field_num+0],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  }
  else
  {
    mat_array[Q*field_num+0] = NULL;
  }
  ierr = MatAssemblyBegin(amc_pressure_force_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(amc_pressure_force_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatDuplicate(amc_pressure_force_mat,MAT_COPY_VALUES,&mat_array[Q*field_num+1]); CHKERRQ(ierr);
  ierr = MatAssemblyBegin(mat_array[Q*field_num+1],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(mat_array[Q*field_num+1],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  mat_array[Q*field_num+2] = NULL;
  if(_monolithic_thermal_bool)
  {
    mat_array[Q*field_num+3] = NULL;
  }
  ierr = VecDuplicate(amc_pressure_force_rhs,&vec_array[field_num]); CHKERRQ(ierr);
  ierr = VecCopy(amc_pressure_force_rhs,vec_array[field_num]); CHKERRQ(ierr);
  if (_pressure_axial_momentum_tight_coupling)
  {
    ierr = VecAXPY(vec_array[field_num], 1.0, amc_sys_mdot_rhs); CHKERRQ(ierr);
  }
  else
  {
      unsigned int last_node = (iblock + 1) * _block_size;
      unsigned int first_node = iblock * _block_size + 1;
      populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
      Vec ls; VecDuplicate(amc_sys_mdot_rhs,&ls);
      MatMult(amc_sys_mdot_mat, prod, ls);
      VecAXPY(ls, -1.0, amc_sys_mdot_rhs);
      //VecView(ls, PETSC_VIEWER_STDOUT_WORLD);
      VecAXPY(vec_array[field_num], -1.0, ls);
      VecDestroy(&ls);
  }

//  MatView(amc_pressure_force_mat, PETSC_VIEWER_STDOUT_WORLD);
//  VecView(vec_array[field_num], PETSC_VIEWER_STDOUT_WORLD);

//  MatGetRowMaxAbs(amc_pressure_force_mat, amc_pressure_force_rhs, NULL);
//  VecView(amc_pressure_force_rhs, PETSC_VIEWER_STDOUT_WORLD);
  if (_verbose_subchannel)
    _console << "Lin mom OK." << std::endl;

  // Cross momentum conservation
  field_num = 2;
  mat_array[Q*field_num+0] = NULL;
  if (_pressure_cross_momentum_tight_coupling)
  {
    ierr = MatAssemblyBegin(cmc_pressure_force_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(cmc_pressure_force_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatDuplicate(cmc_pressure_force_mat,MAT_COPY_VALUES,&mat_array[Q*field_num+1]); CHKERRQ(ierr);
    ierr = MatAssemblyBegin(mat_array[Q*field_num+1],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(mat_array[Q*field_num+1],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  }
  else
  {
    mat_array[Q*field_num+1] = NULL;
  }
//  mat_array[Q*field_num+1] = NULL;
  if (false)
  {
    unsigned int last_node = (iblock + 1) * _block_size;
    unsigned int first_node = iblock * _block_size + 1;
    populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
    Vec ls; VecDuplicate(amc_sys_mdot_rhs,&ls);
    MatMult(amc_sys_mdot_mat, prod, ls);

    KSP ksploc; PC  pc;
    Vec sol; VecDuplicate(amc_pressure_force_rhs, &sol);
    KSPCreate(PETSC_COMM_WORLD,&ksploc);
    KSPSetOperators(ksploc,amc_pressure_force_mat,amc_pressure_force_mat);
    KSPGetPC(ksploc,&pc); PCSetType(pc,PCJACOBI);
    KSPSetTolerances(ksploc, _rtol, _atol, _dtol, _maxit);
    KSPSetFromOptions(ksploc);
    VecAXPY(amc_pressure_force_rhs, 1.0, ls);
    KSPSolve(ksploc,amc_pressure_force_rhs,sol);
    KSPDestroy(&ksploc); VecDestroy(&ls);

    Vec sol_holder_P; createPetscVector(sol_holder_P, _block_size * _n_gaps);
    ierr = MatMult(cmc_pressure_force_mat, sol, sol_holder_P); CHKERRQ(ierr);
    ierr = VecAXPY(sol_holder_P, -1.0, cmc_pressure_force_rhs); CHKERRQ(ierr);

    Vec diag_Wij;
    ierr = VecDuplicate(cmc_sys_Wij_rhs,&diag_Wij); CHKERRQ(ierr);
    ierr = MatGetDiagonal(cmc_sys_Wij_mat,diag_Wij); CHKERRQ(ierr);
    VecPointwiseDivide(sol_holder_P, sol_holder_P, diag_Wij);

    Vec sumWij_loc;
    ierr = VecDuplicate(mc_axial_convection_rhs,&sumWij_loc); CHKERRQ(ierr);
    ierr = VecSet(sumWij_loc, 0.0); CHKERRQ(ierr);

    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        PetscScalar sumWij = 0.0;
        // Calculate sum of crossflow into channel i from channels j around i
        unsigned int counter = 0;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap);
          unsigned int i_ch_loc = chans.first;
          PetscInt row_vec = i_ch_loc + _n_channels*iz_ind;
          PetscScalar loc_Wij_value;
          VecGetValues(sol_holder_P,1,&row_vec,&loc_Wij_value);
          sumWij += _subchannel_mesh.getCrossflowSign(i_ch, counter) * loc_Wij_value;
          counter++;
        }
        PetscInt row_vec = i_ch + _n_channels*iz_ind;
        VecSetValues(sumWij_loc,1,&row_vec,&sumWij,INSERT_VALUES);
      }
    }

    populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
    VecAbs(prod);
    VecAbs(sumWij_loc);
    VecPointwiseDivide(sumWij_loc, sumWij_loc, prod);

    //VecView(sumWij_loc, PETSC_VIEWER_STDOUT_WORLD);
    PetscScalar max_value;
    //VecMax(sumWij_loc, NULL, &max_value);
    VecMean(sumWij_loc, &max_value);
    if (_verbose_subchannel)
      _console << "Max val: " << max_value << std::endl;

    Vec Wij_new_loc, Wij_old_loc;
    ierr = VecDuplicate(cmc_sys_Wij_rhs,&Wij_old_loc); CHKERRQ(ierr);
    ierr = VecDuplicate(cmc_sys_Wij_rhs,&Wij_new_loc); CHKERRQ(ierr);
    populateVectorFromDense<libMesh::DenseMatrix<Real>>(Wij_old_loc, _Wij_old, first_node, last_node, _n_gaps);
    populateVectorFromDense<libMesh::DenseMatrix<Real>>(Wij_new_loc, _Wij, first_node, last_node, _n_gaps);
    VecAXPY(Wij_new_loc, -1.0, Wij_old_loc);
    PetscScalar loc_sum_vec;
    VecAbs(Wij_new_loc);
    VecMean(Wij_new_loc, &loc_sum_vec);
    max_value *= (std::exp(100*loc_sum_vec) - 1.0);

    //VecView(sol_holder_P, PETSC_VIEWER_STDOUT_WORLD);
    VecAbs(sol_holder_P);
    VecScale(sol_holder_P, max_value);

    ierr = MatDiagonalSet(cmc_sys_Wij_mat, sol_holder_P, ADD_VALUES); CHKERRQ(ierr);

    VecDestroy(&sol);
    VecDestroy(&sol_holder_P);
    VecDestroy(&diag_Wij);
    VecDestroy(&sumWij_loc);
    VecDestroy(&Wij_new_loc);
    VecDestroy(&Wij_old_loc);

    //    PetscScalar relaxation_factor = 0.5;
    //    Vec diag_Wij;
    //    ierr = VecDuplicate(cmc_sys_Wij_rhs,&diag_Wij); CHKERRQ(ierr);
    //    ierr = MatGetDiagonal(cmc_sys_Wij_mat,diag_Wij); CHKERRQ(ierr);
    //    ierr = VecScale(diag_Wij, 1.0/relaxation_factor); CHKERRQ(ierr);
    //    ierr = MatDiagonalSet(cmc_sys_Wij_mat, diag_Wij, ADD_VALUES); CHKERRQ(ierr);
    //    populateVectorFromDense<libMesh::DenseMatrix<Real>>(Wij_vec, _Wij_old, first_node, last_node, _n_gaps);
    //    ierr = VecScale(diag_Wij, (1.0-relaxation_factor)); CHKERRQ(ierr);
    //    ierr = VecPointwiseMult(Wij_vec, Wij_vec, diag_Wij); CHKERRQ(ierr);
    //    VecDestroy(&diag_Wij);
  }
  if (false)
  {
    PetscScalar local_shift = -1.0;
    ierr =  MatShift(cmc_sys_Wij_mat,local_shift); CHKERRQ(ierr);
  }

  ierr = MatAssemblyBegin(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(cmc_sys_Wij_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatDuplicate(cmc_sys_Wij_mat,MAT_COPY_VALUES,&mat_array[Q*field_num+2]); CHKERRQ(ierr);
  //ierr = MatScale(mat_array[Q*field_num+2], -1.0); CHKERRQ(ierr);
  ierr = MatAssemblyBegin(mat_array[Q*field_num+2],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  ierr = MatAssemblyEnd(mat_array[Q*field_num+2],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
  if(_monolithic_thermal_bool)
  {
    mat_array[Q*field_num+3] = NULL;
  }

  ierr = VecDuplicate(cmc_sys_Wij_rhs,&vec_array[field_num]); CHKERRQ(ierr);
  ierr = VecCopy(cmc_sys_Wij_rhs,vec_array[field_num]); CHKERRQ(ierr);
  if (_pressure_cross_momentum_tight_coupling)
  {
    ierr = VecAXPY(vec_array[field_num], 1.0, cmc_pressure_force_rhs); CHKERRQ(ierr);
    //ierr = VecScale(vec_array[field_num], -1.0); CHKERRQ(ierr);
  }
  else
  {
    Vec sol_holder_P; createPetscVector(sol_holder_P, _block_size * _n_gaps);
    populateVectorFromHandle<SolutionHandle *>(prodp, _P_soln, iblock * _block_size, (iblock + 1) * _block_size - 1, _n_channels);

    ierr = MatMult(cmc_pressure_force_mat, prodp, sol_holder_P); CHKERRQ(ierr);
    ierr = VecAXPY(sol_holder_P, -1.0, cmc_pressure_force_rhs); CHKERRQ(ierr);
    ierr = VecScale(sol_holder_P, 1.0); CHKERRQ(ierr);
    ierr = VecAXPY(vec_array[field_num], 1.0, sol_holder_P); CHKERRQ(ierr);
  }

  // Automated relaxation for cross flows
  if (false)
  {
    PetscScalar safety_factor = 0.5;
    Vec sol_holder_P; createPetscVector(sol_holder_P, _block_size * _n_gaps);
    Vec sol_holder_W; createPetscVector(sol_holder_W, _block_size * _n_gaps);
    populateVectorFromHandle<SolutionHandle *>(prodp, _P_soln, iblock * _block_size, (iblock + 1) * _block_size - 1, _n_channels);
    populateVectorFromDense<libMesh::DenseMatrix<Real>>(Wij_vec, _Wij, first_node, last_node, _n_gaps);

    Vec diag_Wij;
    ierr = VecDuplicate(cmc_sys_Wij_rhs,&diag_Wij); CHKERRQ(ierr);
    ierr = MatGetDiagonal(cmc_sys_Wij_mat,diag_Wij); CHKERRQ(ierr);
    //VecView(diag_Wij, PETSC_VIEWER_STDOUT_WORLD);
    ierr = VecScale(diag_Wij,-1.0); CHKERRQ(ierr);
    ierr = MatDiagonalSet(cmc_sys_Wij_mat, diag_Wij, ADD_VALUES); CHKERRQ(ierr);
    ierr = VecScale(diag_Wij,-1.0); CHKERRQ(ierr);

    MatMult(cmc_sys_Wij_mat, Wij_vec, sol_holder_W);
    VecAXPY(sol_holder_W, -1.0, cmc_sys_Wij_rhs);
    VecAbs(sol_holder_W);
    MatMult(cmc_pressure_force_mat, prodp, sol_holder_P);
    VecAXPY(sol_holder_P, -1.0, cmc_pressure_force_rhs);
    VecAbs(sol_holder_W);
    VecAXPY(sol_holder_W, 1.0, sol_holder_P);

    PetscScalar min_mdot = 1.0;
    populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
    ierr = VecAbs(prod); CHKERRQ(ierr);
    ierr = VecMin(prod, NULL, &min_mdot); CHKERRQ(ierr);
    min_mdot *= safety_factor;
    min_mdot += 1e-10;
    ierr = VecScale(sol_holder_W, 1.0/min_mdot);

    Vec unit_vector;
    ierr = VecDuplicate(cmc_sys_Wij_rhs,&unit_vector); CHKERRQ(ierr);
    ierr = VecSet(unit_vector,1.0); CHKERRQ(ierr);
    ierr = VecAXPY(diag_Wij, 1e-10, unit_vector); CHKERRQ(ierr);
    ierr = VecPointwiseDivide(sol_holder_W, sol_holder_W, diag_Wij); CHKERRQ(ierr);

    PetscScalar constraint_max;
    PetscInt max_pos;
    Vec abs_diag_Wij;
    ierr = VecDuplicate(cmc_sys_Wij_rhs,&abs_diag_Wij); CHKERRQ(ierr);
    ierr = VecCopy(diag_Wij,abs_diag_Wij); CHKERRQ(ierr);
    ierr = VecAbs(abs_diag_Wij); CHKERRQ(ierr);
    ierr = VecPointwiseDivide(sol_holder_W, sol_holder_W, abs_diag_Wij); CHKERRQ(ierr);
    ierr = VecMax(sol_holder_W, &max_pos, &constraint_max); CHKERRQ(ierr);

    PetscScalar diag_value;
    ierr = VecGetValues(diag_Wij, 1, &max_pos, &diag_value); CHKERRQ(ierr);
    PetscScalar scaling = diag_value * constraint_max;
    if (_verbose_subchannel)
    {
      _console << "Max constraint: " << constraint_max << std::endl;
      _console << "Diagonal value: " << diag_value << std::endl;
    }
    if (_verbose_subchannel)
      _console << "Relaxation diagonal factor for Wij: " << scaling << std::endl;
    ierr =  MatShift(mat_array[Q*field_num+2],-1.0*std::abs(scaling)); CHKERRQ(ierr);

    VecDestroy(&sol_holder_P);
    VecDestroy(&sol_holder_W);
    VecDestroy(&diag_Wij);
    VecDestroy(&unit_vector);
    VecDestroy(&abs_diag_Wij);
  }

//  MatGetRowMaxAbs(cmc_sys_Wij_mat, cmc_sys_Wij_rhs, NULL);
//  VecView(cmc_sys_Wij_rhs, PETSC_VIEWER_STDOUT_WORLD);

  if (_verbose_subchannel)
    _console << "Cross mom ok." << std::endl;

  // Energy conservation
  if(_monolithic_thermal_bool)
  {
    field_num = 3;
    mat_array[Q*field_num+0] = NULL;
    mat_array[Q*field_num+1] = NULL;
    mat_array[Q*field_num+2] = NULL;
    ierr = MatAssemblyBegin(hc_sys_h_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(hc_sys_h_mat,MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatDuplicate(hc_sys_h_mat,MAT_COPY_VALUES,&mat_array[Q*field_num+3]); CHKERRQ(ierr);
    ierr = MatAssemblyBegin(mat_array[Q*field_num+3],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(mat_array[Q*field_num+3],MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = VecDuplicate(hc_sys_h_rhs,&vec_array[field_num]); CHKERRQ(ierr);
    ierr = VecCopy(hc_sys_h_rhs,vec_array[field_num]); CHKERRQ(ierr);
  }
  if (_verbose_subchannel)
    _console << "Energy ok." << std::endl;

  // Relaxing linear system
  if (true)
  {

    // Estimating cross-flow resistances to achieve realizable solves
    populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
    Vec mdot_estimate; createPetscVector(mdot_estimate, _block_size * _n_channels);
    Vec pmat_diag; createPetscVector(pmat_diag, _block_size * _n_channels);
    Vec p_estimate; createPetscVector(p_estimate, _block_size * _n_channels);
    Vec unity_vec; createPetscVector(unity_vec, _block_size * _n_channels); VecSet(unity_vec, 1.0);
    Vec sol_holder_P; createPetscVector(sol_holder_P, _block_size * _n_gaps);
    Vec diag_Wij_loc; createPetscVector(diag_Wij_loc, _block_size * _n_gaps);
    Vec Wij_estimate; createPetscVector(Wij_estimate, _block_size * _n_gaps);
    Vec unity_vec_Wij; createPetscVector(unity_vec_Wij, _block_size * _n_gaps); VecSet(unity_vec_Wij, 1.0);
    Vec _Wij_loc_vec; createPetscVector(_Wij_loc_vec, _block_size * _n_gaps);
    Vec _Wij_old_loc_vec; createPetscVector(_Wij_old_loc_vec, _block_size * _n_gaps);

    ierr = MatMult(mat_array[Q], prod, mdot_estimate); CHKERRQ(ierr);
    ierr = MatGetDiagonal(mat_array[Q+1], pmat_diag); CHKERRQ(ierr);
    ierr = VecAXPY(pmat_diag, 1e-10, unity_vec); CHKERRQ(ierr);
    ierr = VecPointwiseDivide(p_estimate, mdot_estimate, pmat_diag); CHKERRQ(ierr);

    ierr = MatMult(mat_array[2*Q+1], p_estimate, sol_holder_P); CHKERRQ(ierr);
    ierr = VecAXPY(sol_holder_P, -1.0, cmc_pressure_force_rhs); CHKERRQ(ierr);
    ierr = MatGetDiagonal(mat_array[2*Q+2],diag_Wij_loc); CHKERRQ(ierr);
    ierr = VecAXPY(diag_Wij_loc, 1e-10, unity_vec_Wij); CHKERRQ(ierr);
    ierr = VecPointwiseDivide(Wij_estimate, sol_holder_P, diag_Wij_loc); CHKERRQ(ierr);

    Vec sumWij_loc; createPetscVector(sumWij_loc, _block_size * _n_channels);
    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        PetscScalar sumWij = 0.0; unsigned int counter = 0;
        for (auto i_gap : _subchannel_mesh.getChannelGaps(i_ch))
        {
          auto chans = _subchannel_mesh.getGapNeighborChannels(i_gap); unsigned int i_ch_loc = chans.first;
          PetscInt row_vec = i_ch_loc + _n_channels*iz_ind;
          PetscScalar loc_Wij_value; VecGetValues(sol_holder_P,1,&row_vec,&loc_Wij_value);
          sumWij += _subchannel_mesh.getCrossflowSign(i_ch, counter) * loc_Wij_value;
          counter++;
        }
        PetscInt row_vec = i_ch + _n_channels*iz_ind;
        VecSetValues(sumWij_loc,1,&row_vec,&sumWij,INSERT_VALUES);
      }
    }

    PetscScalar min_mdot; VecAbs(prod);
    ierr = VecMin(prod, NULL, &min_mdot); CHKERRQ(ierr);
    if (_verbose_subchannel)
      _console << "Minimum estimated mdot: " << min_mdot << std::endl;

    VecAbs(sumWij_loc);
    ierr = VecMax(sumWij_loc, NULL, &max_sumWij); CHKERRQ(ierr);
    max_sumWij = std::max(1e-10, max_sumWij);
    if (_verbose_subchannel)
      _console << "Maximum estimated Wij: " << max_sumWij << std::endl;

    populateVectorFromDense<libMesh::DenseMatrix<Real>>(_Wij_loc_vec, _Wij, first_node, last_node, _n_gaps); VecAbs(_Wij_loc_vec);
    populateVectorFromDense<libMesh::DenseMatrix<Real>>(_Wij_old_loc_vec, _Wij_old, first_node, last_node, _n_gaps); VecAbs(_Wij_old_loc_vec);
    ierr = VecAXPY(_Wij_loc_vec, -1.0, _Wij_old_loc_vec); CHKERRQ(ierr);
    PetscScalar relax_factor; VecAbs(_Wij_loc_vec); VecMean(_Wij_loc_vec, &relax_factor);
    relax_factor = relax_factor/max_sumWij + 0.5;
    if (_verbose_subchannel)
      _console << "Relax base value: " << relax_factor << std::endl;

    PetscScalar resistance_relaxation = 0.9;
    _added_K = max_sumWij / min_mdot;
    if (_verbose_subchannel)
      _console << "New cross resistance: " << _added_K << std::endl;
    _added_K = (_added_K * resistance_relaxation + (1.0 - resistance_relaxation) * _added_K_old) * relax_factor;
    if (_verbose_subchannel)
      _console << "Relaxed cross resistance: " << _added_K << std::endl;
    if (_added_K < 10 && _added_K >= 1.0) _added_K = 1.0; //(1.0 - resistance_relaxation);
    if (_added_K < 1.0 && _added_K >= 0.1) _added_K = 0.5;
    if (_added_K < 0.1 && _added_K >= 0.01) _added_K = 1./3.;
    if (_added_K < 1e-2 && _added_K >= 1e-3) _added_K = 0.1;
    if (_added_K < 1e-3) _added_K = 1.0*_added_K;
    if (_verbose_subchannel)
      _console << "Actual added cross resistance: " << _added_K << std::endl;
    ierr = VecScale(unity_vec_Wij, _added_K); CHKERRQ(ierr);
    _added_K_old = _added_K;

    // Adding cross resistances
    ierr = MatDiagonalSet(mat_array[2*Q+2], unity_vec_Wij, ADD_VALUES); CHKERRQ(ierr);

    VecDestroy(&mdot_estimate); VecDestroy(&pmat_diag); VecDestroy(&unity_vec);
    VecDestroy(&p_estimate); VecDestroy(&sol_holder_P); VecDestroy(&diag_Wij_loc);
    VecDestroy(&unity_vec_Wij); VecDestroy(&Wij_estimate); VecDestroy(&sumWij_loc);
    VecDestroy(&_Wij_loc_vec); VecDestroy(&_Wij_old_loc_vec);

    // Auto-computing relaxation factors

    PetscScalar relaxation_factor_mdot, relaxation_factor_P, relaxation_factor_Wij;
//    if (relax_factor < 1E-10)
//    {
    relaxation_factor_mdot = 1.0;
    relaxation_factor_P = 1.0; //std::exp(-5.0);
    relaxation_factor_Wij = 0.1;
//    }
//    else
//    {
//      PetscScalar relax_factor_loc = std::min(relax_factor, 7.0);
//      relaxation_factor_mdot = std::exp(-1.0*relax_factor_loc);
//      relaxation_factor_P = std::exp(-1.0*relax_factor_loc);
//      relax_factor_loc = std::min(relax_factor, 7.0);
//      relaxation_factor_Wij = std::exp(-1.0*relax_factor_loc);
//    }
    if (_verbose_subchannel)
    {
      _console << "Relax mdot: " << relaxation_factor_mdot << std::endl;
      _console << "Relax P: " << relaxation_factor_P << std::endl;
      _console << "Relax Wij: " << relaxation_factor_Wij << std::endl;
    }

    PetscInt field_num = 0;
    Vec diag_mdot;
    ierr = VecDuplicate(vec_array[field_num],&diag_mdot); CHKERRQ(ierr);
    ierr = MatGetDiagonal(mat_array[Q*field_num+field_num],diag_mdot); CHKERRQ(ierr);
    ierr = VecScale(diag_mdot, 1.0/relaxation_factor_mdot); CHKERRQ(ierr);
    ierr = MatDiagonalSet(mat_array[Q*field_num+field_num], diag_mdot, INSERT_VALUES); CHKERRQ(ierr);
    populateVectorFromHandle<SolutionHandle *>(prod, _mdot_soln, first_node, last_node, _n_channels);
    ierr = VecScale(diag_mdot, (1.0-relaxation_factor_mdot)); CHKERRQ(ierr);
    ierr = VecPointwiseMult(prod, prod, diag_mdot); CHKERRQ(ierr);
    ierr = VecAXPY(vec_array[field_num], 1.0, prod); CHKERRQ(ierr);
    VecDestroy(&diag_mdot);

    if (_verbose_subchannel)
      _console << "mdot relaxed" << std::endl;

    field_num = 1;
    Vec diag_P;
    ierr = VecDuplicate(vec_array[field_num],&diag_P); CHKERRQ(ierr);
    ierr = MatGetDiagonal(mat_array[Q*field_num+field_num],diag_P); CHKERRQ(ierr);
    ierr = VecScale(diag_P, 1.0/relaxation_factor_P); CHKERRQ(ierr);
    ierr = MatDiagonalSet(mat_array[Q*field_num+field_num], diag_P, INSERT_VALUES); CHKERRQ(ierr);
    if (_verbose_subchannel)
      _console << "Mat assembled" << std::endl;
    populateVectorFromHandle<SolutionHandle *>(prod, _P_soln, first_node, last_node, _n_channels);
    ierr = VecScale(diag_P, (1.0-relaxation_factor_P)); CHKERRQ(ierr);
    ierr = VecPointwiseMult(prod, prod, diag_P); CHKERRQ(ierr);
    ierr = VecAXPY(vec_array[field_num], 1.0, prod); CHKERRQ(ierr);
    VecDestroy(&diag_P);

    if (_verbose_subchannel)
      _console << "P relaxed" << std::endl;

    field_num = 2;
    Vec diag_Wij;
    ierr = VecDuplicate(vec_array[field_num],&diag_Wij); CHKERRQ(ierr);
    ierr = MatGetDiagonal(mat_array[Q*field_num+field_num],diag_Wij); CHKERRQ(ierr);
    ierr = VecScale(diag_Wij, 1.0/relaxation_factor_Wij); CHKERRQ(ierr);
    ierr = MatDiagonalSet(mat_array[Q*field_num+field_num], diag_Wij, INSERT_VALUES); CHKERRQ(ierr);
    populateVectorFromDense<libMesh::DenseMatrix<Real>>(Wij_vec, _Wij, first_node, last_node, _n_gaps);
    ierr = VecScale(diag_Wij, (1.0-relaxation_factor_Wij)); CHKERRQ(ierr);
    ierr = VecPointwiseMult(Wij_vec, Wij_vec, diag_Wij); CHKERRQ(ierr);
    ierr = VecAXPY(vec_array[field_num], 1.0, Wij_vec); CHKERRQ(ierr);
    VecDestroy(&diag_Wij);

    if (_verbose_subchannel)
      _console << "Wij relaxed" << std::endl;
  }
  if (_verbose_subchannel)
    _console << "Linear solver relaxed." << std::endl;

  // Creating nested matrices
  ierr = MatCreateNest(PETSC_COMM_WORLD,Q,NULL,Q,NULL,mat_array.data(),&A_nest); CHKERRQ(ierr);
  ierr = VecCreateNest(PETSC_COMM_WORLD,Q,NULL,vec_array.data(),&b_nest); CHKERRQ(ierr);
  if (_verbose_subchannel)
    _console << "Nested system created." << std::endl;

  /// Setting up linear solver
  // Creating linear solver
  ierr = KSPCreate(PETSC_COMM_WORLD, &ksp); CHKERRQ(ierr);
  ierr = KSPSetType(ksp,KSPFGMRES); CHKERRQ(ierr);
  // Setting KSP operators
  ierr = KSPSetOperators(ksp, A_nest, A_nest); CHKERRQ(ierr);
  // Set KSP and PC options
  ierr = KSPGetPC(ksp, &pc); CHKERRQ(ierr);
  PCSetType(pc,PCFIELDSPLIT);
  ierr = KSPSetTolerances(ksp,_rtol, _atol, _dtol, _maxit); CHKERRQ(ierr);
  // Splitting fields
  std::vector<IS> rows(Q);
  //IS rows[Q];
  PetscInt M = 0;
  ierr = MatNestGetISs(A_nest,rows.data(),NULL); CHKERRQ(ierr);
  for (unsigned int j=0; j<Q; ++j) {
    IS expand1;
    ISDuplicate(rows[M],&expand1);
    M += 1;
    PCFieldSplitSetIS(pc,NULL,expand1);
    ISDestroy(&expand1);
  }
  if (_verbose_subchannel)
    _console << "Linear solver assembled." << std::endl;

  /// Solving
  ierr = VecDuplicate(b_nest,&x_nest); CHKERRQ(ierr);
  ierr = VecSet(x_nest, 0.0); CHKERRQ(ierr);
  ierr = KSPSolve(ksp, b_nest, x_nest); CHKERRQ(ierr);

  /// Destroying solver elements
  ierr = VecDestroy(&b_nest);CHKERRQ(ierr);
  ierr = MatDestroy(&A_nest);CHKERRQ(ierr);
  ierr = KSPDestroy(&ksp);CHKERRQ(ierr);
  for (unsigned int i = 0; i < Q*Q; i++)
  {
    ierr = MatDestroy(&mat_array[i]);CHKERRQ(ierr);
  }
  for (unsigned int i = 0; i < Q; i++)
  {
    ierr = VecDestroy(&vec_array[i]);CHKERRQ(ierr);
  }
  if (_verbose_subchannel)
    _console << "Solver elements destroyed." << std::endl;

  /// Recovering the solutions
  Vec sol_mdot, sol_p, sol_Wij;
  if (_verbose_subchannel)
    _console << "Vectors created." << std::endl;
  PetscInt num_vecs;
  Vec *loc_vecs;
  ierr = VecNestGetSubVecs(x_nest,&num_vecs,&loc_vecs); CHKERRQ(ierr);
  if (_verbose_subchannel)
    _console << "Starting extraction." << std::endl;
  VecDuplicate(mc_axial_convection_rhs,&sol_mdot);
  VecCopy(loc_vecs[0],sol_mdot);
  ierr = VecDuplicate(amc_sys_mdot_rhs,&sol_p); CHKERRQ(ierr);
  ierr = VecCopy(loc_vecs[1],sol_p); CHKERRQ(ierr);
  ierr = VecDuplicate(cmc_sys_Wij_rhs,&sol_Wij); CHKERRQ(ierr);
  ierr = VecCopy(loc_vecs[2],sol_Wij); CHKERRQ(ierr);
  if (_verbose_subchannel)
    _console << "Getting individual field solutions from coupled solver." << std::endl;

  /// Assigning the solutions to arrays
  PetscScalar * sol_mdot_array; VecGetArray(sol_mdot, &sol_mdot_array);
  PetscScalar * sol_p_array; VecGetArray(sol_p, &sol_p_array);
  PetscScalar * sol_Wij_array; VecGetArray(sol_Wij, &sol_Wij_array);

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    auto iz_ind = iz - first_node;
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      PetscScalar value = sol_mdot_array[iz_ind*_n_channels + i_ch];
      _mdot_soln->set(node_out, value);
    }
  }

  for (unsigned int iz = last_node; iz > first_node - 1; iz--)
  {
    auto iz_ind = iz - first_node;
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz-1);
      PetscScalar value = sol_p_array[iz_ind*_n_channels + i_ch];
      _P_soln->set(node_in, value);
    }
  }

  for (unsigned int iz = first_node+1; iz < last_node + 1; iz++)
  {
    auto iz_ind = iz - first_node - 1;
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      _Wij(i_gap, iz - 1 - iblock * _block_size) =
          sol_Wij_array[iz_ind*_n_gaps + i_gap];
    }
  }

  if(_monolithic_thermal_bool)
  {
    Vec sol_h;
    ierr = VecDuplicate(cmc_sys_Wij_rhs,&sol_h); CHKERRQ(ierr);
    ierr = VecCopy(loc_vecs[3],sol_h); CHKERRQ(ierr);
    PetscScalar * sol_h_array; VecGetArray(sol_h, &sol_h_array);

    for (unsigned int iz = first_node; iz < last_node + 1; iz++)
    {
      auto iz_ind = iz - first_node;
      for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
      {
        auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);

        if (sol_h_array[iz_ind * _n_channels + i_ch] < 0)
        {
          _console << "Wij = : " << _Wij << "\n";
          mooseError(name(),
                     " : Calculation of negative Enthalpy h_out = : ",
                     sol_h_array[iz_ind * _n_channels + i_ch],
                     " Axial Level= : ",
                     iz);
        }
        _h_soln->set(node_out, sol_h_array[iz_ind * _n_channels + i_ch]);
      }
    }

    ierr = VecDestroy(&sol_h);CHKERRQ(ierr);
  }

  // Populating sum_Wij
  MatMult(mc_sumWij_mat, sol_Wij, prod);
  PetscScalar * xx;
  VecGetArray(prod, &xx);
  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    unsigned int iz_ind = iz - first_node;
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      PetscScalar value = xx[iz_ind*_n_channels + i_ch];
      _SumWij_soln->set(node_out, value);
    }
  }

  Vec sumWij_loc; createPetscVector(sumWij_loc, _block_size * _n_channels);
  populateVectorFromHandle<SolutionHandle *>(prod, _SumWij_soln, first_node, last_node, _n_channels);
  PetscScalar max_sumWij_new; VecAbs(prod);
  ierr = VecMax(prod, NULL, &max_sumWij_new); CHKERRQ(ierr);
  if (_verbose_subchannel)
    _console << "Maximum estimated Wij new: " << max_sumWij_new << std::endl;
  correction_factor = max_sumWij_new/max_sumWij;
  if (_verbose_subchannel)
    _console << "Correction factor: " << correction_factor << std::endl;
  if (_verbose_subchannel)
    _console << "Solutions assigned to MOOSE variables." << std::endl;

  /// Destroying arrays
  ierr = VecDestroy(&x_nest);CHKERRQ(ierr);
  ierr = VecDestroy(&sol_mdot);CHKERRQ(ierr);
  ierr = VecDestroy(&sol_p);CHKERRQ(ierr);
  ierr = VecDestroy(&sol_Wij);CHKERRQ(ierr);
  if (_verbose_subchannel)
    _console << "Solutions destroyed." << std::endl;

  return ierr;

}

void
SubChannel1PhaseProblem::initializeSolution()
{

  unsigned int last_node = _n_cells;
  unsigned int first_node = 1;

  for (unsigned int iz = first_node; iz < last_node + 1; iz++)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_out = _subchannel_mesh.getChannelNode(i_ch, iz);
      auto * node_in = _subchannel_mesh.getChannelNode(i_ch, iz-1);
      _mdot_soln->set(node_out, (*_mdot_soln)(node_in));
      _h_soln->set(node_out, (*_h_soln)(node_in));
    }
  }

  for (unsigned int iz = last_node; iz > first_node - 1; iz--)
  {
    for (unsigned int i_ch = 0; i_ch < _n_channels; i_ch++)
    {
      auto * node_bottom = _subchannel_mesh.getChannelNode(i_ch, iz-1);
      auto * node_top = _subchannel_mesh.getChannelNode(i_ch, iz-1);
      _P_soln->set(node_bottom, (*_P_soln)(node_top));
    }
  }

  for (unsigned int iz = first_node+1; iz < last_node + 1; iz++)
  {
    for (unsigned int i_gap = 0; i_gap < _n_gaps; i_gap++)
    {
      _Wij(i_gap, iz) = _Wij(i_gap, iz-1);
    }
  }

}

void
SubChannel1PhaseProblem::externalSolve()
{
  _console << "Executing subchannel solver\n";
  auto P_error = 1.0;
  unsigned int P_it = 0;
  unsigned int P_it_max;

  if (_segregated_bool)
    P_it_max = 2 * _n_blocks;
  else
    P_it_max = 100;

  if ((_n_blocks == 1) && (_segregated_bool))
    P_it_max = 1;
  if (! _segregated_bool)
  {
    initializeSolution();
    if (_verbose_subchannel)
      _console << "Solution initialized" << std::endl;
  }
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

        if(_segregated_bool)
        {
          computeWijFromSolve(iblock);
          if (_compute_power)
          {
            computeh(iblock);
            computeT(iblock);
          }
        }
        else
        {
          if (_monolithic_thermal_bool)
          {
            implicitPetscSolve(iblock);
            computeT(iblock);
          }
          else
          {
            implicitPetscSolve(iblock);
            if (_verbose_subchannel)
              _console << "Done with main solve." << std::endl;
            if (_compute_power)
            {
              if (_verbose_subchannel)
                _console << "Starting enthalpy solve." << std::endl;
              computeh(iblock);
              if (_verbose_subchannel)
                _console << "Done with enthalpy solve." << std::endl;
              computeT(iblock);
            }
            if (_verbose_subchannel)
              _console << "Done with thermal solve." << std::endl;
          }
        }

        if (_compute_density)
          computeRho(iblock);

        if (_compute_viscosity)
          computeMu(iblock);

        if (_verbose_subchannel)
          _console << "Done updating thermophysical properties." << std::endl;

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
    if (_verbose_subchannel)
    {
      _console << "Iteration:  " << P_it << std::endl;
      _console << "Maximum iterations: " << P_it_max << std::endl;
    }
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
  _aux->update();
}

void SubChannel1PhaseProblem::syncSolutions(Direction /*direction*/) {}
