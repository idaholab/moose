#include "SubProblem.h"
#include "Factory.h"
#include "Problem.h"
#include "Variable.h"

// libMesh
#include "quadrature_gauss.h"

namespace Moose {


SubProblem::SubProblem(Problem & problem, const std::string & name) :
    _problem(problem),
    _mesh(problem.mesh()),
    _name(name)
{

}

Moose::Variable &
SubProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  return *_vars[tid][var_name];
}

void
SubProblem::attachQuadratureRule(QBase *qrule, THREAD_ID tid)
{
  for (std::map<std::string, Moose::Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Moose::Variable *var = it->second;
    var->attachQuadratureRule (qrule);
  }
}

void
SubProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::map<std::string, Moose::Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Moose::Variable *var = it->second;
    var->reinit(elem);
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->computeElemValues();
  }
}

void
SubProblem::reinitElemFace(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  for (std::map<std::string, Moose::Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Moose::Variable *var = it->second;
    var->sizeResidual();
    var->sizeJacobianBlock();
    var->reinit (elem, side);
  }
}

void
SubProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  for (std::map<std::string, Moose::Variable *>::iterator it = _vars[tid].begin(); it != _vars[tid].end(); ++it)
  {
    Moose::Variable *var = it->second;
    if (var->feType().family == LAGRANGE)
    {
      var->reinit (node);
      var->computeNodalValues();
    }
  }
}

} // namespace
