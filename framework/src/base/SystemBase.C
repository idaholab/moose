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

#include "SystemBase.h"
#include "Factory.h"
#include "SubProblem.h"
#include "MooseVariable.h"
#include "Conversion.h"
#include "Parser.h"
#include "AllLocalDofIndicesThread.h"
#include "MooseTypes.h"

// libMesh
#include "quadrature_gauss.h"

/// Free function used for a libMesh callback
void extraSendList(std::vector<unsigned int> & send_list, void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSendList(send_list);
}

/// Free function used for a libMesh callback
void extraSparsity(SparsityPattern::Graph & sparsity,
                   std::vector<unsigned int> & n_nz,
                   std::vector<unsigned int> & n_oz,
                   void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSparsity(sparsity, n_nz, n_oz);
}

SystemBase::SystemBase(SubProblem & subproblem, const std::string & name) :
    _subproblem(subproblem),
    _mesh(subproblem.mesh()),
    _name(name),
    _currently_computing_jacobian(false),
    _vars(libMesh::n_threads()),
    _var_map()
{
}

MooseVariable &
SystemBase::getVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariable * var = _vars[tid].getVariable(var_name);
  if (var == NULL)
    mooseError("variable " + var_name + " does not exist in this system");
  return *var;
}

MooseVariable &
SystemBase::getVariable(THREAD_ID tid, unsigned int var_number)
{
  MooseVariable * var = _vars[tid].all()[var_number];
  if (var == NULL)
    mooseError("variable #" + Moose::stringify(var_number) + " does not exist in this system");
  return *var;
}

MooseVariableScalar &
SystemBase::getScalarVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariableScalar * var = _vars[tid].getScalarVariable(var_name);
  if (var == NULL)
    mooseError("variable " + var_name + " does not exist in this system");
  return *var;
}

MooseVariableScalar &
SystemBase::getScalarVariable(THREAD_ID tid, unsigned int var_number)
{
  MooseVariableScalar * var = _vars[tid].scalars()[var_number];
  if (var == NULL)
    mooseError("variable #" + Moose::stringify(var_number) + " does not exist in this system");
  return *var;
}

const std::set<SubdomainID> *
SystemBase::getVariableBlocks(unsigned int var_number)
{
  mooseAssert(_var_map.find(var_number) != _var_map.end(), "Variable does not exist.");
  if (_var_map[var_number].empty())
    return NULL;
  else
    return & _var_map[var_number];
}

void
SystemBase::addVariableToZeroOnResidual(std::string var_name)
{
  _vars_to_be_zeroed_on_residual.push_back(var_name);
}

void
SystemBase::zeroVariables()
{
  if(_vars_to_be_zeroed_on_residual.size() > 0)
  {
    AllLocalDofIndicesThread aldit(system(), _vars_to_be_zeroed_on_residual);
    ConstElemRange & elem_range = *_mesh.getActiveLocalElementRange();
    Threads::parallel_reduce(elem_range, aldit);

    std::set<unsigned int> dof_indices_to_zero = aldit._all_dof_indices;

    NumericVector<Number> & solution = this->solution();

    solution.close();

    for(std::set<unsigned int>::iterator it = dof_indices_to_zero.begin();
        it != dof_indices_to_zero.end();
        ++it)
      solution.set(*it, 0);

    solution.close();

    // Call update to update the current_local_solution for this system
    system().update();
  }
}

Order
SystemBase::getMinQuadratureOrder()
{
  Order order = CONSTANT;
  std::vector<MooseVariable *> vars = _vars[0].all();
  for (std::vector<MooseVariable *>::iterator it = vars.begin(); it != vars.end(); ++it)
  {
    FEType fe_type = (*it)->feType();
    if (fe_type.default_quadrature_order() > order)
      order = fe_type.default_quadrature_order();
  }

  return order;
}

void
SystemBase::prepare(THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->prepare();
  }
}

void
SystemBase::prepareNeighbor(THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->prepareNeighbor();
  }
}


void
SystemBase::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeElemValues();
  }
}

void
SystemBase::reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
/*
  for (std::set<MooseVariable *>::iterator it = _vars[tid].boundaryVars(bnd_id).begin();
    it != _vars[tid].boundaryVars(bnd_id).end();
     ++it)
*/
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeElemValuesFace();
  }
}

void
SystemBase::reinitNeighborFace(const Elem * /*elem*/, unsigned int /*side*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeNeighborValuesFace();
  }
}

void
SystemBase::reinitNeighbor(const Elem * /*elem*/, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->computeNeighborValues();
  }
}

void
SystemBase::reinitNode(const Node * /*node*/, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    if (var->isNodal())
    {
      var->reinit_node();
      var->computeNodalValues();
    }
  }
}

void
SystemBase::reinitNodeFace(const Node * /*node*/, BoundaryID /*bnd_id*/, THREAD_ID tid)
{
/*  for (std::set<MooseVariable *>::iterator it = _vars[tid].boundaryVars(bnd_id).begin();
       it != _vars[tid].boundaryVars(bnd_id).end();
       ++it)
*/
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    if (var->isNodal())
    {
      var->reinit_node();
      var->computeNodalValues();
    }
  }
}

void
SystemBase::reinitNodeNeighbor(const Node * /*node*/, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    if (var->isNodal())
    {
      var->reinit_nodeNeighbor();
      var->computeNodalNeighborValues();
    }
  }
}

void
SystemBase::reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->reinitNodes(nodes);
    var->computeNodalValues();
  }
}

void
SystemBase::reinitScalars(THREAD_ID tid)
{
  for (std::vector<MooseVariableScalar *>::iterator it = _vars[tid].scalars().begin(); it != _vars[tid].scalars().end(); ++it)
  {
    MooseVariableScalar *var = *it;
    var->reinit();
  }
}


void
SystemBase::addInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = &_subproblem;
  parameters.set<SystemBase *>("_sys") = this;

  const std::string & var_name = parameters.get<VariableName>("variable");
  const std::vector<SubdomainName> & blocks = parameters.get<std::vector<SubdomainName> >("block");

  for(unsigned int tid=0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    if (blocks.size() > 0)
      for (unsigned int i = 0; i < blocks.size(); i++)
      {
        SubdomainID blk_id = _mesh.getSubdomainID(blocks[i]);
        _vars[tid].addInitialCondition(var_name, blk_id, static_cast<InitialCondition *>(Factory::instance()->create(ic_name, name, parameters)));
      }
    else
      _vars[tid].addInitialCondition(var_name, Moose::ANY_BLOCK_ID, static_cast<InitialCondition *>(Factory::instance()->create(ic_name, name, parameters)));
  }
}

void
SystemBase::addScalarInitialCondition(const std::string & ic_name, const std::string & name, InputParameters parameters)
{
  parameters.set<SubProblem *>("_subproblem") = &_subproblem;
  parameters.set<SystemBase *>("_sys") = this;

  const std::string & var_name = parameters.get<VariableName>("variable");

  for(unsigned int tid=0; tid < libMesh::n_threads(); tid++)
  {
    parameters.set<THREAD_ID>("_tid") = tid;
    _vars[tid].addScalarInitialCondition(var_name, static_cast<ScalarInitialCondition *>(Factory::instance()->create(ic_name, name, parameters)));
  }
}

