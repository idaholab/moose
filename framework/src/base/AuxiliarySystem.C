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

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "Factory.h"
#include "AuxKernel.h"
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
    _fe_problem(subproblem),
    _sys(subproblem.es().add_system<TransientExplicitSystem>(name)),
    _current_solution(NULL),
    _serialized_solution(*NumericVector<Number>::build(_fe_problem.comm()).release()),
    _solution_previous_nl(NULL),
    _u_dot(addVector("u_dot", true, GHOSTED)),
    _need_serialized_solution(false)
{
  _nodal_vars.resize(libMesh::n_threads());
  _elem_vars.resize(libMesh::n_threads());
}

AuxiliarySystem::~AuxiliarySystem() { delete &_serialized_solution; }

void
AuxiliarySystem::init()
{
}

void
AuxiliarySystem::initialSetup()
{
  if (_fe_problem.needsPreviousNewtonIteration())
    _solution_previous_nl = &addVector("u_previous_newton", true, GHOSTED);

  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.sort(tid);
    _aux_scalar_storage.initialSetup(tid);

    _nodal_aux_storage.sort(tid);
    _nodal_aux_storage.initialSetup(tid);

    _elemental_aux_storage.sort(tid);
    _elemental_aux_storage.initialSetup(tid);
  }
}

void
AuxiliarySystem::timestepSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.timestepSetup(tid);
    _nodal_aux_storage.timestepSetup(tid);
    _elemental_aux_storage.timestepSetup(tid);
  }
}

void
AuxiliarySystem::subdomainSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.subdomainSetup(tid);
    _nodal_aux_storage.subdomainSetup(tid);
    _elemental_aux_storage.subdomainSetup(tid);
  }
}

void
AuxiliarySystem::jacobianSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.jacobianSetup(tid);
    _nodal_aux_storage.jacobianSetup(tid);
    _elemental_aux_storage.jacobianSetup(tid);
  }
}

void
AuxiliarySystem::residualSetup()
{
  for (unsigned int tid = 0; tid < libMesh::n_threads(); tid++)
  {
    _aux_scalar_storage.residualSetup(tid);
    _nodal_aux_storage.residualSetup(tid);
    _elemental_aux_storage.residualSetup(tid);
  }
}

void
AuxiliarySystem::updateActive(THREAD_ID tid)
{
  _aux_scalar_storage.updateActive(tid);
  _nodal_aux_storage.updateActive(tid);
  _elemental_aux_storage.updateActive(tid);
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
    MooseVariable * var = dynamic_cast<MooseVariable *>(_vars[tid].getVariable(var_name));
    if (var != NULL)
    {
      if (var->feType().family == LAGRANGE)
        _nodal_vars[tid][var_name] = var;
      else
        _elem_vars[tid][var_name] = var;
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
    std::shared_ptr<AuxKernel> kernel =
        _factory.create<AuxKernel>(kernel_name, name, parameters, tid);
    if (kernel->isNodal())
      _nodal_aux_storage.addObject(kernel, tid);
    else
      _elemental_aux_storage.addObject(kernel, tid);
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
  for (const auto & it : _nodal_vars[tid])
  {
    MooseVariable * var = it.second;
    var->computeElemValues();
  }

  for (const auto & it : _elem_vars[tid])
  {
    MooseVariable * var = it.second;
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
  for (const auto & it : _nodal_vars[tid])
  {
    MooseVariable * var = it.second;
    var->computeElemValuesFace();
  }

  for (const auto & it : _elem_vars[tid])
  {
    MooseVariable * var = it.second;
    var->reinitAux();
    var->reinitAuxNeighbor();
    var->computeElemValuesFace();
  }
}

NumericVector<Number> &
AuxiliarySystem::solutionUDot()
{
  return _u_dot;
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
  if (_fe_problem.dt() > 0.)
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
    if (_fe_problem.dt() > 0.)
      _time_integrator->computeTimeDerivatives();
  }

  if (_vars[0].variables().size() > 0)
  {
    computeNodalVars(type);
    // compute time derivatives of nodal aux variables _after_ the values were updated
    if (_fe_problem.dt() > 0.)
      _time_integrator->computeTimeDerivatives();
  }

  if (_vars[0].variables().size() > 0)
  {
    computeElementalVars(type);
    // compute time derivatives of elemental aux variables _after_ the values were updated
    if (_fe_problem.dt() > 0.)
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

  // Nodal AuxKernels
  {
    const std::vector<std::shared_ptr<AuxKernel>> & auxs = _nodal_aux_storage.getActiveObjects();
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
AuxiliarySystem::computeScalarVars(ExecFlagType type)
{
  // Reference to the current storage container
  const MooseObjectWarehouse<AuxScalarKernel> & storage = _aux_scalar_storage[type];

  if (storage.hasActiveObjects())
  {
    std::string compute_aux_tag = "computeScalarAux(" + Moose::stringify(type) + ")";
    Moose::perf_log.push(compute_aux_tag, "Execution");

    PARALLEL_TRY
    {
      // FIXME: run multi-threaded
      THREAD_ID tid = 0;
      if (storage.hasActiveObjects())
      {
        _fe_problem.reinitScalars(tid);

        // Call compute() method on all active AuxScalarKernel objects
        const std::vector<std::shared_ptr<AuxScalarKernel>> & objects =
            storage.getActiveObjects(tid);
        for (const auto & obj : objects)
          obj->compute();

        const std::vector<MooseVariableScalar *> & scalar_vars = getScalarVariables(tid);
        for (const auto & var : scalar_vars)
          var->insert(solution());
      }
    }
    PARALLEL_CATCH;

    Moose::perf_log.pop(compute_aux_tag, "Execution");

    solution().close();
    _sys.update();
  }
}

void
AuxiliarySystem::computeNodalVars(ExecFlagType type)
{
  // Reference to the Nodal AuxKernel storage
  const MooseObjectWarehouse<AuxKernel> & nodal = _nodal_aux_storage[type];

  if (nodal.hasActiveBlockObjects())
  {
    std::string compute_aux_tag = "computeNodalAux(" + Moose::stringify(type) + ")";
    Moose::perf_log.push(compute_aux_tag, "Execution");

    // Block Nodal AuxKernels
    PARALLEL_TRY
    {
      ConstNodeRange & range = *_mesh.getLocalNodeRange();
      ComputeNodalAuxVarsThread navt(_fe_problem, nodal);
      Threads::parallel_reduce(range, navt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
    Moose::perf_log.pop(compute_aux_tag, "Execution");
  }

  if (nodal.hasActiveBoundaryObjects())
  {
    std::string compute_aux_tag = "computeNodalAuxBCs(" + Moose::stringify(type) + ")";
    Moose::perf_log.push(compute_aux_tag, "Execution");

    // Boundary Nodal AuxKernels
    PARALLEL_TRY
    {
      ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
      ComputeNodalAuxBcsThread nabt(_fe_problem, nodal);
      Threads::parallel_reduce(bnd_nodes, nabt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
    Moose::perf_log.pop(compute_aux_tag, "Execution");
  }
}

void
AuxiliarySystem::computeElementalVars(ExecFlagType type)
{
  // Reference to the Nodal AuxKernel storage
  const MooseObjectWarehouse<AuxKernel> & elemental = _elemental_aux_storage[type];

  if (elemental.hasActiveBlockObjects())
  {
    std::string compute_aux_tag = "computeElemAux(" + Moose::stringify(type) + ")";
    Moose::perf_log.push(compute_aux_tag, "Execution");

    // Block Elemental AuxKernels
    PARALLEL_TRY
    {
      ConstElemRange & range = *_mesh.getActiveLocalElementRange();
      ComputeElemAuxVarsThread eavt(_fe_problem, elemental, true);
      Threads::parallel_reduce(range, eavt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
    Moose::perf_log.pop(compute_aux_tag, "Execution");
  }

  // Boundary Elemental AuxKernels
  if (elemental.hasActiveBoundaryObjects())
  {
    std::string compute_aux_tag = "computeElemAuxBCs(" + Moose::stringify(type) + ")";
    Moose::perf_log.push(compute_aux_tag, "Execution");

    PARALLEL_TRY
    {
      ConstBndElemRange & bnd_elems = *_mesh.getBoundaryElementRange();
      ComputeElemAuxBcsThread eabt(_fe_problem, elemental, true);
      Threads::parallel_reduce(bnd_elems, eabt);

      solution().close();
      _sys.update();
    }
    PARALLEL_CATCH;
    Moose::perf_log.pop(compute_aux_tag, "Execution");
  }
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
  std::vector<MooseVariable *> vars = _vars[0].variables();
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
  return _elemental_aux_storage.hasActiveBoundaryObjects(bnd_id);
}

void
AuxiliarySystem::setPreviousNewtonSolution()
{
  // Evaluate aux variables to get the solution vector
  compute(EXEC_LINEAR);
}
