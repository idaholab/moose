#include "DisplacedSystem.h"

DisplacedSystem::DisplacedSystem(ProblemInterface & problem, const std::string & name) :
    SystemTempl<TransientExplicitSystem>(problem, name)
{
}

DisplacedSystem::~DisplacedSystem()
{
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
DisplacedSystem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::vector<MooseVariable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    MooseVariable *var = *it;
    var->reinit();
    var->computeElemValues();
  }
}

void
DisplacedSystem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
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
DisplacedSystem::reinitNode(const Node * node, THREAD_ID tid)
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
DisplacedSystem::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
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
