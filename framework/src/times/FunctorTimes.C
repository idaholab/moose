//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorTimes.h"

registerMooseObject("MooseApp", FunctorTimes);

InputParameters
FunctorTimes::validParams()
{
  InputParameters params = Times::validParams();
  params += NonADFunctorInterface::validParams();
  params.addClassDescription(
      "Times created by evaluating a functor at the (0,0,0) point and the current time");

  params.addRequiredParam<MooseFunctorName>("functor", "Functor to evaluate to provide the time");
  params.addParam<MooseFunctorName>("factor", 1, "Factor to multiply the evaluated time with");

  // Times are known for all processes already
  params.set<bool>("auto_broadcast") = false;
  // Timestep_begin seems like a decent default here
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  return params;
}

FunctorTimes::FunctorTimes(const InputParameters & parameters)
  : Times(parameters),
    NonADFunctorInterface(this),
    _functor(getFunctor<Real>("functor")),
    _factor(getFunctor<Real>("factor"))
{
}

void
FunctorTimes::initialize()
{
  _fe_problem.mesh().errorIfDistributedMesh(type());
  // Locate the origin on the mesh
  Point p(0, 0, 0);
  auto pl = _fe_problem.mesh().getMesh().sub_point_locator();
  auto * elem = (*pl)(p);
  if (!elem)
    mooseError("Origin point not in local mesh, cannot evaluate the functor there");
  Moose::ElemArg elem_origin = makeElemArg(elem);

  const auto t = determineState();
  // Initialize is by default what is called by ::execute()
  _times.push_back(_factor(elem_origin, t) * _functor(elem_origin, t));
  // if this is performed multiple times (fixed point iterations, similar results at various
  // execution flags) it will be caught by our logic to make the vector hold unique times
}
