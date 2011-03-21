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


// SubProblem /////

SubProblem::SubProblem(MooseMesh & mesh, Problem * parent) :
    SubProblemInterface(),
    _parent(parent == NULL ? this : parent),
    _mesh(mesh),
    _eq(parent == NULL ? *new EquationSystems(_mesh) : parent->es()),
    _transient(false),
    _time(_parent != this ? _parent->time() : _eq.parameters.set<Real>("time")),
    _t_step(_parent != this ? _parent->timeStep() : _eq.parameters.set<int>("t_step")),
    _dt(_parent != this ? _parent->dt() : _eq.parameters.set<Real>("dt"))
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
