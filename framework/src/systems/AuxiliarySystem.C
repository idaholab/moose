//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "Factory.h"
#include "AuxKernel.h"
#include "AuxVectorKernel.h"
#include "AuxScalarKernel.h"
#include "MaterialData.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "ComputeNodalAuxVarsThread.h"
#include "ComputeNodalAuxBcsThread.h"
#include "ComputeElemAuxVarsThread.h"
#include "ComputeElemAuxBcsThread.h"
#include "Parser.h"
#include "TimeIntegrator.h"
#include "Conversion.h"

#include "libmesh/quadrature_gauss.h"
#include "libmesh/node_range.h"
#include "libmesh/numeric_vector.h"

// AuxiliarySystem ////////

AuxiliarySystem::AuxiliarySystem(FEProblemBase & subproblem, const std::string & name)
  : SystemBase(subproblem, name, Moose::VAR_AUXILIARY),
    PerfGraphInterface(subproblem.getMooseApp().perfGraph(), "AuxiliarySystem"),
    _fe_problem(subproblem),
    _sys(subproblem.es().add_system<TransientExplicitSystem>(name)),
    _current_solution(NULL),
    _serialized_solution(*NumericVector<Number>::build(_fe_problem.comm()).release()),
    _solution_previous_nl(NULL),
    _u_dot(NULL),
    _u_dotdot(NULL),
    _u_dot_old(NULL),
    _u_dotdot_old(NULL),
    _need_serialized_solution(false),
    _aux_scalar_storage(_app.getExecuteOnEnum()),
    _nodal_aux_storage(_app.getExecuteOnEnum()),
    _elemental_aux_storage(_app.getExecuteOnEnum()),
    _nodal_vec_aux_storage(_app.getExecuteOnEnum()),
    _elemental_vec_aux_storage(_app.getExecuteOnEnum()),
    _compute_scalar_vars_timer(registerTimedSection("computeScalarVars", 1)),
    _compute_nodal_vars_timer(registerTimedSection("computeNodalVars", 1)),
    _compute_nodal_vec_vars_timer(registerTimedSection("computeNodalVecVars", 1)),
    _compute_elemental_vars_timer(registerTimedSection("computeElementalVars", 1)),
    _compute_elemental_vec_vars_timer(registerTimedSection("computeElementalVecVars", 1))
{
  _nodal_vars.resize(libMesh::n_threads());
  _nodal_std_vars.resize(libMesh::n_threads());
  _nodal_vec_vars.resize(libMesh::n_threads());
  _elem_vars.resize(libMesh::n_threads());
  _elem_std_vars.resize(libMesh::n_threads());
  _elem_vec_vars.resize(libMesh::n_threads());
}

AuxiliarySystem::~AuxiliarySystem() { delete &_serialized_solution; }

void
AuxiliarySystem::init()
{
}

void
AuxiliarySystem::addDotVectors()
{
  if (_fe_problem.uDotRequested())
    _u_dot = &addVector("u_dot", true, GHOSTED);
  if (_fe_problem.uDotDotRequested())
    _u_dotdot = &addVector("u_dotdot", true, GHOSTED);
  if (_fe_problem.uDotOldRequested())
    _u_dot_old = &addVector("u_dot_old", true, GHOSTED);
  if (_fe_problem.uDotDotOldRequested())
    _u_dotdot_old = &addVector("u_dotdot_old", true, GHOSTED);
}

void
AuxiliarySystem::addExtraVectors()
{
  if (_fe_problem.needsPreviousNewtonIteration())
    _solution_previous_nl = &addVector("u_previous_newton", true, GHOSTED);
}

void
AuxiliarySystem::initialSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.sort(tid);
    _aux_scalar_storage.initialSetup(tid);

    _nodal_aux_storage.sort(tid);
    _nodal_aux_storage.initialSetup(tid);

    _nodal_vec_aux_storage.sort(tid);
    _nodal_vec_aux_storage.initialSetup(tid);

    _elemental_aux_storage.sort(tid);
    _elemental_aux_storage.initialSetup(tid);

    _elemental_vec_aux_storage.sort(tid);
    _elemental_vec_aux_storage.initialSetup(tid);
  }
}

void
AuxiliarySystem::timestepSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.timestepSetup(tid);
    _nodal_aux_storage.timestepSetup(tid);
    _nodal_vec_aux_storage.timestepSetup(tid);
    _elemental_aux_storage.timestepSetup(tid);
    _elemental_vec_aux_storage.timestepSetup(tid);
  }
}

void
AuxiliarySystem::subdomainSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.subdomainSetup(tid);
    _nodal_aux_storage.subdomainSetup(tid);
    _nodal_vec_aux_storage.subdomainSetup(tid);
    _elemental_aux_storage.subdomainSetup(tid);
    _elemental_vec_aux_storage.subdomainSetup(tid);
  }
}

void
AuxiliarySystem::jacobianSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.jacobianSetup(tid);
    _nodal_aux_storage.jacobianSetup(tid);
    _nodal_vec_aux_storage.jacobianSetup(tid);
    _elemental_aux_storage.jacobianSetup(tid);
    _elemental_vec_aux_storage.jacobianSetup(tid);
  }
}

void
AuxiliarySystem::residualSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.residualSetup(tid);
    _nodal_aux_storage.residualSetup(tid);
    _nodal_vec_aux_storage.residualSetup(tid);
    _elemental_aux_storage.residualSetup(tid);
    _elemental_vec_aux_storage.residualSetup(tid);
  }
}

void
AuxiliarySystem::updateActive(THREAD_ID tid)
{
  _aux_scalar_storage.updateActive(tid);
  _nodal_aux_storage.updateActive(tid);
  _nodal_vec_aux_storage.updateActive(tid);
  _elemental_aux_storage.updateActive(tid);
  _elemental_vec_aux_storage.updateActive(tid);
}

void
AuxiliarySystem::addVariable(const std::string & var_name,
                             const FEType & type,
                             Real scale_factor,
                             const std::set<SubdomainID> * const active_subdomains /* = NULL*/)
{
  SystemBase::addVariable(var_name, type, scale_factor, active_subdomains);
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    if (type.family == LAGRANGE_VEC)
    {
      VectorMooseVariable * var = _vars[tid].getFieldVariable<RealVectorValue>(var_name);
      if (var)
      {
        _nodal_vars[tid].push_back(var);
        _nodal_vec_vars[tid].push_back(var);
      }
    }

    else
    {
      MooseVariable * var = _vars[tid].getFieldVariable<Real>(var_name);

      if (var)
      {
        if (var->feType().family == LAGRANGE)
        {
          _nodal_vars[tid].push_back(var);
          _nodal_std_vars[tid].push_back(var);
        }
        else
        {
          _elem_vars[tid].push_back(var);
          _elem_std_vars[tid].push_back(var);
        }
      }
    }
  }
}

void
AuxiliarySystem::addTimeIntegrator(const std::string & type,
                                   const std::string & name,
                                   InputParameters parameters)
{
  parameters.set<SystemBase *>("_sys") = this;
  _time_integrator = _factory.create<TimeIntegrator>(type, name, parameters);
}

void
AuxiliarySystem::addKernel(const std::string & kernel_name,
                           const std::string & name,
                           InputParameters parameters)
{
  parameters.set<AuxiliarySystem *>("_aux_sys") = this;

  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    if (parameters.get<std::string>("_moose_base") == "AuxKernel")
    {
      std::shared_ptr<AuxKernel> kernel =
          _factory.create<AuxKernel>(kernel_name, name, parameters, tid);
      if (kernel->isNodal())
        _nodal_aux_storage.addObject(kernel, tid);
      else
        _elemental_aux_storage.addObject(kernel, tid);
    }

    else if (parameters.get<std::string>("_moose_base") == "AuxVectorKernel")
    {
      std::shared_ptr<AuxVectorKernel> kernel =
          _factory.create<AuxVectorKernel>(kernel_name, name, parameters, tid);
      if (kernel->isNodal())
        _nodal_vec_aux_storage.addObject(kernel, tid);
      else
        _elemental_vec_aux_storage.addObject(kernel, tid);
    }
  }
}

void
AuxiliarySystem::addScalarKernel(const std::string & kernel_name,
                                 const std::string & name,
                                 InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    std::shared_ptr<AuxScalarKernel> kernel =
        _factory.create<AuxScalarKernel>(kernel_name, name, parameters, tid);
    _aux_scalar_storage.addObject(kernel, tid);
  }
}

void
AuxiliarySystem::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{
  for (auto * var : _nodal_vars[tid])
    var->computeElemValues();

  for (auto * var : _elem_vars[tid])
  {
    var->reinitAux();
    var->computeElemValues();
  }
}

void
AuxiliarySystem::reinitElemFace(const Elem * /*elem*/,
                                unsigned int /*side*/,
                                BoundaryID /*bnd_id*/,
                                THREAD_ID tid)
{
  for (auto * var : _nodal_vars[tid])
    var->computeElemValuesFace();

  for (auto * var : _elem_vars[tid])
  {
    var->reinitAux();
    var->reinitAuxNeighbor();
    var->computeElemValuesFace();
  }
}

NumericVector<Number> &
AuxiliarySystem::serializedSolution()
{
  _need_serialized_solution = true;
  return _serialized_solution;
}

void
AuxiliarySystem::serializeSolution()
{
  if (_need_serialized_solution &&
      _sys.n_dofs() > 0) // libMesh does not like serializing of empty vectors
  {
    if (!_serialized_solution.initialized() || _serialized_solution.size() != _sys.n_dofs())
    {
      _serialized_solution.clear();
      _serialized_solution.init(_sys.n_dofs(), false, SERIAL);
    }

    solution().localize(_serialized_solution);
  }
}

void
AuxiliarySystem::compute(ExecFlagType type)
{
  // avoid division by dt which might be zero.
  if (_fe_problem.dt() > 0. && _time_integrator)
    _time_integrator->preStep();

  // We need to compute time derivatives every time each kind of the variables is finished, because:
  //
  //  a) the user might want to use the aux variable value somewhere, thus we need to provide the
  //  up-to-date value
  //  b) time integration system works with the whole vectors of solutions, thus we cannot update
  //  only a part of the vector
  //

  if (_vars[0].scalars().size() > 0)
  {
    computeScalarVars(type);
    // compute time derivatives of scalar aux variables _after_ the values were updated
    if (_fe_problem.dt() > 0. && _time_integrator)
      _time_integrator->computeTimeDerivatives();
  }

  if (_vars[0].fieldVariables().size() > 0)
  {
    computeNodalVars(type);
    computeNodalVecVars(type);
    // compute time derivatives of nodal aux variables _after_ the values were updated
    if (_fe_problem.dt() > 0. && _time_integrator)
      _time_integrator->computeTimeDerivatives();
  }

  if (_vars[0].fieldVariables().size() > 0)
  {
    computeElementalVars(type);
    computeElementalVecVars(type);
    // compute time derivatives of elemental aux variables _after_ the values were updated
    if (_fe_problem.dt() > 0. && _time_integrator)
      _time_integrator->computeTimeDerivatives();
  }

  if (_need_serialized_solution)
    serializeSolution();
}

std::set<std::string>
AuxiliarySystem::getDependObjects(ExecFlagType type)
{
  std::set<std::string> depend_objects;

  // Elemental AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _elemental_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Elemental AuxVectorKernels
  {
    const std::vector<std::shared_ptr<AuxVectorKernel>> & auxs =
        _elemental_vec_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _nodal_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal AuxVectorKernels
  {
    const std::vector<std::shared_ptr<AuxVectorKernel>> & auxs =
        _nodal_vec_aux_storage[type].getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  return depend_objects;
}

std::set<std::string>
AuxiliarySystem::getDependObjects()
{
  std::set<std::string> depend_objects;

  // Elemental AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs =
        _elemental_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Elemental AuxVectorKernels
  {
    const std::vector<std::shared_ptr<AuxVectorKernel>> & auxs =
        _elemental_vec_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs = _nodal_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  // Nodal AuxVectorKernels
  {
    const std::vector<std::shared_ptr<AuxVectorKernel>> & auxs =
        _nodal_vec_aux_storage.getActiveObjects();
    for (const auto & aux : auxs)
    {
      const std::set<std::string> & uo = aux->getDependObjects();
      depend_objects.insert(uo.begin(), uo.end());
    }
  }

  return depend_objects;
}

NumericVector<Number> &
AuxiliarySystem::addVector(const std::string & vector_name,
                           const bool project,
                           const ParallelType type)
{
  if (hasVector(vector_name))
    return getVector(vector_name);

  NumericVector<Number> * vec = &_sys.add_vector(vector_name, project, type);

  return *vec;
}

void
AuxiliarySystem::setScalarVariableCoupleableTags(ExecFlagType type)
{
  const MooseObjectWarehouse<AuxScalarKernel> & storage = _aux_scalar_storage[type];
  const std::vector<std::shared_ptr<AuxScalarKernel>> & objects = storage.getActiveObjects(0);

  std::set<TagID> needed_sc_var_matrix_tags;
  std::set<TagID> needed_sc_var_vector_tags;
  for (const auto & obj : objects)
  {
    auto & sc_var_coup_vtags = obj->getScalarVariableCoupleableVectorTags();
    needed_sc_var_vector_tags.insert(sc_var_coup_vtags.begin(), sc_var_coup_vtags.end());

    auto & sc_var_coup_mtags = obj->getScalarVariableCoupleableMatrixTags();
    needed_sc_var_matrix_tags.insert(sc_var_coup_mtags.begin(), sc_var_coup_mtags.end());
  }

  _fe_problem.setActiveScalarVariableCoupleableMatrixTags(needed_sc_var_matrix_tags, 0);
  _fe_problem.setActiveScalarVariableCoupleableVectorTags(needed_sc_var_vector_tags, 0);
}

void
AuxiliarySystem::clearScalarVariableCoupleableTags()
{
  _fe_problem.clearActiveScalarVariableCoupleableMatrixTags(0);
  _fe_problem.clearActiveScalarVariableCoupleableVectorTags(0);
}

void
AuxiliarySystem::computeScalarVars(ExecFlagType type)
{
  setScalarVariableCoupleableTags(type);

  // Reference to the current storage container
  const MooseObjectWarehouse<AuxScalarKernel> & storage = _aux_scalar_storage[type];

  if (storage.hasActiveObjects())
  {
    TIME_SECTION(_compute_scalar_vars_timer);

    PARALLEL_TRY
    {
      // FIXME: run multi-threaded
      THREAD_ID tid = 0;
      if (storage.hasActiveObjects())
      {
        _fe_problem.reinitScalars(tid);

        const std::vector<std::shared_ptr<AuxScalarKernel>> & objects =
            storage.getActiveObjects(tid);

        // Call compute() method on all active AuxScalarKernel objects
        for (const auto & obj : objects)
          obj->compute();

        const std::vector<MooseVariableScalar *> & scalar_vars = getScalarVariables(tid);
        for (const auto & var : scalar_vars)
          var->insert(solution());
      }
    }
    PARALLEL_CATCH;

    solution().close();
    _sys.update();
  }

  clearScalarVariableCoupleableTags();
}

void
AuxiliarySystem::computeNodalVars(ExecFlagType type)
{
  const MooseObjectWarehouse<AuxKernel> & nodal = _nodal_aux_storage[type];
  computeNodalVarsHelper<AuxKernel>(nodal, _nodal_std_vars, _compute_nodal_vars_timer);
}

void
AuxiliarySystem::computeNodalVecVars(ExecFlagType type)
{
  const MooseObjectWarehouse<AuxVectorKernel> & nodal = _nodal_vec_aux_storage[type];
  computeNodalVarsHelper<AuxVectorKernel>(nodal, _nodal_vec_vars, _compute_nodal_vec_vars_timer);
}

void
AuxiliarySystem::computeElementalVars(ExecFlagType type)
{
  const MooseObjectWarehouse<AuxKernel> & elemental = _elemental_aux_storage[type];
  computeElementalVarsHelper<AuxKernel>(elemental, _elem_std_vars, _compute_elemental_vars_timer);
}

void
AuxiliarySystem::computeElementalVecVars(ExecFlagType type)
{
  const MooseObjectWarehouse<AuxVectorKernel> & elemental = _elemental_vec_aux_storage[type];
  computeElementalVarsHelper<AuxVectorKernel>(
      elemental, _elem_vec_vars, _compute_elemental_vec_vars_timer);
}

void
AuxiliarySystem::augmentSparsity(SparsityPattern::Graph & /*sparsity*/,
                                 std::vector<dof_id_type> & /*n_nz*/,
                                 std::vector<dof_id_type> & /*n_oz*/)
{
}

Order
AuxiliarySystem::getMinQuadratureOrder()
{
  Order order = CONSTANT;
  std::vector<MooseVariableFEBase *> vars = _vars[0].fieldVariables();
  for (const auto & var : vars)
  {
    if (!var->isNodal()) // nodal aux variables do not need quadrature
    {
      FEType fe_type = var->feType();
      if (fe_type.default_quadrature_order() > order)
        order = fe_type.default_quadrature_order();
    }
  }

  return order;
}

bool
AuxiliarySystem::needMaterialOnSide(BoundaryID bnd_id)
{
  return _elemental_aux_storage.hasActiveBoundaryObjects(bnd_id) ||
         _elemental_vec_aux_storage.hasActiveBoundaryObjects(bnd_id);
}

void
AuxiliarySystem::setPreviousNewtonSolution()
{
  // Evaluate aux variables to get the solution vector
  compute(EXEC_LINEAR);
}

template <typename AuxKernelType>
void
AuxiliarySystem::computeElementalVarsHelper(
    const MooseObjectWarehouse<AuxKernelType> & warehouse,
    const std::vector<std::vector<MooseVariableFEBase *>> & vars,
    const PerfID timer)
{
  if (warehouse.hasActiveBlockObjects())
  {
    TIME_SECTION(timer);

    // Block Elemental AuxKernels
    PARALLEL_TRY
    {
      ConstElemRange & range = *_mesh.getActiveLocalElementRange();
      ComputeElemAuxVarsThread<AuxKernelType> eavt(_fe_problem, warehouse, vars, true);
      Threads::parallel_reduce(range, eavt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
  }

  // Boundary Elemental AuxKernels
  if (warehouse.hasActiveBoundaryObjects())
  {
    TIME_SECTION(_compute_elemental_vec_vars_timer);

    PARALLEL_TRY
    {
      ConstBndElemRange & bnd_elems = *_mesh.getBoundaryElementRange();
      ComputeElemAuxBcsThread<AuxKernelType> eabt(_fe_problem, warehouse, vars, true);
      Threads::parallel_reduce(bnd_elems, eabt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
  }
}

template <typename AuxKernelType>
void
AuxiliarySystem::computeNodalVarsHelper(
    const MooseObjectWarehouse<AuxKernelType> & warehouse,
    const std::vector<std::vector<MooseVariableFEBase *>> & vars,
    const PerfID timer)
{
  if (warehouse.hasActiveBlockObjects())
  {
    TIME_SECTION(timer);

    // Block Nodal AuxKernels
    PARALLEL_TRY
    {
      ConstNodeRange & range = *_mesh.getLocalNodeRange();
      ComputeNodalAuxVarsThread<AuxKernelType> navt(_fe_problem, warehouse, vars);
      Threads::parallel_reduce(range, navt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
  }

  if (warehouse.hasActiveBoundaryObjects())
  {
    TIME_SECTION(timer);

    // Boundary Nodal AuxKernels
    PARALLEL_TRY
    {
      ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
      ComputeNodalAuxBcsThread<AuxKernelType> nabt(_fe_problem, warehouse, vars);
      Threads::parallel_reduce(bnd_nodes, nabt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
  }
}

template void AuxiliarySystem::computeElementalVarsHelper<AuxKernel>(
    const MooseObjectWarehouse<AuxKernel> &,
    const std::vector<std::vector<MooseVariableFEBase *>> &,
    const PerfID);
template void AuxiliarySystem::computeElementalVarsHelper<AuxVectorKernel>(
    const MooseObjectWarehouse<AuxVectorKernel> &,
    const std::vector<std::vector<MooseVariableFEBase *>> &,
    const PerfID);
template void AuxiliarySystem::computeNodalVarsHelper<AuxKernel>(
    const MooseObjectWarehouse<AuxKernel> &,
    const std::vector<std::vector<MooseVariableFEBase *>> &,
    const PerfID);
template void AuxiliarySystem::computeNodalVarsHelper<AuxVectorKernel>(
    const MooseObjectWarehouse<AuxVectorKernel> &,
    const std::vector<std::vector<MooseVariableFEBase *>> &,
    const PerfID);
