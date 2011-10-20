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

#include "SubProblem.h"
#include "Factory.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SubProblem>()
{
  InputParameters params = validParams<Problem>();
  params.addRequiredParam<MooseMesh *>("mesh", "The Mesh");
  params.addParam<Problem *>("parent", NULL, "This problem's parent problem (if any)");
  return params;
}

// SubProblem /////

SubProblem::SubProblem(const std::string & name, InputParameters parameters) :
    Problem(name, parameters),
    SubProblemInterface(),
    _parent(parameters.get<Problem *>("parent") == NULL ? this : parameters.get<Problem *>("parent")),
    _mesh(*parameters.get<MooseMesh *>("mesh")),
    _eq(_parent == this ? *new EquationSystems(_mesh) : _parent->es()),
    _transient(false),
    _time(_eq.parameters.set<Real>("time")),
    _t_step(_eq.parameters.set<int>("t_step")),
    _dt(_eq.parameters.set<Real>("dt"))
{
  if (_parent == this)
  {
    _time = 0.0;
    _t_step = 0;
    _dt = 0;
    _dt_old = _dt;
    _eq.parameters.set<Problem *>("_problem") = this;
  }
}

SubProblem::~SubProblem()
{
  if (_parent == this)
    delete &_eq;
}

void
SubProblem::init()
{
  _eq.init();
  _eq.print_info();
}
