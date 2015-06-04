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

#include "libmesh/quadrature_gauss.h"
#include "libmesh/node_range.h"

// AuxiliarySystem ////////

AuxiliarySystem::AuxiliarySystem(FEProblem & subproblem, const std::string & name) :
    SystemTempl<TransientExplicitSystem>(subproblem, name, Moose::VAR_AUXILIARY),
    _fe_problem(subproblem),
    _serialized_solution(*NumericVector<Number>::build(_fe_problem.comm()).release()),
    _u_dot(addVector("u_dot", true, GHOSTED)),
    _need_serialized_solution(false)
{
  _nodal_vars.resize(libMesh::n_threads());
  _elem_vars.resize(libMesh::n_threads());
}

AuxiliarySystem::~AuxiliarySystem()
{
  delete &_serialized_solution;
}

void
AuxiliarySystem::init()
{
}

void
AuxiliarySystem::initialSetup()
{
  for (unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _auxs(EXEC_INITIAL)[i].initialSetup();
    _auxs(EXEC_TIMESTEP_BEGIN)[i].initialSetup();
    _auxs(EXEC_TIMESTEP_END)[i].initialSetup();
    _auxs(EXEC_NONLINEAR)[i].initialSetup();
    _auxs(EXEC_LINEAR)[i].initialSetup();
  }
}

void
AuxiliarySystem::timestepSetup()
{
  for (unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _auxs(EXEC_TIMESTEP_BEGIN)[i].timestepSetup();
    _auxs(EXEC_TIMESTEP_END)[i].timestepSetup();
    _auxs(EXEC_NONLINEAR)[i].timestepSetup();
    _auxs(EXEC_LINEAR)[i].timestepSetup();
  }
}

void
AuxiliarySystem::jacobianSetup()
{
  for (unsigned int i=0; i<libMesh::n_threads(); i++)
  {
    _auxs(EXEC_NONLINEAR)[i].jacobianSetup();
    _auxs(EXEC_LINEAR)[i].jacobianSetup();
  }
}

void
AuxiliarySystem::residualSetup()
{
  for (unsigned int i=0; i<libMesh::n_threads(); i++)
    _auxs(EXEC_LINEAR)[i].residualSetup();
}

void
AuxiliarySystem::addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains/* = NULL*/)
{
  SystemTempl<TransientExplicitSystem>::addVariable(var_name, type, scale_factor, active_subdomains);
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
AuxiliarySystem::addTimeIntegrator(const std::string & type, const std::string & name, InputParameters parameters)
{
  parameters.set<SystemBase *>("_sys") = this;
  _time_integrator = MooseSharedNamespace::static_pointer_cast<TimeIntegrator>(_factory.create(type, name, parameters));
}

void
AuxiliarySystem::addKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  parameters.set<AuxiliarySystem *>("_aux_sys") = this;
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    MooseSharedPointer<AuxKernel> kernel = MooseSharedNamespace::static_pointer_cast<AuxKernel>(_factory.create(kernel_name, name, parameters));

    // Add this AuxKernel to multiple ExecStores
    const std::vector<ExecFlagType> & exec_flags = kernel->execFlags();
    for (unsigned int i=0; i<exec_flags.size(); ++i)
      _auxs(exec_flags[i])[tid].addAuxKernel(kernel);

    _fe_problem._objects_by_name[tid][name].push_back(kernel.get());

    if (kernel->boundaryRestricted())
    {
      const std::set<BoundaryID> & boundary_ids = kernel->boundaryIDs();
      for (std::set<BoundaryID>::const_iterator it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
        _vars[tid].addBoundaryVar(*it, &kernel->variable());
    }
  }
}

void
AuxiliarySystem::addScalarKernel(const std::string & kernel_name, const std::string & name, InputParameters parameters)
{
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;

    MooseSharedPointer<AuxScalarKernel> kernel = MooseSharedNamespace::static_pointer_cast<AuxScalarKernel>(_factory.create(kernel_name, name, parameters));

    // Add this AuxKernel to multiple ExecStores
    const std::vector<ExecFlagType> & exec_flags = kernel->execFlags();
    for (unsigned int i=0; i<exec_flags.size(); ++i)
      _auxs(exec_flags[i])[tid].addScalarKernel(kernel);

    _fe_problem._objects_by_name[tid][name].push_back(kernel.get());
  }
}

void
AuxiliarySystem::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{
  for (std::map<std::string, MooseVariable *>::iterator it = _nodal_vars[tid].begin(); it != _nodal_vars[tid].end(); ++it)
  {
    MooseVariable *var = it->second;
    var->computeElemValues();
  }

  for (std::map<std::string, MooseVariable *>::iterator it = _elem_vars[tid].begin(); it != _elem_vars[tid].end(); ++it)
  {
    MooseVariable *var = it->second;
    var->reinitAux();
    var->computeElemValues();
  }
}

void
AuxiliarySystem::reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  for (std::map<std::string, MooseVariable *>::iterator it = _nodal_vars[tid].begin(); it != _nodal_vars[tid].end(); ++it)
  {
    MooseVariable *var = it->second;
    var->computeElemValuesFace();
  }

  for (std::map<std::string, MooseVariable *>::iterator it = _elem_vars[tid].begin(); it != _elem_vars[tid].end(); ++it)
  {
    MooseVariable *var = it->second;
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
  if (_need_serialized_solution && _sys.n_dofs() > 0)            // libMesh does not like serializing of empty vectors
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
AuxiliarySystem::compute(ExecFlagType type/* = EXEC_LINEAR*/)
{
  // avoid division by dt which might be zero.
  if (_fe_problem.dt() > 0.)
    _time_integrator->preStep();

  // We need to compute time derivatives every time each kind of the variables is finished, because:
  //
  //  a) the user might want to use the aux variable value somewhere, thus we need to provide the up-to-date value
  //  b) time integration system works with the whole vectors of solutions, thus we cannot update only a part of the vector
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

  const std::vector<AuxKernel *> & auxs = _auxs(type)[0].all();
  for (std::vector<AuxKernel *>::const_iterator it = auxs.begin(); it != auxs.end(); ++it)
  {
    const std::set<std::string> & uo = (*it)->getDependObjects();
    depend_objects.insert(uo.begin(), uo.end());
  }
  return depend_objects;
}

NumericVector<Number> &
AuxiliarySystem::addVector(const std::string & vector_name, const bool project, const ParallelType type)
{
  if (hasVector(vector_name))
    return getVector(vector_name);

  NumericVector<Number> * vec = &_sys.add_vector(vector_name, project, type);

  return *vec;
}

void
AuxiliarySystem::computeScalarVars(ExecFlagType type)
{
  Moose::perf_log.push("update_aux_vars_scalar()","Solve");

  std::vector<AuxWarehouse> & auxs = _auxs(type);
  PARALLEL_TRY {
    // FIXME: run multi-threaded
    THREAD_ID tid = 0;
    if (auxs[tid].scalars().size() > 0)
    {
      _fe_problem.reinitScalars(tid);

      const std::vector<AuxScalarKernel *> & scalars = auxs[tid].scalars();
      for (std::vector<AuxScalarKernel *>::const_iterator it = scalars.begin(); it != scalars.end(); ++it)
      {
        AuxScalarKernel * kernel = *it;
        kernel->compute();
      }

      const std::vector<MooseVariableScalar *> & scalar_vars = getScalarVariables(tid);
      for (std::vector<MooseVariableScalar *>::const_iterator it = scalar_vars.begin(); it != scalar_vars.end(); ++it)
      {
        MooseVariableScalar * var = *it;
        var->insert(solution());
      }
    }
  }
  PARALLEL_CATCH;
  Moose::perf_log.pop("update_aux_vars_scalar()","Solve");

  solution().close();
  _sys.update();
}

void
AuxiliarySystem::computeNodalVars(ExecFlagType type)
{
  std::vector<AuxWarehouse> & auxs = _auxs(type);

  // Do we have some kernels to evaluate?
  bool have_block_kernels = false;
  for (std::set<SubdomainID>::const_iterator subdomain_it = _mesh.meshSubdomains().begin();
      subdomain_it != _mesh.meshSubdomains().end();
      ++subdomain_it)
  {
    have_block_kernels |= (auxs[0].activeBlockNodalKernels(*subdomain_it).size() > 0);
  }

  Moose::perf_log.push("update_aux_vars_nodal()","Solve");
  PARALLEL_TRY {
    if (have_block_kernels)
    {
      ConstNodeRange & range = *_mesh.getLocalNodeRange();
      ComputeNodalAuxVarsThread navt(_fe_problem, *this, auxs);
      Threads::parallel_reduce(range, navt);

      solution().close();
      _sys.update();
    }
  }
  PARALLEL_CATCH;
  Moose::perf_log.pop("update_aux_vars_nodal()","Solve");

  //Boundary AuxKernels
  Moose::perf_log.push("update_aux_vars_nodal_bcs()","Solve");
  PARALLEL_TRY {
    // after converting this into NodeRange, we can run it in parallel
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    ComputeNodalAuxBcsThread nabt(_fe_problem, *this, auxs);
    Threads::parallel_reduce(bnd_nodes, nabt);

    solution().close();
    _sys.update();
  }
  PARALLEL_CATCH;
  Moose::perf_log.pop("update_aux_vars_nodal_bcs()","Solve");
}

void
AuxiliarySystem::computeElementalVars(ExecFlagType type)
{
  Moose::perf_log.push("update_aux_vars_elemental()","Solve");

  std::vector<AuxWarehouse> & auxs = _auxs(type);
  bool need_materials = true; //type != EXEC_INITIAL;

  PARALLEL_TRY {
    bool element_auxs_to_compute = false;

    for (unsigned int i=0; i<auxs.size(); i++)
      element_auxs_to_compute |= auxs[i].allElementKernels().size();

    if (element_auxs_to_compute)
    {
      ConstElemRange & range = *_mesh.getActiveLocalElementRange();
      ComputeElemAuxVarsThread eavt(_fe_problem, *this, auxs, need_materials);
      Threads::parallel_reduce(range, eavt);

      solution().close();
      _sys.update();
    }

    bool bnd_auxs_to_compute = false;
    for (unsigned int i=0; i<auxs.size(); i++)
      bnd_auxs_to_compute |= auxs[i].allElementalBCs().size();
    if (bnd_auxs_to_compute)
    {
      ConstBndElemRange & bnd_elems = *_mesh.getBoundaryElementRange();
      ComputeElemAuxBcsThread eabt(_fe_problem, *this, auxs, need_materials);
      Threads::parallel_reduce(bnd_elems, eabt);

      solution().close();
      _sys.update();
    }

  }
  PARALLEL_CATCH;
  Moose::perf_log.pop("update_aux_vars_elemental()","Solve");
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
  for (std::vector<MooseVariable *>::iterator it = vars.begin(); it != vars.end(); ++it)
  {
    if (!(*it)->isNodal()) // nodal aux variables do not need quadrature
    {
      FEType fe_type = (*it)->feType();
      if (fe_type.default_quadrature_order() > order)
        order = fe_type.default_quadrature_order();
    }
  }

  return order;
}

bool
AuxiliarySystem::needMaterialOnSide(BoundaryID bnd_id)
{
  for (unsigned int i=0; i < Moose::exec_types.size(); ++i)
    if (!_auxs(Moose::exec_types[i])[0].elementalBCs(bnd_id).empty() ||
        !_auxs(Moose::exec_types[i])[0].elementalBCs(Moose::ANY_BOUNDARY_ID).empty())
      return true;

  return false;
}
