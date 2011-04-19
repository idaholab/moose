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

#include "DisplacedSystem.h"
#include "DisplacedProblem.h"

DisplacedSystem::DisplacedSystem(DisplacedProblem & problem, SystemBase & undisplaced_system, const std::string & name) :
    SystemTempl<TransientExplicitSystem>(problem, name),
    _undisplaced_system(undisplaced_system)
{
}

DisplacedSystem::~DisplacedSystem()
{
}

void
DisplacedSystem::init()
{
  dofMap().attach_extra_send_list_function(&extraSendList, this);
}

void
DisplacedSystem::prepare(THREAD_ID tid)
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
DisplacedSystem::reinitElem(const Elem * /*elem*/, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->reinit();
    var->computeElemValues();
  }
}

void
DisplacedSystem::reinitElemFace(const Elem * /*elem*/, unsigned int /*side*/, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::set<MooseVariable *>::iterator it = _vars[tid].boundaryVars(bnd_id).begin();
       it != _vars[tid].boundaryVars(bnd_id).end();
       ++it)
  {
    MooseVariable *var = *it;
    var->reinit();
    var->computeElemValuesFace();
  }
}

void
DisplacedSystem::reinitNode(const Node * /*node*/, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    if (var->feType().family == LAGRANGE)
    {
      var->reinit_node();
      var->computeNodalValues();
    }
  }
}

void
DisplacedSystem::reinitNodeFace(const Node * /*node*/, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::set<MooseVariable *>::iterator it = _vars[tid].boundaryVars(bnd_id).begin();
       it != _vars[tid].boundaryVars(bnd_id).end();
       ++it)
  {
    MooseVariable *var = *it;
    if (var->feType().family == LAGRANGE)
    {
      var->reinit_node();
      var->computeNodalValues();
   }
  }
}

NumericVector<Number> &
DisplacedSystem::getVector(std::string name)
{
  if(_sys.have_vector(name))
    return _sys.get_vector(name);
  else
    return _undisplaced_system.getVector(name);
}

