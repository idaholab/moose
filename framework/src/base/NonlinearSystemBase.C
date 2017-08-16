/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "PetscSupport.h"
#include "Factory.h"
#include "ParallelUniqueId.h"
#include "ThreadedElementLoop.h"
#include "MaterialData.h"
#include "ComputeResidualThread.h"
#include "ComputeJacobianThread.h"
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
#include "PresetNodalBC.h"
#include "NodalBC.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
#include "InterfaceKernel.h"
#include "ElementDamper.h"
#include "NodalDamper.h"
#include "GeneralDamper.h"
#include "DisplacedProblem.h"
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "NodalConstraint.h"
#include "NodeFaceConstraint.h"
#include "FaceFaceConstraint.h"
#include "ElemElemConstraint.h"
#include "ScalarKernel.h"
#include "Parser.h"
#include "Split.h"
#include "FieldSplitPreconditioner.h"
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "NodalKernel.h"
#include "DiracKernel.h"
#include "NodalKernel.h"
#include "TimeIntegrator.h"
#include "Predictor.h"
#include "Assembly.h"
#include "ElementPairLocator.h"
#include "ODETimeKernel.h"
#include "AllLocalDofIndicesThread.h"

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

// PETSc
#ifdef LIBMESH_HAVE_PETSC
#include "petscsnes.h"
#if !PETSC_VERSION_LESS_THAN(3, 3, 0)
#include <PetscDMMoose.h>
EXTERN_C_BEGIN
extern PetscErrorCode DMCreate_Moose(DM);
EXTERN_C_END
#endif
#endif

NonlinearSystemBase::NonlinearSystemBase(FEProblemBase & fe_problem,
                                         System & sys,
                                         const std::string & name)
  : SystemBase(fe_problem, name, Moose::VAR_NONLINEAR),
    ConsoleStreamInterface(fe_problem.getMooseApp()),
    _fe_problem(fe_problem),
    _sys(sys),
    _last_rnorm(0.),
    _last_nl_rnorm(0.),
    _l_abs_step_tol(1e-10),
    _initial_residual_before_preset_bcs(0.),
    _initial_residual_after_preset_bcs(0.),
    _current_nl_its(0),
    _compute_initial_residual_before_preset_bcs(true),
    _current_solution(NULL),
    _residual_ghosted(NULL),
    _serialized_solution(*NumericVector<Number>::build(_communicator).release()),
    _solution_previous_nl(NULL),
    _residual_copy(*NumericVector<Number>::build(_communicator).release()),
    _u_dot(&addVector("u_dot", true, GHOSTED)),
    _Re_time(NULL),
    _Re_non_time(&addVector("Re_non_time", false, GHOSTED)),
    _scalar_kernels(/*threaded=*/false),
    _nodal_bcs(/*threaded=*/false),
    _preset_nodal_bcs(/*threaded=*/false),
    _splits(/*threaded=*/false),
    _increment_vec(NULL),
    _pc_side(Moose::PCS_DEFAULT),
    _ksp_norm(Moose::KSPN_UNPRECONDITIONED),
    _use_finite_differenced_preconditioner(false),
    _have_decomposition(false),
    _use_field_split_preconditioner(false),
    _add_implicit_geometric_coupling_entries_to_jacobian(false),
    _assemble_constraints_separately(false),
    _need_serialized_solution(false),
    _need_residual_copy(false),
    _need_residual_ghosted(false),
    _debugging_residuals(false),
    _doing_dg(false),
    _n_iters(0),
    _n_linear_iters(0),
    _n_residual_evaluations(0),
    _final_residual(0.),
    _computing_initial_residual(false),
    _print_all_var_norms(false),
    _has_save_in(false),
    _has_diag_save_in(false),
    _has_nodalbc_save_in(false),
    _has_nodalbc_diag_save_in(false)
{
}

NonlinearSystemBase::~NonlinearSystemBase()
{
  delete &_serialized_solution;
  delete &_residual_copy;
}

void
NonlinearSystemBase::init()
{
  Moose::setup_perf_log.push("NonlinerSystem::init()", "Setup");

  if (_fe_problem.hasDampers())
    setupDampers();

  _current_solution = _sys.current_local_solution.get();

  if (_need_serialized_solution)
    _serialized_solution.init(_sys.n_dofs(), false, SERIAL);

  if (_need_residual_copy)
    _residual_copy.init(_sys.n_dofs(), false, SERIAL);

  Moose::setup_perf_log.pop("NonlinerSystem::init()", "Setup");
}

void
NonlinearSystemBase::turnOffJacobian()
{
  system().set_basic_system_only();
  nonlinearSolver()->jacobian = NULL;
}

void
NonlinearSystemBase::addExtraVectors()
{
  if (_fe_problem.needsPreviousNewtonIteration())
    _solution_previous_nl = &addVector("u_previous_newton", true, GHOSTED);
}

void
NonlinearSystemBase::restoreSolutions()
{
  // call parent
  SystemBase::restoreSolutions();
  // and update _current_solution
  _current_solution = _sys.current_local_solution.get();
}

void
NonlinearSystemBase::initialSetup()
{
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
  }
  _scalar_kernels.initialSetup();
  _constraints.initialSetup();
  _general_dampers.initialSetup();
  _nodal_bcs.initialSetup();
}

void
NonlinearSystemBase::timestepSetup()
{
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
  }
  _scalar_kernels.initialSetup();
  _constraints.timestepSetup();
  _general_dampers.timestepSetup();
  _nodal_bcs.timestepSetup();
}

void
NonlinearSystemBase::setDecomposition(const std::vector<std::string> & splits)
{
  /// Although a single top-level split is allowed in Problem, treat it as a list of splits for conformity with the Split input syntax.
  if (splits.size() && splits.size() != 1)
  {
    std::ostringstream err;
    err << "Only a single top-level split is allowed in a Problem's decomposition.";
    mooseError(err.str());
  }
  if (splits.size())
  {
    _decomposition_split = splits[0];
    _have_decomposition = true;
  }
  else
  {
    _have_decomposition = false;
  }
}

void
NonlinearSystemBase::setupFieldDecomposition()
{
  if (!_have_decomposition)
    return;

  std::shared_ptr<Split> top_split = getSplit(_decomposition_split);
  top_split->setup();
}

void
NonlinearSystemBase::addTimeIntegrator(const std::string & type,
                                       const std::string & name,
                                       InputParameters parameters)
{
  parameters.set<SystemBase *>("_sys") = this;

  std::shared_ptr<TimeIntegrator> ti = _factory.create<TimeIntegrator>(type, name, parameters);
  _time_integrator = ti;
}

void
NonlinearSystemBase::addKernel(const std::string & kernel_name,
                               const std::string & name,
                               InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to warehouse
    std::shared_ptr<KernelBase> kernel =
        _factory.create<KernelBase>(kernel_name, name, parameters, tid);
    _kernels.addObject(kernel, tid);

    // Store time/non-time kernels separately
    std::shared_ptr<TimeKernel> t_kernel = std::dynamic_pointer_cast<TimeKernel>(kernel);
    if (t_kernel)
      _time_kernels.addObject(kernel, tid);
    else
      _non_time_kernels.addObject(kernel, tid);

    addEigenKernels(kernel, tid);
  }

  if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
NonlinearSystemBase::addNodalKernel(const std::string & kernel_name,
                                    const std::string & name,
                                    InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to the warehouse
    std::shared_ptr<NodalKernel> kernel =
        _factory.create<NodalKernel>(kernel_name, name, parameters, tid);
    _nodal_kernels.addObject(kernel, tid);
  }

  if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
NonlinearSystemBase::addScalarKernel(const std::string & kernel_name,
                                     const std::string & name,
                                     InputParameters parameters)
{
  std::shared_ptr<ScalarKernel> kernel =
      _factory.create<ScalarKernel>(kernel_name, name, parameters);
  _scalar_kernels.addObject(kernel);

  // Store time/non-time ScalarKernels separately
  ODETimeKernel * t_kernel = dynamic_cast<ODETimeKernel *>(kernel.get());

  if (t_kernel)
    _time_scalar_kernels.addObject(kernel);
  else
    _non_time_scalar_kernels.addObject(kernel);
}

void
NonlinearSystemBase::addBoundaryCondition(const std::string & bc_name,
                                          const std::string & name,
                                          InputParameters parameters)
{
  // ThreadID
  THREAD_ID tid = 0;

  // Create the object
  std::shared_ptr<BoundaryCondition> bc =
      _factory.create<BoundaryCondition>(bc_name, name, parameters, tid);

  // Active BoundaryIDs for the object
  const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  _vars[tid].addBoundaryVar(boundary_ids, &bc->variable());

  // Cast to the various types of BCs
  std::shared_ptr<NodalBC> nbc = std::dynamic_pointer_cast<NodalBC>(bc);
  std::shared_ptr<IntegratedBC> ibc = std::dynamic_pointer_cast<IntegratedBC>(bc);

  // NodalBC
  if (nbc)
  {
    _nodal_bcs.addObject(nbc);
    _vars[tid].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

    if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
      _has_nodalbc_save_in = true;
    if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
      _has_nodalbc_diag_save_in = true;

    // PresetNodalBC
    std::shared_ptr<PresetNodalBC> pnbc = std::dynamic_pointer_cast<PresetNodalBC>(bc);
    if (pnbc)
      _preset_nodal_bcs.addObject(pnbc);
  }

  // IntegratedBC
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

      // Active BoundaryIDs for the object
      const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
      _vars[tid].addBoundaryVar(boundary_ids, &bc->variable());

      ibc = std::static_pointer_cast<IntegratedBC>(bc);

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
                                   InputParameters parameters)
{
  std::shared_ptr<Constraint> constraint = _factory.create<Constraint>(c_name, name, parameters);
  _constraints.addObject(constraint);

  if (constraint && constraint->addCouplingEntriesToJacobian())
    addImplicitGeometricCouplingEntriesToJacobian(true);
}

void
NonlinearSystemBase::addDiracKernel(const std::string & kernel_name,
                                    const std::string & name,
                                    InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<DiracKernel> kernel =
        _factory.create<DiracKernel>(kernel_name, name, parameters, tid);
    _dirac_kernels.addObject(kernel, tid);
  }
}

void
NonlinearSystemBase::addDGKernel(std::string dg_kernel_name,
                                 const std::string & name,
                                 InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::shared_ptr<DGKernel> dg_kernel =
        _factory.create<DGKernel>(dg_kernel_name, name, parameters, tid);
    _dg_kernels.addObject(dg_kernel, tid);
  }

  _doing_dg = true;
}

void
NonlinearSystemBase::addInterfaceKernel(std::string interface_kernel_name,
                                        const std::string & name,
                                        InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    std::shared_ptr<InterfaceKernel> interface_kernel =
        _factory.create<InterfaceKernel>(interface_kernel_name, name, parameters, tid);

    const std::set<BoundaryID> & boundary_ids = interface_kernel->boundaryIDs();
    _vars[tid].addBoundaryVar(boundary_ids, &interface_kernel->variable());

    _interface_kernels.addObject(interface_kernel, tid);
    _vars[tid].addBoundaryVars(boundary_ids, interface_kernel->getCoupledVars());
  }

  _doing_dg = true;
}

void
NonlinearSystemBase::addDamper(const std::string & damper_name,
                               const std::string & name,
                               InputParameters parameters)
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
                              InputParameters parameters)
{
  std::shared_ptr<Split> split = _factory.create<Split>(split_name, name, parameters);
  _splits.addObject(split);
}

std::shared_ptr<Split>
NonlinearSystemBase::getSplit(const std::string & name)
{
  return _splits.getActiveObject(name);
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
NonlinearSystemBase::computeResidual(NumericVector<Number> & residual, Moose::KernelType type)
{
  Moose::perf_log.push("compute_residual()", "Execution");

  _n_residual_evaluations++;

  Moose::enableFPE();

  for (const auto & numeric_vec : _vecs_to_zero_for_residual)
    if (hasVector(numeric_vec))
    {
      NumericVector<Number> & vec = getVector(numeric_vec);
      vec.close();
      vec.zero();
    }

  try
  {
    residual.zero();
    if (_Re_time)
      _Re_time->zero();
    _Re_non_time->zero();
    computeResidualInternal(type);
    if (_Re_time)
      _Re_time->close();
    _Re_non_time->close();
    if (_time_integrator)
      _time_integrator->postStep(residual);
    else
      residual += *_Re_non_time;
    residual.close();

    computeNodalBCs(residual, type);

    // If we are debugging residuals we need one more assignment to have the ghosted copy up to date
    if (_need_residual_ghosted && _debugging_residuals)
    {
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

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_residual()", "Execution");
}

void
NonlinearSystemBase::onTimestepBegin()
{
  if (_time_integrator)
    _time_integrator->preSolve();
  if (_predictor.get())
    _predictor->timestepSetup();
}

void
NonlinearSystemBase::setInitialSolution()
{
  NumericVector<Number> & initial_solution(solution());
  if (_predictor.get() && _predictor->shouldApply())
  {
    _predictor->apply(initial_solution);
    _fe_problem.predictorCleanup(initial_solution);
  }

  // do nodal BC
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
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
    }
  }

  _sys.solution->close();
  update();

  // Set constraint slave values
  setConstraintSlaveValues(initial_solution, false);

  if (_fe_problem.getDisplacedProblem())
    setConstraintSlaveValues(initial_solution, true);
}

void
NonlinearSystemBase::setPredictor(std::shared_ptr<Predictor> predictor)
{
  _predictor = predictor;
}

void
NonlinearSystemBase::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  _kernels.subdomainSetup(subdomain, tid);
  _nodal_kernels.subdomainSetup(subdomain, tid);
  _element_dampers.subdomainSetup(subdomain, tid);
  _nodal_dampers.subdomainSetup(subdomain, tid);
}

NumericVector<Number> &
NonlinearSystemBase::solutionUDot()
{
  return *_u_dot;
}

NumericVector<Number> &
NonlinearSystemBase::residualVector(Moose::KernelType type)
{
  switch (type)
  {
    case Moose::KT_TIME:
      if (!_Re_time)
        _Re_time = &addVector("Re_time", false, GHOSTED);
      return *_Re_time;
    case Moose::KT_NONTIME:
      return *_Re_non_time;
    case Moose::KT_ALL:
      return *_Re_non_time;

    default:
      mooseError("Trying to get residual vector that is not available");
  }
}

bool
NonlinearSystemBase::hasResidualVector(Moose::KernelType type) const
{
  switch (type)
  {
    case Moose::KT_TIME:
      return _Re_time;
    case Moose::KT_NONTIME:
      return _Re_non_time;
    case Moose::KT_ALL:
      return _Re_non_time;

    default:
      mooseError("Trying to get residual vector that is not available");
  }
}

void
NonlinearSystemBase::computeTimeDerivatives()
{
  if (_time_integrator)
  {
    _time_integrator->preStep();
    _time_integrator->computeTimeDerivatives();
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
      std::vector<dof_id_type> & slave_node_ids = nc->getSlaveNodeId();
      std::vector<dof_id_type> & master_node_ids = nc->getMasterNodeId();

      if ((slave_node_ids.size() > 0) && (master_node_ids.size() > 0))
      {
        _fe_problem.reinitNodes(master_node_ids, tid);
        _fe_problem.reinitNodesNeighbor(slave_node_ids, tid);
        nc->computeResidual(residual);
      }
    }
    _fe_problem.addCachedResidualDirectly(residual, tid);
    residual.close();
  }
}

void
NonlinearSystemBase::enforceNodalConstraintsJacobian(SparseMatrix<Number> & jacobian)
{
  THREAD_ID tid = 0; // constraints are going to be done single-threaded
  jacobian.close();
  if (_constraints.hasActiveNodalConstraints())
  {
    const auto & ncs = _constraints.getActiveNodalConstraints();
    for (const auto & nc : ncs)
    {
      std::vector<dof_id_type> & slave_node_ids = nc->getSlaveNodeId();
      std::vector<dof_id_type> & master_node_ids = nc->getMasterNodeId();

      if ((slave_node_ids.size() > 0) && (master_node_ids.size() > 0))
      {
        _fe_problem.reinitNodes(master_node_ids, tid);
        _fe_problem.reinitNodesNeighbor(slave_node_ids, tid);
        nc->computeJacobian(jacobian);
      }
    }
    _fe_problem.addCachedJacobian(jacobian, tid);
    jacobian.close();
  }
}

bool
NonlinearSystemBase::updateLagMul(bool displaced)
{

  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  for (const auto & it : *penetration_locators)
  {
    PenetrationLocator & pen_loc = *(it.second);

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const auto & ncs = _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (const auto & nc : ncs)
      {
        if (nc->haveAugLM())
        {
          if (!nc->contactConverged())
          {
            nc->updateLagMul(false);
            // restoreSolutions();
            return true;
          }
        }
      }
    }
  }
  return false;
}

void
NonlinearSystemBase::initLagMul(bool displaced)
{
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  for (const auto & it : *penetration_locators)
  {
    PenetrationLocator & pen_loc = *(it.second);

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const auto & ncs = _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (const auto & nc : ncs)
      {
        if (nc->haveAugLM())
        {
          _console << "Initialize the Lagrangin Multiplier\n";
          nc->updateLagMul(true);
          return;
        }
      }
    }
  }
}

bool
NonlinearSystemBase::haveAugLM(bool displaced)
{
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      NULL;
  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool haveAugLagMul = false;

  for (const auto & it : *penetration_locators)
  {
    PenetrationLocator & pen_loc = *(it.second);

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const auto & ncs = _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);
      for (const auto & nc : ncs)
      {
        if (nc->haveAugLM())
        {
          _console << "The contact is enforced using augmented Lagrangin Multiplier\n";
          return true;
        }
      }
    }
  }
  return haveAugLagMul;
}

void
NonlinearSystemBase::setConstraintSlaveValues(NumericVector<Number> & solution, bool displaced)
{
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied = false;

  for (const auto & it : *penetration_locators)
  {
    PenetrationLocator & pen_loc = *(it.second);

    std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const auto & constraints =
          _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (unsigned int i = 0; i < slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.nodeRef(slave_node_num);

        if (slave_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            const Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // reinit variables at the node
            _fe_problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            _fe_problem.prepareAssembly(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _fe_problem.setNeighborSubdomainID(master_elem, 0);
            _fe_problem.reinitNeighborPhys(master_elem, master_side, points, 0);

            for (const auto & nfc : constraints)
              if (nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeSlaveValue(solution);
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

  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied;
  bool residual_has_inserted_values = false;
  if (!_assemble_constraints_separately)
    constraints_applied = false;
  for (const auto & it : *penetration_locators)
  {
    if (_assemble_constraints_separately)
    {
      // Reset the constraint_applied flag before each new constraint, as they need to be assembled
      // separately
      constraints_applied = false;
    }
    PenetrationLocator & pen_loc = *(it.second);

    std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const auto & constraints =
          _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (unsigned int i = 0; i < slave_nodes.size(); i++)
      {
        dof_id_type slave_node_num = slave_nodes[i];
        Node & slave_node = _mesh.nodeRef(slave_node_num);

        if (slave_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            const Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // *These next steps MUST be done in this order!*

            // This reinits the variables that exist on the slave node
            _fe_problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            // This will set aside residual and jacobian space for the variables that have dofs on
            // the slave node
            _fe_problem.prepareAssembly(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _fe_problem.setNeighborSubdomainID(master_elem, 0);
            _fe_problem.reinitNeighborPhys(master_elem, master_side, points, 0);

            for (const auto & nfc : constraints)
              if (nfc->shouldApply())
              {
                constraints_applied = true;
                nfc->computeResidual();

                if (nfc->overwriteSlaveResidual())
                {
                  _fe_problem.setResidual(residual, 0);
                  residual_has_inserted_values = true;
                }
                else
                  _fe_problem.cacheResidual(0);
                _fe_problem.cacheResidualNeighbor(0);
              }
          }
        }
      }
    }
    if (_assemble_constraints_separately)
    {
      // Make sure that slave contribution to master are assembled, and ghosts have been exchanged,
      // as current masters might become slaves on next iteration
      // and will need to contribute their former slaves' contributions
      // to the future masters.
      // See if constraints were applied anywhere
      _communicator.max(constraints_applied);

      if (constraints_applied)
      {
        // If any of the above constraints inserted values in the residual, it needs to be assembled
        // before adding the cached residuals below.
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

  THREAD_ID tid = 0;
  // go over mortar interfaces
  auto & ifaces = _mesh.getMortarInterfaces();
  for (const auto & iface : ifaces)
  {
    if (_constraints.hasActiveFaceFaceConstraints(iface->_name))
    {
      const auto & face_constraints = _constraints.getActiveFaceFaceConstraints(iface->_name);

      // go over elements on that interface
      const std::vector<Elem *> & elems = iface->_elems;
      for (const auto & elem : elems)
      {
        // for each element process constraints on the
        _fe_problem.setCurrentSubdomainID(elem, tid);
        _fe_problem.prepare(elem, tid);
        _fe_problem.reinitElem(elem, tid);

        for (const auto & ffc : face_constraints)
        {
          ffc->reinit();
          ffc->computeResidual();
        }
        _fe_problem.cacheResidual(tid);

        // evaluate residuals that go into master and slave side
        for (const auto & ffc : face_constraints)
        {
          ffc->reinitSide(Moose::Master);
          ffc->computeResidualSide(Moose::Master);
          _fe_problem.cacheResidual(tid);

          ffc->reinitSide(Moose::Slave);
          ffc->computeResidualSide(Moose::Slave);
          _fe_problem.cacheResidual(tid);
        }
      }
      _fe_problem.addCachedResidual(tid);
    }
  }

  // go over element-element constraint interface
  std::map<unsigned int, std::shared_ptr<ElementPairLocator>> * element_pair_locators = nullptr;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    element_pair_locators = &geom_search_data._element_pair_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    element_pair_locators = &displaced_geom_search_data._element_pair_locators;
  }

  for (const auto & it : *element_pair_locators)
  {
    ElementPairLocator & elem_pair_loc = *(it.second);

    if (_constraints.hasActiveElemElemConstraints(it.first))
    {
      // ElemElemConstraint objects
      const auto & _element_constraints = _constraints.getActiveElemElemConstraints(it.first);

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
          _fe_problem.reinitElemPhys(elem1, info._elem1_constraint_q_point, tid);
          _fe_problem.setNeighborSubdomainID(elem2, tid);
          _fe_problem.reinitNeighborPhys(elem2, info._elem2_constraint_q_point, tid);

          ec->subProblem().prepareShapes(ec->variable().number(), tid);
          ec->subProblem().prepareNeighborShapes(ec->variable().number(), tid);

          ec->reinit(info);
          ec->computeResidual();
          _fe_problem.cacheResidual(tid);
          _fe_problem.cacheResidualNeighbor(tid);
        }
        _fe_problem.addCachedResidual(tid);
      }
    }
  }
}

void
NonlinearSystemBase::computeResidualInternal(Moose::KernelType type)
{
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

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  // residual contributions from the domain
  PARALLEL_TRY
  {
    Moose::perf_log.push("computeKernels()", "Execution");

    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();

    ComputeResidualThread cr(_fe_problem, type);

    Threads::parallel_reduce(elem_range, cr);

    unsigned int n_threads = libMesh::n_threads();
    for (unsigned int i = 0; i < n_threads;
         i++) // Add any cached residuals that might be hanging around
      _fe_problem.addCachedResidual(i);

    Moose::perf_log.pop("computeKernels()", "Execution");
  }
  PARALLEL_CATCH;

  // residual contributions from the scalar kernels
  PARALLEL_TRY
  {
    // do scalar kernels (not sure how to thread this)
    if (_scalar_kernels.hasActiveObjects())
    {
      Moose::perf_log.push("computScalarKernels()", "Execution");

      const std::vector<std::shared_ptr<ScalarKernel>> * scalars;

      // Use the right subset of ScalarKernels depending on the KernelType.
      switch (type)
      {
        case Moose::KT_ALL:
          scalars = &(_scalar_kernels.getActiveObjects());
          break;

        case Moose::KT_TIME:
          scalars = &(_time_scalar_kernels.getActiveObjects());
          break;

        case Moose::KT_NONTIME:
          scalars = &(_non_time_scalar_kernels.getActiveObjects());
          break;

        default:
          mooseError("Unrecognized KernelType in computeResidualInternal().");
      }

      bool have_scalar_contributions = false;
      for (const auto & scalar_kernel : *scalars)
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

      Moose::perf_log.pop("computScalarKernels()", "Execution");
    }
  }
  PARALLEL_CATCH;

  // residual contributions from Block NodalKernels
  PARALLEL_TRY
  {
    if (_nodal_kernels.hasActiveBlockObjects())
    {
      Moose::perf_log.push("computNodalKernels()", "Execution");

      ComputeNodalKernelsThread cnk(_fe_problem, _nodal_kernels);

      ConstNodeRange & range = *_mesh.getLocalNodeRange();

      if (range.begin() != range.end())
      {
        _fe_problem.reinitNode(*range.begin(), 0);

        Threads::parallel_reduce(range, cnk);

        unsigned int n_threads = libMesh::n_threads();
        for (unsigned int i = 0; i < n_threads;
             i++) // Add any cached residuals that might be hanging around
          _fe_problem.addCachedResidual(i);
      }

      Moose::perf_log.pop("computNodalKernels()", "Execution");
    }
  }
  PARALLEL_CATCH;

  // residual contributions from boundary NodalKernels
  PARALLEL_TRY
  {
    if (_nodal_kernels.hasActiveBoundaryObjects())
    {
      Moose::perf_log.push("computNodalKernelBCs()", "Execution");

      ComputeNodalKernelBcsThread cnk(_fe_problem, _nodal_kernels);

      ConstBndNodeRange & bnd_node_range = *_mesh.getBoundaryNodeRange();

      Threads::parallel_reduce(bnd_node_range, cnk);

      unsigned int n_threads = libMesh::n_threads();
      for (unsigned int i = 0; i < n_threads;
           i++) // Add any cached residuals that might be hanging around
        _fe_problem.addCachedResidual(i);

      Moose::perf_log.pop("computNodalKernelBCs()", "Execution");
    }
  }
  PARALLEL_CATCH;

  if (_need_residual_copy)
  {
    _Re_non_time->close();
    _Re_non_time->localize(_residual_copy);
  }

  if (_need_residual_ghosted)
  {
    _Re_non_time->close();
    *_residual_ghosted = *_Re_non_time;
    _residual_ghosted->close();
  }

  PARALLEL_TRY { computeDiracContributions(); }
  PARALLEL_CATCH;

  if (_fe_problem._has_constraints)
  {
    PARALLEL_TRY { enforceNodalConstraintsResidual(*_Re_non_time); }
    PARALLEL_CATCH;
    _Re_non_time->close();
  }

  // Add in Residual contributions from Constraints
  if (_fe_problem._has_constraints)
  {
    PARALLEL_TRY
    {
      // Undisplaced Constraints
      constraintResiduals(*_Re_non_time, false);

      // Displaced Constraints
      if (_fe_problem.getDisplacedProblem())
        constraintResiduals(*_Re_non_time, true);
    }
    PARALLEL_CATCH;
    _Re_non_time->close();
  }
}

void
NonlinearSystemBase::computeNodalBCs(NumericVector<Number> & residual,
                                     Moose::KernelType kernel_type)
{
  // We need to close the diag_save_in variables on the aux system before NodalBCs clear the dofs on
  // boundary nodes
  if (_has_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  PARALLEL_TRY
  {
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();

    if (!bnd_nodes.empty())
    {
      Moose::perf_log.push("computeNodalBCs()", "Execution");

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
              {
                if (kernel_type == Moose::KT_EIGEN)
                  nbc->setBCOnEigen(true);
                else
                  nbc->setBCOnEigen(false);

                nbc->computeResidual(residual);
              }
          }
        }
      }

      Moose::perf_log.pop("computeNodalBCs()", "Execution");
    }
  }
  PARALLEL_CATCH;

  residual.close();
  if (_Re_time)
    _Re_time->close();
  _Re_non_time->close();
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
    GeometricSearchData & geom_search_data, std::map<dof_id_type, std::vector<dof_id_type>> & graph)
{
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> & nearest_node_locators =
      geom_search_data._nearest_node_locators;

  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  for (const auto & it : nearest_node_locators)
  {
    std::vector<dof_id_type> & slave_nodes = it.second->_slave_nodes;

    for (const auto & slave_node : slave_nodes)
    {
      std::set<dof_id_type> unique_slave_indices;
      std::set<dof_id_type> unique_master_indices;

      auto node_to_elem_pair = node_to_elem_map.find(slave_node);
      if (node_to_elem_pair != node_to_elem_map.end())
      {
        const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

        // Get the dof indices from each elem connected to the node
        for (const auto & cur_elem : elems)
        {
          std::vector<dof_id_type> dof_indices;
          dofMap().dof_indices(_mesh.elemPtr(cur_elem), dof_indices);

          for (const auto & dof : dof_indices)
            unique_slave_indices.insert(dof);
        }
      }

      std::vector<dof_id_type> master_nodes = it.second->_neighbor_nodes[slave_node];

      for (const auto & master_node : master_nodes)
      {
        auto master_node_to_elem_pair = node_to_elem_map.find(master_node);
        mooseAssert(master_node_to_elem_pair != node_to_elem_map.end(),
                    "Missing entry in node to elem map");
        const std::vector<dof_id_type> & master_node_elems = master_node_to_elem_pair->second;

        // Get the dof indices from each elem connected to the node
        for (const auto & cur_elem : master_node_elems)
        {
          std::vector<dof_id_type> dof_indices;
          dofMap().dof_indices(_mesh.elemPtr(cur_elem), dof_indices);

          for (const auto & dof : dof_indices)
            unique_master_indices.insert(dof);
        }
      }

      for (const auto & slave_id : unique_slave_indices)
        for (const auto & master_id : unique_master_indices)
        {
          graph[slave_id].push_back(master_id);
          graph[master_id].push_back(slave_id);
        }
    }
  }

  // handle node-to-node constraints
  const auto & ncs = _constraints.getActiveNodalConstraints();
  for (const auto & nc : ncs)
  {
    std::vector<dof_id_type> master_dofs;
    std::vector<dof_id_type> & master_node_ids = nc->getMasterNodeId();
    for (const auto & node_id : master_node_ids)
    {
      Node * node = _mesh.queryNodePtr(node_id);
      if (node && node->processor_id() == this->processor_id())
      {
        getNodeDofs(node_id, master_dofs);
      }
    }

    _communicator.allgather(master_dofs);

    std::vector<dof_id_type> slave_dofs;
    std::vector<dof_id_type> & slave_node_ids = nc->getSlaveNodeId();
    for (const auto & node_id : slave_node_ids)
    {
      Node * node = _mesh.queryNodePtr(node_id);
      if (node && node->processor_id() == this->processor_id())
      {
        getNodeDofs(node_id, slave_dofs);
      }
    }

    _communicator.allgather(slave_dofs);

    for (const auto & master_id : master_dofs)
      for (const auto & slave_id : slave_dofs)
      {
        graph[master_id].push_back(slave_id);
        graph[slave_id].push_back(master_id);
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
NonlinearSystemBase::addImplicitGeometricCouplingEntries(SparseMatrix<Number> & jacobian,
                                                         GeometricSearchData & geom_search_data)
{
  std::map<dof_id_type, std::vector<dof_id_type>> graph;

  findImplicitGeometricCouplingEntries(geom_search_data, graph);

  for (const auto & it : graph)
  {
    dof_id_type dof = it.first;
    const std::vector<dof_id_type> & row = it.second;

    for (const auto & coupled_dof : row)
      jacobian.add(dof, coupled_dof, 0);
  }
}

void
NonlinearSystemBase::constraintJacobians(SparseMatrix<Number> & jacobian, bool displaced)
{
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
#else
  if (!_fe_problem.errorOnJacobianNonzeroReallocation())
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                 MAT_NEW_NONZERO_ALLOCATION_ERR,
                 PETSC_FALSE);
  if (_fe_problem.ignoreZerosInJacobian())
    MatSetOption(
        static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_IGNORE_ZERO_ENTRIES, PETSC_TRUE);
#endif

  std::vector<numeric_index_type> zero_rows;
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> * penetration_locators =
      NULL;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    penetration_locators = &geom_search_data._penetration_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    penetration_locators = &displaced_geom_search_data._penetration_locators;
  }

  bool constraints_applied;
  if (!_assemble_constraints_separately)
    constraints_applied = false;
  for (const auto & it : *penetration_locators)
  {
    if (_assemble_constraints_separately)
    {
      // Reset the constraint_applied flag before each new constraint, as they need to be assembled
      // separately
      constraints_applied = false;
    }
    PenetrationLocator & pen_loc = *(it.second);

    std::vector<dof_id_type> & slave_nodes = pen_loc._nearest_node._slave_nodes;

    BoundaryID slave_boundary = pen_loc._slave_boundary;

    zero_rows.clear();
    if (_constraints.hasActiveNodeFaceConstraints(slave_boundary, displaced))
    {
      const auto & constraints =
          _constraints.getActiveNodeFaceConstraints(slave_boundary, displaced);

      for (const auto & slave_node_num : slave_nodes)
      {
        Node & slave_node = _mesh.nodeRef(slave_node_num);

        if (slave_node.processor_id() == processor_id())
        {
          if (pen_loc._penetration_info[slave_node_num])
          {
            PenetrationInfo & info = *pen_loc._penetration_info[slave_node_num];

            const Elem * master_elem = info._elem;
            unsigned int master_side = info._side_num;

            // reinit variables at the node
            _fe_problem.reinitNodeFace(&slave_node, slave_boundary, 0);

            _fe_problem.prepareAssembly(0);
            _fe_problem.reinitOffDiagScalars(0);

            std::vector<Point> points;
            points.push_back(info._closest_point);

            // reinit variables on the master element's face at the contact point
            _fe_problem.setNeighborSubdomainID(master_elem, 0);
            _fe_problem.reinitNeighborPhys(master_elem, master_side, points, 0);
            for (const auto & nfc : constraints)
            {
              nfc->_jacobian = &jacobian;

              if (nfc->shouldApply())
              {
                constraints_applied = true;

                nfc->subProblem().prepareShapes(nfc->variable().number(), 0);
                nfc->subProblem().prepareNeighborShapes(nfc->variable().number(), 0);

                nfc->computeJacobian();

                if (nfc->overwriteSlaveJacobian())
                {
                  // Add this variable's dof's row to be zeroed
                  zero_rows.push_back(nfc->variable().nodalDofIndex());
                }

                std::vector<dof_id_type> slave_dofs(1, nfc->variable().nodalDofIndex());

                // Cache the jacobian block for the slave side
                _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kee,
                                                           slave_dofs,
                                                           nfc->_connected_dof_indices,
                                                           nfc->variable().scalingFactor());

                // Cache the jacobian block for the master side
                if (nfc->addCouplingEntriesToJacobian())
                  _fe_problem.assembly(0).cacheJacobianBlock(
                      nfc->_Kne,
                      nfc->masterVariable().dofIndicesNeighbor(),
                      nfc->_connected_dof_indices,
                      nfc->variable().scalingFactor());

                _fe_problem.cacheJacobian(0);
                if (nfc->addCouplingEntriesToJacobian())
                  _fe_problem.cacheJacobianNeighbor(0);

                // Do the off-diagonals next
                const std::vector<MooseVariable *> coupled_vars = nfc->getCoupledMooseVars();
                for (const auto & jvar : coupled_vars)
                {
                  // Only compute jacobians for nonlinear variables
                  if (jvar->kind() != Moose::VAR_NONLINEAR)
                    continue;

                  // Only compute Jacobian entries if this coupling is being used by the
                  // preconditioner
                  if (nfc->variable().number() == jvar->number() ||
                      !_fe_problem.areCoupled(nfc->variable().number(), jvar->number()))
                    continue;

                  // Need to zero out the matrices first
                  _fe_problem.prepareAssembly(0);

                  nfc->subProblem().prepareShapes(nfc->variable().number(), 0);
                  nfc->subProblem().prepareNeighborShapes(jvar->number(), 0);

                  nfc->computeOffDiagJacobian(jvar->number());

                  // Cache the jacobian block for the slave side
                  _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kee,
                                                             slave_dofs,
                                                             nfc->_connected_dof_indices,
                                                             nfc->variable().scalingFactor());

                  // Cache the jacobian block for the master side
                  if (nfc->addCouplingEntriesToJacobian())
                    _fe_problem.assembly(0).cacheJacobianBlock(nfc->_Kne,
                                                               nfc->variable().dofIndicesNeighbor(),
                                                               nfc->_connected_dof_indices,
                                                               nfc->variable().scalingFactor());

                  _fe_problem.cacheJacobian(0);
                  if (nfc->addCouplingEntriesToJacobian())
                    _fe_problem.cacheJacobianNeighbor(0);
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
#ifdef LIBMESH_HAVE_PETSC
// Necessary for speed
#if PETSC_VERSION_LESS_THAN(3, 0, 0)
        MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3, 1, 0)
        // In Petsc 3.0.0, MatSetOption has three args...the third arg
        // determines whether the option is set (true) or unset (false)
        MatSetOption(
            static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS, PETSC_TRUE);
#else
        MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                     MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                     PETSC_TRUE);
#endif
#endif

        jacobian.close();
        jacobian.zero_rows(zero_rows, 0.0);
        jacobian.close();
        _fe_problem.addCachedJacobian(jacobian, 0);
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
#ifdef LIBMESH_HAVE_PETSC
// Necessary for speed
#if PETSC_VERSION_LESS_THAN(3, 0, 0)
      MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3, 1, 0)
      // In Petsc 3.0.0, MatSetOption has three args...the third arg
      // determines whether the option is set (true) or unset (false)
      MatSetOption(
          static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS, PETSC_TRUE);
#else
      MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                   MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                   PETSC_TRUE);
#endif
#endif

      jacobian.close();
      jacobian.zero_rows(zero_rows, 0.0);
      jacobian.close();
      _fe_problem.addCachedJacobian(jacobian, 0);
      jacobian.close();
    }
  }

  THREAD_ID tid = 0;
  // go over mortar interfaces
  auto & ifaces = _mesh.getMortarInterfaces();
  for (const auto & iface : ifaces)
  {
    if (_constraints.hasActiveFaceFaceConstraints(iface->_name))
    {
      // FaceFaceConstraint objects
      const auto & face_constraints = _constraints.getActiveFaceFaceConstraints(iface->_name);

      // go over elements on that interface
      const std::vector<Elem *> & elems = iface->_elems;
      for (const auto & elem : elems)
      {
        // for each element process constraints on the
        for (const auto & ffc : face_constraints)
        {
          _fe_problem.setCurrentSubdomainID(elem, tid);
          _fe_problem.prepare(elem, tid);
          _fe_problem.reinitElem(elem, tid);
          ffc->reinit();
          ffc->subProblem().prepareShapes(ffc->variable().number(), tid);
          ffc->computeJacobian();
          _fe_problem.cacheJacobian(tid);

          ffc->reinitSide(Moose::Master);
          ffc->computeJacobianSide(Moose::Master);
          _fe_problem.cacheJacobian(tid);

          ffc->reinitSide(Moose::Slave);
          ffc->computeJacobianSide(Moose::Slave);
          _fe_problem.cacheJacobian(tid);
        }

        _fe_problem.addCachedJacobian(jacobian, tid);
      }
    }
  }

  // go over element-element constraint interface
  std::map<unsigned int, std::shared_ptr<ElementPairLocator>> * element_pair_locators = nullptr;

  if (!displaced)
  {
    GeometricSearchData & geom_search_data = _fe_problem.geomSearchData();
    element_pair_locators = &geom_search_data._element_pair_locators;
  }
  else
  {
    GeometricSearchData & displaced_geom_search_data =
        _fe_problem.getDisplacedProblem()->geomSearchData();
    element_pair_locators = &displaced_geom_search_data._element_pair_locators;
  }

  for (const auto & it : *element_pair_locators)
  {
    ElementPairLocator & elem_pair_loc = *(it.second);

    if (_constraints.hasActiveElemElemConstraints(it.first))
    {
      // ElemElemConstraint objects
      const auto & _element_constraints = _constraints.getActiveElemElemConstraints(it.first);

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
          _fe_problem.reinitElemPhys(elem1, info._elem1_constraint_q_point, tid);
          _fe_problem.setNeighborSubdomainID(elem2, tid);
          _fe_problem.reinitNeighborPhys(elem2, info._elem2_constraint_q_point, tid);

          ec->subProblem().prepareShapes(ec->variable().number(), tid);
          ec->subProblem().prepareNeighborShapes(ec->variable().number(), tid);

          ec->reinit(info);
          ec->computeJacobian();
          _fe_problem.cacheJacobian(tid);
          _fe_problem.cacheJacobianNeighbor(tid);
        }
        _fe_problem.addCachedJacobian(jacobian, tid);
      }
    }
  }
}

void
NonlinearSystemBase::computeScalarKernelsJacobians(SparseMatrix<Number> & jacobian)
{
  // Compute the diagonal block for scalar variables
  if (_scalar_kernels.hasActiveObjects())
  {
    const auto & scalars = _scalar_kernels.getActiveObjects();

    _fe_problem.reinitScalars(/*tid=*/0);

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
          _fe_problem.addJacobianOffDiagScalar(jacobian, kernel->variable().number());
          have_scalar_contributions = true;
          break;
        }
      }
    }

    if (have_scalar_contributions)
      _fe_problem.addJacobianScalar(jacobian);
  }
}

void
NonlinearSystemBase::computeJacobianInternal(SparseMatrix<Number> & jacobian,
                                             Moose::KernelType kernel_type)
{
#ifdef LIBMESH_HAVE_PETSC
// Necessary for speed
#if PETSC_VERSION_LESS_THAN(3, 0, 0)
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3, 1, 0)
  // In Petsc 3.0.0, MatSetOption has three args...the third arg
  // determines whether the option is set (true) or unset (false)
  MatSetOption(
      static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS, PETSC_TRUE);
#else
  MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
               MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
               PETSC_TRUE);
#endif
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
#else
  if (!_fe_problem.errorOnJacobianNonzeroReallocation())
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                 MAT_NEW_NONZERO_ALLOCATION_ERR,
                 PETSC_FALSE);
#endif

#endif

  // jacobianSetup /////
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

  // reinit scalar variables
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  PARALLEL_TRY
  {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    switch (_fe_problem.coupling())
    {
      case Moose::COUPLING_DIAG:
      {
        ComputeJacobianThread cj(_fe_problem, jacobian, kernel_type);
        Threads::parallel_reduce(elem_range, cj);

        unsigned int n_threads = libMesh::n_threads();
        for (unsigned int i = 0; i < n_threads;
             i++) // Add any Jacobian contributions still hanging around
          _fe_problem.addCachedJacobian(jacobian, i);

        // Block restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBlockObjects())
        {
          ComputeNodalKernelJacobiansThread cnkjt(_fe_problem, _nodal_kernels, jacobian);
          ConstNodeRange & range = *_mesh.getLocalNodeRange();
          Threads::parallel_reduce(range, cnkjt);

          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads;
               i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }

        // Boundary restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBoundaryObjects())
        {
          ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, _nodal_kernels, jacobian);
          ConstBndNodeRange & bnd_range = *_mesh.getBoundaryNodeRange();

          Threads::parallel_reduce(bnd_range, cnkjt);
          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads;
               i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }
      }
      break;

      default:
      case Moose::COUPLING_CUSTOM:
      {
        ComputeFullJacobianThread cj(_fe_problem, jacobian, kernel_type);
        Threads::parallel_reduce(elem_range, cj);
        unsigned int n_threads = libMesh::n_threads();

        for (unsigned int i = 0; i < n_threads; i++)
          _fe_problem.addCachedJacobian(jacobian, i);

        // Block restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBlockObjects())
        {
          ComputeNodalKernelJacobiansThread cnkjt(_fe_problem, _nodal_kernels, jacobian);
          ConstNodeRange & range = *_mesh.getLocalNodeRange();
          Threads::parallel_reduce(range, cnkjt);

          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads;
               i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }

        // Boundary restricted Nodal Kernels
        if (_nodal_kernels.hasActiveBoundaryObjects())
        {
          ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, _nodal_kernels, jacobian);
          ConstBndNodeRange & bnd_range = *_mesh.getBoundaryNodeRange();

          Threads::parallel_reduce(bnd_range, cnkjt);

          unsigned int n_threads = libMesh::n_threads();
          for (unsigned int i = 0; i < n_threads;
               i++) // Add any cached jacobians that might be hanging around
            _fe_problem.assembly(i).addCachedJacobianContributions(jacobian);
        }
      }
      break;
    }

    computeDiracContributions(&jacobian);
    computeScalarKernelsJacobians(jacobian);

    static bool first = true;

    // This adds zeroes into geometric coupling entries to ensure they stay in the matrix
    if (first && (_add_implicit_geometric_coupling_entries_to_jacobian))
    {
      first = false;
      addImplicitGeometricCouplingEntries(jacobian, _fe_problem.geomSearchData());

      if (_fe_problem.getDisplacedProblem())
        addImplicitGeometricCouplingEntries(jacobian,
                                            _fe_problem.getDisplacedProblem()->geomSearchData());
    }
  }
  PARALLEL_CATCH;
  jacobian.close();

  PARALLEL_TRY
  {
    // Add in Jacobian contributions from Constraints
    if (_fe_problem._has_constraints)
    {
      // Nodal Constraints
      enforceNodalConstraintsJacobian(jacobian);

      // Undisplaced Constraints
      constraintJacobians(jacobian, false);

      // Displaced Constraints
      if (_fe_problem.getDisplacedProblem())
        constraintJacobians(jacobian, true);
    }
  }
  PARALLEL_CATCH;
  jacobian.close();

  // We need to close the save_in variables on the aux system before NodalBCs clear the dofs on
  // boundary nodes
  if (_has_diag_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  PARALLEL_TRY
  {
    // Cache the information about which BCs are coupled to which
    // variables, so we don't have to figure it out for each node.
    std::map<std::string, std::set<unsigned int>> bc_involved_vars;
    const std::set<BoundaryID> & all_boundary_ids = _mesh.getBoundaryIDs();
    for (const auto & bid : all_boundary_ids)
    {
      // Get reference to all the NodalBCs for this ID.  This is only
      // safe if there are NodalBCs there to be gotten...
      if (_nodal_bcs.hasActiveBoundaryObjects(bid))
      {
        const auto & bcs = _nodal_bcs.getActiveBoundaryObjects(bid);
        for (const auto & bc : bcs)
        {
          const std::vector<MooseVariable *> & coupled_moose_vars = bc->getCoupledMooseVars();

          // Create the set of "involved" MOOSE nonlinear vars, which includes all coupled vars and
          // the BC's own variable
          std::set<unsigned int> & var_set = bc_involved_vars[bc->name()];
          for (const auto & coupled_var : coupled_moose_vars)
            if (coupled_var->kind() == Moose::VAR_NONLINEAR)
              var_set.insert(coupled_var->number());

          var_set.insert(bc->variable().number());
        }
      }
    }

    // Get variable coupling list.  We do all the NodalBC stuff on
    // thread 0...  The couplingEntries() data structure determines
    // which variables are "coupled" as far as the preconditioner is
    // concerned, not what variables a boundary condition specifically
    // depends on.
    std::vector<std::pair<MooseVariable *, MooseVariable *>> & coupling_entries =
        _fe_problem.couplingEntries(/*_tid=*/0);

    // Compute Jacobians for NodalBCs
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (const auto & bnode : bnd_nodes)
    {
      BoundaryID boundary_id = bnode->_bnd_id;
      Node * node = bnode->_node;

      if (_nodal_bcs.hasActiveBoundaryObjects(boundary_id) &&
          node->processor_id() == processor_id())
      {
        _fe_problem.reinitNodeFace(node, boundary_id, 0);

        const auto & bcs = _nodal_bcs.getActiveBoundaryObjects(boundary_id);
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
        }
      }
    } // end loop over boundary nodes

    // For the matrix in the right side of generalized eigenvalue problems, its conresponding
    // rows are zeroed if homogeneous Dirichlet boundary conditions are used.
    if (kernel_type == Moose::KT_EIGEN)
      _fe_problem.assembly(0).zeroCachedJacobianContributions(jacobian);
    // Set the cached NodalBC values in the Jacobian matrix
    else
      _fe_problem.assembly(0).setCachedJacobianContributions(jacobian);
  }
  PARALLEL_CATCH;
  jacobian.close();

  // We need to close the save_in variables on the aux system before NodalBCs clear the dofs on
  // boundary nodes
  if (_has_nodalbc_diag_save_in)
    _fe_problem.getAuxiliarySystem().solution().close();

  if (hasDiagSaveIn())
    _fe_problem.getAuxiliarySystem().update();
}

void
NonlinearSystemBase::setVariableGlobalDoFs(const std::string & var_name)
{
  AllLocalDofIndicesThread aldit(_sys.system(), {var_name});
  ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
  Threads::parallel_reduce(elem_range, aldit);
  _communicator.set_union(aldit._all_dof_indices);
  _var_all_dof_indices.assign(aldit._all_dof_indices.begin(), aldit._all_dof_indices.end());
}

void
NonlinearSystemBase::computeJacobian(SparseMatrix<Number> & jacobian, Moose::KernelType kernel_type)
{
  Moose::perf_log.push("compute_jacobian()", "Execution");

  Moose::enableFPE();

  try
  {
    jacobian.zero();
    computeJacobianInternal(jacobian, kernel_type);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_jacobian()", "Execution");
}

void
NonlinearSystemBase::computeJacobianBlocks(std::vector<JacobianBlock *> & blocks)
{
  Moose::perf_log.push("compute_jacobian_block()", "Execution");

  Moose::enableFPE();

  for (unsigned int i = 0; i < blocks.size(); i++)
  {
    SparseMatrix<Number> & jacobian = blocks[i]->_jacobian;

#ifdef LIBMESH_HAVE_PETSC
// Necessary for speed
#if PETSC_VERSION_LESS_THAN(3, 0, 0)
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS);
#elif PETSC_VERSION_LESS_THAN(3, 1, 0)
    // In Petsc 3.0.0, MatSetOption has three args...the third arg
    // determines whether the option is set (true) or unset (false)
    MatSetOption(
        static_cast<PetscMatrix<Number> &>(jacobian).mat(), MAT_KEEP_ZEROED_ROWS, PETSC_TRUE);
#else
    MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                 MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
                 PETSC_TRUE);
#endif
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
#else
    if (!_fe_problem.errorOnJacobianNonzeroReallocation())
      MatSetOption(static_cast<PetscMatrix<Number> &>(jacobian).mat(),
                   MAT_NEW_NONZERO_ALLOCATION_ERR,
                   PETSC_FALSE);
#endif

#endif

    jacobian.zero();
  }

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
    _fe_problem.reinitScalars(tid);

  PARALLEL_TRY
  {
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    ComputeJacobianBlocksThread cjb(_fe_problem, blocks);
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
      ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
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
                // The first zero is for the variable number... there is only one variable in each
                // mini-system
                // The second zero only works with Lagrange elements!
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

  Moose::enableFPE(false);

  Moose::perf_log.pop("compute_jacobian_block()", "Execution");
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
    _constraints.updateActive();
    _scalar_kernels.updateActive();
  }
}

Real
NonlinearSystemBase::computeDamping(const NumericVector<Number> & solution,
                                    const NumericVector<Number> & update)
{
  Moose::perf_log.push("compute_dampers()", "Execution");

  // Default to no damping
  Real damping = 1.0;
  bool has_active_dampers = false;

  if (_element_dampers.hasActiveObjects())
  {
    has_active_dampers = true;
    *_increment_vec = update;
    ComputeElemDampingThread cid(_fe_problem);
    Threads::parallel_reduce(*_mesh.getActiveLocalElementRange(), cid);
    damping = std::min(cid.damping(), damping);
  }

  if (_nodal_dampers.hasActiveObjects())
  {
    has_active_dampers = true;
    *_increment_vec = update;
    ComputeNodalDampingThread cndt(_fe_problem);
    Threads::parallel_reduce(*_mesh.getLocalNodeRange(), cndt);
    damping = std::min(cndt.damping(), damping);
  }

  if (_general_dampers.hasActiveObjects())
  {
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

  _communicator.min(damping);

  if (has_active_dampers && damping < 1.0)
    _console << " Damping factor: " << damping << "\n";

  Moose::perf_log.pop("compute_dampers()", "Execution");

  return damping;
}

void
NonlinearSystemBase::computeDiracContributions(SparseMatrix<Number> * jacobian)
{
  _fe_problem.clearDiracInfo();

  std::set<const Elem *> dirac_elements;

  if (_dirac_kernels.hasActiveObjects())
  {
    Moose::perf_log.push("computeDiracContributions()", "Execution");

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

    ComputeDiracThread cd(_fe_problem, jacobian);

    _fe_problem.getDiracElements(dirac_elements);

    DistElemRange range(dirac_elements.begin(), dirac_elements.end(), 1);
    // TODO: Make Dirac work thread!
    // Threads::parallel_reduce(range, cd);

    cd(range);

    Moose::perf_log.pop("computeDiracContributions()", "Execution");
  }

  if (jacobian == NULL)
    _Re_non_time->close();
}

NumericVector<Number> &
NonlinearSystemBase::residualCopy()
{
  _need_residual_copy = true;
  return _residual_copy;
}

NumericVector<Number> &
NonlinearSystemBase::residualGhosted()
{
  _need_residual_ghosted = true;
  if (!_residual_ghosted)
    _residual_ghosted = &addVector("residual_ghosted", false, GHOSTED);
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

    std::map<dof_id_type, std::vector<dof_id_type>> graph;

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

      const std::vector<dof_id_type> & row = git.second;

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
NonlinearSystemBase::serializeSolution()
{
  if (_need_serialized_solution)
  {
    if (!_serialized_solution.initialized() || _serialized_solution.size() != _sys.n_dofs())
    {
      _serialized_solution.clear();
      _serialized_solution.init(_sys.n_dofs(), false, SERIAL);
    }

    _current_solution->localize(_serialized_solution);
  }
}

void
NonlinearSystemBase::setSolution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  if (_need_serialized_solution)
    serializeSolution();
}

void
NonlinearSystemBase::setSolutionUDot(const NumericVector<Number> & udot)
{
  *_u_dot = udot;
}

NumericVector<Number> &
NonlinearSystemBase::serializedSolution()
{
  if (!_serialized_solution.initialized())
    _serialized_solution.init(_sys.n_dofs(), false, SERIAL);

  _need_serialized_solution = true;
  return _serialized_solution;
}

void
NonlinearSystemBase::setPreconditioner(std::shared_ptr<MoosePreconditioner> pc)
{
  if (_preconditioner.get() != nullptr)
    mooseError("More than one active Preconditioner detected");

  _preconditioner = pc;
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
  // Check kernel coverage of subdomains (blocks) in your mesh
  std::set<SubdomainID> input_subdomains;
  std::set<std::string> kernel_variables;

  bool global_kernels_exist = _kernels.hasActiveBlockObjects(Moose::ANY_BLOCK_ID);
  global_kernels_exist |= _scalar_kernels.hasActiveObjects();
  global_kernels_exist |= _nodal_kernels.hasActiveObjects();

  _kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _nodal_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _scalar_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  _constraints.subdomainsCovered(input_subdomains, kernel_variables);

  if (!global_kernels_exist)
  {
    std::set<SubdomainID> difference;
    std::set_difference(mesh_subdomains.begin(),
                        mesh_subdomains.end(),
                        input_subdomains.begin(),
                        input_subdomains.end(),
                        std::inserter(difference, difference.end()));

    if (!difference.empty())
    {
      std::stringstream missing_block_ids;
      std::copy(difference.begin(),
                difference.end(),
                std::ostream_iterator<unsigned int>(missing_block_ids, " "));
      mooseError("Each subdomain must contain at least one Kernel.\nThe following block(s) lack an "
                 "active kernel: " +
                 missing_block_ids.str());
    }
  }

  std::set<VariableName> variables(getVariableNames().begin(), getVariableNames().end());

  std::set<VariableName> difference;
  std::set_difference(variables.begin(),
                      variables.end(),
                      kernel_variables.begin(),
                      kernel_variables.end(),
                      std::inserter(difference, difference.end()));

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
  return _time_kernels.hasActiveObjects();
}

void
NonlinearSystemBase::setPCSide(MooseEnum pcs)
{
  if (pcs == "left")
    _pc_side = Moose::PCS_LEFT;
  else if (pcs == "right")
    _pc_side = Moose::PCS_RIGHT;
  else if (pcs == "symmetric")
    _pc_side = Moose::PCS_SYMMETRIC;
  else if (pcs == "default")
    _pc_side = Moose::PCS_DEFAULT;
  else
    mooseError("Unknown PC side specified.");
}

void
NonlinearSystemBase::setMooseKSPNormType(MooseEnum kspnorm)
{
  if (kspnorm == "none")
    _ksp_norm = Moose::KSPN_NONE;
  else if (kspnorm == "preconditioned")
    _ksp_norm = Moose::KSPN_PRECONDITIONED;
  else if (kspnorm == "unpreconditioned")
    _ksp_norm = Moose::KSPN_UNPRECONDITIONED;
  else if (kspnorm == "natural")
    _ksp_norm = Moose::KSPN_NATURAL;
  else if (kspnorm == "default")
    _ksp_norm = Moose::KSPN_DEFAULT;
  else
    mooseError("Unknown ksp norm type specified.");
}

bool
NonlinearSystemBase::needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const
{
  return _integrated_bcs.hasActiveBoundaryObjects(bnd_id, tid);
}

bool NonlinearSystemBase::needMaterialOnSide(SubdomainID /*subdomain_id*/, THREAD_ID /*tid*/) const
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
  if (_solution_previous_nl)
    *_solution_previous_nl = soln;
}
