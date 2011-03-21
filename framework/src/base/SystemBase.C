#include "SystemBase.h"
#include "Factory.h"
#include "SubProblem.h"
#include "MooseVariable.h"

// libMesh
#include "quadrature_gauss.h"


SystemBase::SystemBase(ProblemInterface & problem, const std::string & name) :
    _problem(problem),
    _mesh(problem.mesh()),
    _name(name)
{
  _vars.resize(libMesh::n_threads());
}

MooseVariable &
SystemBase::getVariable(THREAD_ID tid, const std::string & var_name)
{
  MooseVariable * var = _vars[tid].getVariable(var_name);
  if (var == NULL)
    mooseError("variable " + var_name + " does not exist in this system");
  return *var;
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
SystemBase::reinitNode(const Node * node, THREAD_ID tid)
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
SystemBase::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
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
