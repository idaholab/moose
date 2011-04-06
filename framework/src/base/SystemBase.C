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

// libMesh
#include "quadrature_gauss.h"

/// Free function used for a libMesh callback
void extraSendList(std::vector<unsigned int> & send_list, void * context)
{
  SystemBase * sys = static_cast<SystemBase *>(context);
  sys->augmentSendList(send_list);
}

SystemBase::SystemBase(SubProblemInterface & subproblem, const std::string & name) :
    _problem(*subproblem.parent()),
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

const std::set<subdomain_id_type> *
SystemBase::getVariableBlocks(unsigned int var_number)
{
  mooseAssert(_var_map.find(var_number) != _var_map.end(), "Variable does not exist.");
  if (_var_map[var_number].empty())
    return NULL;
  else
    return & _var_map[var_number];
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
    var->sizeResidual();
    var->sizeJacobianBlock();
  }
}

void
SystemBase::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->reinit();
    var->computeElemValues();
  }
}

void
SystemBase::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
/*  for (std::set<MooseVariable *>::iterator it = _vars[tid].boundaryVars(bnd_id).begin();
      it != _vars[tid].boundaryVars(bnd_id).end();
       ++it)
*/
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->reinit();
    var->computeElemValuesFace();
  }
}

void
SystemBase::reinitNode(const Node * node, THREAD_ID tid)
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
SystemBase::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
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
