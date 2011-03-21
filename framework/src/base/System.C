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
  return *_vars[tid][var_name];
}

void
System::attachQuadratureRule(QBase *qrule, THREAD_ID tid)
{
  for (std::map<std::string, Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Variable *var = it->second;
    var->attachQuadratureRule (qrule);
  }
}

void
System::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::map<std::string, Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Variable *var = it->second;
    var->reinit(elem);
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->computeElemValues();
  }
}

void
System::reinitElemFace(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  for (std::map<std::string, Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Variable *var = it->second;
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->reinit (elem, side);
  }
}

void
System::reinitNode(const Node * node, THREAD_ID tid)
{
  for (std::map<std::string, Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Variable *var = it->second;
    if (var->feType().family == LAGRANGE)
    {
      var->reinit (node);
      var->computeNodalValues();
    }
  }
}

} // namespace
