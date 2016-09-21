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

#include "ParallelUniqueId.h"
#include "AssemblySystem.h"
#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseVariable.h"
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
#if !PETSC_VERSION_LESS_THAN(3,3,0)
#include <PetscDMMoose.h>
EXTERN_C_BEGIN
extern PetscErrorCode DMCreate_Moose(DM);
EXTERN_C_END
#endif
#endif


AssemblySystem::AssemblySystem(FEProblem & fe_problem, Factory & factory, const std::string & name) :
    ConsoleStreamInterface(fe_problem.getMooseApp()),
    _fe_problem(fe_problem),
    _last_rnorm(0.),
    _last_nl_rnorm(0.),
    _l_abs_step_tol(1e-10),
    _initial_residual_before_preset_bcs(0.),
    _initial_residual_after_preset_bcs(0.),
    _current_nl_its(0),
    _compute_initial_residual_before_preset_bcs(true),
    _current_solution(NULL),
    _scalar_kernels(/*threaded=*/false),
    _nodal_bcs(/*threaded=*/false),
    _preset_nodal_bcs(/*threaded=*/false),
    _splits(/*threaded=*/false),
    _factory_assbly(factory),
    _mesh_assbly(fe_problem.mesh()),
    _increment_vec(NULL),
    _pc_side(Moose::PCS_RIGHT),
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

AssemblySystem::~AssemblySystem()
{

}


void
AssemblySystem::initialSetup()
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
    _integrated_bcs.initialSetup(tid);
  }
  _scalar_kernels.initialSetup();
  _constraints.initialSetup();
  _general_dampers.initialSetup();
  _nodal_bcs.initialSetup();
}

void
AssemblySystem::timestepSetup()
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
    _integrated_bcs.timestepSetup(tid);
  }
  _scalar_kernels.initialSetup();
  _constraints.timestepSetup();
  _general_dampers.timestepSetup();
  _nodal_bcs.timestepSetup();
}


void
AssemblySystem::setDecomposition(const std::vector<std::string>& splits)
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
  }  else {
    _have_decomposition = false;
  }
}


void
AssemblySystem::setupFieldDecomposition()
{
  if (!_have_decomposition) return;
  MooseSharedPointer<Split> top_split = getSplit(_decomposition_split);
  top_split->setup();
}



void
AssemblySystem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to warehouse
    MooseSharedPointer<KernelBase> kernel = _factory_assbly.create<KernelBase>(kernel_name, name, parameters, tid);
    _kernels.addObject(kernel, tid);

    // Store time/non-time kernels separately
    MooseSharedPointer<TimeKernel> t_kernel = MooseSharedNamespace::dynamic_pointer_cast<TimeKernel>(kernel);
    if (t_kernel)
      _time_kernels.addObject(kernel, tid);
    else
      _non_time_kernels.addObject(kernel, tid);
  }

  if (parameters.get<std::vector<AuxVariableName> >("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName> >("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
AssemblySystem::addNodalKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    // Create the kernel object via the factory and add to the warehouse
    MooseSharedPointer<NodalKernel> kernel = _factory_assbly.create<NodalKernel>(kernel_name, name, parameters, tid);
    _nodal_kernels.addObject(kernel, tid);
  }

  if (parameters.get<std::vector<AuxVariableName> >("save_in").size() > 0)
    _has_save_in = true;
  if (parameters.get<std::vector<AuxVariableName> >("diag_save_in").size() > 0)
    _has_diag_save_in = true;
}

void
AssemblySystem::addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  MooseSharedPointer<ScalarKernel> kernel =
    _factory_assbly.create<ScalarKernel>(kernel_name, name, parameters);
  _scalar_kernels.addObject(kernel);

  // Store time/non-time ScalarKernels separately
  ODETimeKernel * t_kernel = dynamic_cast<ODETimeKernel *>(kernel.get());

  if (t_kernel)
    _time_scalar_kernels.addObject(kernel);
  else
    _non_time_scalar_kernels.addObject(kernel);
}

void
AssemblySystem::addConstraint(const std::string & c_name, const std::string & name, InputParameters parameters)
{
  MooseSharedPointer<Constraint> constraint = _factory_assbly.create<Constraint>(c_name, name, parameters);
  _constraints.addObject(constraint);

  if (constraint && constraint->addCouplingEntriesToJacobian())
    addImplicitGeometricCouplingEntriesToJacobian(true);
}

void
AssemblySystem::addDiracKernel(const  std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    MooseSharedPointer<DiracKernel> kernel = _factory_assbly.create<DiracKernel>(kernel_name, name, parameters, tid);
    _dirac_kernels.addObject(kernel, tid);
  }
}

void
AssemblySystem::addDGKernel(std::string dg_kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    MooseSharedPointer<DGKernel> dg_kernel = _factory_assbly.create<DGKernel>(dg_kernel_name, name, parameters, tid);
    _dg_kernels.addObject(dg_kernel, tid);
  }

  _doing_dg = true;
}


void
AssemblySystem::addDamper(const std::string & damper_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid=0; tid < libMesh::n_threads(); ++tid)
  {
    MooseSharedPointer<Damper> damper = _factory_assbly.create<Damper>(damper_name, name, parameters, tid);

    // Attempt to cast to the damper types
    MooseSharedPointer<ElementDamper> ed = MooseSharedNamespace::dynamic_pointer_cast<ElementDamper>(damper);
    MooseSharedPointer<GeneralDamper> gd = MooseSharedNamespace::dynamic_pointer_cast<GeneralDamper>(damper);

    if (gd)
    {
      _general_dampers.addObject(gd);
      break; // not threaded
    }
    else if (ed)
      _element_dampers.addObject(ed,tid);
    else
      mooseError("Invalid damper type");
  }
}

void
AssemblySystem::addSplit(const  std::string & split_name, const std::string & name, InputParameters parameters)
{
  MooseSharedPointer<Split> split = _factory_assbly.create<Split>(split_name, name, parameters);
  _splits.addObject(split);
}

MooseSharedPointer<Split>
AssemblySystem::getSplit(const std::string & name)
{
  return _splits.getActiveObject(name);
}

void
AssemblySystem::zeroVectorForResidual(const std::string & vector_name)
{
  for (unsigned int i = 0; i < _vecs_to_zero_for_residual.size(); ++i)
    if (vector_name == _vecs_to_zero_for_residual[i])
      return;

  _vecs_to_zero_for_residual.push_back(vector_name);
}


void
AssemblySystem::onTimestepBegin()
{
  _time_integrator->preSolve();
  if (_predictor.get())
    _predictor->timestepSetup();
}


void AssemblySystem::setPredictor(MooseSharedPointer<Predictor> predictor)
{
  _predictor = predictor;
}

void
AssemblySystem::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
{
  _kernels.subdomainSetup(subdomain, tid);
  _nodal_kernels.subdomainSetup(subdomain, tid);
  _element_dampers.subdomainSetup(subdomain, tid);
}



void
AssemblySystem::computeTimeDerivatives()
{
  _time_integrator->preStep();
  _time_integrator->computeTimeDerivatives();
}

void
AssemblySystem::enforceNodalConstraintsResidual(NumericVector<Number> & residual)
{
  THREAD_ID tid = 0; // constraints are going to be done single-threaded
  residual.close();
  if (_constraints.hasActiveNodalConstraints())
  {
    const std::vector<MooseSharedPointer<NodalConstraint> > & ncs = _constraints.getActiveNodalConstraints();
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
AssemblySystem::enforceNodalConstraintsJacobian(SparseMatrix<Number> & jacobian)
{
  THREAD_ID tid = 0;    // constraints are going to be done single-threaded
  jacobian.close();
  if (_constraints.hasActiveNodalConstraints())
  {
    const std::vector<MooseSharedPointer<NodalConstraint> > & ncs = _constraints.getActiveNodalConstraints();
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



void
AssemblySystem::computeScalarKernelsJacobians(SparseMatrix<Number> & jacobian)
{
  // Compute the diagonal block for scalar variables
  if (_scalar_kernels.hasActiveObjects())
  {
    const std::vector<MooseSharedPointer<ScalarKernel> > & scalars = _scalar_kernels.getActiveObjects();

    _fe_problem.reinitScalars(/*tid=*/0);
    for (const auto & kernel : scalars)
    {
      kernel->reinit();
      kernel->computeJacobian();
      _fe_problem.addJacobianOffDiagScalar(jacobian, kernel->variable().number());
    }
    _fe_problem.addJacobianScalar(jacobian);
  }
}



void
AssemblySystem::updateActive(THREAD_ID tid)
{
  _element_dampers.updateActive(tid);
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



void
AssemblySystem::setPreconditioner(MooseSharedPointer<MoosePreconditioner> pc)
{
  if (_preconditioner.get() != NULL)
    mooseError("More than one active Preconditioner detected");

  _preconditioner = pc;
}


void
AssemblySystem::reinitIncrementForDampers(THREAD_ID tid)
{
  // Find the variables that are being operated on by active element dampers
  std::set<MooseVariable *> damped_vars;

  const std::vector<MooseSharedPointer<ElementDamper> > & edampers = _element_dampers.getActiveObjects(tid);
  for (const auto & damper : edampers)
    damped_vars.insert(damper->getVariable());

  for (const auto & var : damped_vars)
    var->computeIncrement(*_increment_vec);
}


bool
AssemblySystem::containsTimeKernel()
{
  return _time_kernels.hasActiveObjects();
}

void
AssemblySystem::setPCSide(MooseEnum pcs)
{
  if (pcs == "left")
    _pc_side = Moose::PCS_LEFT;
  else if (pcs == "right")
    _pc_side = Moose::PCS_RIGHT;
  else if (pcs == "symmetric")
    _pc_side = Moose::PCS_SYMMETRIC;
  else
    mooseError("Unknown PC side specified.");
}

bool
AssemblySystem::needMaterialOnSide(BoundaryID bnd_id, THREAD_ID tid) const
{
  return _integrated_bcs.hasActiveBoundaryObjects(bnd_id, tid);
}

bool
AssemblySystem::needMaterialOnSide(SubdomainID /*subdomain_id*/, THREAD_ID /*tid*/) const
{
  return _doing_dg;
}

bool
AssemblySystem::doingDG() const
{
  return _doing_dg;
}
