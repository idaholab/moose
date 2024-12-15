//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "PetscSupport.h"
#include "Factory.h"
#include "ParallelUniqueId.h"
#include "ThreadedElementLoop.h"
#include "MaterialData.h"
#include "ComputeResidualThread.h"
#include "ComputeResidualAndJacobianThread.h"
#include "ComputeFVFluxThread.h"
#include "ComputeJacobianThread.h"
#include "ComputeJacobianForScalingThread.h"
#include "ComputeFullJacobianThread.h"
#include "ComputeJacobianBlocksThread.h"
#include "ComputeDiracThread.h"
#include "ComputeElemDampingThread.h"
#include "ComputeNodalDampingThread.h"
#include "ComputeNodalKernelsThread.h"
#include "ComputeNodalKernelBcsThread.h"
#include "ComputeNodalKernelJacobiansThread.h"
#include "ComputeNodalKernelBCJacobiansThread.h"
#include "TimeKernel.h"
#include "BoundaryCondition.h"
#include "DirichletBCBase.h"
#include "NodalBCBase.h"
#include "IntegratedBCBase.h"
#include "DGKernel.h"
#include "InterfaceKernelBase.h"
#include "ElementDamper.h"
#include "NodalDamper.h"
#include "GeneralDamper.h"
#include "DisplacedProblem.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "NodeElemConstraint.h"
#include "MortarConstraint.h"
#include "ElemElemConstraint.h"
#include "ScalarKernelBase.h"
#include "Parser.h"
#include "Split.h"
#include "FieldSplitPreconditioner.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "NodalKernelBase.h"
#include "DiracKernelBase.h"
#include "TimeIntegrator.h"
#include "Predictor.h"
#include "Assembly.h"
#include "ElementPairLocator.h"
#include "ODETimeKernel.h"
#include "AllLocalDofIndicesThread.h"
#include "FloatingPointExceptionGuard.h"
#include "ADKernel.h"
#include "ADDirichletBCBase.h"
#include "Moose.h"
#include "ConsoleStream.h"
#include "MooseError.h"
#include "FVElementalKernel.h"
#include "FVScalarLagrangeMultiplierConstraint.h"
#include "FVBoundaryScalarLagrangeMultiplierConstraint.h"
#include "FVFluxKernel.h"
#include "FVScalarLagrangeMultiplierInterface.h"
#include "UserObject.h"
#include "OffDiagonalScalingMatrix.h"
#include "HDGKernel.h"
#include "HDGIntegratedBC.h"

// libMesh
#include "libmesh/nonlinear_solver.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/dense_vector.h"
#include "libmesh/boundary_info.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/mesh.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/dense_submatrix.h"
#include "libmesh/dof_map.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/default_coupling.h"
#include "libmesh/diagonal_matrix.h"
#include "libmesh/fe_interface.h"
#include "libmesh/petsc_solver_exception.h"

#include <ios>

#include "petscsnes.h"
#include <PetscDMMoose.h>
EXTERN_C_BEGIN
extern PetscErrorCode DMCreate_Moose(DM);
EXTERN_C_END

using namespace libMesh;

NonlinearSystemBase::NonlinearSystemBase(FEProblemBase & fe_problem,
                                         System & sys,
                                         const std::string & name)
  : SolverSystem(fe_problem, fe_problem, name, Moose::VAR_SOLVER),
    PerfGraphInterface(fe_problem.getMooseApp().perfGraph(), "NonlinearSystemBase"),
    _sys(sys),
    _last_nl_rnorm(0.),
    _current_nl_its(0),
    _residual_ghosted(NULL),
    _Re_time_tag(-1),
    _Re_time(NULL),
    _Re_non_time_tag(-1),
    _Re_non_time(NULL),
    _scalar_kernels(/*threaded=*/false),
    _nodal_bcs(/*threaded=*/false),
    _preset_nodal_bcs(/*threaded=*/false),
    _ad_preset_nodal_bcs(/*threaded=*/false),
    _splits(/*threaded=*/false),
    _increment_vec(NULL),
    _use_finite_differenced_preconditioner(false),
    _fdcoloring(nullptr),
    _have_decomposition(false),
    _use_field_split_preconditioner(false),
    _add_implicit_geometric_coupling_entries_to_jacobian(false),
    _assemble_constraints_separately(false),
    _need_residual_ghosted(false),
    _debugging_residuals(false),
    _doing_dg(false),
    _n_iters(0),
    _n_linear_iters(0),
    _n_residual_evaluations(0),
    _final_residual(0.),
    _computing_pre_smo_residual(false),
    _pre_smo_residual(0),
    _initial_residual(0),
    _use_pre_smo_residual(false),
    _print_all_var_norms(false),
    _has_save_in(false),
    _has_diag_save_in(false),
    _has_nodalbc_save_in(false),
    _has_nodalbc_diag_save_in(false),
    _computed_scaling(false),
    _compute_scaling_once(true),
    _resid_vs_jac_scaling_param(0),
    _off_diagonals_in_auto_scaling(false),
    _auto_scaling_initd(false)
{
  getResidualNonTimeVector();
  // Don't need to add the matrix - it already exists (for now)
  _Ke_system_tag = _fe_problem.addMatrixTag("SYSTEM");

  // The time matrix tag is not normally used - but must be added to the system
  // in case it is so that objects can have 'time' in their matrix tags by default
  _fe_problem.addMatrixTag("TIME");

  _Re_tag = _fe_problem.addVectorTag("RESIDUAL");

  _sys.identify_variable_groups(_fe_problem.identifyVariableGroupsInNL());

  if (!_fe_problem.defaultGhosting())
  {
    auto & dof_map = _sys.get_dof_map();
    dof_map.remove_algebraic_ghosting_functor(dof_map.default_algebraic_ghosting());
    dof_map.set_implicit_neighbor_dofs(false);
  }
}

NonlinearSystemBase::~NonlinearSystemBase() = default;

void
NonlinearSystemBase::preInit()
{
  SolverSystem::preInit();

  if (_fe_problem.hasDampers())
    setupDampers();

  if (_residual_copy.get())
    _residual_copy->init(_sys.n_dofs(), false, SERIAL);
}

void
NonlinearSystemBase::turnOffJacobian()
{
  system().set_basic_system_only();
  nonlinearSolver()->jacobian = NULL;
}

void
NonlinearSystemBase::initialSetup()
{
  TIME_SECTION("nlInitialSetup", 2, "Setting Up Nonlinear System");

  SolverSystem::initialSetup();

  {
    TIME_SECTION("kernelsInitialSetup", 2, "Setting Up Kernels/BCs/Constraints");

    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    {
      _kernels.initialSetup(tid);
      _nodal_kernels.initialSetup(tid);
      _dirac_kernels.initialSetup(tid);
      if (_doing_dg)
        _dg_kernels.initialSetup(tid);
      _interface_kernels.initialSetup(tid);

      _element_dampers.initialSetup(tid);
      _nodal_dampers.initialSetup(tid);
      _integrated_bcs.initialSetup(tid);

      if (_fe_problem.haveFV())
      {
        std::vector<FVElementalKernel *> fv_elemental_kernels;
        _fe_problem.theWarehouse()
            .query()
            .template condition<AttribSystem>("FVElementalKernel")
            .template condition<AttribThread>(tid)
            .queryInto(fv_elemental_kernels);

        for (auto * fv_kernel : fv_elemental_kernels)
          fv_kernel->initialSetup();

        std::vector<FVFluxKernel *> fv_flux_kernels;
        _fe_problem.theWarehouse()
            .query()
            .template condition<AttribSystem>("FVFluxKernel")
            .template condition<AttribThread>(tid)
            .queryInto(fv_flux_kernels);

        for (auto * fv_kernel : fv_flux_kernels)
          fv_kernel->initialSetup();
      }
    }

    _scalar_kernels.initialSetup();
    _constraints.initialSetup();
    _general_dampers.initialSetup();
    _nodal_bcs.initialSetup();
  }

  {
    TIME_SECTION("mortarSetup", 2, "Initializing Mortar Interfaces");

    auto create_mortar_functors = [this](const bool displaced)
    {
      // go over mortar interfaces and construct functors
      const auto & mortar_interfaces = _fe_problem.getMortarInterfaces(displaced);
      for (const auto & mortar_interface : mortar_interfaces)
      {
        const auto primary_secondary_boundary_pair = mortar_interface.first;
        if (!_constraints.hasActiveMortarConstraints(primary_secondary_boundary_pair, displaced))
          continue;

        const auto & mortar_generation_object = mortar_interface.second;

        auto & mortar_constraints =
            _constraints.getActiveMortarConstraints(primary_secondary_boundary_pair, displaced);

        auto & subproblem = displaced
                                ? static_cast<SubProblem &>(*_fe_problem.getDisplacedProblem())
                                : static_cast<SubProblem &>(_fe_problem);

        auto & mortar_functors =
            displaced ? _displaced_mortar_functors : _undisplaced_mortar_functors;

        mortar_functors.emplace(primary_secondary_boundary_pair,
                                ComputeMortarFunctor(mortar_constraints,
                                                     mortar_generation_object,
                                                     subproblem,
                                                     _fe_problem,
                                                     displaced,
                                                     subproblem.assembly(0, number())));
      }
    };

    create_mortar_functors(false);
    create_mortar_functors(true);
  }

  if (_automatic_scaling)
  {
    if (_off_diagonals_in_auto_scaling)
      _scaling_matrix = std::make_unique<OffDiagonalScalingMatrix<Number>>(_communicator);
    else
      _scaling_matrix = std::make_unique<DiagonalMatrix<Number>>(_communicator);
  }
}

void
NonlinearSystemBase::timestepSetup()
{
  SolverSystem::timestepSetup();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.timestepSetup(tid);
    _nodal_kernels.timestepSetup(tid);
    _dirac_kernels.timestepSetup(tid);
    if (_doing_dg)
      _dg_kernels.timestepSetup(tid);
    _interface_kernels.timestepSetup(tid);
    _element_dampers.timestepSetup(tid);
    _nodal_dampers.timestepSetup(tid);
    _integrated_bcs.timestepSetup(tid);

    if (_fe_problem.haveFV())
    {
      std::vector<FVFluxBC *> bcs;
      _fe_problem.theWarehouse()
          .query()
          .template condition<AttribSystem>("FVFluxBC")
          .template condition<AttribThread>(tid)
          .queryInto(bcs);

      std::vector<FVInterfaceKernel *> iks;
      _fe_problem.theWarehouse()
          .query()
          .template condition<AttribSystem>("FVInterfaceKernel")
          .template condition<AttribThread>(tid)
          .queryInto(iks);

      std::vector<FVFluxKernel *> kernels;
      _fe_problem.theWarehouse()
          .query()
          .template condition<AttribSystem>("FVFluxKernel")
          .template condition<AttribThread>(tid)
          .queryInto(kernels);

      for (auto * bc : bcs)
        bc->timestepSetup();
      for (auto * ik : iks)
        ik->timestepSetup();
      for (auto * kernel : kernels)
        kernel->timestepSetup();
    }
  }
  _scalar_kernels.timestepSetup();
  _constraints.timestepSetup();
  _general_dampers.timestepSetup();
  _nodal_bcs.timestepSetup();
}

void
NonlinearSystemBase::customSetup(const ExecFlagType & exec_type)
{
  SolverSystem::customSetup(exec_type);

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.customSetup(exec_type, tid);
    _nodal_kernels.customSetup(exec_type, tid);
    _dirac_kernels.customSetup(exec_type, tid);
    if (_doing_dg)
      _dg_kernels.customSetup(exec_type, tid);
    _interface_kernels.customSetup(exec_type, tid);
    _element_dampers.customSetup(exec_type, tid);
    _nodal_dampers.customSetup(exec_type, tid);
    _integrated_bcs.customSetup(exec_type, tid);

    if (_fe_problem.haveFV())
    {
      std::vector<FVFluxBC *> bcs;
      _fe_problem.theWarehouse()
          .query()
          .template condition<AttribSystem>("FVFluxBC")
          .template condition<AttribThread>(tid)
          .queryInto(bcs);

      std::vector<FVInterfaceKernel *> iks;
      _fe_problem.theWarehouse()
          .query()
          .template condition<AttribSystem>("FVInterfaceKernel")
          .template condition<AttribThread>(tid)
          .queryInto(iks);

      std::vector<FVFluxKernel *> kernels;
      _fe_problem.theWarehouse()
          .query()
          .template condition<AttribSystem>("FVFluxKernel")
          .template condition<AttribThread>(tid)
          .queryInto(kernels);

      for (auto * bc : bcs)
        bc->customSetup(exec_type);
      for (auto * ik : iks)
        ik->customSetup(exec_type);
      for (auto * kernel : kernels)
        kernel->customSetup(exec_type);
    }
  }
  _scalar_kernels.customSetup(exec_type);
  _constraints.customSetup(exec_type);
  _general_dampers.customSetup(exec_type);
  _nodal_bcs.customSetup(exec_type);
}

void
NonlinearSystemBase::setupDM()
{
  if (haveFieldSplitPreconditioner())
    Moose::PetscSupport::petscSetupDM(*this, _decomposition_split);
}

void
NonlinearSystemBase::setDecomposition(const std::vector<std::string> & splits)
{
  /// Although a single top-level split is allowed in Problem, treat it as a list of splits for conformity with the Split input syntax.
  if (splits.size() && splits.size() != 1)
    mooseError("Only a single top-level split is allowed in a Problem's decomposition.");

  if (splits.size())
  {
    _decomposition_split = splits[0];
    _have_decomposition = true;
  }
  else
    _have_decomposition = false;
}

void
NonlinearSystemBase::setupFieldDecomposition()
{
  if (!_have_decomposition)
    return;

  std::shared_ptr<Split> top_split = getSplit(_decomposition_split);
  top_split->setup(*this);
}

void
NonlinearSystemBase::addKernel(const std::string & kernel_name,
                               const std::string & name,
                               InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to warehouse
    std::shared_ptr<KernelBase> kernel =
        _factory.create<KernelBase>(kernel_name, name, parameters, tid);
    _kernels.addObject(kernel, tid);
    postAddResidualObject(*kernel);
  }

  if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
NonlinearSystemBase::addHDGKernel(const std::string & kernel_name,
                                  const std::string & name,
                                  InputParameters & parameters)
{
  // The hybridized objects require that the residual and Jacobian be computed together
  residualAndJacobianTogether();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<MooseObjectWarehouse<HDGIntegratedBC> *>("hibc_warehouse") = &_hybridized_ibcs;
    // Create the kernel object via the factory and add to warehouse
    auto kernel = _factory.create<HDGKernel>(kernel_name, name, parameters, tid);
    _kernels.addObject(kernel, tid);
    _hybridized_kernels.addObject(kernel, tid);
    postAddResidualObject(*kernel);
  }
}

void
NonlinearSystemBase::addHDGIntegratedBC(const std::string & bc_name,
                                        const std::string & name,
                                        InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the bc object via the factory and add to warehouse
    auto bc = _factory.create<HDGIntegratedBC>(bc_name, name, parameters, tid);
    _hybridized_ibcs.addObject(bc, tid);
  }
}

void
NonlinearSystemBase::addNodalKernel(const std::string & kernel_name,
                                    const std::string & name,
                                    InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to the warehouse
    std::shared_ptr<NodalKernelBase> kernel =
        _factory.create<NodalKernelBase>(kernel_name, name, parameters, tid);
    _nodal_kernels.addObject(kernel, tid);
    postAddResidualObject(*kernel);
  }

  if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
NonlinearSystemBase::addScalarKernel(const std::string & kernel_name,
                                     const std::string & name,
                                     InputParameters & parameters)
{
  std::shared_ptr<ScalarKernelBase> kernel =
      _factory.create<ScalarKernelBase>(kernel_name, name, parameters);
  postAddResidualObject(*kernel);
  _scalar_kernels.addObject(kernel);
}

void
NonlinearSystemBase::addBoundaryCondition(const std::string & bc_name,
                                          const std::string & name,
                                          InputParameters & parameters)
{
  // ThreadID
  THREAD_ID tid = 0;

  // Create the object
  std::shared_ptr<BoundaryCondition> bc =
      _factory.create<BoundaryCondition>(bc_name, name, parameters, tid);
  postAddResidualObject(*bc);

  // Active BoundaryIDs for the object
  const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  auto bc_var = dynamic_cast<const MooseVariableFieldBase *>(&bc->variable());
  _vars[tid].addBoundaryVar(boundary_ids, bc_var);

  // Cast to the various types of BCs
  std::shared_ptr<NodalBCBase> nbc = std::dynamic_pointer_cast<NodalBCBase>(bc);
  std::shared_ptr<IntegratedBCBase> ibc = std::dynamic_pointer_cast<IntegratedBCBase>(bc);

  // NodalBCBase
  if (nbc)
  {
    if (nbc->checkNodalVar() && !nbc->variable().isNodal())
      mooseError("Trying to use nodal boundary condition '",
                 nbc->name(),
                 "' on a non-nodal variable '",
                 nbc->variable().name(),
                 "'.");

    _nodal_bcs.addObject(nbc);
    _vars[tid].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

    if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
      _has_nodalbc_save_in = true;
    if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
      _has_nodalbc_diag_save_in = true;

    // DirichletBCs that are preset
    std::shared_ptr<DirichletBCBase> dbc = std::dynamic_pointer_cast<DirichletBCBase>(bc);
    if (dbc && dbc->preset())
      _preset_nodal_bcs.addObject(dbc);

    std::shared_ptr<ADDirichletBCBase> addbc = std::dynamic_pointer_cast<ADDirichletBCBase>(bc);
    if (addbc && addbc->preset())
      _ad_preset_nodal_bcs.addObject(addbc);
  }

  // IntegratedBCBase
  else if (ibc)
  {
    _integrated_bcs.addObject(ibc, tid);
    _vars[tid].addBoundaryVars(boundary_ids, ibc->getCoupledVars());

    if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
      _has_save_in = true;
    if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
      _has_diag_save_in = true;

    for (tid = 1; tid < libMesh::n_threads(); tid++)
    {
      // Create the object
      bc = _factory.create<BoundaryCondition>(bc_name, name, parameters, tid);

      // Give users opportunity to set some parameters
      postAddResidualObject(*bc);

      // Active BoundaryIDs for the object
      const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
      _vars[tid].addBoundaryVar(boundary_ids, bc_var);

      ibc = std::static_pointer_cast<IntegratedBCBase>(bc);

      _integrated_bcs.addObject(ibc, tid);
      _vars[tid].addBoundaryVars(boundary_ids, ibc->getCoupledVars());
    }
  }

  else
    mooseError("Unknown BoundaryCondition type for object named ", bc->name());
}

void
NonlinearSystemBase::addConstraint(const std::string & c_name,
                                   const std::string & name,
                                   InputParameters & parameters)
{
  std::shared_ptr<Constraint> constraint = _factory.create<Constraint>(c_name, name, parameters);
  _constraints.addObject(constraint);
  postAddResidualObject(*constraint);

  if (constraint && constraint->addCouplingEntriesToJacobian())
    addImplicitGeometricCouplingEntriesToJacobian(true);
}

void
NonlinearSystemBase::addDiracKernel(const std::string & kernel_name,
                                    const std::string & name,
                                    InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<DiracKernelBase> kernel =
        _factory.create<DiracKernelBase>(kernel_name, name, parameters, tid);
    postAddResidualObject(*kernel);
    _dirac_kernels.addObject(kernel, tid);
  }
}

void
NonlinearSystemBase::addDGKernel(std::string dg_kernel_name,
                                 const std::string & name,
                                 InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    auto dg_kernel = _factory.create<DGKernelBase>(dg_kernel_name, name, parameters, tid);
    _dg_kernels.addObject(dg_kernel, tid);
    postAddResidualObject(*dg_kernel);
  }

  _doing_dg = true;

  if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
NonlinearSystemBase::addInterfaceKernel(std::string interface_kernel_name,
                                        const std::string & name,
                                        InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::shared_ptr<InterfaceKernelBase> interface_kernel =
        _factory.create<InterfaceKernelBase>(interface_kernel_name, name, parameters, tid);
    postAddResidualObject(*interface_kernel);

    const std::set<BoundaryID> & boundary_ids = interface_kernel->boundaryIDs();
    auto ik_var = dynamic_cast<const MooseVariableFieldBase *>(&interface_kernel->variable());
    _vars[tid].addBoundaryVar(boundary_ids, ik_var);

    _interface_kernels.addObject(interface_kernel, tid);
    _vars[tid].addBoundaryVars(boundary_ids, interface_kernel->getCoupledVars());
  }
}

void
NonlinearSystemBase::addDamper(const std::string & damper_name,
                               const std::string & name,
                               InputParameters & parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::shared_ptr<Damper> damper = _factory.create<Damper>(damper_name, name, parameters, tid);

    // Attempt to cast to the damper types
    std::shared_ptr<ElementDamper> ed = std::dynamic_pointer_cast<ElementDamper>(damper);
    std::shared_ptr<NodalDamper> nd = std::dynamic_pointer_cast<NodalDamper>(damper);
    std::shared_ptr<GeneralDamper> gd = std::dynamic_pointer_cast<GeneralDamper>(damper);

    if (gd)
    {
      _general_dampers.addObject(gd);
      break; // not threaded
    }
    else if (ed)
      _element_dampers.addObject(ed, tid);
    else if (nd)
      _nodal_dampers.addObject(nd, tid);
    else
      mooseError("Invalid damper type");
  }
}

void
NonlinearSystemBase::addSplit(const std::string & split_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  std::shared_ptr<Split> split = _factory.create<Split>(split_name, name, parameters);
  _splits.addObject(split);
}

std::shared_ptr<Split>
NonlinearSystemBase::getSplit(const std::string & name)
{
  return _splits.getActiveObject(name);
}

bool
NonlinearSystemBase::shouldEvaluatePreSMOResidual() const
{
  if (_fe_problem.solverParams()._type == Moose::ST_LINEAR)
    return false;

  // The legacy behavior (#10464) _always_ performs the pre-SMO residual evaluation
  // regardless of whether it is needed.
  //
  // This is not ideal and has been fixed by #23472. This legacy option ensures a smooth transition
  // to the new behavior. Modules and Apps that want to migrate to the new behavior should set this
  // parameter to false.
  if (_app.parameters().get<bool>("use_legacy_initial_residual_evaluation_behavior"))
    return true;

  return _use_pre_smo_residual;
}

Real
NonlinearSystemBase::referenceResidual() const
{
  return usePreSMOResidual() ? preSMOResidual() : initialResidual();
}

Real
NonlinearSystemBase::preSMOResidual() const
{
  if (!shouldEvaluatePreSMOResidual())
    mooseError("pre-SMO residual is requested but not evaluated.");

  return _pre_smo_residual;
}

Real
NonlinearSystemBase::initialResidual() const
{
  return _initial_residual;
}

void
NonlinearSystemBase::setInitialResidual(Real r)
{
  _initial_residual = r;
}

void
NonlinearSystemBase::zeroVectorForResidual(const std::string & vector_name)
{
  for (unsigned int i = 0; i < _vecs_to_zero_for_residual.size(); ++i)
    if (vector_name == _vecs_to_zero_for_residual[i])
      return;

  _vecs_to_zero_for_residual.push_back(vector_name);
}

void
NonlinearSystemBase::computeResidualTag(NumericVector<Number> & residual, TagID tag_id)
{
  _nl_vector_tags.clear();
  _nl_vector_tags.insert(tag_id);
  _nl_vector_tags.insert(residualVectorTag());

  associateVectorToTag(residual, residualVectorTag());

  computeResidualTags(_nl_vector_tags);

  disassociateVectorFromTag(residual, residualVectorTag());
}

void
NonlinearSystemBase::computeResidual(NumericVector<Number> & residual, TagID tag_id)
{
  mooseDeprecated(" Please use computeResidualTag");

  computeResidualTag(residual, tag_id);
}

void
NonlinearSystemBase::computeResidualTags(const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("nl::computeResidualTags", 5);

  _fe_problem.setCurrentNonlinearSystem(number());
  _fe_problem.setCurrentlyComputingResidual(true);

  bool required_residual = tags.find(residualVectorTag()) == tags.end() ? false : true;

  _n_residual_evaluations++;

  // not suppose to do anythin on matrix
  deactiveAllMatrixTags();

  FloatingPointExceptionGuard fpe_guard(_app);

  for (const auto & numeric_vec : _vecs_to_zero_for_residual)
    if (hasVector(numeric_vec))
    {
      NumericVector<Number> & vec = getVector(numeric_vec);
      vec.close();
      vec.zero();
    }

  try
  {
    zeroTaggedVectors(tags);
    computeResidualInternal(tags);
    closeTaggedVectors(tags);

    if (required_residual)
    {
      auto & residual = getVector(residualVectorTag());
      if (!_time_integrators.empty())
      {
        for (auto & ti : _time_integrators)
          ti->postResidual(residual);
      }
      else
        residual += *_Re_non_time;
      residual.close();
    }
    if (_fe_problem.computingScalingResidual())
      // We don't want to do nodal bcs or anything else
      return;

    computeNodalBCs(tags);
    closeTaggedVectors(tags);

    // If we are debugging residuals we need one more assignment to have the ghosted copy up to
    // date
    if (_need_residual_ghosted && _debugging_residuals && required_residual)
    {
      auto & residual = getVector(residualVectorTag());

      *_residual_ghosted = residual;
      _residual_ghosted->close();
    }
    // Need to close and update the aux system in case residuals were saved to it.
    if (_has_nodalbc_save_in)
      _fe_problem.getAuxiliarySystem().solution().close();
    if (hasSaveIn())
      _fe_problem.getAuxiliarySystem().update();
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }

  // not supposed to do anything on matrix
  activeAllMatrixTags();

  _fe_problem.setCurrentlyComputingResidual(false);
}

void
NonlinearSystemBase::computeResidualAndJacobianTags(const std::set<TagID> & vector_tags,
                                                    const std::set<TagID> & matrix_tags)
{
  const bool required_residual =
      vector_tags.find(residualVectorTag()) == vector_tags.end() ? false : true;

  try
  {
    zeroTaggedVectors(vector_tags);
    computeResidualAndJacobianInternal(vector_tags, matrix_tags);
    closeTaggedVectors(vector_tags);
    closeTaggedMatrices(matrix_tags);

    if (required_residual)
    {
      auto & residual = getVector(residualVectorTag());
      if (!_time_integrators.empty())
      {
        for (auto & ti : _time_integrators)
          ti->postResidual(residual);
      }
      else
        residual += *_Re_non_time;
      residual.close();
    }

    computeNodalBCsResidualAndJacobian();
    closeTaggedVectors(vector_tags);
    closeTaggedMatrices(matrix_tags);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
}

void
NonlinearSystemBase::onTimestepBegin()
{
  for (auto & ti : _time_integrators)
    ti->preSolve();
  if (_predictor.get())
    _predictor->timestepSetup();
}

void
NonlinearSystemBase::setInitialSolution()
{
  deactiveAllMatrixTags();

  NumericVector<Number> & initial_solution(solution());
  if (_predictor.get())
  {
    if (_predictor->shouldApply())
    {
      TIME_SECTION("applyPredictor", 2, "Applying Predictor");

      _predictor->apply(initial_solution);
      _fe_problem.predictorCleanup(initial_solution);
    }
    else
      _console << " Skipping predictor this step" << std::endl;
  }

  // do nodal BC
  {
    TIME_SECTION("initialBCs", 2, "Applying BCs To Initial Condition");

    const ConstBndNodeRange & bnd_nodes = _fe_problem.getCurrentAlgebraicBndNodeRange();
    for (const auto & bnode : bnd_nodes)
    {
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      if (node->processor_id() == processor_id())
      {
        // reinit variables in nodes
        _fe_problem.reinitNodeFace(node, boundary_id, 0);

        if (_preset_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
        {
          const auto & preset_bcs = _preset_nodal_bcs.getActiveBoundaryObjects(boundary_id);
          for (const auto & preset_bc : preset_bcs)
            preset_bc->computeValue(initial_solution);
        }
        if (_ad_preset_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
        {
          const auto & preset_bcs_res = _ad_preset_nodal_bcs.getActiveBoundaryObjects(boundary_id);
          for (const auto & preset_bc : preset_bcs_res)
            preset_bc->computeValue(initial_solution);
        }
      }
    }
  }

  _sys.solution->close();
  update();

  // Set constraint secondary values
  setConstraintSecondaryValues(initial_solution, false);

  if (_fe_problem.getDisplacedProblem())
    setConstraintSecondaryValues(initial_solution, true);
}

void
NonlinearSystemBase::setPredictor(std::shared_ptr<Predictor> predictor)
{
  _predictor = predictor;
}

void
NonlinearSystemBase::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  SolverSystem::subdomainSetup();

  _kernels.subdomainSetup(subdomain, tid);
  _nodal_kernels.subdomainSetup(subdomain, tid);
  _element_dampers.subdomainSetup(subdomain, tid);
  _nodal_dampers.subdomainSetup(subdomain, tid);
}

NumericVector<Number> &
NonlinearSystemBase::getResidualTimeVector()
{
  if (!_Re_time)
  {
    _Re_time_tag = _fe_problem.addVectorTag("TIME");

    // Most applications don't need the expense of ghosting
    ParallelType ptype = _need_residual_ghosted ? GHOSTED : PARALLEL;
    _Re_time = &addVector(_Re_time_tag, false, ptype);
  }
  else if (_need_residual_ghosted && _Re_time->type() == PARALLEL)
  {
    const auto vector_name = _subproblem.vectorTagName(_Re_time_tag);

    // If an application changes its mind, the libMesh API lets us
    // change the vector.
    _Re_time = &system().add_vector(vector_name, false, GHOSTED);
  }

  return *_Re_time;
}

NumericVector<Number> &
NonlinearSystemBase::getResidualNonTimeVector()
{
  if (!_Re_non_time)
  {
    _Re_non_time_tag = _fe_problem.addVectorTag("NONTIME");

    // Most applications don't need the expense of ghosting
    ParallelType ptype = _need_residual_ghosted ? GHOSTED : PARALLEL;
    _Re_non_time = &addVector(_Re_non_time_tag, false, ptype);
  }
  else if (_need_residual_ghosted && _Re_non_time->type() == PARALLEL)
  {
    const auto vector_name = _subproblem.vectorTagName(_Re_non_time_tag);

    // If an application changes its mind, the libMesh API lets us
    // change the vector.
    _Re_non_time = &system().add_vector(vector_name, false, GHOSTED);
  }

  return *_Re_non_time;
}

NumericVector<Number> &
NonlinearSystemBase::residualVector(TagID tag)
{
  mooseDeprecated("Please use getVector()");
  switch (tag)
  {
    case 0:
      return getResidualNonTimeVector();

    case 1:
      return getResidualTimeVector();

    default:
      mooseError("The required residual vector is not available");
  }
}

void
NonlinearSystemBase::enforceNodalConstraintsResidual(NumericVector<Number> & residual)
{
  THREAD_ID tid = 0; // constraints are going to be done single-threaded
  residual.close();
  if (_constraints.hasActiveNodalConstraints())
  {
    const auto & ncs = _constraints.getActiveNodalConstraints();
    for (const auto & nc : ncs)
    {
      std::vector<dof_id_type> & secondary_node_ids = nc->getSecondaryNodeId();
      std::vector<dof_id_type> & primary_node_ids = nc->getPrimaryNodeId();

      if ((secondary_node_ids.size() > 0) && (primary_node_ids.size() > 0))
      {
        _fe_problem.reinitNodes(primary_node_ids, tid);
        _fe_problem.reinitNodesNeighbor(secondary_node_ids, tid);
        nc->computeResidual(residual);
      }
    }
    _fe_problem.addCachedResidualDirectly(residual, tid);
    residual.close();
  }
}

void
NonlinearSystemBase::enforceNodalConstraintsJacobian()
{
  if (!hasMatrix(systemMatrixTag()))
    mooseError(" A system matrix is required");

  auto & jacobian = getMatrix(systemMatrixTag());
  THREAD_ID tid = 0; // constraints are going to be done single-threaded

  if (_constraints.hasActiveNodalConstraints())
  {
    const auto & ncs = _constraints.getActiveNodalConstraints();
    for (const auto & nc : ncs)
    {
      std::vector<dof_id_type> & secondary_node_ids = nc->getSecondaryNodeId();
      std::vector<dof_id_type> & primary_node_ids = nc->getPrimaryNodeId();

      if ((secondary_node_ids.size() > 0) && (primary_node_ids.size() > 0))
      {
        _fe_problem.reinitNodes(primary_node_ids, tid);
        _fe_problem.reinitNodesNeighbor(secondary_node_ids, tid);
        nc->computeJacobian(jacobian);
      }
    }
    _fe_problem.addCachedJacobian(tid);
    jacobian.close();
  }
}

void
NonlinearSystemBase::reinitNodeFace(const Node & secondary_node,
                                    const BoundaryID secondary_boundary,
                                    const PenetrationInfo & info,
                                    const bool displaced)
{
  auto & subproblem = displaced ? static_cast<SubProblem &>(*_fe_problem.getDisplacedProblem())
                                : static_cast<SubProblem &>(_fe_problem);

  const Elem * primary_elem = info._elem;
  unsigned int primary_side = info._side_num;
  std::vector<Point> points;
  points.push_back(info._closest_point);

  // *These next steps MUST be done in this order!*
  // ADL: This is a Chesterton's fence situation. I don't know which calls exactly the above comment
  // is referring to. If I had to guess I would guess just the reinitNodeFace and prepareAssembly
  // calls since the former will size the variable's dof indices and then the latter will resize the
  // residual/Jacobian based off the variable's cached dof indices size

  // This reinits the variables that exist on the secondary node
  _fe_problem.reinitNodeFace(&secondary_node, secondary_boundary, 0);

  // This will set aside residual and jacobian space for the variables that have dofs on
  // the secondary node
  _fe_problem.prepareAssembly(0);

  _fe_problem.setNeighborSubdomainID(primary_elem, 0);

  //
  // Reinit material on undisplaced mesh
  //

  const Elem * const undisplaced_primary_elem =
      displaced ? _mesh.elemPtr(primary_elem->id()) : primary_elem;
  const Point undisplaced_primary_physical_point =
      [&points, displaced, primary_elem, undisplaced_primary_elem]()
  {
    if (displaced)
    {
      const Point reference_point =
          FEMap::inverse_map(primary_elem->dim(), primary_elem, points[0]);
      return FEMap::map(primary_elem->dim(), undisplaced_primary_elem, reference_point);
    }
    else
      // If our penetration locator is on the reference mesh, then our undisplaced
      // physical point is simply the point coming from the penetration locator
      return points[0];
  }();

  _fe_problem.reinitNeighborPhys(
      undisplaced_primary_elem, primary_side, {undisplaced_primary_physical_point}, 0);
  // Stateful material properties are only initialized for neighbor material data for internal faces
  // for discontinuous Galerkin methods or for conforming interfaces for interface kernels. We don't
  // have either of those use cases here where we likely have disconnected meshes
  _fe_problem.reinitMaterialsNeighbor(primary_elem->subdomain_id(), 0, /*swap_stateful=*/false);

  // Reinit points for constraint enforcement
  if (displaced)
    subproblem.reinitNeighborPhys(primary_elem, primary_side, points, 0);
}

void
NonlinearSystemBase::setConstraintSecondaryValues(NumericVector<Number> & solution, bool displaced)
{

  if (displaced)
    mooseAssert(_fe_problem.getDisplacedProblem(),
                "If we're calling this method with displaced = true, then we better well have a "
                "displaced problem");
  auto & subproblem = displaced ? static_cast<SubProblem &>(*_fe_problem.getDisplacedProblem())
                                : static_cast<SubProblem &>(_fe_problem);
  const auto & penetration_locators = subproblem.geomSearchData()._penetration_locators;

  bool constraints_applied = false;

  for (const auto & it : penetration_locators)
  {
    PenetrationLocator & pen_loc = *(it.second);

    std::vector<dof_id_type> & secondary_nodes = pen_loc._nearest_node._secondary_nodes;

    BoundaryID secondary_boundary = pen_loc._secondary_boundary;
    BoundaryID primary_boundary = pen_loc._primary_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(secondary_boundary, displaced))
    {
      const auto & constraints =
          _constraints.getActiveNodeFaceConstraints(secondary_boundary, displaced);
      std::unordered_set<unsigned int> needed_mat_props;
      for (const auto & constraint : constraints)
      {
        const auto & mp_deps = constraint->getMatPropDependencies();
        needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
      }
      _fe_problem.setActiveMaterialProperties(needed_mat_props, /*tid=*/0);

      for (unsigned int i = 0; i < secondary_nodes.size(); i++)
      {
        dof_id_type secondary_node_num = secondary_nodes[i];
        Node & secondary_node = _mesh.nodeRef(secondary_node_num);

        if (secondary_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[secondary_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[secondary_node_num];

            reinitNodeFace(secondary_node, secondary_boundary, info, displaced);

            for (const auto & nfc : constraints)
            {
              if (nfc->isExplicitConstraint())
                continue;
              // Return if this constraint does not correspond to the primary-secondary pair
              // prepared by the outer loops.
              // This continue statement is required when, e.g. one secondary surface constrains
              // more than one primary surface.
              if (nfc->secondaryBoundary() != secondary_boundary ||
                  nfc->primaryBoundary() != primary_boundary)
                continue;

              if (nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeSecondaryValue(solution);
              }

              if (nfc->hasWritableCoupledVariables())
              {
                Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
                for (auto * var : nfc->getWritableCoupledVariables())
                {
                  if (var->isNodalDefined())
                    var->insert(_fe_problem.getAuxiliarySystem().solution());
                }
              }
            }
          }
        }
      }
    }
  }

  // go over NodeELemConstraints
  std::set<dof_id_type> unique_secondary_node_ids;

  for (const auto & secondary_id : _mesh.meshSubdomains())
  {
    for (const auto & primary_id : _mesh.meshSubdomains())
    {
      if (_constraints.hasActiveNodeElemConstraints(secondary_id, primary_id, displaced))
      {
        const auto & constraints =
            _constraints.getActiveNodeElemConstraints(secondary_id, primary_id, displaced);

        // get unique set of ids of all nodes on current block
        unique_secondary_node_ids.clear();
        const MeshBase & meshhelper = _mesh.getMesh();
        for (const auto & elem : as_range(meshhelper.active_subdomain_elements_begin(secondary_id),
                                          meshhelper.active_subdomain_elements_end(secondary_id)))
        {
          for (auto & n : elem->node_ref_range())
            unique_secondary_node_ids.insert(n.id());
        }

        for (auto secondary_node_id : unique_secondary_node_ids)
        {
          Node & secondary_node = _mesh.nodeRef(secondary_node_id);

          // check if secondary node is on current processor
          if (secondary_node.processor_id() == processor_id())
          {
            // This reinits the variables that exist on the secondary node
            _fe_problem.reinitNodeFace(&secondary_node, secondary_id, 0);

            // This will set aside residual and jacobian space for the variables that have dofs
            // on the secondary node
            _fe_problem.prepareAssembly(0);

            for (const auto & nec : constraints)
            {
              if (nec->shouldApply())
              {
                constraints_applied = true;
                nec->computeSecondaryValue(solution);
              }
            }
          }
        }
      }
    }
  }

  // See if constraints were applied anywhere
  _communicator.max(constraints_applied);

  if (constraints_applied)
  {
    solution.close();
    update();
  }
}

void
NonlinearSystemBase::constraintResiduals(NumericVector<Number> & residual, bool displaced)
{
  // Make sure the residual is in a good state
  residual.close();

  if (displaced)
    mooseAssert(_fe_problem.getDisplacedProblem(),
                "If we're calling this method with displaced = true, then we better well have a "
                "displaced problem");
  auto & subproblem = displaced ? static_cast<SubProblem &>(*_fe_problem.getDisplacedProblem())
                                : static_cast<SubProblem &>(_fe_problem);
  const auto & penetration_locators = subproblem.geomSearchData()._penetration_locators;

  bool constraints_applied;
  bool residual_has_inserted_values = false;
  if (!_assemble_constraints_separately)
    constraints_applied = false;
  for (const auto & it : penetration_locators)
  {
    if (_assemble_constraints_separately)
    {
      // Reset the constraint_applied flag before each new constraint, as they need to be
      // assembled separately
      constraints_applied = false;
    }
    PenetrationLocator & pen_loc = *(it.second);

    std::vector<dof_id_type> & secondary_nodes = pen_loc._nearest_node._secondary_nodes;

    BoundaryID secondary_boundary = pen_loc._secondary_boundary;
    BoundaryID primary_boundary = pen_loc._primary_boundary;

    bool has_writable_variables(false);

    if (_constraints.hasActiveNodeFaceConstraints(secondary_boundary, displaced))
    {
      const auto & constraints =
          _constraints.getActiveNodeFaceConstraints(secondary_boundary, displaced);

      for (unsigned int i = 0; i < secondary_nodes.size(); i++)
      {
        dof_id_type secondary_node_num = secondary_nodes[i];
        Node & secondary_node = _mesh.nodeRef(secondary_node_num);

        if (secondary_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[secondary_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[secondary_node_num];

            reinitNodeFace(secondary_node, secondary_boundary, info, displaced);

            for (const auto & nfc : constraints)
            {
              // Return if this constraint does not correspond to the primary-secondary pair
              // prepared by the outer loops.
              // This continue statement is required when, e.g. one secondary surface constrains
              // more than one primary surface.
              if (nfc->secondaryBoundary() != secondary_boundary ||
                  nfc->primaryBoundary() != primary_boundary)
                continue;

              if (nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeResidual();

                if (nfc->overwriteSecondaryResidual())
                {
                  // The below will actually overwrite the residual for every single dof that
                  // lives on the node. We definitely don't want to do that!
                  // _fe_problem.setResidual(residual, 0);

                  const auto & secondary_var = nfc->variable();
                  const auto & secondary_dofs = secondary_var.dofIndices();
                  mooseAssert(secondary_dofs.size() == secondary_var.count(),
                              "We are on a node so there should only be one dof per variable (for "
                              "an ArrayVariable we should have a number of dofs equal to the "
                              "number of components");

                  // Assume that if the user is overwriting the secondary residual, then they are
                  // supplying residuals that do not correspond to their other physics
                  // (e.g. Kernels), hence we should not apply a scalingFactor that is normally
                  // based on the order of their other physics (e.g. Kernels)
                  std::vector<Number> values = {nfc->secondaryResidual()};
                  residual.insert(values, secondary_dofs);
                  residual_has_inserted_values = true;
                }
                else
                  _fe_problem.cacheResidual(0);
                _fe_problem.cacheResidualNeighbor(0);
              }
              if (nfc->hasWritableCoupledVariables())
              {
                Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
                has_writable_variables = true;
                for (auto * var : nfc->getWritableCoupledVariables())
                {
                  if (var->isNodalDefined())
                    var->insert(_fe_problem.getAuxiliarySystem().solution());
                }
              }
            }
          }
        }
      }
    }
    _communicator.max(has_writable_variables);

    if (has_writable_variables)
    {
      // Explicit contact dynamic constraints write to auxiliary variables and update the old
      // displacement solution on the constraint boundaries. Close solutions and update system
      // accordingly.
      _fe_problem.getAuxiliarySystem().solution().close();
      _fe_problem.getAuxiliarySystem().system().update();
      solutionOld().close();
    }

    if (_assemble_constraints_separately)
    {
      // Make sure that secondary contribution to primary are assembled, and ghosts have been
      // exchanged, as current primaries might become secondaries on next iteration and will need to
      // contribute their former secondaries' contributions to the future primaries. See if
      // constraints were applied anywhere
      _communicator.max(constraints_applied);

      if (constraints_applied)
      {
        // If any of the above constraints inserted values in the residual, it needs to be
        // assembled before adding the cached residuals below.
        _communicator.max(residual_has_inserted_values);
        if (residual_has_inserted_values)
        {
          residual.close();
          residual_has_inserted_values = false;
        }
        _fe_problem.addCachedResidualDirectly(residual, 0);
        residual.close();

        if (_need_residual_ghosted)
          *_residual_ghosted = residual;
      }
    }
  }
  if (!_assemble_constraints_separately)
  {
    _communicator.max(constraints_applied);

    if (constraints_applied)
    {
      // If any of the above constraints inserted values in the residual, it needs to be assembled
      // before adding the cached residuals below.
      _communicator.max(residual_has_inserted_values);
      if (residual_has_inserted_values)
        residual.close();

      _fe_problem.addCachedResidualDirectly(residual, 0);
      residual.close();

      if (_need_residual_ghosted)
        *_residual_ghosted = residual;
    }
  }

  // go over element-element constraint interface
  THREAD_ID tid = 0;
  const auto & element_pair_locators = subproblem.geomSearchData()._element_pair_locators;
  for (const auto & it : element_pair_locators)
  {
    ElementPairLocator & elem_pair_loc = *(it.second);

    if (_constraints.hasActiveElemElemConstraints(it.first, displaced))
    {
      // ElemElemConstraint objects
      const auto & _element_constraints =
          _constraints.getActiveElemElemConstraints(it.first, displaced);

      // go over pair elements
      const std::list<std::pair<const Elem *, const Elem *>> & elem_pairs =
          elem_pair_loc.getElemPairs();
      for (const auto & pr : elem_pairs)
      {
        const Elem * elem1 = pr.first;
        const Elem * elem2 = pr.second;

        if (elem1->processor_id() != processor_id())
          continue;

        const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(pr);

        // for each element process constraints on the
        for (const auto & ec : _element_constraints)
        {
          _fe_problem.setCurrentSubdomainID(elem1, tid);
          subproblem.reinitElemPhys(elem1, info._elem1_constraint_q_point, tid);
          _fe_problem.setNeighborSubdomainID(elem2, tid);
          subproblem.reinitNeighborPhys(elem2, info._elem2_constraint_q_point, tid);

          ec->prepareShapes(ec->variable().number());
          ec->prepareNeighborShapes(ec->variable().number());

          ec->reinit(info);
          ec->computeResidual();
          _fe_problem.cacheResidual(tid);
          _fe_problem.cacheResidualNeighbor(tid);
        }
        _fe_problem.addCachedResidual(tid);
      }
    }
  }

  // go over NodeELemConstraints
  std::set<dof_id_type> unique_secondary_node_ids;

  constraints_applied = false;
  residual_has_inserted_values = false;
  for (const auto & secondary_id : _mesh.meshSubdomains())
  {
    for (const auto & primary_id : _mesh.meshSubdomains())
    {
      if (_constraints.hasActiveNodeElemConstraints(secondary_id, primary_id, displaced))
      {
        const auto & constraints =
            _constraints.getActiveNodeElemConstraints(secondary_id, primary_id, displaced);

        // get unique set of ids of all nodes on current block
        unique_secondary_node_ids.clear();
        const MeshBase & meshhelper = _mesh.getMesh();
        for (const auto & elem : as_range(meshhelper.active_subdomain_elements_begin(secondary_id),
                                          meshhelper.active_subdomain_elements_end(secondary_id)))
        {
          for (auto & n : elem->node_ref_range())
            unique_secondary_node_ids.insert(n.id());
        }

        for (auto secondary_node_id : unique_secondary_node_ids)
        {
          Node & secondary_node = _mesh.nodeRef(secondary_node_id);
          // check if secondary node is on current processor
          if (secondary_node.processor_id() == processor_id())
          {
            // This reinits the variables that exist on the secondary node
            _fe_problem.reinitNodeFace(&secondary_node, secondary_id, 0);

            // This will set aside residual and jacobian space for the variables that have dofs
            // on the secondary node
            _fe_problem.prepareAssembly(0);

            for (const auto & nec : constraints)
            {
              if (nec->shouldApply())
              {
                constraints_applied = true;
                nec->computeResidual();

                if (nec->overwriteSecondaryResidual())
                {
                  _fe_problem.setResidual(residual, 0);
                  residual_has_inserted_values = true;
                }
                else
                  _fe_problem.cacheResidual(0);
                _fe_problem.cacheResidualNeighbor(0);
              }
            }
            _fe_problem.addCachedResidual(0);
          }
        }
      }
    }
  }
  _communicator.max(constraints_applied);

  if (constraints_applied)
  {
    // If any of the above constraints inserted values in the residual, it needs to be assembled
    // before adding the cached residuals below.
    _communicator.max(residual_has_inserted_values);
    if (residual_has_inserted_values)
      residual.close();

    _fe_problem.addCachedResidualDirectly(residual, 0);
    residual.close();

    if (_need_residual_ghosted)
      *_residual_ghosted = residual;
  }

  // We may have additional tagged vectors that also need to be accumulated
  _fe_problem.addCachedResidual(0);
}

void
NonlinearSystemBase::overwriteNodeFace(NumericVector<Number> & soln)
{
  // Overwrite results from integrator in case we have explicit dynamics contact constraints
  auto & subproblem = _fe_problem.getDisplacedProblem()
                          ? static_cast<SubProblem &>(*_fe_problem.getDisplacedProblem())
                          : static_cast<SubProblem &>(_fe_problem);
  const auto & penetration_locators = subproblem.geomSearchData()._penetration_locators;

  for (const auto & it : penetration_locators)
  {
    PenetrationLocator & pen_loc = *(it.second);

    const auto & secondary_nodes = pen_loc._nearest_node._secondary_nodes;
    const BoundaryID secondary_boundary = pen_loc._secondary_boundary;
    const BoundaryID primary_boundary = pen_loc._primary_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(secondary_boundary, true))
    {
      const auto & constraints =
          _constraints.getActiveNodeFaceConstraints(secondary_boundary, true);
      for (const auto i : index_range(secondary_nodes))
      {
        const auto secondary_node_num = secondary_nodes[i];
        const Node & secondary_node = _mesh.nodeRef(secondary_node_num);

        if (secondary_node.processor_id() == processor_id())
          if (pen_loc._penetration_info[secondary_node_num])
            for (const auto & nfc : constraints)
            {
              if (!nfc->isExplicitConstraint())
                continue;

              // Return if this constraint does not correspond to the primary-secondary pair
              // prepared by the outer loops.
              // This continue statement is required when, e.g. one secondary surface constrains
              // more than one primary surface.
              if (nfc->secondaryBoundary() != secondary_boundary ||
                  nfc->primaryBoundary() != primary_boundary)
                continue;

              nfc->overwriteBoundaryVariables(soln, secondary_node);
            }
      }
    }
  }
  soln.close();
}

void
NonlinearSystemBase::residualSetup()
{
  TIME_SECTION("residualSetup", 3);

  SolverSystem::residualSetup();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.residualSetup(tid);
    _nodal_kernels.residualSetup(tid);
    _dirac_kernels.residualSetup(tid);
    if (_doing_dg)
      _dg_kernels.residualSetup(tid);
    _interface_kernels.residualSetup(tid);
    _element_dampers.residualSetup(tid);
    _nodal_dampers.residualSetup(tid);
    _integrated_bcs.residualSetup(tid);
  }
  _scalar_kernels.residualSetup();
  _constraints.residualSetup();
  _general_dampers.residualSetup();
  _nodal_bcs.residualSetup();

  // Avoid recursion
  if (this == &_fe_problem.currentNonlinearSystem())
    _fe_problem.residualSetup();
  _app.solutionInvalidity().resetSolutionInvalidCurrentIteration();
}

void
NonlinearSystemBase::computeResidualInternal(const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("computeResidualInternal", 3);

  residualSetup();

  const auto vector_tag_data = _fe_problem.getVectorTags(tags);

  // Residual contributions from UOs - for now this is used for ray tracing
  // and ray kernels that contribute to the residual (think line sources)
  std::vector<UserObject *> uos;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribExecOns>(EXEC_PRE_KERNELS)
      .queryInto(uos);
  for (auto & uo : uos)
    uo->residualSetup();
  for (auto & uo : uos)
  {
    uo->initialize();
    uo->execute();
    uo->finalize();
  }

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  // residual contributions from the domain
  PARALLEL_TRY
  {
    TIME_SECTION("Kernels", 3 /*, "Computing Kernels"*/);

    const ConstElemRange & elem_range = _fe_problem.getCurrentAlgebraicElementRange();

    ComputeResidualThread cr(_fe_problem, tags);
    Threads::parallel_reduce(elem_range, cr);

    // We pass face information directly to FV residual objects for their evaluation. Consequently
    // we must make sure to do separate threaded loops for 1) undisplaced face information objects
    // and undisplaced residual objects and 2) displaced face information objects and displaced
    // residual objects
    using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    if (_fe_problem.haveFV())
    {
      ComputeFVFluxResidualThread<FVRange> fvr(
          _fe_problem, this->number(), tags, /*on_displaced=*/false);
      FVRange faces(_fe_problem.mesh().ownedFaceInfoBegin(), _fe_problem.mesh().ownedFaceInfoEnd());
      Threads::parallel_reduce(faces, fvr);
    }
    if (auto displaced_problem = _fe_problem.getDisplacedProblem();
        displaced_problem && displaced_problem->haveFV())
    {
      ComputeFVFluxResidualThread<FVRange> fvr(
          _fe_problem, this->number(), tags, /*on_displaced=*/true);
      FVRange faces(displaced_problem->mesh().ownedFaceInfoBegin(),
                    displaced_problem->mesh().ownedFaceInfoEnd());
      Threads::parallel_reduce(faces, fvr);
    }

    unsigned int n_threads = libMesh::n_threads();
    for (unsigned int i = 0; i < n_threads;
         i++) // Add any cached residuals that might be hanging around
      _fe_problem.addCachedResidual(i);
  }
  PARALLEL_CATCH;

  // residual contributions from the scalar kernels
  PARALLEL_TRY
  {
    // do scalar kernels (not sure how to thread this)
    if (_scalar_kernels.hasActiveObjects())
    {
      TIME_SECTION("ScalarKernels", 3 /*, "Computing ScalarKernels"*/);

      MooseObjectWarehouse<ScalarKernelBase> * scalar_kernel_warehouse;
      // This code should be refactored once we can do tags for scalar
      // kernels
      // Should redo this based on Warehouse
      if (!tags.size() || tags.size() == _fe_problem.numVectorTags(Moose::VECTOR_TAG_RESIDUAL))
        scalar_kernel_warehouse = &_scalar_kernels;
      else if (tags.size() == 1)
        scalar_kernel_warehouse =
            &(_scalar_kernels.getVectorTagObjectWarehouse(*(tags.begin()), 0));
      else
        // scalar_kernels is not threading
        scalar_kernel_warehouse = &(_scalar_kernels.getVectorTagsObjectWarehouse(tags, 0));

      bool have_scalar_contributions = false;
      const auto & scalars = scalar_kernel_warehouse->getActiveObjects();
      for (const auto & scalar_kernel : scalars)
      {
        scalar_kernel->reinit();
        const std::vector<dof_id_type> & dof_indices = scalar_kernel->variable().dofIndices();
        const DofMap & dof_map = scalar_kernel->variable().dofMap();
        const dof_id_type first_dof = dof_map.first_dof();
        const dof_id_type end_dof = dof_map.end_dof();
        for (dof_id_type dof : dof_indices)
        {
          if (dof >= first_dof && dof < end_dof)
          {
            scalar_kernel->computeResidual();
            have_scalar_contributions = true;
            break;
          }
        }
      }
      if (have_scalar_contributions)
        _fe_problem.addResidualScalar();
    }
  }
  PARALLEL_CATCH;

  // residual contributions from Block NodalKernels
  PARALLEL_TRY
  {
    if (_nodal_kernels.hasActiveBlockObjects())
    {
      TIME_SECTION("NodalKernels", 3 /*, "Computing NodalKernels"*/);

      ComputeNodalKernelsThread cnk(_fe_problem, _nodal_kernels, tags);

      const ConstNodeRange & range = _fe_problem.getCurrentAlgebraicNodeRange();

      if (range.begin() != range.end())
      {
        _fe_problem.reinitNode(*range.begin(), 0);

        Threads::parallel_reduce(range, cnk);

        unsigned int n_threads = libMesh::n_threads();
        for (unsigned int i = 0; i < n_threads;
             i++) // Add any cached residuals that might be hanging around
          _fe_problem.addCachedResidual(i);
      }
    }
  }
  PARALLEL_CATCH;

  if (_fe_problem.computingScalingResidual())
    // We computed the volumetric objects. We can return now before we get into
    // any strongly enforced constraint conditions or penalty-type objects
    // (DGKernels, IntegratedBCs, InterfaceKernels, Constraints)
    return;

  // residual contributions from boundary NodalKernels
  PARALLEL_TRY
  {
    if (_nodal_kernels.hasActiveBoundaryObjects())
    {
      TIME_SECTION("NodalKernelBCs", 3 /*, "Computing NodalKernelBCs"*/);

      ComputeNodalKernelBcsThread cnk(_fe_problem, _nodal_kernels, tags);

      const ConstBndNodeRange & bnd_node_range = _fe_problem.getCurrentAlgebraicBndNodeRange();

      Threads::parallel_reduce(bnd_node_range, cnk);

      unsigned int n_threads = libMesh::n_threads();
      for (unsigned int i = 0; i < n_threads;
           i++) // Add any cached residuals that might be hanging around
        _fe_problem.addCachedResidual(i);
    }
  }
  PARALLEL_CATCH;

  mortarConstraints(Moose::ComputeType::Residual, tags, {});

  if (_residual_copy.get())
  {
    _Re_non_time->close();
    _Re_non_time->localize(*_residual_copy);
  }

  if (_need_residual_ghosted)
  {
    _Re_non_time->close();
    *_residual_ghosted = *_Re_non_time;
    _residual_ghosted->close();
  }

  PARALLEL_TRY { computeDiracContributions(tags, false); }
  PARALLEL_CATCH;

  if (_fe_problem._has_constraints)
  {
    PARALLEL_TRY { enforceNodalConstraintsResidual(*_Re_non_time); }
    PARALLEL_CATCH;
    _Re_non_time->close();
  }

  // Add in Residual contributions from other Constraints
  if (_fe_problem._has_constraints)
  {
    PARALLEL_TRY
    {
      // Undisplaced Constraints
      constraintResiduals(*_Re_non_time, false);

      // Displaced Constraints
      if (_fe_problem.getDisplacedProblem())
        constraintResiduals(*_Re_non_time, true);

      if (_fe_problem.computingNonlinearResid())
        _constraints.residualEnd();
    }
    PARALLEL_CATCH;
    _Re_non_time->close();
  }

  // Accumulate the occurrence of solution invalid warnings for the current iteration cumulative
  // counters
  _app.solutionInvalidity().sync();
  _app.solutionInvalidity().solutionInvalidAccumulation();
}

void
NonlinearSystemBase::computeResidualAndJacobianInternal(const std::set<TagID> & vector_tags,
                                                        const std::set<TagID> & matrix_tags)
{
  TIME_SECTION("computeResidualAndJacobianInternal", 3);

  // Make matrix ready to use
  activeAllMatrixTags();

  for (auto tag : matrix_tags)
  {
    if (!hasMatrix(tag))
      continue;

    auto & jacobian = getMatrix(tag);
    // Necessary for speed
    if (auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&jacobian))
    {
      LibmeshPetscCall(MatSetOption(petsc_matrix->mat(),
                                    MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                                    PETSC_TRUE));
      if (!_fe_problem.errorOnJacobianNonzeroReallocation())
        LibmeshPetscCall(
            MatSetOption(petsc_matrix->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE));
    }
  }

  residualSetup();

  // Residual contributions from UOs - for now this is used for ray tracing
  // and ray kernels that contribute to the residual (think line sources)
  std::vector<UserObject *> uos;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribExecOns>(EXEC_PRE_KERNELS)
      .queryInto(uos);
  for (auto & uo : uos)
    uo->residualSetup();
  for (auto & uo : uos)
  {
    uo->initialize();
    uo->execute();
    uo->finalize();
  }

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  // residual contributions from the domain
  PARALLEL_TRY
  {
    TIME_SECTION("Kernels", 3 /*, "Computing Kernels"*/);

    const ConstElemRange & elem_range = _fe_problem.getCurrentAlgebraicElementRange();

    ComputeResidualAndJacobianThread crj(_fe_problem, vector_tags, matrix_tags);
    Threads::parallel_reduce(elem_range, crj);

    using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    if (_fe_problem.haveFV())
    {
      ComputeFVFluxRJThread<FVRange> fvrj(
          _fe_problem, this->number(), vector_tags, matrix_tags, /*on_displaced=*/false);
      FVRange faces(_fe_problem.mesh().ownedFaceInfoBegin(), _fe_problem.mesh().ownedFaceInfoEnd());
      Threads::parallel_reduce(faces, fvrj);
    }
    if (auto displaced_problem = _fe_problem.getDisplacedProblem();
        displaced_problem && displaced_problem->haveFV())
    {
      ComputeFVFluxRJThread<FVRange> fvr(
          _fe_problem, this->number(), vector_tags, matrix_tags, /*on_displaced=*/true);
      FVRange faces(displaced_problem->mesh().ownedFaceInfoBegin(),
                    displaced_problem->mesh().ownedFaceInfoEnd());
      Threads::parallel_reduce(faces, fvr);
    }

    mortarConstraints(Moose::ComputeType::ResidualAndJacobian, vector_tags, matrix_tags);

    unsigned int n_threads = libMesh::n_threads();
    for (unsigned int i = 0; i < n_threads;
         i++) // Add any cached residuals that might be hanging around
    {
      _fe_problem.addCachedResidual(i);
      _fe_problem.addCachedJacobian(i);
    }
  }
  PARALLEL_CATCH;
}

void
NonlinearSystemBase::computeNodalBCs(NumericVector<Number> & residual)
{
  _nl_vector_tags.clear();

  const auto & residual_vector_tags = _fe_problem.getVectorTags(Moose::VECTOR_TAG_RESIDUAL);
  for (const auto & residual_vector_tag : residual_vector_tags)
    _nl_vector_tags.insert(residual_vector_tag._id);

  associateVectorToTag(residual, residualVectorTag());
  computeNodalBCs(residual, _nl_vector_tags);
  disassociateVectorFromTag(residual, residualVectorTag());
}

void
NonlinearSystemBase::computeNodalBCs(NumericVector<Number> & residual, const std::set<TagID> & tags)
{
  associateVectorToTag(residual, residualVectorTag());

  computeNodalBCs(tags);

  disassociateVectorFromTag(residual, residualVectorTag());
}

void
NonlinearSystemBase::computeNodalBCs(const std::set<TagID> & tags)
{
  // We need to close the diag_save_in variables on the aux system before NodalBCBases clear the
  // dofs on boundary nodes
  if (_has_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  // Select nodal kernels
  MooseObjectWarehouse<NodalBCBase> * nbc_warehouse;

  if (tags.size() == _fe_problem.numVectorTags(Moose::VECTOR_TAG_RESIDUAL) || !tags.size())
    nbc_warehouse = &_nodal_bcs;
  else if (tags.size() == 1)
    nbc_warehouse = &(_nodal_bcs.getVectorTagObjectWarehouse(*(tags.begin()), 0));
  else
    nbc_warehouse = &(_nodal_bcs.getVectorTagsObjectWarehouse(tags, 0));

  // Return early if there is no nodal kernel
  if (!nbc_warehouse->size())
    return;

  PARALLEL_TRY
  {
    const ConstBndNodeRange & bnd_nodes = _fe_problem.getCurrentAlgebraicBndNodeRange();

    if (!bnd_nodes.empty())
    {
      TIME_SECTION("NodalBCs", 3 /*, "Computing NodalBCs"*/);

      for (const auto & bnode : bnd_nodes)
      {
        BoundaryID boundary_id = bnode->_bnd_id;
        Node * node = bnode->_node;

        if (node->processor_id() == processor_id() &&
            nbc_warehouse->hasActiveBoundaryObjects(boundary_id))
        {
          // reinit variables in nodes
          _fe_problem.reinitNodeFace(node, boundary_id, 0);

          const auto & bcs = nbc_warehouse->getActiveBoundaryObjects(boundary_id);
          for (const auto & nbc : bcs)
            if (nbc->shouldApply())
              nbc->computeResidual();
        }
      }
    }
  }
  PARALLEL_CATCH;

  if (_Re_time)
    _Re_time->close();
  _Re_non_time->close();
}

void
NonlinearSystemBase::computeNodalBCsResidualAndJacobian()
{
  PARALLEL_TRY
  {
    const ConstBndNodeRange & bnd_nodes = _fe_problem.getCurrentAlgebraicBndNodeRange();

    if (!bnd_nodes.empty())
    {
      TIME_SECTION("NodalBCs", 3 /*, "Computing NodalBCs"*/);

      for (const auto & bnode : bnd_nodes)
      {
        BoundaryID boundary_id = bnode->_bnd_id;
        Node * node = bnode->_node;

        if (node->processor_id() == processor_id())
        {
          // reinit variables in nodes
          _fe_problem.reinitNodeFace(node, boundary_id, 0);
          if (_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
          {
            const auto & bcs = _nodal_bcs.getActiveBoundaryObjects(boundary_id);
            for (const auto & nbc : bcs)
              if (nbc->shouldApply())
                nbc->computeResidualAndJacobian();
          }
        }
      }
    }
  }
  PARALLEL_CATCH;

  // Set the cached NodalBCBase values in the Jacobian matrix
  _fe_problem.assembly(0, number()).setCachedJacobian(Assembly::GlobalDataKey{});
}

void
NonlinearSystemBase::getNodeDofs(dof_id_type node_id, std::vector<dof_id_type> & dofs)
{
  const Node & node = _mesh.nodeRef(node_id);
  unsigned int s = number();
  if (node.has_dofs(s))
  {
    for (unsigned int v = 0; v < nVariables(); v++)
      for (unsigned int c = 0; c < node.n_comp(s, v); c++)
        dofs.push_back(node.dof_number(s, v, c));
  }
}

void
NonlinearSystemBase::findImplicitGeometricCouplingEntries(
    GeometricSearchData & geom_search_data,
    std::unordered_map<dof_id_type, std::vector<dof_id_type>> & graph)
{
  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  const auto & nearest_node_locators = geom_search_data._nearest_node_locators;
  for (const auto & it : nearest_node_locators)
  {
    std::vector<dof_id_type> & secondary_nodes = it.second->_secondary_nodes;

    for (const auto & secondary_node : secondary_nodes)
    {
      std::set<dof_id_type> unique_secondary_indices;
      std::set<dof_id_type> unique_primary_indices;

      auto node_to_elem_pair = node_to_elem_map.find(secondary_node);
      if (node_to_elem_pair != node_to_elem_map.end())
      {
        const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

        // Get the dof indices from each elem connected to the node
        for (const auto & cur_elem : elems)
        {
          std::vector<dof_id_type> dof_indices;
          dofMap().dof_indices(_mesh.elemPtr(cur_elem), dof_indices);

          for (const auto & dof : dof_indices)
            unique_secondary_indices.insert(dof);
        }
      }

      std::vector<dof_id_type> primary_nodes = it.second->_neighbor_nodes[secondary_node];

      for (const auto & primary_node : primary_nodes)
      {
        auto primary_node_to_elem_pair = node_to_elem_map.find(primary_node);
        mooseAssert(primary_node_to_elem_pair != node_to_elem_map.end(),
                    "Missing entry in node to elem map");
        const std::vector<dof_id_type> & primary_node_elems = primary_node_to_elem_pair->second;

        // Get the dof indices from each elem connected to the node
        for (const auto & cur_elem : primary_node_elems)
        {
          std::vector<dof_id_type> dof_indices;
          dofMap().dof_indices(_mesh.elemPtr(cur_elem), dof_indices);

          for (const auto & dof : dof_indices)
            unique_primary_indices.insert(dof);
        }
      }

      for (const auto & secondary_id : unique_secondary_indices)
        for (const auto & primary_id : unique_primary_indices)
        {
          graph[secondary_id].push_back(primary_id);
          graph[primary_id].push_back(secondary_id);
        }
    }
  }

  // handle node-to-node constraints
  const auto & ncs = _constraints.getActiveNodalConstraints();
  for (const auto & nc : ncs)
  {
    std::vector<dof_id_type> primary_dofs;
    std::vector<dof_id_type> & primary_node_ids = nc->getPrimaryNodeId();
    for (const auto & node_id : primary_node_ids)
    {
      Node * node = _mesh.queryNodePtr(node_id);
      if (node && node->processor_id() == this->processor_id())
      {
        getNodeDofs(node_id, primary_dofs);
      }
    }

    _communicator.allgather(primary_dofs);

    std::vector<dof_id_type> secondary_dofs;
    std::vector<dof_id_type> & secondary_node_ids = nc->getSecondaryNodeId();
    for (const auto & node_id : secondary_node_ids)
    {
      Node * node = _mesh.queryNodePtr(node_id);
      if (node && node->processor_id() == this->processor_id())
      {
        getNodeDofs(node_id, secondary_dofs);
      }
    }

    _communicator.allgather(secondary_dofs);

    for (const auto & primary_id : primary_dofs)
      for (const auto & secondary_id : secondary_dofs)
      {
        graph[primary_id].push_back(secondary_id);
        graph[secondary_id].push_back(primary_id);
      }
  }

  // Make every entry sorted and unique
  for (auto & it : graph)
  {
    std::vector<dof_id_type> & row = it.second;
    std::sort(row.begin(), row.end());
    std::vector<dof_id_type>::iterator uit = std::unique(row.begin(), row.end());
    row.resize(uit - row.begin());
  }
}

void
NonlinearSystemBase::addImplicitGeometricCouplingEntries(GeometricSearchData & geom_search_data)
{
  if (!hasMatrix(systemMatrixTag()))
    mooseError("Need a system matrix ");

  // At this point, have no idea how to make
  // this work with tag system
  auto & jacobian = getMatrix(systemMatrixTag());

  std::unordered_map<dof_id_type, std::vector<dof_id_type>> graph;

  findImplicitGeometricCouplingEntries(geom_search_data, graph);

  for (const auto & it : graph)
  {
    dof_id_type dof = it.first;
    const auto & row = it.second;

    for (const auto & coupled_dof : row)
      jacobian.add(dof, coupled_dof, 0);
  }
}

void
NonlinearSystemBase::constraintJacobians(bool displaced)
{
  if (!hasMatrix(systemMatrixTag()))
    mooseError("A system matrix is required");

  auto & jacobian = getMatrix(systemMatrixTag());

  if (!_fe_problem.errorOnJacobianNonzeroReallocation())
    LibmeshPetscCall(MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                                  MAT_NEW_NONZERO_ALLOCATION_ERR,
                                  PETSC_FALSE));

  if (_fe_problem.ignoreZerosInJacobian())
    LibmeshPetscCall(MatSetOption(
        static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_IGNORE_ZERO_ENTRIES, PETSC_TRUE));

  std::vector<numeric_index_type> zero_rows;

  if (displaced)
    mooseAssert(_fe_problem.getDisplacedProblem(),
                "If we're calling this method with displaced = true, then we better well have a "
                "displaced problem");
  auto & subproblem = displaced ? static_cast<SubProblem &>(*_fe_problem.getDisplacedProblem())
                                : static_cast<SubProblem &>(_fe_problem);
  const auto & penetration_locators = subproblem.geomSearchData()._penetration_locators;

  bool constraints_applied;
  if (!_assemble_constraints_separately)
    constraints_applied = false;
  for (const auto & it : penetration_locators)
  {
    if (_assemble_constraints_separately)
    {
      // Reset the constraint_applied flag before each new constraint, as they need to be
      // assembled separately
      constraints_applied = false;
    }
    PenetrationLocator & pen_loc = *(it.second);

    std::vector<dof_id_type> & secondary_nodes = pen_loc._nearest_node._secondary_nodes;

    BoundaryID secondary_boundary = pen_loc._secondary_boundary;
    BoundaryID primary_boundary = pen_loc._primary_boundary;

    zero_rows.clear();
    if (_constraints.hasActiveNodeFaceConstraints(secondary_boundary, displaced))
    {
      const auto & constraints =
          _constraints.getActiveNodeFaceConstraints(secondary_boundary, displaced);

      for (const auto & secondary_node_num : secondary_nodes)
      {
        Node & secondary_node = _mesh.nodeRef(secondary_node_num);

        if (secondary_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[secondary_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[secondary_node_num];

            reinitNodeFace(secondary_node, secondary_boundary, info, displaced);
            _fe_problem.reinitOffDiagScalars(0);

            for (const auto & nfc : constraints)
            {
              if (nfc->isExplicitConstraint())
                continue;
              // Return if this constraint does not correspond to the primary-secondary pair
              // prepared by the outer loops.
              // This continue statement is required when, e.g. one secondary surface constrains
              // more than one primary surface.
              if (nfc->secondaryBoundary() != secondary_boundary ||
                  nfc->primaryBoundary() != primary_boundary)
                continue;

              nfc->_jacobian = &jacobian;

              if (nfc->shouldApply())
              {
                constraints_applied = true;

                nfc->prepareShapes(nfc->variable().number());
                nfc->prepareNeighborShapes(nfc->variable().number());

                nfc->computeJacobian();

                if (nfc->overwriteSecondaryJacobian())
                {
                  // Add this variable's dof's row to be zeroed
                  zero_rows.push_back(nfc->variable().nodalDofIndex());
                }

                std::vector<dof_id_type> secondary_dofs(1, nfc->variable().nodalDofIndex());

                // Assume that if the user is overwriting the secondary Jacobian, then they are
                // supplying Jacobians that do not correspond to their other physics
                // (e.g. Kernels), hence we should not apply a scalingFactor that is normally
                // based on the order of their other physics (e.g. Kernels)
                Real scaling_factor =
                    nfc->overwriteSecondaryJacobian() ? 1. : nfc->variable().scalingFactor();

                // Cache the jacobian block for the secondary side
                nfc->addJacobian(_fe_problem.assembly(0, number()),
                                 nfc->_Kee,
                                 secondary_dofs,
                                 nfc->_connected_dof_indices,
                                 scaling_factor);

                // Cache Ken, Kne, Knn
                if (nfc->addCouplingEntriesToJacobian())
                {
                  // Make sure we use a proper scaling factor (e.g. don't use an interior scaling
                  // factor when we're overwriting secondary stuff)
                  nfc->addJacobian(_fe_problem.assembly(0, number()),
                                   nfc->_Ken,
                                   secondary_dofs,
                                   nfc->primaryVariable().dofIndicesNeighbor(),
                                   scaling_factor);

                  // Use _connected_dof_indices to get all the correct columns
                  nfc->addJacobian(_fe_problem.assembly(0, number()),
                                   nfc->_Kne,
                                   nfc->primaryVariable().dofIndicesNeighbor(),
                                   nfc->_connected_dof_indices,
                                   nfc->variable().scalingFactor());

                  // We've handled Ken and Kne, finally handle Knn
                  _fe_problem.cacheJacobianNeighbor(0);
                }

                // Do the off-diagonals next
                const std::vector<MooseVariableFEBase *> coupled_vars = nfc->getCoupledMooseVars();
                for (const auto & jvar : coupled_vars)
                {
                  // Only compute jacobians for nonlinear variables
                  if (jvar->kind() != Moose::VAR_SOLVER)
                    continue;

                  // Only compute Jacobian entries if this coupling is being used by the
                  // preconditioner
                  if (nfc->variable().number() == jvar->number() ||
                      !_fe_problem.areCoupled(
                          nfc->variable().number(), jvar->number(), this->number()))
                    continue;

                  // Need to zero out the matrices first
                  _fe_problem.prepareAssembly(0);

                  nfc->prepareShapes(nfc->variable().number());
                  nfc->prepareNeighborShapes(jvar->number());

                  nfc->computeOffDiagJacobian(jvar->number());

                  // Cache the jacobian block for the secondary side
                  nfc->addJacobian(_fe_problem.assembly(0, number()),
                                   nfc->_Kee,
                                   secondary_dofs,
                                   nfc->_connected_dof_indices,
                                   scaling_factor);

                  // Cache Ken, Kne, Knn
                  if (nfc->addCouplingEntriesToJacobian())
                  {
                    // Make sure we use a proper scaling factor (e.g. don't use an interior scaling
                    // factor when we're overwriting secondary stuff)
                    nfc->addJacobian(_fe_problem.assembly(0, number()),
                                     nfc->_Ken,
                                     secondary_dofs,
                                     jvar->dofIndicesNeighbor(),
                                     scaling_factor);

                    // Use _connected_dof_indices to get all the correct columns
                    nfc->addJacobian(_fe_problem.assembly(0, number()),
                                     nfc->_Kne,
                                     nfc->variable().dofIndicesNeighbor(),
                                     nfc->_connected_dof_indices,
                                     nfc->variable().scalingFactor());

                    // We've handled Ken and Kne, finally handle Knn
                    _fe_problem.cacheJacobianNeighbor(0);
                  }
                }
              }
            }
          }
        }
      }
    }
    if (_assemble_constraints_separately)
    {
      // See if constraints were applied anywhere
      _communicator.max(constraints_applied);

      if (constraints_applied)
      {
        LibmeshPetscCall(MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                                      MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                                      PETSC_TRUE));

        jacobian.close();
        jacobian.zero_rows(zero_rows, 0.0);
        jacobian.close();
        _fe_problem.addCachedJacobian(0);
        jacobian.close();
      }
    }
  }
  if (!_assemble_constraints_separately)
  {
    // See if constraints were applied anywhere
    _communicator.max(constraints_applied);

    if (constraints_applied)
    {
      LibmeshPetscCall(MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                                    MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                                    PETSC_TRUE));

      jacobian.close();
      jacobian.zero_rows(zero_rows, 0.0);
      jacobian.close();
      _fe_problem.addCachedJacobian(0);
      jacobian.close();
    }
  }

  THREAD_ID tid = 0;
  // go over element-element constraint interface
  const auto & element_pair_locators = subproblem.geomSearchData()._element_pair_locators;
  for (const auto & it : element_pair_locators)
  {
    ElementPairLocator & elem_pair_loc = *(it.second);

    if (_constraints.hasActiveElemElemConstraints(it.first, displaced))
    {
      // ElemElemConstraint objects
      const auto & _element_constraints =
          _constraints.getActiveElemElemConstraints(it.first, displaced);

      // go over pair elements
      const std::list<std::pair<const Elem *, const Elem *>> & elem_pairs =
          elem_pair_loc.getElemPairs();
      for (const auto & pr : elem_pairs)
      {
        const Elem * elem1 = pr.first;
        const Elem * elem2 = pr.second;

        if (elem1->processor_id() != processor_id())
          continue;

        const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(pr);

        // for each element process constraints on the
        for (const auto & ec : _element_constraints)
        {
          _fe_problem.setCurrentSubdomainID(elem1, tid);
          subproblem.reinitElemPhys(elem1, info._elem1_constraint_q_point, tid);
          _fe_problem.setNeighborSubdomainID(elem2, tid);
          subproblem.reinitNeighborPhys(elem2, info._elem2_constraint_q_point, tid);

          ec->prepareShapes(ec->variable().number());
          ec->prepareNeighborShapes(ec->variable().number());

          ec->reinit(info);
          ec->computeJacobian();
          _fe_problem.cacheJacobian(tid);
          _fe_problem.cacheJacobianNeighbor(tid);
        }
        _fe_problem.addCachedJacobian(tid);
      }
    }
  }

  // go over NodeELemConstraints
  std::set<dof_id_type> unique_secondary_node_ids;
  constraints_applied = false;
  for (const auto & secondary_id : _mesh.meshSubdomains())
  {
    for (const auto & primary_id : _mesh.meshSubdomains())
    {
      if (_constraints.hasActiveNodeElemConstraints(secondary_id, primary_id, displaced))
      {
        const auto & constraints =
            _constraints.getActiveNodeElemConstraints(secondary_id, primary_id, displaced);

        // get unique set of ids of all nodes on current block
        unique_secondary_node_ids.clear();
        const MeshBase & meshhelper = _mesh.getMesh();
        for (const auto & elem : as_range(meshhelper.active_subdomain_elements_begin(secondary_id),
                                          meshhelper.active_subdomain_elements_end(secondary_id)))
        {
          for (auto & n : elem->node_ref_range())
            unique_secondary_node_ids.insert(n.id());
        }

        for (auto secondary_node_id : unique_secondary_node_ids)
        {
          const Node & secondary_node = _mesh.nodeRef(secondary_node_id);
          // check if secondary node is on current processor
          if (secondary_node.processor_id() == processor_id())
          {
            // This reinits the variables that exist on the secondary node
            _fe_problem.reinitNodeFace(&secondary_node, secondary_id, 0);

            // This will set aside residual and jacobian space for the variables that have dofs
            // on the secondary node
            _fe_problem.prepareAssembly(0);
            _fe_problem.reinitOffDiagScalars(0);

            for (const auto & nec : constraints)
            {
              if (nec->shouldApply())
              {
                constraints_applied = true;

                nec->_jacobian = &jacobian;
                nec->prepareShapes(nec->variable().number());
                nec->prepareNeighborShapes(nec->variable().number());

                nec->computeJacobian();

                if (nec->overwriteSecondaryJacobian())
                {
                  // Add this variable's dof's row to be zeroed
                  zero_rows.push_back(nec->variable().nodalDofIndex());
                }

                std::vector<dof_id_type> secondary_dofs(1, nec->variable().nodalDofIndex());

                // Cache the jacobian block for the secondary side
                nec->addJacobian(_fe_problem.assembly(0, number()),
                                 nec->_Kee,
                                 secondary_dofs,
                                 nec->_connected_dof_indices,
                                 nec->variable().scalingFactor());

                // Cache the jacobian block for the primary side
                nec->addJacobian(_fe_problem.assembly(0, number()),
                                 nec->_Kne,
                                 nec->primaryVariable().dofIndicesNeighbor(),
                                 nec->_connected_dof_indices,
                                 nec->variable().scalingFactor());

                _fe_problem.cacheJacobian(0);
                _fe_problem.cacheJacobianNeighbor(0);

                // Do the off-diagonals next
                const std::vector<MooseVariableFEBase *> coupled_vars = nec->getCoupledMooseVars();
                for (const auto & jvar : coupled_vars)
                {
                  // Only compute jacobians for nonlinear variables
                  if (jvar->kind() != Moose::VAR_SOLVER)
                    continue;

                  // Only compute Jacobian entries if this coupling is being used by the
                  // preconditioner
                  if (nec->variable().number() == jvar->number() ||
                      !_fe_problem.areCoupled(
                          nec->variable().number(), jvar->number(), this->number()))
                    continue;

                  // Need to zero out the matrices first
                  _fe_problem.prepareAssembly(0);

                  nec->prepareShapes(nec->variable().number());
                  nec->prepareNeighborShapes(jvar->number());

                  nec->computeOffDiagJacobian(jvar->number());

                  // Cache the jacobian block for the secondary side
                  nec->addJacobian(_fe_problem.assembly(0, number()),
                                   nec->_Kee,
                                   secondary_dofs,
                                   nec->_connected_dof_indices,
                                   nec->variable().scalingFactor());

                  // Cache the jacobian block for the primary side
                  nec->addJacobian(_fe_problem.assembly(0, number()),
                                   nec->_Kne,
                                   nec->variable().dofIndicesNeighbor(),
                                   nec->_connected_dof_indices,
                                   nec->variable().scalingFactor());

                  _fe_problem.cacheJacobian(0);
                  _fe_problem.cacheJacobianNeighbor(0);
                }
              }
            }
          }
        }
      }
    }
  }
  // See if constraints were applied anywhere
  _communicator.max(constraints_applied);

  if (constraints_applied)
  {
    LibmeshPetscCall(MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                                  MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                                  PETSC_TRUE));

    jacobian.close();
    jacobian.zero_rows(zero_rows, 0.0);
    jacobian.close();
    _fe_problem.addCachedJacobian(0);
    jacobian.close();
  }
}

void
NonlinearSystemBase::computeScalarKernelsJacobians(const std::set<TagID> & tags)
{
  MooseObjectWarehouse<ScalarKernelBase> * scalar_kernel_warehouse;

  if (!tags.size() || tags.size() == _fe_problem.numMatrixTags())
    scalar_kernel_warehouse = &_scalar_kernels;
  else if (tags.size() == 1)
    scalar_kernel_warehouse = &(_scalar_kernels.getMatrixTagObjectWarehouse(*(tags.begin()), 0));
  else
    scalar_kernel_warehouse = &(_scalar_kernels.getMatrixTagsObjectWarehouse(tags, 0));

  // Compute the diagonal block for scalar variables
  if (scalar_kernel_warehouse->hasActiveObjects())
  {
    const auto & scalars = scalar_kernel_warehouse->getActiveObjects();

    _fe_problem.reinitScalars(/*tid=*/0);

    _fe_problem.reinitOffDiagScalars(/*_tid*/ 0);

    bool have_scalar_contributions = false;
    for (const auto & kernel : scalars)
    {
      kernel->reinit();
      const std::vector<dof_id_type> & dof_indices = kernel->variable().dofIndices();
      const DofMap & dof_map = kernel->variable().dofMap();
      const dof_id_type first_dof = dof_map.first_dof();
      const dof_id_type end_dof = dof_map.end_dof();
      for (dof_id_type dof : dof_indices)
      {
        if (dof >= first_dof && dof < end_dof)
        {
          kernel->computeJacobian();
          _fe_problem.addJacobianOffDiagScalar(kernel->variable().number());
          have_scalar_contributions = true;
          break;
        }
      }
    }

    if (have_scalar_contributions)
      _fe_problem.addJacobianScalar();
  }
}

void
NonlinearSystemBase::jacobianSetup()
{
  SolverSystem::jacobianSetup();

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _kernels.jacobianSetup(tid);
    _nodal_kernels.jacobianSetup(tid);
    _dirac_kernels.jacobianSetup(tid);
    if (_doing_dg)
      _dg_kernels.jacobianSetup(tid);
    _interface_kernels.jacobianSetup(tid);
    _element_dampers.jacobianSetup(tid);
    _nodal_dampers.jacobianSetup(tid);
    _integrated_bcs.jacobianSetup(tid);
  }
  _scalar_kernels.jacobianSetup();
  _constraints.jacobianSetup();
  _general_dampers.jacobianSetup();
  _nodal_bcs.jacobianSetup();

  // Avoid recursion
  if (this == &_fe_problem.currentNonlinearSystem())
    _fe_problem.jacobianSetup();
  _app.solutionInvalidity().resetSolutionInvalidCurrentIteration();
}

void
NonlinearSystemBase::computeJacobianInternal(const std::set<TagID> & tags)
{
  TIME_SECTION("computeJacobianInternal", 3);

  _fe_problem.setCurrentNonlinearSystem(number());

  // Make matrix ready to use
  activeAllMatrixTags();

  for (auto tag : tags)
  {
    if (!hasMatrix(tag))
      continue;

    auto & jacobian = getMatrix(tag);
    // Necessary for speed
    if (auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&jacobian))
    {
      LibmeshPetscCall(MatSetOption(petsc_matrix->mat(),
                                    MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                                    PETSC_TRUE));
      if (!_fe_problem.errorOnJacobianNonzeroReallocation())
        LibmeshPetscCall(
            MatSetOption(petsc_matrix->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE));
    }
  }

  jacobianSetup();

  // Jacobian contributions from UOs - for now this is used for ray tracing
  // and ray kernels that contribute to the Jacobian (think line sources)
  std::vector<UserObject *> uos;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribExecOns>(EXEC_PRE_KERNELS)
      .queryInto(uos);
  for (auto & uo : uos)
    uo->jacobianSetup();
  for (auto & uo : uos)
  {
    uo->initialize();
    uo->execute();
    uo->finalize();
  }

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  PARALLEL_TRY
  {
    // We would like to compute ScalarKernels, block NodalKernels, FVFluxKernels, and mortar objects
    // up front because we want these included whether we are computing an ordinary Jacobian or a
    // Jacobian for determining variable scaling factors
    computeScalarKernelsJacobians(tags);

    // Block restricted Nodal Kernels
    if (_nodal_kernels.hasActiveBlockObjects())
    {
      ComputeNodalKernelJacobiansThread cnkjt(_fe_problem, *this, _nodal_kernels, tags);
      const ConstNodeRange & range = _fe_problem.getCurrentAlgebraicNodeRange();
      Threads::parallel_reduce(range, cnkjt);

      unsigned int n_threads = libMesh::n_threads();
      for (unsigned int i = 0; i < n_threads;
           i++) // Add any cached jacobians that might be hanging around
        _fe_problem.assembly(i, number()).addCachedJacobian(Assembly::GlobalDataKey{});
    }

    using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    if (_fe_problem.haveFV())
    {
      // the same loop works for both residual and jacobians because it keys
      // off of FEProblem's _currently_computing_jacobian parameter
      ComputeFVFluxJacobianThread<FVRange> fvj(
          _fe_problem, this->number(), tags, /*on_displaced=*/false);
      FVRange faces(_fe_problem.mesh().ownedFaceInfoBegin(), _fe_problem.mesh().ownedFaceInfoEnd());
      Threads::parallel_reduce(faces, fvj);
    }
    if (auto displaced_problem = _fe_problem.getDisplacedProblem();
        displaced_problem && displaced_problem->haveFV())
    {
      ComputeFVFluxJacobianThread<FVRange> fvr(
          _fe_problem, this->number(), tags, /*on_displaced=*/true);
      FVRange faces(displaced_problem->mesh().ownedFaceInfoBegin(),
                    displaced_problem->mesh().ownedFaceInfoEnd());
      Threads::parallel_reduce(faces, fvr);
    }

    mortarConstraints(Moose::ComputeType::Jacobian, {}, tags);

    // Get our element range for looping over
    const ConstElemRange & elem_range = _fe_problem.getCurrentAlgebraicElementRange();

    if (_fe_problem.computingScalingJacobian())
    {
      // Only compute Jacobians corresponding to the diagonals of volumetric compute objects
      // because this typically gives us a good representation of the physics. NodalBCs and
      // Constraints can introduce dramatically different scales (often order unity).
      // IntegratedBCs and/or InterfaceKernels may use penalty factors. DGKernels may be ok, but
      // they are almost always used in conjunction with Kernels
      ComputeJacobianForScalingThread cj(_fe_problem, tags);
      Threads::parallel_reduce(elem_range, cj);
      unsigned int n_threads = libMesh::n_threads();
      for (unsigned int i = 0; i < n_threads;
           i++) // Add any Jacobian contributions still hanging around
        _fe_problem.addCachedJacobian(i);

      // Check whether any exceptions were thrown and propagate this information for parallel
      // consistency before
      // 1) we do parallel communication when closing tagged matrices
      // 2) early returning before reaching our PARALLEL_CATCH below
      _fe_problem.checkExceptionAndStopSolve();

      closeTaggedMatrices(tags);

      return;
    }

    switch (_fe_problem.coupling())
    {
      case Moose::COUPLING_DIAG:
      {
        ComputeJacobianThread cj(_fe_problem, tags);
        Threads::parallel_reduce(elem_range, cj);

        unsigned int n_threads = libMesh::n_threads();
        for (unsigned int i = 0; i < n_threads;
             i++) // Add any Jacobian contributions still hanging around
          _fe_problem.addCachedJacobian(i);

        // Boundary restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBoundaryObjects())
        {
          ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, *this, _nodal_kernels, tags);
          const ConstBndNodeRange & bnd_range = _fe_problem.getCurrentAlgebraicBndNodeRange();

          Threads::parallel_reduce(bnd_range, cnkjt);
          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads;
               i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i, number()).addCachedJacobian(Assembly::GlobalDataKey{});
        }
      }
      break;

      default:
      case Moose::COUPLING_CUSTOM:
      {
        ComputeFullJacobianThread cj(_fe_problem, tags);
        Threads::parallel_reduce(elem_range, cj);
        unsigned int n_threads = libMesh::n_threads();

        for (unsigned int i = 0; i < n_threads; i++)
          _fe_problem.addCachedJacobian(i);

        // Boundary restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBoundaryObjects())
        {
          ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, *this, _nodal_kernels, tags);
          const ConstBndNodeRange & bnd_range = _fe_problem.getCurrentAlgebraicBndNodeRange();

          Threads::parallel_reduce(bnd_range, cnkjt);
          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads;
               i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i, number()).addCachedJacobian(Assembly::GlobalDataKey{});
        }
      }
      break;
    }

    computeDiracContributions(tags, true);

    static bool first = true;

    // This adds zeroes into geometric coupling entries to ensure they stay in the matrix
    if (first && (_add_implicit_geometric_coupling_entries_to_jacobian))
    {
      first = false;
      addImplicitGeometricCouplingEntries(_fe_problem.geomSearchData());

      if (_fe_problem.getDisplacedProblem())
        addImplicitGeometricCouplingEntries(_fe_problem.getDisplacedProblem()->geomSearchData());
    }
  }
  PARALLEL_CATCH;

  // Have no idea how to have constraints work
  // with the tag system
  PARALLEL_TRY
  {
    // Add in Jacobian contributions from other Constraints
    if (_fe_problem._has_constraints && tags.count(systemMatrixTag()))
    {
      // Some constraints need values from the Jacobian
      closeTaggedMatrices(tags);

      // Nodal Constraints
      enforceNodalConstraintsJacobian();

      // Undisplaced Constraints
      constraintJacobians(false);

      // Displaced Constraints
      if (_fe_problem.getDisplacedProblem())
        constraintJacobians(true);
    }
  }
  PARALLEL_CATCH;

  // We need to close the save_in variables on the aux system before NodalBCBases clear the dofs
  // on boundary nodes
  if (_has_diag_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  PARALLEL_TRY
  {
    MooseObjectWarehouse<NodalBCBase> * nbc_warehouse;
    // Select nodal kernels
    if (tags.size() == _fe_problem.numMatrixTags() || !tags.size())
      nbc_warehouse = &_nodal_bcs;
    else if (tags.size() == 1)
      nbc_warehouse = &(_nodal_bcs.getMatrixTagObjectWarehouse(*(tags.begin()), 0));
    else
      nbc_warehouse = &(_nodal_bcs.getMatrixTagsObjectWarehouse(tags, 0));

    if (nbc_warehouse->hasActiveObjects())
    {
      // We may be switching from add to set. Moreover, we rely on a call to MatZeroRows to enforce
      // the nodal boundary condition constraints which requires that the matrix be truly assembled
      // as opposed to just flushed. Consequently we can't do the following despite any desire to
      // keep our initial sparsity pattern honored (see https://gitlab.com/petsc/petsc/-/issues/852)
      //
      // flushTaggedMatrices(tags);
      closeTaggedMatrices(tags);

      // Cache the information about which BCs are coupled to which
      // variables, so we don't have to figure it out for each node.
      std::map<std::string, std::set<unsigned int>> bc_involved_vars;
      const std::set<BoundaryID> & all_boundary_ids = _mesh.getBoundaryIDs();
      for (const auto & bid : all_boundary_ids)
      {
        // Get reference to all the NodalBCs for this ID.  This is only
        // safe if there are NodalBCBases there to be gotten...
        if (nbc_warehouse->hasActiveBoundaryObjects(bid))
        {
          const auto & bcs = nbc_warehouse->getActiveBoundaryObjects(bid);
          for (const auto & bc : bcs)
          {
            const std::vector<MooseVariableFEBase *> & coupled_moose_vars =
                bc->getCoupledMooseVars();

            // Create the set of "involved" MOOSE nonlinear vars, which includes all coupled vars
            // and the BC's own variable
            std::set<unsigned int> & var_set = bc_involved_vars[bc->name()];
            for (const auto & coupled_var : coupled_moose_vars)
              if (coupled_var->kind() == Moose::VAR_SOLVER)
                var_set.insert(coupled_var->number());

            var_set.insert(bc->variable().number());
          }
        }
      }

      // reinit scalar variables again. This reinit does not re-fill any of the scalar variable
      // solution arrays because that was done above. It only will reorder the derivative
      // information for AD calculations to be suitable for NodalBC calculations
      for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
        _fe_problem.reinitScalars(tid, true);

      // Get variable coupling list.  We do all the NodalBCBase stuff on
      // thread 0...  The couplingEntries() data structure determines
      // which variables are "coupled" as far as the preconditioner is
      // concerned, not what variables a boundary condition specifically
      // depends on.
      auto & coupling_entries = _fe_problem.couplingEntries(/*_tid=*/0, this->number());

      // Compute Jacobians for NodalBCBases
      const ConstBndNodeRange & bnd_nodes = _fe_problem.getCurrentAlgebraicBndNodeRange();
      for (const auto & bnode : bnd_nodes)
      {
        BoundaryID boundary_id = bnode->_bnd_id;
        Node * node = bnode->_node;

        if (nbc_warehouse->hasActiveBoundaryObjects(boundary_id) &&
            node->processor_id() == processor_id())
        {
          _fe_problem.reinitNodeFace(node, boundary_id, 0);

          const auto & bcs = nbc_warehouse->getActiveBoundaryObjects(boundary_id);
          for (const auto & bc : bcs)
          {
            // Get the set of involved MOOSE vars for this BC
            std::set<unsigned int> & var_set = bc_involved_vars[bc->name()];

            // Loop over all the variables whose Jacobian blocks are
            // actually being computed, call computeOffDiagJacobian()
            // for each one which is actually coupled (otherwise the
            // value is zero.)
            for (const auto & it : coupling_entries)
            {
              unsigned int ivar = it.first->number(), jvar = it.second->number();

              // We are only going to call computeOffDiagJacobian() if:
              // 1.) the BC's variable is ivar
              // 2.) jvar is "involved" with the BC (including jvar==ivar), and
              // 3.) the BC should apply.
              if ((bc->variable().number() == ivar) && var_set.count(jvar) && bc->shouldApply())
                bc->computeOffDiagJacobian(jvar);
            }

            const auto & coupled_scalar_vars = bc->getCoupledMooseScalarVars();
            for (const auto & jvariable : coupled_scalar_vars)
              if (hasScalarVariable(jvariable->name()))
                bc->computeOffDiagJacobianScalar(jvariable->number());
          }
        }
      } // end loop over boundary nodes

      // Set the cached NodalBCBase values in the Jacobian matrix
      _fe_problem.assembly(0, number()).setCachedJacobian(Assembly::GlobalDataKey{});
    }
  }
  PARALLEL_CATCH;

  closeTaggedMatrices(tags);

  // We need to close the save_in variables on the aux system before NodalBCBases clear the dofs
  // on boundary nodes
  if (_has_nodalbc_diag_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  if (hasDiagSaveIn())
    _fe_problem.getAuxiliarySystem().update();

  // Accumulate the occurrence of solution invalid warnings for the current iteration cumulative
  // counters
  _app.solutionInvalidity().sync();
  _app.solutionInvalidity().solutionInvalidAccumulation();
}

void
NonlinearSystemBase::computeJacobian(SparseMatrix<Number> & jacobian)
{
  _nl_matrix_tags.clear();

  auto & tags = _fe_problem.getMatrixTags();

  for (auto & tag : tags)
    _nl_matrix_tags.insert(tag.second);

  computeJacobian(jacobian, _nl_matrix_tags);
}

void
NonlinearSystemBase::computeJacobian(SparseMatrix<Number> & jacobian, const std::set<TagID> & tags)
{
  associateMatrixToTag(jacobian, systemMatrixTag());

  computeJacobianTags(tags);

  disassociateMatrixFromTag(jacobian, systemMatrixTag());
}

void
NonlinearSystemBase::computeJacobianTags(const std::set<TagID> & tags)
{
  TIME_SECTION("computeJacobianTags", 5);

  FloatingPointExceptionGuard fpe_guard(_app);

  try
  {
    computeJacobianInternal(tags);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
}

void
NonlinearSystemBase::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks)
{
  _nl_matrix_tags.clear();

  auto & tags = _fe_problem.getMatrixTags();
  for (auto & tag : tags)
    _nl_matrix_tags.insert(tag.second);

  computeJacobianBlocks(blocks, _nl_matrix_tags);
}

void
NonlinearSystemBase::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks,
                                           const std::set<TagID> & tags)
{
  TIME_SECTION("computeJacobianBlocks", 3);
  FloatingPointExceptionGuard fpe_guard(_app);

  for (unsigned int i = 0; i < blocks.size(); i++)
  {
    SparseMatrix<Number> & jacobian = blocks[i]->_jacobian;

    LibmeshPetscCall(MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                                  MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                                  PETSC_TRUE));
    if (!_fe_problem.errorOnJacobianNonzeroReallocation())
      LibmeshPetscCall(MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                                    MAT_NEW_NONZERO_ALLOCATION_ERR,
                                    PETSC_TRUE));

    jacobian.zero();
  }

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  PARALLEL_TRY
  {
    const ConstElemRange & elem_range = _fe_problem.getCurrentAlgebraicElementRange();
    ComputeJacobianBlocksThread cjb(_fe_problem, blocks, tags);
    Threads::parallel_reduce(elem_range, cjb);
  }
  PARALLEL_CATCH;

  for (unsigned int i = 0; i < blocks.size(); i++)
    blocks[i]->_jacobian.close();

  for (unsigned int i = 0; i < blocks.size(); i++)
  {
    libMesh::System & precond_system = blocks[i]->_precond_system;
    SparseMatrix<Number> & jacobian = blocks[i]->_jacobian;

    unsigned int ivar = blocks[i]->_ivar;
    unsigned int jvar = blocks[i]->_jvar;

    // Dirichlet BCs
    std::vector<numeric_index_type> zero_rows;
    PARALLEL_TRY
    {
      const ConstBndNodeRange & bnd_nodes = _fe_problem.getCurrentAlgebraicBndNodeRange();
      for (const auto & bnode : bnd_nodes)
      {
        BoundaryID boundary_id = bnode->_bnd_id;
        Node * node = bnode->_node;

        if (_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
        {
          const auto & bcs = _nodal_bcs.getActiveBoundaryObjects(boundary_id);

          if (node->processor_id() == processor_id())
          {
            _fe_problem.reinitNodeFace(node, boundary_id, 0);

            for (const auto & bc : bcs)
              if (bc->variable().number() == ivar && bc->shouldApply())
              {
                // The first zero is for the variable number... there is only one variable in
                // each mini-system The second zero only works with Lagrange elements!
                zero_rows.push_back(node->dof_number(precond_system.number(), 0, 0));
              }
          }
        }
      }
    }
    PARALLEL_CATCH;

    jacobian.close();

    // This zeroes the rows corresponding to Dirichlet BCs and puts a 1.0 on the diagonal
    if (ivar == jvar)
      jacobian.zero_rows(zero_rows, 1.0);
    else
      jacobian.zero_rows(zero_rows, 0.0);

    jacobian.close();
  }
}

void
NonlinearSystemBase::updateActive(THREAD_ID tid)
{
  _element_dampers.updateActive(tid);
  _nodal_dampers.updateActive(tid);
  _integrated_bcs.updateActive(tid);
  _dg_kernels.updateActive(tid);
  _interface_kernels.updateActive(tid);
  _dirac_kernels.updateActive(tid);
  _kernels.updateActive(tid);
  _nodal_kernels.updateActive(tid);
  if (tid == 0)
  {
    _general_dampers.updateActive();
    _nodal_bcs.updateActive();
    _preset_nodal_bcs.updateActive();
    _ad_preset_nodal_bcs.updateActive();
    _constraints.updateActive();
    _scalar_kernels.updateActive();
  }
}

Real
NonlinearSystemBase::computeDamping(const NumericVector<Number> & solution,
                                    const NumericVector<Number> & update)
{
  // Default to no damping
  Real damping = 1.0;
  bool has_active_dampers = false;

  try
  {
    if (_element_dampers.hasActiveObjects())
    {
      PARALLEL_TRY
      {
        TIME_SECTION("computeDampers", 3, "Computing Dampers");
        has_active_dampers = true;
        *_increment_vec = update;
        ComputeElemDampingThread cid(_fe_problem, *this);
        Threads::parallel_reduce(_fe_problem.getCurrentAlgebraicElementRange(), cid);
        damping = std::min(cid.damping(), damping);
      }
      PARALLEL_CATCH;
    }

    if (_nodal_dampers.hasActiveObjects())
    {
      PARALLEL_TRY
      {
        TIME_SECTION("computeDamping::element", 3, "Computing Element Damping");

        has_active_dampers = true;
        *_increment_vec = update;
        ComputeNodalDampingThread cndt(_fe_problem, *this);
        Threads::parallel_reduce(_fe_problem.getCurrentAlgebraicNodeRange(), cndt);
        damping = std::min(cndt.damping(), damping);
      }
      PARALLEL_CATCH;
    }

    if (_general_dampers.hasActiveObjects())
    {
      PARALLEL_TRY
      {
        TIME_SECTION("computeDamping::general", 3, "Computing General Damping");

        has_active_dampers = true;
        const auto & gdampers = _general_dampers.getActiveObjects();
        for (const auto & damper : gdampers)
        {
          Real gd_damping = damper->computeDamping(solution, update);
          try
          {
            damper->checkMinDamping(gd_damping);
          }
          catch (MooseException & e)
          {
            _fe_problem.setException(e.what());
          }
          damping = std::min(gd_damping, damping);
        }
      }
      PARALLEL_CATCH;
    }
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }

  _communicator.min(damping);

  if (has_active_dampers && damping < 1.0)
    _console << " Damping factor: " << damping << std::endl;

  return damping;
}

void
NonlinearSystemBase::computeDiracContributions(const std::set<TagID> & tags, bool is_jacobian)
{
  _fe_problem.clearDiracInfo();

  std::set<const Elem *> dirac_elements;

  if (_dirac_kernels.hasActiveObjects())
  {
    TIME_SECTION("computeDirac", 3, "Computing DiracKernels");

    // TODO: Need a threading fix... but it's complicated!
    for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
    {
      const auto & dkernels = _dirac_kernels.getActiveObjects(tid);
      for (const auto & dkernel : dkernels)
      {
        dkernel->clearPoints();
        dkernel->addPoints();
      }
    }

    ComputeDiracThread cd(_fe_problem, tags, is_jacobian);

    _fe_problem.getDiracElements(dirac_elements);

    DistElemRange range(dirac_elements.begin(), dirac_elements.end(), 1);
    // TODO: Make Dirac work thread!
    // Threads::parallel_reduce(range, cd);

    cd(range);
  }
}

NumericVector<Number> &
NonlinearSystemBase::residualCopy()
{
  if (!_residual_copy.get())
    _residual_copy = NumericVector<Number>::build(_communicator);

  return *_residual_copy;
}

NumericVector<Number> &
NonlinearSystemBase::residualGhosted()
{
  _need_residual_ghosted = true;
  if (!_residual_ghosted)
  {
    // The first time we realize we need a ghosted residual vector,
    // we add it.
    _residual_ghosted = &addVector("residual_ghosted", false, GHOSTED);

    // If we've already realized we need time and/or non-time
    // residual vectors, but we haven't yet realized they need to be
    // ghosted, fix that now.
    //
    // If an application changes its mind, the libMesh API lets us
    // change the vector.
    if (_Re_time)
    {
      const auto vector_name = _subproblem.vectorTagName(_Re_time_tag);
      _Re_time = &system().add_vector(vector_name, false, GHOSTED);
    }
    if (_Re_non_time)
    {
      const auto vector_name = _subproblem.vectorTagName(_Re_non_time_tag);
      _Re_non_time = &system().add_vector(vector_name, false, GHOSTED);
    }
  }
  return *_residual_ghosted;
}

void
NonlinearSystemBase::augmentSparsity(SparsityPattern::Graph & sparsity,
                                     std::vector<dof_id_type> & n_nz,
                                     std::vector<dof_id_type> & n_oz)
{
  if (_add_implicit_geometric_coupling_entries_to_jacobian)
  {
    _fe_problem.updateGeomSearch();

    std::unordered_map<dof_id_type, std::vector<dof_id_type>> graph;

    findImplicitGeometricCouplingEntries(_fe_problem.geomSearchData(), graph);

    if (_fe_problem.getDisplacedProblem())
      findImplicitGeometricCouplingEntries(_fe_problem.getDisplacedProblem()->geomSearchData(),
                                           graph);

    const dof_id_type first_dof_on_proc = dofMap().first_dof(processor_id());
    const dof_id_type end_dof_on_proc = dofMap().end_dof(processor_id());

    // The total number of dofs on and off processor
    const dof_id_type n_dofs_on_proc = dofMap().n_local_dofs();
    const dof_id_type n_dofs_not_on_proc = dofMap().n_dofs() - dofMap().n_local_dofs();

    for (const auto & git : graph)
    {
      dof_id_type dof = git.first;
      dof_id_type local_dof = dof - first_dof_on_proc;

      if (dof < first_dof_on_proc || dof >= end_dof_on_proc)
        continue;

      const auto & row = git.second;

      SparsityPattern::Row & sparsity_row = sparsity[local_dof];

      unsigned int original_row_length = sparsity_row.size();

      sparsity_row.insert(sparsity_row.end(), row.begin(), row.end());

      SparsityPattern::sort_row(
          sparsity_row.begin(), sparsity_row.begin() + original_row_length, sparsity_row.end());

      // Fix up nonzero arrays
      for (const auto & coupled_dof : row)
      {
        if (coupled_dof < first_dof_on_proc || coupled_dof >= end_dof_on_proc)
        {
          if (n_oz[local_dof] < n_dofs_not_on_proc)
            n_oz[local_dof]++;
        }
        else
        {
          if (n_nz[local_dof] < n_dofs_on_proc)
            n_nz[local_dof]++;
        }
      }
    }
  }
}

void
NonlinearSystemBase::setSolutionUDot(const NumericVector<Number> & u_dot)
{
  *_u_dot = u_dot;
}

void
NonlinearSystemBase::setSolutionUDotDot(const NumericVector<Number> & u_dotdot)
{
  *_u_dotdot = u_dotdot;
}

void
NonlinearSystemBase::setSolutionUDotOld(const NumericVector<Number> & u_dot_old)
{
  *_u_dot_old = u_dot_old;
}

void
NonlinearSystemBase::setSolutionUDotDotOld(const NumericVector<Number> & u_dotdot_old)
{
  *_u_dotdot_old = u_dotdot_old;
}

void
NonlinearSystemBase::setPreconditioner(std::shared_ptr<MoosePreconditioner> pc)
{
  if (_preconditioner.get() != nullptr)
    mooseError("More than one active Preconditioner detected");

  _preconditioner = pc;
}

MoosePreconditioner const *
NonlinearSystemBase::getPreconditioner() const
{
  return _preconditioner.get();
}

void
NonlinearSystemBase::setupDampers()
{
  _increment_vec = &_sys.add_vector("u_increment", true, GHOSTED);
}

void
NonlinearSystemBase::reinitIncrementAtQpsForDampers(THREAD_ID /*tid*/,
                                                    const std::set<MooseVariable *> & damped_vars)
{
  for (const auto & var : damped_vars)
    var->computeIncrementAtQps(*_increment_vec);
}

void
NonlinearSystemBase::reinitIncrementAtNodeForDampers(THREAD_ID /*tid*/,
                                                     const std::set<MooseVariable *> & damped_vars)
{
  for (const auto & var : damped_vars)
    var->computeIncrementAtNode(*_increment_vec);
}

void
NonlinearSystemBase::checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains) const
{
  // Obtain all blocks and variables covered by all kernels
  std::set<SubdomainID> input_subdomains;
  std::set<std::string> kernel_variables;

  bool global_kernels_exist = false;
  global_kernels_exist |= _scalar_kernels.hasActiveObjects();
  global_kernels_exist |= _nodal_kernels.hasActiveObjects();

  _kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _dg_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _nodal_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _scalar_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _constraints.subdomainsCovered(input_subdomains, kernel_variables);

  if (_fe_problem.haveFV())
  {
    std::vector<FVElementalKernel *> fv_elemental_kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVElementalKernel")
        .queryInto(fv_elemental_kernels);

    for (auto fv_kernel : fv_elemental_kernels)
    {
      if (fv_kernel->blockRestricted())
        for (auto block_id : fv_kernel->blockIDs())
          input_subdomains.insert(block_id);
      else
        global_kernels_exist = true;
      kernel_variables.insert(fv_kernel->variable().name());

      // Check for lagrange multiplier
      if (dynamic_cast<FVScalarLagrangeMultiplierConstraint *>(fv_kernel))
        kernel_variables.insert(dynamic_cast<FVScalarLagrangeMultiplierConstraint *>(fv_kernel)
                                    ->lambdaVariable()
                                    .name());
    }

    std::vector<FVFluxKernel *> fv_flux_kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVFluxKernel")
        .queryInto(fv_flux_kernels);

    for (auto fv_kernel : fv_flux_kernels)
    {
      if (fv_kernel->blockRestricted())
        for (auto block_id : fv_kernel->blockIDs())
          input_subdomains.insert(block_id);
      else
        global_kernels_exist = true;
      kernel_variables.insert(fv_kernel->variable().name());
    }

    std::vector<FVInterfaceKernel *> fv_interface_kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVInterfaceKernel")
        .queryInto(fv_interface_kernels);

    for (auto fvik : fv_interface_kernels)
      if (auto scalar_fvik = dynamic_cast<FVScalarLagrangeMultiplierInterface *>(fvik))
        kernel_variables.insert(scalar_fvik->lambdaVariable().name());

    std::vector<FVFluxBC *> fv_flux_bcs;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVFluxBC")
        .queryInto(fv_flux_bcs);

    for (auto fvbc : fv_flux_bcs)
      if (auto scalar_fvbc = dynamic_cast<FVBoundaryScalarLagrangeMultiplierConstraint *>(fvbc))
        kernel_variables.insert(scalar_fvbc->lambdaVariable().name());
  }

  // Check kernel coverage of subdomains (blocks) in your mesh
  if (!global_kernels_exist)
  {
    std::set<SubdomainID> difference;
    std::set_difference(mesh_subdomains.begin(),
                        mesh_subdomains.end(),
                        input_subdomains.begin(),
                        input_subdomains.end(),
                        std::inserter(difference, difference.end()));

    // there supposed to be no kernels on this lower-dimensional subdomain
    for (const auto & id : _mesh.interiorLowerDBlocks())
      difference.erase(id);
    for (const auto & id : _mesh.boundaryLowerDBlocks())
      difference.erase(id);

    if (!difference.empty())
    {
      std::vector<SubdomainID> difference_vec =
          std::vector<SubdomainID>(difference.begin(), difference.end());
      std::vector<SubdomainName> difference_names = _mesh.getSubdomainNames(difference_vec);
      std::stringstream missing_block_names;
      std::copy(difference_names.begin(),
                difference_names.end(),
                std::ostream_iterator<std::string>(missing_block_names, " "));
      std::stringstream missing_block_ids;
      std::copy(difference.begin(),
                difference.end(),
                std::ostream_iterator<unsigned int>(missing_block_ids, " "));

      mooseError("Each subdomain must contain at least one Kernel.\nThe following block(s) lack an "
                 "active kernel: " +
                     missing_block_names.str(),
                 " (ids: ",
                 missing_block_ids.str(),
                 ")");
    }
  }

  // Check kernel use of variables
  std::set<VariableName> variables(getVariableNames().begin(), getVariableNames().end());

  std::set<VariableName> difference;
  std::set_difference(variables.begin(),
                      variables.end(),
                      kernel_variables.begin(),
                      kernel_variables.end(),
                      std::inserter(difference, difference.end()));

  // skip checks for varaibles defined on lower-dimensional subdomain
  std::set<VariableName> vars(difference);
  for (auto & var_name : vars)
  {
    auto blks = getSubdomainsForVar(var_name);
    for (const auto & id : blks)
      if (_mesh.interiorLowerDBlocks().count(id) > 0 || _mesh.boundaryLowerDBlocks().count(id) > 0)
        difference.erase(var_name);
  }

  if (!difference.empty())
  {
    std::stringstream missing_kernel_vars;
    std::copy(difference.begin(),
              difference.end(),
              std::ostream_iterator<std::string>(missing_kernel_vars, " "));
    mooseError("Each variable must be referenced by at least one active Kernel.\nThe following "
               "variable(s) lack an active kernel: " +
               missing_kernel_vars.str());
  }
}

bool
NonlinearSystemBase::containsTimeKernel()
{
  auto & time_kernels = _kernels.getVectorTagObjectWarehouse(timeVectorTag(), 0);

  return time_kernels.hasActiveObjects();
}

std::vector<std::string>
NonlinearSystemBase::timeKernelVariableNames()
{
  std::vector<std::string> variable_names;
  const auto & time_kernels = _kernels.getVectorTagObjectWarehouse(timeVectorTag(), 0);
  if (time_kernels.hasActiveObjects())
    for (const auto & kernel : time_kernels.getObjects())
      variable_names.push_back(kernel->variable().name());

  return variable_names;
}

bool
NonlinearSystemBase::needBoundaryMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const
{
  return _integrated_bcs.hasActiveBoundaryObjects(bnd_id, tid);
}

bool
NonlinearSystemBase::needInterfaceMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const
{
  return _interface_kernels.hasActiveBoundaryObjects(bnd_id, tid);
}

bool
NonlinearSystemBase::needSubdomainMaterialOnSide(SubdomainID /*subdomain_id*/,
                                                 THREAD_ID /*tid*/) const
{
  return _doing_dg;
}

bool
NonlinearSystemBase::doingDG() const
{
  return _doing_dg;
}

void
NonlinearSystemBase::setPreviousNewtonSolution(const NumericVector<Number> & soln)
{
  if (hasVector(Moose::PREVIOUS_NL_SOLUTION_TAG))
    getVector(Moose::PREVIOUS_NL_SOLUTION_TAG) = soln;
}

void
NonlinearSystemBase::mortarConstraints(const Moose::ComputeType compute_type,
                                       const std::set<TagID> & vector_tags,
                                       const std::set<TagID> & matrix_tags)
{
  parallel_object_only();

  try
  {
    for (auto & map_pr : _undisplaced_mortar_functors)
      map_pr.second(compute_type, vector_tags, matrix_tags);

    for (auto & map_pr : _displaced_mortar_functors)
      map_pr.second(compute_type, vector_tags, matrix_tags);
  }
  catch (MetaPhysicL::LogicError &)
  {
    mooseError(
        "We caught a MetaPhysicL error in NonlinearSystemBase::mortarConstraints. This is very "
        "likely due to AD not having a sufficiently large derivative container size. Please run "
        "MOOSE configure with the '--with-derivative-size=<n>' option");
  }
}

void
NonlinearSystemBase::setupScalingData()
{
  if (_auto_scaling_initd)
    return;

  // Want the libMesh count of variables, not MOOSE, e.g. I don't care about array variable counts
  const auto n_vars = system().n_vars();

  if (_scaling_group_variables.empty())
  {
    _var_to_group_var.reserve(n_vars);
    _num_scaling_groups = n_vars;

    for (const auto var_number : make_range(n_vars))
      _var_to_group_var.emplace(var_number, var_number);
  }
  else
  {
    std::set<unsigned int> var_numbers, var_numbers_covered, var_numbers_not_covered;
    for (const auto var_number : make_range(n_vars))
      var_numbers.insert(var_number);

    _num_scaling_groups = _scaling_group_variables.size();

    for (const auto group_index : index_range(_scaling_group_variables))
      for (const auto & var_name : _scaling_group_variables[group_index])
      {
        if (!hasVariable(var_name) && !hasScalarVariable(var_name))
          mooseError("'",
                     var_name,
                     "', provided to the 'scaling_group_variables' parameter, does not exist in "
                     "the nonlinear system.");

        const MooseVariableBase & var =
            hasVariable(var_name)
                ? static_cast<MooseVariableBase &>(getVariable(0, var_name))
                : static_cast<MooseVariableBase &>(getScalarVariable(0, var_name));
        auto map_pair = _var_to_group_var.emplace(var.number(), group_index);
        if (!map_pair.second)
          mooseError("Variable ", var_name, " is contained in multiple scaling grouplings");
        var_numbers_covered.insert(var.number());
      }

    std::set_difference(var_numbers.begin(),
                        var_numbers.end(),
                        var_numbers_covered.begin(),
                        var_numbers_covered.end(),
                        std::inserter(var_numbers_not_covered, var_numbers_not_covered.begin()));

    _num_scaling_groups = _scaling_group_variables.size() + var_numbers_not_covered.size();

    auto index = static_cast<unsigned int>(_scaling_group_variables.size());
    for (auto var_number : var_numbers_not_covered)
      _var_to_group_var.emplace(var_number, index++);
  }

  _variable_autoscaled.resize(n_vars, true);
  const auto & number_to_var_map = _vars[0].numberToVariableMap();

  if (_ignore_variables_for_autoscaling.size())
    for (const auto i : index_range(_variable_autoscaled))
      if (std::find(_ignore_variables_for_autoscaling.begin(),
                    _ignore_variables_for_autoscaling.end(),
                    libmesh_map_find(number_to_var_map, i)->name()) !=
          _ignore_variables_for_autoscaling.end())
        _variable_autoscaled[i] = false;

  _auto_scaling_initd = true;
}

bool
NonlinearSystemBase::computeScaling()
{
  if (_compute_scaling_once && _computed_scaling)
    return true;

  _console << "\nPerforming automatic scaling calculation\n" << std::endl;

  TIME_SECTION("computeScaling", 3, "Computing Automatic Scaling");

  // It's funny but we need to assemble our vector of scaling factors here otherwise we will be
  // applying scaling factors of 0 during Assembly of our scaling Jacobian
  assembleScalingVector();

  // container for repeated access of element global dof indices
  std::vector<dof_id_type> dof_indices;

  if (!_auto_scaling_initd)
    setupScalingData();

  std::vector<Real> inverse_scaling_factors(_num_scaling_groups, 0);
  std::vector<Real> resid_inverse_scaling_factors(_num_scaling_groups, 0);
  std::vector<Real> jac_inverse_scaling_factors(_num_scaling_groups, 0);
  auto & dof_map = dofMap();

  // what types of scaling do we want?
  bool jac_scaling = _resid_vs_jac_scaling_param < 1. - TOLERANCE;
  bool resid_scaling = _resid_vs_jac_scaling_param > TOLERANCE;

  const NumericVector<Number> & scaling_residual = RHS();

  if (jac_scaling)
  {
    // if (!_auto_scaling_initd)
    // We need to reinit this when the number of dofs changes
    // but there is no good way to track that
    // In theory, it is the job of libmesh system to track this,
    // but this special matrix is not owned by libMesh system
    // Let us reinit eveytime since it is not expensive
    {
      auto init_vector = NumericVector<Number>::build(this->comm());
      init_vector->init(system().n_dofs(), system().n_local_dofs(), /*fast=*/false, PARALLEL);

      _scaling_matrix->clear();
      _scaling_matrix->init(*init_vector);
    }

    _fe_problem.computingScalingJacobian(true);
    // Dispatch to derived classes to ensure that we use the correct matrix tag
    computeScalingJacobian();
    _fe_problem.computingScalingJacobian(false);
  }

  if (resid_scaling)
  {
    _fe_problem.computingScalingResidual(true);
    _fe_problem.computingNonlinearResid(true);
    // Dispatch to derived classes to ensure that we use the correct vector tag
    computeScalingResidual();
    _fe_problem.computingNonlinearResid(false);
    _fe_problem.computingScalingResidual(false);
  }

  // Did something bad happen during residual/Jacobian scaling computation?
  if (_fe_problem.getFailNextNonlinearConvergenceCheck())
    return false;

  auto examine_dof_indices = [this,
                              jac_scaling,
                              resid_scaling,
                              &dof_map,
                              &jac_inverse_scaling_factors,
                              &resid_inverse_scaling_factors,
                              &scaling_residual](const auto & dof_indices, const auto var_number)
  {
    for (auto dof_index : dof_indices)
      if (dof_map.local_index(dof_index))
      {
        if (jac_scaling)
        {
          // For now we will use the diagonal for determining scaling
          auto mat_value = (*_scaling_matrix)(dof_index, dof_index);
          auto & factor = jac_inverse_scaling_factors[_var_to_group_var[var_number]];
          factor = std::max(factor, std::abs(mat_value));
        }
        if (resid_scaling)
        {
          auto vec_value = scaling_residual(dof_index);
          auto & factor = resid_inverse_scaling_factors[_var_to_group_var[var_number]];
          factor = std::max(factor, std::abs(vec_value));
        }
      }
  };

  // Compute our scaling factors for the spatial field variables
  for (const auto & elem : _fe_problem.getCurrentAlgebraicElementRange())
    for (const auto i : make_range(system().n_vars()))
      if (_variable_autoscaled[i] && system().variable_type(i).family != SCALAR)
      {
        dof_map.dof_indices(elem, dof_indices, i);
        examine_dof_indices(dof_indices, i);
      }

  for (const auto i : make_range(system().n_vars()))
    if (_variable_autoscaled[i] && system().variable_type(i).family == SCALAR)
    {
      dof_map.SCALAR_dof_indices(dof_indices, i);
      examine_dof_indices(dof_indices, i);
    }

  if (resid_scaling)
    _communicator.max(resid_inverse_scaling_factors);
  if (jac_scaling)
    _communicator.max(jac_inverse_scaling_factors);

  if (jac_scaling && resid_scaling)
    for (MooseIndex(inverse_scaling_factors) i = 0; i < inverse_scaling_factors.size(); ++i)
    {
      // Be careful not to take log(0)
      if (!resid_inverse_scaling_factors[i])
      {
        if (!jac_inverse_scaling_factors[i])
          inverse_scaling_factors[i] = 1;
        else
          inverse_scaling_factors[i] = jac_inverse_scaling_factors[i];
      }
      else if (!jac_inverse_scaling_factors[i])
        // We know the resid is not zero
        inverse_scaling_factors[i] = resid_inverse_scaling_factors[i];
      else
        inverse_scaling_factors[i] =
            std::exp(_resid_vs_jac_scaling_param * std::log(resid_inverse_scaling_factors[i]) +
                     (1 - _resid_vs_jac_scaling_param) * std::log(jac_inverse_scaling_factors[i]));
    }
  else if (jac_scaling)
    inverse_scaling_factors = jac_inverse_scaling_factors;
  else if (resid_scaling)
    inverse_scaling_factors = resid_inverse_scaling_factors;
  else
    mooseError("We shouldn't be calling this routine if we're not performing any scaling");

  // We have to make sure that our scaling values are not zero
  for (auto & scaling_factor : inverse_scaling_factors)
    if (scaling_factor == 0)
      scaling_factor = 1;

  // Now flatten the group scaling factors to the individual variable scaling factors
  std::vector<Real> flattened_inverse_scaling_factors(system().n_vars());
  for (const auto i : index_range(flattened_inverse_scaling_factors))
    flattened_inverse_scaling_factors[i] = inverse_scaling_factors[_var_to_group_var[i]];

  // Now set the scaling factors for the variables
  applyScalingFactors(flattened_inverse_scaling_factors);
  if (auto displaced_problem = _fe_problem.getDisplacedProblem().get())
    displaced_problem->systemBaseNonlinear(number()).applyScalingFactors(
        flattened_inverse_scaling_factors);

  _computed_scaling = true;
  return true;
}

void
NonlinearSystemBase::assembleScalingVector()
{
  if (!hasVector("scaling_factors"))
    // No variables have indicated they need scaling
    return;

  auto & scaling_vector = getVector("scaling_factors");

  const auto & lm_mesh = _mesh.getMesh();
  const auto & dof_map = dofMap();

  const auto & field_variables = _vars[0].fieldVariables();
  const auto & scalar_variables = _vars[0].scalars();

  std::vector<dof_id_type> dof_indices;

  for (const Elem * const elem :
       as_range(lm_mesh.active_local_elements_begin(), lm_mesh.active_local_elements_end()))
    for (const auto * const field_var : field_variables)
    {
      const auto & factors = field_var->arrayScalingFactor();
      for (const auto i : make_range(field_var->count()))
      {
        dof_map.dof_indices(elem, dof_indices, field_var->number() + i);
        for (const auto dof : dof_indices)
          scaling_vector.set(dof, factors[i]);
      }
    }

  for (const auto * const scalar_var : scalar_variables)
  {
    mooseAssert(scalar_var->count() == 1,
                "Scalar variables should always have only one component.");
    dof_map.SCALAR_dof_indices(dof_indices, scalar_var->number());
    for (const auto dof : dof_indices)
      scaling_vector.set(dof, scalar_var->scalingFactor());
  }

  // Parallel assemble
  scaling_vector.close();

  if (auto * displaced_problem = _fe_problem.getDisplacedProblem().get())
    // copy into the corresponding displaced system vector because they should be the exact same
    displaced_problem->systemBaseNonlinear(number()).getVector("scaling_factors") = scaling_vector;
}

bool
NonlinearSystemBase::preSolve()
{
  // Clear the iteration counters
  _current_l_its.clear();
  _current_nl_its = 0;

  // Initialize the solution vector using a predictor and known values from nodal bcs
  setInitialSolution();

  // Now that the initial solution has ben set, potentially perform a residual/Jacobian evaluation
  // to determine variable scaling factors
  if (_automatic_scaling)
  {
    const bool scaling_succeeded = computeScaling();
    if (!scaling_succeeded)
      return false;
  }

  // We do not know a priori what variable a global degree of freedom corresponds to, so we need a
  // map from global dof to scaling factor. We just use a ghosted NumericVector for that mapping
  assembleScalingVector();

  return true;
}

void
NonlinearSystemBase::destroyColoring()
{
  if (matrixFromColoring())
    LibmeshPetscCall(MatFDColoringDestroy(&_fdcoloring));
}
