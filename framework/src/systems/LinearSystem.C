//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearSystem.h"
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

#include <ios>

#include "petscsnes.h"

LinearSystem::LinearSystem(FEProblemBase & fe_problem, const std::string & name)
  : SystemBase(fe_problem, name, Moose::VAR_LINEAR),
    PerfGraphInterface(fe_problem.getMooseApp().perfGraph(), "LinearSystem"),
    _fe_problem(fe_problem),
    _sys(fe_problem.es().add_system<LinearImplicitSystem>(name)),
    _current_solution(NULL),
    _rhs_time_tag(-1),
    _rhs_time(NULL),
    _rhs_non_time_tag(-1),
    _rhs_non_time(NULL),
    _pc_side(Moose::PCS_DEFAULT),
    _ksp_norm(Moose::KSPN_UNPRECONDITIONED),
    _n_linear_iters(0)
{
  getRightHandSideNonTimeVector();
  // Don't need to add the matrix - it already exists (for now)
  _system_matrix_system_tag = _fe_problem.addMatrixTag("SYSTEM");

  // The time matrix tag is not normally used - but must be added to the system
  // in case it is so that objects can have 'time' in their matrix tags by default
  _fe_problem.addMatrixTag("TIME");

  // We create a tag for the right hand side, the vector is already in the libmesh system
  _rhs_tag = _fe_problem.addVectorTag("RHS");
}

LinearSystem::~LinearSystem() = default;

void
LinearSystem::init()
{
  SystemBase::init();

  _current_solution = _sys.current_local_solution.get();
}

void
LinearSystem::restoreSolutions()
{
  // call parent
  SystemBase::restoreSolutions();
  // and update _current_solution
  _current_solution = _sys.current_local_solution.get();
}

void
LinearSystem::initialSetup()
{
  TIME_SECTION("lInitialSetup", 2, "Setting Up Linear System");

  SystemBase::initialSetup();

  {
    TIME_SECTION("kernelsInitialSetup", 2, "Setting Up Kernels/BCs/Constraints");

    // for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
    // {
    //   _kernels.initialSetup(tid);
    //   _nodal_kernels.initialSetup(tid);
    //   _dirac_kernels.initialSetup(tid);
    //   if (_doing_dg)
    //     _dg_kernels.initialSetup(tid);
    //   _interface_kernels.initialSetup(tid);

    //   _element_dampers.initialSetup(tid);
    //   _nodal_dampers.initialSetup(tid);
    //   _integrated_bcs.initialSetup(tid);

    //   if (_fe_problem.haveFV())
    //   {
    //     std::vector<FVElementalKernel *> fv_elemental_kernels;
    //     _fe_problem.theWarehouse()
    //         .query()
    //         .template condition<AttribSystem>("FVElementalKernel")
    //         .template condition<AttribThread>(tid)
    //         .queryInto(fv_elemental_kernels);

    //     for (auto * fv_kernel : fv_elemental_kernels)
    //       fv_kernel->initialSetup();

    //     std::vector<FVFluxKernel *> fv_flux_kernels;
    //     _fe_problem.theWarehouse()
    //         .query()
    //         .template condition<AttribSystem>("FVFluxKernel")
    //         .template condition<AttribThread>(tid)
    //         .queryInto(fv_flux_kernels);

    //     for (auto * fv_kernel : fv_flux_kernels)
    //       fv_kernel->initialSetup();
    //   }
    // }

    // _scalar_kernels.initialSetup();
    // _constraints.initialSetup();
    // _general_dampers.initialSetup();
    // _nodal_bcs.initialSetup();
  }
}

void
LinearSystem::timestepSetup()
{
  SystemBase::timestepSetup();

  // for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  // {
  //   _kernels.timestepSetup(tid);
  //   _nodal_kernels.timestepSetup(tid);
  //   _dirac_kernels.timestepSetup(tid);
  //   if (_doing_dg)
  //     _dg_kernels.timestepSetup(tid);
  //   _interface_kernels.timestepSetup(tid);
  //   _element_dampers.timestepSetup(tid);
  //   _nodal_dampers.timestepSetup(tid);
  //   _integrated_bcs.timestepSetup(tid);

  //   if (_fe_problem.haveFV())
  //   {
  //     std::vector<FVFluxBC *> bcs;
  //     _fe_problem.theWarehouse()
  //         .query()
  //         .template condition<AttribSystem>("FVFluxBC")
  //         .template condition<AttribThread>(tid)
  //         .queryInto(bcs);

  //     std::vector<FVInterfaceKernel *> iks;
  //     _fe_problem.theWarehouse()
  //         .query()
  //         .template condition<AttribSystem>("FVInterfaceKernel")
  //         .template condition<AttribThread>(tid)
  //         .queryInto(iks);

  //     std::vector<FVFluxKernel *> kernels;
  //     _fe_problem.theWarehouse()
  //         .query()
  //         .template condition<AttribSystem>("FVFluxKernel")
  //         .template condition<AttribThread>(tid)
  //         .queryInto(kernels);

  //     for (auto * bc : bcs)
  //       bc->timestepSetup();
  //     for (auto * ik : iks)
  //       ik->timestepSetup();
  //     for (auto * kernel : kernels)
  //       kernel->timestepSetup();
  //   }
  // }
  // _scalar_kernels.timestepSetup();
  // _constraints.timestepSetup();
  // _general_dampers.timestepSetup();
  // _nodal_bcs.timestepSetup();
}

void
LinearSystem::customSetup(const ExecFlagType & exec_type)
{
  SystemBase::customSetup(exec_type);

  // for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  // {
  //   _kernels.customSetup(exec_type, tid);
  //   _nodal_kernels.customSetup(exec_type, tid);
  //   _dirac_kernels.customSetup(exec_type, tid);
  //   if (_doing_dg)
  //     _dg_kernels.customSetup(exec_type, tid);
  //   _interface_kernels.customSetup(exec_type, tid);
  //   _element_dampers.customSetup(exec_type, tid);
  //   _nodal_dampers.customSetup(exec_type, tid);
  //   _integrated_bcs.customSetup(exec_type, tid);

  //   if (_fe_problem.haveFV())
  //   {
  //     std::vector<FVFluxBC *> bcs;
  //     _fe_problem.theWarehouse()
  //         .query()
  //         .template condition<AttribSystem>("FVFluxBC")
  //         .template condition<AttribThread>(tid)
  //         .queryInto(bcs);

  //     std::vector<FVInterfaceKernel *> iks;
  //     _fe_problem.theWarehouse()
  //         .query()
  //         .template condition<AttribSystem>("FVInterfaceKernel")
  //         .template condition<AttribThread>(tid)
  //         .queryInto(iks);

  //     std::vector<FVFluxKernel *> kernels;
  //     _fe_problem.theWarehouse()
  //         .query()
  //         .template condition<AttribSystem>("FVFluxKernel")
  //         .template condition<AttribThread>(tid)
  //         .queryInto(kernels);

  //     for (auto * bc : bcs)
  //       bc->customSetup(exec_type);
  //     for (auto * ik : iks)
  //       ik->customSetup(exec_type);
  //     for (auto * kernel : kernels)
  //       kernel->customSetup(exec_type);
  //   }
  // }
  // _scalar_kernels.customSetup(exec_type);
  // _constraints.customSetup(exec_type);
  // _general_dampers.customSetup(exec_type);
  // _nodal_bcs.customSetup(exec_type);
}

void
LinearSystem::addTimeIntegrator(const std::string & type,
                                const std::string & name,
                                InputParameters & parameters)
{
  parameters.set<SystemBase *>("_sys") = this;

  std::shared_ptr<TimeIntegrator> ti = _factory.create<TimeIntegrator>(type, name, parameters);
  _time_integrator = ti;
}

void
LinearSystem::addLinearKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  // for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  // {
  //   // Create the kernel object via the factory and add to warehouse
  //   std::shared_ptr<KernelBase> kernel =
  //       _factory.create<KernelBase>(kernel_name, name, parameters, tid);
  //   _kernels.addObject(kernel, tid);
  //   postAddResidualObject(*kernel);
  // }

  // if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
  //   _has_save_in = true;
  // if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
  //   _has_diag_save_in = true;
}

void
LinearSystem::addBoundaryCondition(const std::string & bc_name,
                                   const std::string & name,
                                   InputParameters & parameters)
{
  // // ThreadID
  // THREAD_ID tid = 0;

  // // Create the object
  // std::shared_ptr<BoundaryCondition> bc =
  //     _factory.create<BoundaryCondition>(bc_name, name, parameters, tid);
  // postAddResidualObject(*bc);

  // // Active BoundaryIDs for the object
  // const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  // auto bc_var = dynamic_cast<const MooseVariableFieldBase *>(&bc->variable());
  // _vars[tid].addBoundaryVar(boundary_ids, bc_var);

  // // Cast to the various types of BCs
  // std::shared_ptr<NodalBCBase> nbc = std::dynamic_pointer_cast<NodalBCBase>(bc);
  // std::shared_ptr<IntegratedBCBase> ibc = std::dynamic_pointer_cast<IntegratedBCBase>(bc);

  // // NodalBCBase
  // if (nbc)
  // {
  //   if (nbc->checkNodalVar() && !nbc->variable().isNodal())
  //     mooseError("Trying to use nodal boundary condition '",
  //                nbc->name(),
  //                "' on a non-nodal variable '",
  //                nbc->variable().name(),
  //                "'.");

  //   _nodal_bcs.addObject(nbc);
  //   _vars[tid].addBoundaryVars(boundary_ids, nbc->getCoupledVars());

  //   if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
  //     _has_nodalbc_save_in = true;
  //   if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
  //     _has_nodalbc_diag_save_in = true;

  //   // DirichletBCs that are preset
  //   std::shared_ptr<DirichletBCBase> dbc = std::dynamic_pointer_cast<DirichletBCBase>(bc);
  //   if (dbc && dbc->preset())
  //     _preset_nodal_bcs.addObject(dbc);

  //   std::shared_ptr<ADDirichletBCBase> addbc = std::dynamic_pointer_cast<ADDirichletBCBase>(bc);
  //   if (addbc && addbc->preset())
  //     _ad_preset_nodal_bcs.addObject(addbc);
  // }

  // // IntegratedBCBase
  // else if (ibc)
  // {
  //   _integrated_bcs.addObject(ibc, tid);
  //   _vars[tid].addBoundaryVars(boundary_ids, ibc->getCoupledVars());

  //   if (parameters.get<std::vector<AuxVariableName>>("save_in").size() > 0)
  //     _has_save_in = true;
  //   if (parameters.get<std::vector<AuxVariableName>>("diag_save_in").size() > 0)
  //     _has_diag_save_in = true;

  //   for (tid = 1; tid < libMesh::n_threads(); tid++)
  //   {
  //     // Create the object
  //     bc = _factory.create<BoundaryCondition>(bc_name, name, parameters, tid);

  //     // Give users opportunity to set some parameters
  //     postAddResidualObject(*bc);

  //     // Active BoundaryIDs for the object
  //     const std::set<BoundaryID> & boundary_ids = bc->boundaryIDs();
  //     _vars[tid].addBoundaryVar(boundary_ids, bc_var);

  //     ibc = std::static_pointer_cast<IntegratedBCBase>(bc);

  //     _integrated_bcs.addObject(ibc, tid);
  //     _vars[tid].addBoundaryVars(boundary_ids, ibc->getCoupledVars());
  //   }
  // }

  // else
  //   mooseError("Unknown BoundaryCondition type for object named ", bc->name());
}

void
LinearSystem::computeRightHandSideTag(NumericVector<Number> & residual, TagID tag_id)
{
  // _nl_vector_tags.clear();
  // _nl_vector_tags.insert(tag_id);
  // _nl_vector_tags.insert(residualVectorTag());

  // associateVectorToTag(residual, residualVectorTag());

  // computeResidualTags(_nl_vector_tags);

  // disassociateVectorFromTag(residual, residualVectorTag());
}

void
LinearSystem::computeRightHandSideTags(const std::set<TagID> & tags)
{
  parallel_object_only();

  TIME_SECTION("nl::computeResidualTags", 5);

  // _fe_problem.setCurrentNonlinearSystem(number());
  // _fe_problem.setCurrentlyComputingResidual(true);

  // bool required_residual = tags.find(residualVectorTag()) == tags.end() ? false : true;

  // _n_residual_evaluations++;

  // // not suppose to do anythin on matrix
  // deactiveAllMatrixTags();

  // FloatingPointExceptionGuard fpe_guard(_app);

  // for (const auto & numeric_vec : _vecs_to_zero_for_residual)
  //   if (hasVector(numeric_vec))
  //   {
  //     NumericVector<Number> & vec = getVector(numeric_vec);
  //     vec.close();
  //     vec.zero();
  //   }

  // try
  // {
  //   zeroTaggedVectors(tags);
  //   computeResidualInternal(tags);
  //   closeTaggedVectors(tags);

  //   if (required_residual)
  //   {
  //     auto & residual = getVector(residualVectorTag());
  //     if (_time_integrator)
  //       _time_integrator->postResidual(residual);
  //     else
  //       residual += *_Re_non_time;
  //     residual.close();
  //   }
  //   if (_fe_problem.computingScalingResidual())
  //     // We don't want to do nodal bcs or anything else
  //     return;

  //   computeNodalBCs(tags);
  //   closeTaggedVectors(tags);

  //   // If we are debugging residuals we need one more assignment to have the ghosted copy up to
  //   // date
  //   if (_need_residual_ghosted && _debugging_residuals && required_residual)
  //   {
  //     auto & residual = getVector(residualVectorTag());

  //     *_residual_ghosted = residual;
  //     _residual_ghosted->close();
  //   }
  //   // Need to close and update the aux system in case residuals were saved to it.
  //   if (_has_nodalbc_save_in)
  //     _fe_problem.getAuxiliarySystem().solution().close();
  //   if (hasSaveIn())
  //     _fe_problem.getAuxiliarySystem().update();
  // }
  // catch (MooseException & e)
  // {
  //   // The buck stops here, we have already handled the exception by
  //   // calling stopSolve(), it is now up to PETSc to return a
  //   // "diverged" reason during the next solve.
  // }

  // // not supposed to do anything on matrix
  // activeAllMatrixTags();

  // _fe_problem.setCurrentlyComputingResidual(false);
}

void
LinearSystem::computeLinearSystemTags(const std::set<TagID> & vector_tags,
                                      const std::set<TagID> & matrix_tags)
{
  // const bool required_residual =
  //     vector_tags.find(residualVectorTag()) == vector_tags.end() ? false : true;

  // try
  // {
  //   zeroTaggedVectors(vector_tags);
  //   computeResidualAndJacobianInternal(vector_tags, matrix_tags);
  //   closeTaggedVectors(vector_tags);
  //   closeTaggedMatrices(matrix_tags);

  //   if (required_residual)
  //   {
  //     auto & residual = getVector(residualVectorTag());
  //     if (_time_integrator)
  //       _time_integrator->postResidual(residual);
  //     else
  //       residual += *_Re_non_time;
  //     residual.close();
  //   }

  //   computeNodalBCsResidualAndJacobian();
  //   closeTaggedVectors(vector_tags);
  //   closeTaggedMatrices(matrix_tags);
  // }
  // catch (MooseException & e)
  // {
  //   // The buck stops here, we have already handled the exception by
  //   // calling stopSolve(), it is now up to PETSc to return a
  //   // "diverged" reason during the next solve.
  // }
}

void
LinearSystem::onTimestepBegin()
{
  // if (_time_integrator)
  //   _time_integrator->preSolve();
  // if (_predictor.get())
  //   _predictor->timestepSetup();
}

void
LinearSystem::setInitialSolution()
{
  // deactiveAllMatrixTags();

  // NumericVector<Number> & initial_solution(solution());
  // if (_predictor.get() && _predictor->shouldApply())
  // {
  //   TIME_SECTION("applyPredictor", 2, "Applying Predictor");

  //   _predictor->apply(initial_solution);
  //   _fe_problem.predictorCleanup(initial_solution);
  // }

  // // do nodal BC
  // {
  //   TIME_SECTION("initialBCs", 2, "Applying BCs To Initial Condition");

  //   ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  //   for (const auto & bnode : bnd_nodes)
  //   {
  //     BoundaryID boundary_id = bnode->_bnd_id;
  //     Node * node = bnode->_node;

  //     if (node->processor_id() == processor_id())
  //     {
  //       // reinit variables in nodes
  //       _fe_problem.reinitNodeFace(node, boundary_id, 0);

  //       if (_preset_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
  //       {
  //         const auto & preset_bcs = _preset_nodal_bcs.getActiveBoundaryObjects(boundary_id);
  //         for (const auto & preset_bc : preset_bcs)
  //           preset_bc->computeValue(initial_solution);
  //       }
  //       if (_ad_preset_nodal_bcs.hasActiveBoundaryObjects(boundary_id))
  //       {
  //         const auto & preset_bcs_res =
  //         _ad_preset_nodal_bcs.getActiveBoundaryObjects(boundary_id); for (const auto & preset_bc
  //         : preset_bcs_res)
  //           preset_bc->computeValue(initial_solution);
  //       }
  //     }
  //   }
  // }

  // _sys.solution->close();
  // update();

  // // Set constraint secondary values
  // setConstraintSecondaryValues(initial_solution, false);

  // if (_fe_problem.getDisplacedProblem())
  //   setConstraintSecondaryValues(initial_solution, true);
}

void
LinearSystem::setPredictor(std::shared_ptr<Predictor> predictor)
{
  _predictor = predictor;
}

// void
// LinearSystem::subdomainSetup(SubdomainID subdomain, THREAD_ID tid)
// {
//   SystemBase::subdomainSetup();

//   // _kernels.subdomainSetup(subdomain, tid);
//   // _nodal_kernels.subdomainSetup(subdomain, tid);
//   // _element_dampers.subdomainSetup(subdomain, tid);
//   // _nodal_dampers.subdomainSetup(subdomain, tid);
// }

NumericVector<Number> &
LinearSystem::getRightHandSideTimeVector()
{
  // if (!_Re_time)
  // {
  //   _Re_time_tag = _fe_problem.addVectorTag("TIME");

  //   // Most applications don't need the expense of ghosting
  //   ParallelType ptype = _need_residual_ghosted ? GHOSTED : PARALLEL;
  //   _Re_time = &addVector(_Re_time_tag, false, ptype);
  // }
  // else if (_need_residual_ghosted && _Re_time->type() == PARALLEL)
  // {
  //   const auto vector_name = _subproblem.vectorTagName(_Re_time_tag);

  //   // If an application changes its mind, the libMesh API lets us
  //   // change the vector.
  //   _Re_time = &system().add_vector(vector_name, false, GHOSTED);
  // }

  return *_rhs_time;
}

NumericVector<Number> &
LinearSystem::getRightHandSideNonTimeVector()
{
  // if (!_Re_non_time)
  // {
  //   _Re_non_time_tag = _fe_problem.addVectorTag("NONTIME");

  //   // Most applications don't need the expense of ghosting
  //   ParallelType ptype = _need_residual_ghosted ? GHOSTED : PARALLEL;
  //   _Re_non_time = &addVector(_Re_non_time_tag, false, ptype);
  // }
  // else if (_need_residual_ghosted && _Re_non_time->type() == PARALLEL)
  // {
  //   const auto vector_name = _subproblem.vectorTagName(_Re_non_time_tag);

  //   // If an application changes its mind, the libMesh API lets us
  //   // change the vector.
  //   _Re_non_time = &system().add_vector(vector_name, false, GHOSTED);
  // }

  return *_rhs_non_time;
}

void
LinearSystem::computeRightHandSideInternal(const std::set<TagID> & tags)
{
  parallel_object_only();

  // TIME_SECTION("computeResidualInternal", 3);

  // residualSetup();

  // const auto vector_tag_data = _fe_problem.getVectorTags(tags);

  // // Residual contributions from UOs - for now this is used for ray tracing
  // // and ray kernels that contribute to the residual (think line sources)
  // std::vector<UserObject *> uos;
  // _fe_problem.theWarehouse()
  //     .query()
  //     .condition<AttribSystem>("UserObject")
  //     .condition<AttribExecOns>(EXEC_PRE_KERNELS)
  //     .queryInto(uos);
  // for (auto & uo : uos)
  //   uo->residualSetup();
  // for (auto & uo : uos)
  // {
  //   uo->initialize();
  //   uo->execute();
  //   uo->finalize();
  // }

  // // reinit scalar variables
  // for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  //   _fe_problem.reinitScalars(tid);

  // // residual contributions from the domain
  // PARALLEL_TRY
  // {
  //   TIME_SECTION("Kernels", 3 /*, "Computing Kernels"*/);

  //   ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();

  //   ComputeResidualThread cr(_fe_problem, tags);
  //   Threads::parallel_reduce(elem_range, cr);

  //   if (_fe_problem.haveFV())
  //   {
  //     using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
  //     ComputeFVFluxResidualThread<FVRange> fvr(_fe_problem, this->number(), tags);
  //     FVRange faces(_fe_problem.mesh().ownedFaceInfoBegin(),
  //     _fe_problem.mesh().ownedFaceInfoEnd()); Threads::parallel_reduce(faces, fvr);
  //   }

  //   unsigned int n_threads = libMesh::n_threads();
  //   for (unsigned int i = 0; i < n_threads;
  //        i++) // Add any cached residuals that might be hanging around
  //     _fe_problem.addCachedResidual(i);
  // }
  // PARALLEL_CATCH;

  // // residual contributions from the scalar kernels
  // PARALLEL_TRY
  // {
  //   // do scalar kernels (not sure how to thread this)
  //   if (_scalar_kernels.hasActiveObjects())
  //   {
  //     TIME_SECTION("ScalarKernels", 3 /*, "Computing ScalarKernels"*/);

  //     MooseObjectWarehouse<ScalarKernelBase> * scalar_kernel_warehouse;
  //     // This code should be refactored once we can do tags for scalar
  //     // kernels
  //     // Should redo this based on Warehouse
  //     if (!tags.size() || tags.size() == _fe_problem.numVectorTags(Moose::VECTOR_TAG_RESIDUAL))
  //       scalar_kernel_warehouse = &_scalar_kernels;
  //     else if (tags.size() == 1)
  //       scalar_kernel_warehouse =
  //           &(_scalar_kernels.getVectorTagObjectWarehouse(*(tags.begin()), 0));
  //     else
  //       // scalar_kernels is not threading
  //       scalar_kernel_warehouse = &(_scalar_kernels.getVectorTagsObjectWarehouse(tags, 0));

  //     bool have_scalar_contributions = false;
  //     const auto & scalars = scalar_kernel_warehouse->getActiveObjects();
  //     for (const auto & scalar_kernel : scalars)
  //     {
  //       scalar_kernel->reinit();
  //       const std::vector<dof_id_type> & dof_indices = scalar_kernel->variable().dofIndices();
  //       const DofMap & dof_map = scalar_kernel->variable().dofMap();
  //       const dof_id_type first_dof = dof_map.first_dof();
  //       const dof_id_type end_dof = dof_map.end_dof();
  //       for (dof_id_type dof : dof_indices)
  //       {
  //         if (dof >= first_dof && dof < end_dof)
  //         {
  //           scalar_kernel->computeResidual();
  //           have_scalar_contributions = true;
  //           break;
  //         }
  //       }
  //     }
  //     if (have_scalar_contributions)
  //       _fe_problem.addResidualScalar();
  //   }
  // }
  // PARALLEL_CATCH;

  // // residual contributions from Block NodalKernels
  // PARALLEL_TRY
  // {
  //   if (_nodal_kernels.hasActiveBlockObjects())
  //   {
  //     TIME_SECTION("NodalKernels", 3 /*, "Computing NodalKernels"*/);

  //     ComputeNodalKernelsThread cnk(_fe_problem, _nodal_kernels, tags);

  //     ConstNodeRange & range = *_mesh.getLocalNodeRange();

  //     if (range.begin() != range.end())
  //     {
  //       _fe_problem.reinitNode(*range.begin(), 0);

  //       Threads::parallel_reduce(range, cnk);

  //       unsigned int n_threads = libMesh::n_threads();
  //       for (unsigned int i = 0; i < n_threads;
  //            i++) // Add any cached residuals that might be hanging around
  //         _fe_problem.addCachedResidual(i);
  //     }
  //   }
  // }
  // PARALLEL_CATCH;

  // if (_fe_problem.computingScalingResidual())
  //   // We computed the volumetric objects. We can return now before we get into
  //   // any strongly enforced constraint conditions or penalty-type objects
  //   // (DGKernels, IntegratedBCs, InterfaceKernels, Constraints)
  //   return;

  // // residual contributions from boundary NodalKernels
  // PARALLEL_TRY
  // {
  //   if (_nodal_kernels.hasActiveBoundaryObjects())
  //   {
  //     TIME_SECTION("NodalKernelBCs", 3 /*, "Computing NodalKernelBCs"*/);

  //     ComputeNodalKernelBcsThread cnk(_fe_problem, _nodal_kernels, tags);

  //     ConstBndNodeRange & bnd_node_range = *_mesh.getBoundaryNodeRange();

  //     Threads::parallel_reduce(bnd_node_range, cnk);

  //     unsigned int n_threads = libMesh::n_threads();
  //     for (unsigned int i = 0; i < n_threads;
  //          i++) // Add any cached residuals that might be hanging around
  //       _fe_problem.addCachedResidual(i);
  //   }
  // }
  // PARALLEL_CATCH;

  // mortarConstraints(Moose::ComputeType::Residual, tags, {});

  // if (_residual_copy.get())
  // {
  //   _Re_non_time->close();
  //   _Re_non_time->localize(*_residual_copy);
  // }

  // if (_need_residual_ghosted)
  // {
  //   _Re_non_time->close();
  //   *_residual_ghosted = *_Re_non_time;
  //   _residual_ghosted->close();
  // }

  // PARALLEL_TRY { computeDiracContributions(tags, false); }
  // PARALLEL_CATCH;

  // if (_fe_problem._has_constraints)
  // {
  //   PARALLEL_TRY { enforceNodalConstraintsResidual(*_Re_non_time); }
  //   PARALLEL_CATCH;
  //   _Re_non_time->close();
  // }

  // // Add in Residual contributions from other Constraints
  // if (_fe_problem._has_constraints)
  // {
  //   PARALLEL_TRY
  //   {
  //     // Undisplaced Constraints
  //     constraintResiduals(*_Re_non_time, false);

  //     // Displaced Constraints
  //     if (_fe_problem.getDisplacedProblem())
  //       constraintResiduals(*_Re_non_time, true);

  //     if (_fe_problem.computingNonlinearResid())
  //       _constraints.residualEnd();
  //   }
  //   PARALLEL_CATCH;
  //   _Re_non_time->close();
  // }

  // // Accumulate the occurrence of solution invalid warnings for the current iteration cumulative
  // // counters
  // _app.solutionInvalidity().solutionInvalidAccumulation();
}

void
LinearSystem::computeLinearSystemTagsInternal(const std::set<TagID> & vector_tags,
                                              const std::set<TagID> & matrix_tags)
{
  TIME_SECTION("computeLinearSystemInternal", 3);

  // // Make matrix ready to use
  // activeAllMatrixTags();

  // for (auto tag : matrix_tags)
  // {
  //   if (!hasMatrix(tag))
  //     continue;

  //   auto & jacobian = getMatrix(tag);
  //   // Necessary for speed
  //   if (auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&jacobian))
  //   {
  //     MatSetOption(petsc_matrix->mat(),
  //                  MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
  //                  PETSC_TRUE);
  //     if (!_fe_problem.errorOnJacobianNonzeroReallocation())
  //       MatSetOption(petsc_matrix->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  //   }
  // }

  // residualSetup();

  // // Residual contributions from UOs - for now this is used for ray tracing
  // // and ray kernels that contribute to the residual (think line sources)
  // std::vector<UserObject *> uos;
  // _fe_problem.theWarehouse()
  //     .query()
  //     .condition<AttribSystem>("UserObject")
  //     .condition<AttribExecOns>(EXEC_PRE_KERNELS)
  //     .queryInto(uos);
  // for (auto & uo : uos)
  //   uo->residualSetup();
  // for (auto & uo : uos)
  // {
  //   uo->initialize();
  //   uo->execute();
  //   uo->finalize();
  // }

  // // reinit scalar variables
  // for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  //   _fe_problem.reinitScalars(tid);

  // // residual contributions from the domain
  // PARALLEL_TRY
  // {
  //   TIME_SECTION("Kernels", 3 /*, "Computing Kernels"*/);

  //   ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();

  //   ComputeResidualAndJacobianThread crj(_fe_problem, vector_tags, matrix_tags);
  //   Threads::parallel_reduce(elem_range, crj);

  //   if (_fe_problem.haveFV())
  //   {
  //     using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
  //     ComputeFVFluxRJThread<FVRange> fvrj(_fe_problem, this->number(), vector_tags, matrix_tags);
  //     FVRange faces(_fe_problem.mesh().ownedFaceInfoBegin(),
  //     _fe_problem.mesh().ownedFaceInfoEnd()); Threads::parallel_reduce(faces, fvrj);
  //   }

  //   mortarConstraints(Moose::ComputeType::ResidualAndJacobian, vector_tags, matrix_tags);

  //   unsigned int n_threads = libMesh::n_threads();
  //   for (unsigned int i = 0; i < n_threads;
  //        i++) // Add any cached residuals that might be hanging around
  //   {
  //     _fe_problem.addCachedResidual(i);
  //     _fe_problem.addCachedJacobian(i);
  //   }
  // }
  // PARALLEL_CATCH;
}

void
LinearSystem::computeSystemMatricesInternal(const std::set<TagID> & tags)
{
  TIME_SECTION("computeSystemMatricesInternal", 3);

  _fe_problem.setCurrentNonlinearSystem(number());

  // // Make matrix ready to use
  // activeAllMatrixTags();

  // for (auto tag : tags)
  // {
  //   if (!hasMatrix(tag))
  //     continue;

  //   auto & jacobian = getMatrix(tag);
  //   // Necessary for speed
  //   if (auto petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&jacobian))
  //   {
  //     MatSetOption(petsc_matrix->mat(),
  //                  MAT_KEEP_NONZERO_PATTERN, // This is changed in 3.1
  //                  PETSC_TRUE);
  //     if (!_fe_problem.errorOnJacobianNonzeroReallocation())
  //       MatSetOption(petsc_matrix->mat(), MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);
  //   }
  // }

  // jacobianSetup();

  // // Jacobian contributions from UOs - for now this is used for ray tracing
  // // and ray kernels that contribute to the Jacobian (think line sources)
  // std::vector<UserObject *> uos;
  // _fe_problem.theWarehouse()
  //     .query()
  //     .condition<AttribSystem>("UserObject")
  //     .condition<AttribExecOns>(EXEC_PRE_KERNELS)
  //     .queryInto(uos);
  // for (auto & uo : uos)
  //   uo->jacobianSetup();
  // for (auto & uo : uos)
  // {
  //   uo->initialize();
  //   uo->execute();
  //   uo->finalize();
  // }

  // // reinit scalar variables
  // for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  //   _fe_problem.reinitScalars(tid);

  // PARALLEL_TRY
  // {
  //   // We would like to compute ScalarKernels, block NodalKernels, FVFluxKernels, and mortar objects
  //   // up front because we want these included whether we are computing an ordinary Jacobian or a
  //   // Jacobian for determining variable scaling factors
  //   computeScalarKernelsJacobians(tags);

  //   // Block restricted Nodal Kernels
  //   if (_nodal_kernels.hasActiveBlockObjects())
  //   {
  //     ComputeNodalKernelJacobiansThread cnkjt(_fe_problem, _nodal_kernels, tags);
  //     ConstNodeRange & range = *_mesh.getLocalNodeRange();
  //     Threads::parallel_reduce(range, cnkjt);

  //     unsigned int n_threads = libMesh::n_threads();
  //     for (unsigned int i = 0; i < n_threads;
  //          i++) // Add any cached jacobians that might be hanging around
  //       _fe_problem.assembly(i, number()).addCachedJacobian(Assembly::GlobalDataKey{});
  //   }

  //   if (_fe_problem.haveFV())
  //   {
  //     // the same loop works for both residual and jacobians because it keys
  //     // off of FEProblem's _currently_computing_jacobian parameter
  //     using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
  //     ComputeFVFluxJacobianThread<FVRange> fvj(_fe_problem, this->number(), tags);
  //     FVRange faces(_fe_problem.mesh().ownedFaceInfoBegin(),
  //     _fe_problem.mesh().ownedFaceInfoEnd()); Threads::parallel_reduce(faces, fvj);
  //   }

  //   mortarConstraints(Moose::ComputeType::Jacobian, {}, tags);

  //   // Get our element range for looping over
  //   ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();

  //   if (_fe_problem.computingScalingJacobian())
  //   {
  //     // Only compute Jacobians corresponding to the diagonals of volumetric compute objects
  //     // because this typically gives us a good representation of the physics. NodalBCs and
  //     // Constraints can introduce dramatically different scales (often order unity).
  //     // IntegratedBCs and/or InterfaceKernels may use penalty factors. DGKernels may be ok, but
  //     // they are almost always used in conjunction with Kernels
  //     ComputeJacobianForScalingThread cj(_fe_problem, tags);
  //     Threads::parallel_reduce(elem_range, cj);
  //     unsigned int n_threads = libMesh::n_threads();
  //     for (unsigned int i = 0; i < n_threads;
  //          i++) // Add any Jacobian contributions still hanging around
  //       _fe_problem.addCachedJacobian(i);

  //     closeTaggedMatrices(tags);

  //     return;
  //   }

  //   switch (_fe_problem.coupling())
  //   {
  //     case Moose::COUPLING_DIAG:
  //     {
  //       ComputeJacobianThread cj(_fe_problem, tags);
  //       Threads::parallel_reduce(elem_range, cj);

  //       unsigned int n_threads = libMesh::n_threads();
  //       for (unsigned int i = 0; i < n_threads;
  //            i++) // Add any Jacobian contributions still hanging around
  //         _fe_problem.addCachedJacobian(i);

  //       // Boundary restricted Nodal Kernels
  //       if (_nodal_kernels.hasActiveBoundaryObjects())
  //       {
  //         ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, _nodal_kernels, tags);
  //         ConstBndNodeRange & bnd_range = *_mesh.getBoundaryNodeRange();

  //         Threads::parallel_reduce(bnd_range, cnkjt);
  //         unsigned int n_threads = libMesh::n_threads();
  //         for (unsigned int i = 0; i < n_threads;
  //              i++) // Add any cached jacobians that might be hanging around
  //           _fe_problem.assembly(i, number()).addCachedJacobian(Assembly::GlobalDataKey{});
  //       }
  //     }
  //     break;

  //     default:
  //     case Moose::COUPLING_CUSTOM:
  //     {
  //       ComputeFullJacobianThread cj(_fe_problem, tags);
  //       Threads::parallel_reduce(elem_range, cj);
  //       unsigned int n_threads = libMesh::n_threads();

  //       for (unsigned int i = 0; i < n_threads; i++)
  //         _fe_problem.addCachedJacobian(i);

  //       // Boundary restricted Nodal Kernels
  //       if (_nodal_kernels.hasActiveBoundaryObjects())
  //       {
  //         ComputeNodalKernelBCJacobiansThread cnkjt(_fe_problem, _nodal_kernels, tags);
  //         ConstBndNodeRange & bnd_range = *_mesh.getBoundaryNodeRange();

  //         Threads::parallel_reduce(bnd_range, cnkjt);
  //         unsigned int n_threads = libMesh::n_threads();
  //         for (unsigned int i = 0; i < n_threads;
  //              i++) // Add any cached jacobians that might be hanging around
  //           _fe_problem.assembly(i, number()).addCachedJacobian(Assembly::GlobalDataKey{});
  //       }
  //     }
  //     break;
  //   }

  //   computeDiracContributions(tags, true);

  //   static bool first = true;

  //   // This adds zeroes into geometric coupling entries to ensure they stay in the matrix
  //   if (first && (_add_implicit_geometric_coupling_entries_to_jacobian))
  //   {
  //     first = false;
  //     addImplicitGeometricCouplingEntries(_fe_problem.geomSearchData());

  //     if (_fe_problem.getDisplacedProblem())
  //       addImplicitGeometricCouplingEntries(_fe_problem.getDisplacedProblem()->geomSearchData());
  //   }
  // }
  // PARALLEL_CATCH;

  // // Have no idea how to have constraints work
  // // with the tag system
  // PARALLEL_TRY
  // {
  //   // Add in Jacobian contributions from other Constraints
  //   if (_fe_problem._has_constraints)
  //   {
  //     // Some constraints need values from the Jacobian
  //     closeTaggedMatrices(tags);

  //     // Nodal Constraints
  //     enforceNodalConstraintsJacobian();

  //     // Undisplaced Constraints
  //     constraintJacobians(false);

  //     // Displaced Constraints
  //     if (_fe_problem.getDisplacedProblem())
  //       constraintJacobians(true);
  //   }
  // }
  // PARALLEL_CATCH;

  // // We need to close the save_in variables on the aux system before NodalBCBases clear the dofs
  // // on boundary nodes
  // if (_has_diag_save_in)
  //   _fe_problem.getAuxiliarySystem().solution().close();

  // PARALLEL_TRY
  // {
  //   MooseObjectWarehouse<NodalBCBase> * nbc_warehouse;
  //   // Select nodal kernels
  //   if (tags.size() == _fe_problem.numMatrixTags() || !tags.size())
  //     nbc_warehouse = &_nodal_bcs;
  //   else if (tags.size() == 1)
  //     nbc_warehouse = &(_nodal_bcs.getMatrixTagObjectWarehouse(*(tags.begin()), 0));
  //   else
  //     nbc_warehouse = &(_nodal_bcs.getMatrixTagsObjectWarehouse(tags, 0));

  //   if (nbc_warehouse->hasActiveObjects())
  //   {
  //     // We may be switching from add to set. Moreover, we rely on a call to MatZeroRows to enforce
  //     // the nodal boundary condition constraints which requires that the matrix be truly assembled
  //     // as opposed to just flushed. Consequently we can't do the following despite any desire to
  //     // keep our initial sparsity pattern honored (see https://gitlab.com/petsc/petsc/-/issues/852)
  //     //
  //     // flushTaggedMatrices(tags);
  //     closeTaggedMatrices(tags);

  //     // Cache the information about which BCs are coupled to which
  //     // variables, so we don't have to figure it out for each node.
  //     std::map<std::string, std::set<unsigned int>> bc_involved_vars;
  //     const std::set<BoundaryID> & all_boundary_ids = _mesh.getBoundaryIDs();
  //     for (const auto & bid : all_boundary_ids)
  //     {
  //       // Get reference to all the NodalBCs for this ID.  This is only
  //       // safe if there are NodalBCBases there to be gotten...
  //       if (nbc_warehouse->hasActiveBoundaryObjects(bid))
  //       {
  //         const auto & bcs = nbc_warehouse->getActiveBoundaryObjects(bid);
  //         for (const auto & bc : bcs)
  //         {
  //           const std::vector<MooseVariableFEBase *> & coupled_moose_vars =
  //               bc->getCoupledMooseVars();

  //           // Create the set of "involved" MOOSE nonlinear vars, which includes all coupled vars
  //           // and the BC's own variable
  //           std::set<unsigned int> & var_set = bc_involved_vars[bc->name()];
  //           for (const auto & coupled_var : coupled_moose_vars)
  //             if (coupled_var->kind() == Moose::VAR_NONLINEAR)
  //               var_set.insert(coupled_var->number());

  //           var_set.insert(bc->variable().number());
  //         }
  //       }
  //     }

  //     // reinit scalar variables again. This reinit does not re-fill any of the scalar variable
  //     // solution arrays because that was done above. It only will reorder the derivative
  //     // information for AD calculations to be suitable for NodalBC calculations
  //     for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  //       _fe_problem.reinitScalars(tid, true);

  //     // Get variable coupling list.  We do all the NodalBCBase stuff on
  //     // thread 0...  The couplingEntries() data structure determines
  //     // which variables are "coupled" as far as the preconditioner is
  //     // concerned, not what variables a boundary condition specifically
  //     // depends on.
  //     auto & coupling_entries = _fe_problem.couplingEntries(/*_tid=*/0);

  //     // Compute Jacobians for NodalBCBases
  //     ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  //     for (const auto & bnode : bnd_nodes)
  //     {
  //       BoundaryID boundary_id = bnode->_bnd_id;
  //       Node * node = bnode->_node;

  //       if (nbc_warehouse->hasActiveBoundaryObjects(boundary_id) &&
  //           node->processor_id() == processor_id())
  //       {
  //         _fe_problem.reinitNodeFace(node, boundary_id, 0);

  //         const auto & bcs = nbc_warehouse->getActiveBoundaryObjects(boundary_id);
  //         for (const auto & bc : bcs)
  //         {
  //           // Get the set of involved MOOSE vars for this BC
  //           std::set<unsigned int> & var_set = bc_involved_vars[bc->name()];

  //           // Loop over all the variables whose Jacobian blocks are
  //           // actually being computed, call computeOffDiagJacobian()
  //           // for each one which is actually coupled (otherwise the
  //           // value is zero.)
  //           for (const auto & it : coupling_entries)
  //           {
  //             unsigned int ivar = it.first->number(), jvar = it.second->number();

  //             // We are only going to call computeOffDiagJacobian() if:
  //             // 1.) the BC's variable is ivar
  //             // 2.) jvar is "involved" with the BC (including jvar==ivar), and
  //             // 3.) the BC should apply.
  //             if ((bc->variable().number() == ivar) && var_set.count(jvar) && bc->shouldApply())
  //               bc->computeOffDiagJacobian(jvar);
  //           }

  //           const auto & coupled_scalar_vars = bc->getCoupledMooseScalarVars();
  //           for (const auto & jvariable : coupled_scalar_vars)
  //             if (hasScalarVariable(jvariable->name()))
  //               bc->computeOffDiagJacobianScalar(jvariable->number());
  //         }
  //       }
  //     } // end loop over boundary nodes

  //     // Set the cached NodalBCBase values in the Jacobian matrix
  //     _fe_problem.assembly(0, number()).setCachedJacobian(Assembly::GlobalDataKey{});
  //   }
  // }
  // PARALLEL_CATCH;

  // closeTaggedMatrices(tags);

  // // We need to close the save_in variables on the aux system before NodalBCBases clear the dofs
  // // on boundary nodes
  // if (_has_nodalbc_diag_save_in)
  //   _fe_problem.getAuxiliarySystem().solution().close();

  // if (hasDiagSaveIn())
  //   _fe_problem.getAuxiliarySystem().update();

  // // Accumulate the occurrence of solution invalid warnings for the current iteration cumulative
  // // counters
  // _app.solutionInvalidity().solutionInvalidAccumulation();
}

void
LinearSystem::computeSystemMatrix(SparseMatrix<Number> & matrix)
{
  // _nl_matrix_tags.clear();

  // auto & tags = _fe_problem.getMatrixTags();

  // for (auto & tag : tags)
  //   _nl_matrix_tags.insert(tag.second);

  // computeJacobian(jacobian, _nl_matrix_tags);
}

void
LinearSystem::computeSystemMatrix(SparseMatrix<Number> & matrix, const std::set<TagID> & tags)
{
  associateMatrixToTag(matrix, systemMatrixTag());

  computeSystemMatrixTags(tags);

  disassociateMatrixFromTag(matrix, systemMatrixTag());
}

void
LinearSystem::computeSystemMatrixTags(const std::set<TagID> & tags)
{
  TIME_SECTION("computeSystemMatrixTags", 5);

  FloatingPointExceptionGuard fpe_guard(_app);

  try
  {
    computeSystemMatricesInternal(tags);
  }
  catch (MooseException & e)
  {
    // The buck stops here, we have already handled the exception by
    // calling stopSolve(), it is now up to PETSc to return a
    // "diverged" reason during the next solve.
  }
}

void
LinearSystem::updateActive(THREAD_ID tid)
{
  // _element_dampers.updateActive(tid);
  // _nodal_dampers.updateActive(tid);
  // _integrated_bcs.updateActive(tid);
  // _dg_kernels.updateActive(tid);
  // _interface_kernels.updateActive(tid);
  // _dirac_kernels.updateActive(tid);
  // _kernels.updateActive(tid);
  // _nodal_kernels.updateActive(tid);
  // if (tid == 0)
  // {
  //   _general_dampers.updateActive();
  //   _nodal_bcs.updateActive();
  //   _preset_nodal_bcs.updateActive();
  //   _ad_preset_nodal_bcs.updateActive();
  //   _constraints.updateActive();
  //   _scalar_kernels.updateActive();
  // }
}

void
LinearSystem::augmentSparsity(SparsityPattern::Graph & sparsity,
                              std::vector<dof_id_type> & n_nz,
                              std::vector<dof_id_type> & n_oz)
{
  // if (_add_implicit_geometric_coupling_entries_to_jacobian)
  // {
  //   _fe_problem.updateGeomSearch();

  //   std::unordered_map<dof_id_type, std::vector<dof_id_type>> graph;

  //   findImplicitGeometricCouplingEntries(_fe_problem.geomSearchData(), graph);

  //   if (_fe_problem.getDisplacedProblem())
  //     findImplicitGeometricCouplingEntries(_fe_problem.getDisplacedProblem()->geomSearchData(),
  //                                          graph);

  //   const dof_id_type first_dof_on_proc = dofMap().first_dof(processor_id());
  //   const dof_id_type end_dof_on_proc = dofMap().end_dof(processor_id());

  //   // The total number of dofs on and off processor
  //   const dof_id_type n_dofs_on_proc = dofMap().n_local_dofs();
  //   const dof_id_type n_dofs_not_on_proc = dofMap().n_dofs() - dofMap().n_local_dofs();

  //   for (const auto & git : graph)
  //   {
  //     dof_id_type dof = git.first;
  //     dof_id_type local_dof = dof - first_dof_on_proc;

  //     if (dof < first_dof_on_proc || dof >= end_dof_on_proc)
  //       continue;

  //     const auto & row = git.second;

  //     SparsityPattern::Row & sparsity_row = sparsity[local_dof];

  //     unsigned int original_row_length = sparsity_row.size();

  //     sparsity_row.insert(sparsity_row.end(), row.begin(), row.end());

  //     SparsityPattern::sort_row(
  //         sparsity_row.begin(), sparsity_row.begin() + original_row_length, sparsity_row.end());

  //     // Fix up nonzero arrays
  //     for (const auto & coupled_dof : row)
  //     {
  //       if (coupled_dof < first_dof_on_proc || coupled_dof >= end_dof_on_proc)
  //       {
  //         if (n_oz[local_dof] < n_dofs_not_on_proc)
  //           n_oz[local_dof]++;
  //       }
  //       else
  //       {
  //         if (n_nz[local_dof] < n_dofs_on_proc)
  //           n_nz[local_dof]++;
  //       }
  //     }
  //   }
  // }
}

void
LinearSystem::serializeSolution()
{
  if (_serialized_solution.get())
  {
    if (!_serialized_solution->initialized() || _serialized_solution->size() != _sys.n_dofs())
    {
      _serialized_solution->clear();
      _serialized_solution->init(_sys.n_dofs(), false, SERIAL);
    }

    _current_solution->localize(*_serialized_solution);
  }
}

NumericVector<Number> &
LinearSystem::serializedSolution()
{
  if (!_serialized_solution.get())
  {
    _serialized_solution = NumericVector<Number>::build(_communicator);
    _serialized_solution->init(_sys.n_dofs(), false, SERIAL);
  }

  return *_serialized_solution;
}

void
LinearSystem::setSolution(const NumericVector<Number> & soln)
{
  _current_solution = &soln;

  auto tag = _subproblem.getVectorTagID(Moose::SOLUTION_TAG);
  associateVectorToTag(const_cast<NumericVector<Number> &>(soln), tag);

  if (_serialized_solution.get())
    serializeSolution();
}

void
LinearSystem::setPreconditioner(std::shared_ptr<MoosePreconditioner> pc)
{
  if (_preconditioner.get() != nullptr)
    mooseError("More than one active Preconditioner detected");

  _preconditioner = pc;
}

MoosePreconditioner const *
LinearSystem::getPreconditioner() const
{
  return _preconditioner.get();
}

void
LinearSystem::checkKernelCoverage(const std::set<SubdomainID> & mesh_subdomains) const
{
  // // Obtain all blocks and variables covered by all kernels
  // std::set<SubdomainID> input_subdomains;
  // std::set<std::string> kernel_variables;

  // bool global_kernels_exist = false;
  // global_kernels_exist |= _scalar_kernels.hasActiveObjects();
  // global_kernels_exist |= _nodal_kernels.hasActiveObjects();

  // _kernels.subdomainsCovered(input_subdomains, kernel_variables);
  // _dg_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  // _nodal_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  // _scalar_kernels.subdomainsCovered(input_subdomains, kernel_variables);
  // _constraints.subdomainsCovered(input_subdomains, kernel_variables);

  // if (_fe_problem.haveFV())
  // {
  //   std::vector<FVElementalKernel *> fv_elemental_kernels;
  //   _fe_problem.theWarehouse()
  //       .query()
  //       .template condition<AttribSystem>("FVElementalKernel")
  //       .queryInto(fv_elemental_kernels);

  //   for (auto fv_kernel : fv_elemental_kernels)
  //   {
  //     if (fv_kernel->blockRestricted())
  //       for (auto block_id : fv_kernel->blockIDs())
  //         input_subdomains.insert(block_id);
  //     else
  //       global_kernels_exist = true;
  //     kernel_variables.insert(fv_kernel->variable().name());

  //     // Check for lagrange multiplier
  //     if (dynamic_cast<FVScalarLagrangeMultiplierConstraint *>(fv_kernel))
  //       kernel_variables.insert(dynamic_cast<FVScalarLagrangeMultiplierConstraint *>(fv_kernel)
  //                                   ->lambdaVariable()
  //                                   .name());
  //   }

  //   std::vector<FVFluxKernel *> fv_flux_kernels;
  //   _fe_problem.theWarehouse()
  //       .query()
  //       .template condition<AttribSystem>("FVFluxKernel")
  //       .queryInto(fv_flux_kernels);

  //   for (auto fv_kernel : fv_flux_kernels)
  //   {
  //     if (fv_kernel->blockRestricted())
  //       for (auto block_id : fv_kernel->blockIDs())
  //         input_subdomains.insert(block_id);
  //     else
  //       global_kernels_exist = true;
  //     kernel_variables.insert(fv_kernel->variable().name());
  //   }

  //   std::vector<FVInterfaceKernel *> fv_interface_kernels;
  //   _fe_problem.theWarehouse()
  //       .query()
  //       .template condition<AttribSystem>("FVInterfaceKernel")
  //       .queryInto(fv_interface_kernels);

  //   for (auto fvik : fv_interface_kernels)
  //     if (auto scalar_fvik = dynamic_cast<FVScalarLagrangeMultiplierInterface *>(fvik))
  //       kernel_variables.insert(scalar_fvik->lambdaVariable().name());

  //   std::vector<FVFluxBC *> fv_flux_bcs;
  //   _fe_problem.theWarehouse()
  //       .query()
  //       .template condition<AttribSystem>("FVFluxBC")
  //       .queryInto(fv_flux_bcs);

  //   for (auto fvbc : fv_flux_bcs)
  //     if (auto scalar_fvbc = dynamic_cast<FVBoundaryScalarLagrangeMultiplierConstraint *>(fvbc))
  //       kernel_variables.insert(scalar_fvbc->lambdaVariable().name());
  // }

  // // Check kernel coverage of subdomains (blocks) in your mesh
  // if (!global_kernels_exist)
  // {
  //   std::set<SubdomainID> difference;
  //   std::set_difference(mesh_subdomains.begin(),
  //                       mesh_subdomains.end(),
  //                       input_subdomains.begin(),
  //                       input_subdomains.end(),
  //                       std::inserter(difference, difference.end()));

  //   // there supposed to be no kernels on this lower-dimensional subdomain
  //   difference.erase(Moose::INTERNAL_SIDE_LOWERD_ID);
  //   difference.erase(Moose::BOUNDARY_SIDE_LOWERD_ID);

  //   if (!difference.empty())
  //   {
  //     std::vector<SubdomainID> difference_vec =
  //         std::vector<SubdomainID>(difference.begin(), difference.end());
  //     std::vector<SubdomainName> difference_names = _mesh.getSubdomainNames(difference_vec);
  //     std::stringstream missing_block_names;
  //     std::copy(difference_names.begin(),
  //               difference_names.end(),
  //               std::ostream_iterator<std::string>(missing_block_names, " "));
  //     std::stringstream missing_block_ids;
  //     std::copy(difference.begin(),
  //               difference.end(),
  //               std::ostream_iterator<unsigned int>(missing_block_ids, " "));

  //     mooseError("Each subdomain must contain at least one Kernel.\nThe following block(s) lack
  //     an "
  //                "active kernel: " +
  //                    missing_block_names.str(),
  //                " (ids: ",
  //                missing_block_ids.str(),
  //                ")");
  //   }
  // }

  // // Check kernel use of variables
  // std::set<VariableName> variables(getVariableNames().begin(), getVariableNames().end());

  // std::set<VariableName> difference;
  // std::set_difference(variables.begin(),
  //                     variables.end(),
  //                     kernel_variables.begin(),
  //                     kernel_variables.end(),
  //                     std::inserter(difference, difference.end()));

  // // skip checks for varaibles defined on lower-dimensional subdomain
  // std::set<VariableName> vars(difference);
  // for (auto & var_name : vars)
  // {
  //   auto blks = getSubdomainsForVar(var_name);
  //   if (blks.count(Moose::INTERNAL_SIDE_LOWERD_ID) || blks.count(Moose::BOUNDARY_SIDE_LOWERD_ID))
  //     difference.erase(var_name);
  // }

  // if (!difference.empty())
  // {
  //   std::stringstream missing_kernel_vars;
  //   std::copy(difference.begin(),
  //             difference.end(),
  //             std::ostream_iterator<std::string>(missing_kernel_vars, " "));
  //   mooseError("Each variable must be referenced by at least one active Kernel.\nThe following "
  //              "variable(s) lack an active kernel: " +
  //              missing_kernel_vars.str());
  // }
}

bool
LinearSystem::containsTimeKernel()
{
  auto & time_kernels = _kernels.getVectorTagObjectWarehouse(timeVectorTag(), 0);

  return time_kernels.hasActiveObjects();
}

void
LinearSystem::setPCSide(MooseEnum pcs)
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
LinearSystem::setMooseKSPNormType(MooseEnum kspnorm)
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
