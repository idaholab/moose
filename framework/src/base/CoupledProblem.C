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

#include "CoupledProblem.h"
#include "FEProblem.h"
#include "MooseVariable.h"
#include "InputParameters.h"

template<>
InputParameters validParams<CoupledProblem>()
{
  InputParameters params = validParams<Problem>();
  params.addRequiredParam<MooseMesh *>("mesh", "The Mesh");
  return params;
}


CoupledProblem::CoupledProblem(const std::string & name, InputParameters parameters) :
    Problem(name, parameters),
    _mesh(parameters.get<MooseMesh *>("mesh")),
    _eq(*_mesh)
{
  _eq.parameters.set<Problem *>("_problem") = this;
}

CoupledProblem::~CoupledProblem()
{
}

void
CoupledProblem::addSubProblem(const std::string & file_name, FEProblem *subproblem)
{
  _subproblems[file_name] = subproblem;
  _map[subproblem->getNonlinearSystem().sys().name()] = subproblem;
}

FEProblem *
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
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
  {
    FEProblem * problem = it->second;
    if (problem->hasVariable(var_name))
      return true;
  }

  return false;
}

MooseVariable &
CoupledProblem::getVariable(THREAD_ID tid, const std::string & var_name)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
  {
    FEProblem * problem = it->second;
    if (problem->hasVariable(var_name))
      return problem->getVariable(tid, var_name);
  }

  mooseError("Unknown variable " + var_name);
}

void
CoupledProblem::prepare(const Elem * elem, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->prepare(elem, tid);
}

void
CoupledProblem::prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->prepare(elem, ivar, jvar, dof_indices, tid);
}

void
CoupledProblem::reinitElem(const Elem * elem, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitElem(elem, tid);
}

void
CoupledProblem::reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitElemFace(elem, side, bnd_id, tid);
}

void
CoupledProblem::reinitNode(const Node * node, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitNode(node, tid);
}

void
CoupledProblem::reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitNodeFace(node, bnd_id, tid);
}

void
CoupledProblem::reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitNeighbor(elem, side, tid);
}

void
CoupledProblem::reinitNeighbor(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->reinitNeighbor(neighbor, neighbor_side, physical_points, tid);
}

void
CoupledProblem::subdomainSetup(unsigned int /*subdomain*/, THREAD_ID /*tid*/)
{
}

void
CoupledProblem::subdomainSetupSide(unsigned int /*subdomain*/, THREAD_ID /*tid*/)
{
}

void
CoupledProblem::init()
{
  _eq.init();
  _eq.print_info();

  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
  {
    FEProblem * sp = it->second;
    sp->init2();                        // obviously I ran out of proper names ;-)
  }
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
CoupledProblem::computePostprocessors(ExecFlagType /*type = EXEC_TIMESTEP*/)
{
}

void
CoupledProblem::outputPostprocessors(bool force)
{
}

Real &
CoupledProblem::getPostprocessorValue(const std::string & /*name*/, THREAD_ID /*tid*/)
{
  static Real pps;
  return pps;
}

void
CoupledProblem::reinitMaterials(unsigned int /*blk_id*/, THREAD_ID /*tid*/)
{
}

void
CoupledProblem::reinitMaterialsFace(unsigned int /*blk_id*/, unsigned int /*side*/, THREAD_ID /*tid*/)
{
}

void
CoupledProblem::copySolutionsBackwards()
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->copySolutionsBackwards();
}

void
CoupledProblem::dump()
{
//  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
//    it->second->dump();
}

void
CoupledProblem::output(bool force)
{
  for (std::map<std::string, FEProblem *>::iterator it = _subproblems.begin(); it != _subproblems.end(); ++it)
    it->second->output(force);
}

void
CoupledProblem::restartFromFile(const std::string & /*file_name*/)
{
  mooseWarning("Restart of CoupledProblem is not implemented, doing nothing...");
}
