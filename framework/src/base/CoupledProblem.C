#include "CoupledProblem.h"
#include "MProblem.h"
#include "Variable.h"

namespace Moose
{

CoupledProblem::CoupledProblem(Mesh * mesh) :
    _mesh(mesh),
    _eq(*mesh),
    _time(_eq.parameters.set<Real>("time")),
    _t_step(_eq.parameters.set<int>("t_step")),
    _dt(_eq.parameters.set<Real>("dt"))
{
  _eq.parameters.set<Problem *>("_problem") = this;
}

CoupledProblem::~CoupledProblem()
{
}

void
CoupledProblem::addSubProblem(const std::string & file_name, SubProblem *subproblem)
{
  _subproblems[file_name] = subproblem;
  _map[subproblem->nl()->name()] = subproblem;
}

SubProblem *
CoupledProblem::subProblem(const std::string & name)
{
  return _subproblems[name];
}

void
CoupledProblem::solveOrder(const std::vector<std::string> & solve_order)
{
  _solve_order = solve_order;
}

bool
CoupledProblem::hasVariable(const std::string & var_name)
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
  {
    Moose::SubProblem * problem = it->second;
    if (problem->hasVariable(var_name))
      return true;
  }

  return false;
}

Variable &
CoupledProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
  {
    Moose::SubProblem * problem = it->second;
    if (problem->hasVariable(var_name))
      return problem->getVariable(tid, var_name);
  }

  mooseError("Unknown variable " + var_name);
}

void
CoupledProblem::attachQuadratureRule(QBase *qrule, THREAD_ID tid)
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->attachQuadratureRule(qrule, tid);
}

void
CoupledProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitElem(elem, tid);
}

void
CoupledProblem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitElemFace(elem, side, bnd_id, tid);
}

void
CoupledProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitNode(node, tid);
}

void
CoupledProblem::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitNodeFace(node, bnd_id, tid);
}

void
CoupledProblem::init()
{
  _eq.init();
  _eq.print_info();
}

void
CoupledProblem::update()
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->update();
}

void
CoupledProblem::computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual)
{
  _map[sys.name()]->computeResidual(sys, soln, residual);
}

void
CoupledProblem::computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> &  jacobian)
{
  _map[sys.name()]->computeJacobian(sys, soln, jacobian);
}

Number
CoupledProblem::initialValue (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name)
{
  return _map[sys_name]->initialValue(p, parameters, sys_name, var_name);
}

Gradient
CoupledProblem::initialGradient (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name)
{
  return _map[sys_name]->initialValue(p, parameters, sys_name, var_name);
}

void
CoupledProblem::initialCondition(EquationSystems & es, const std::string & system_name)
{
  _map[system_name]->initialCondition(es, system_name);
}

void
CoupledProblem::transient(bool trans)
{
  _transient = trans;
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->transient(trans);
}

void
CoupledProblem::copySolutionsBackwards()
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->copySolutionsBackwards();
}

void
CoupledProblem::dump()
{
  for (std::map<std::string, Moose::SubProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->dump();
}


} // namespace
