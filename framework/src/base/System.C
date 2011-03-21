#include "System.h"
#include "Factory.h"
#include "SubProblem.h"
#include "Variable.h"

// libMesh
#include "quadrature_gauss.h"

namespace Moose {


System::System(SubProblem & problem, const std::string & name) :
    _problem(problem),
    _mesh(problem.mesh()),
    _name(name)
{
  _vars.resize(libMesh::n_threads());
}

Variable &
System::getVariable(THREAD_ID tid, const std::string & var_name)
{
  return *_vars[tid].getVariable(var_name);
}

Order
System::getMinQuadratureOrder()
{
  Order order = CONSTANT;
  std::vector<Variable *> vars = _vars[0].all();
  for (std::vector<Variable *>::iterator it = vars.begin(); it != vars.end(); ++it)
  {
    FEType fe_type = (*it)->feType();
    if (fe_type.default_quadrature_order() > order)
      order = fe_type.default_quadrature_order();
  }

  return order;
}

void
System::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::vector<Variable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    Variable *var = *it;
    var->reinit();
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->computeElemValues();
  }
}

void
System::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::set<Variable *>::iterator it = _vars[tid].boundaryVars(bnd_id).begin();
       it != _vars[tid].boundaryVars(bnd_id).end();
       ++it)
  {
    Variable *var = *it;
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->reinit();
  }
}

void
System::reinitNode(const Node * node, THREAD_ID tid)
{
  for (std::vector<Variable *>::iterator it = _vars[tid].all().begin(); it != _vars[tid].all().end(); ++it)
  {
    Variable *var = *it;
    if (var->feType().family == LAGRANGE)
    {
      var->reinit_node();
      var->computeNodalValues();
    }
  }
}

void
System::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::set<Variable *>::iterator it = _vars[tid].boundaryVars(bnd_id).begin();
       it != _vars[tid].boundaryVars(bnd_id).end();
       ++it)
  {
    Variable *var = *it;
    if (var->feType().family == LAGRANGE)
    {
      var->reinit_node();
      var->computeNodalValues();
    }
  }
}

} // namespace
