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

#include "MaterialLimitTimeStep.h"

template<>
InputParameters validParams<MaterialLimitTimeStep>()
{
  InputParameters params = validParams<ElementVariablePostprocessor>();

  params.addClassDescription("This postprocessor estimates a timestep that reduces the increment change in a aux variable below a given threshold.");
  params.addParam<Real>("max_increment", 1e-4, "Maximum increment of material property");

  return params;
}

MaterialLimitTimeStep::MaterialLimitTimeStep(const InputParameters & parameters):
    ElementVariablePostprocessor(parameters),
    _limit(getParam<Real>("max_increment")),
    _u_old(valueOld())
{
}

void
MaterialLimitTimeStep::initialize()
{
  // Make minimum limit 10 orders of magnitude smaller than limit such that max timestep = 1e10
  // This also prevents a divide by zero in getValue
  _max_inc = _limit * 1e-10;
}

void
MaterialLimitTimeStep::computeQpValue()
{
  const Real increment = std::abs(_u[_qp] - _u_old[_qp]);
  _max_inc = std::max(increment, _max_inc);
}

Real
MaterialLimitTimeStep::getValue()
{
  gatherMax(_max_inc);
  return _dt * _limit / _max_inc;
}

void
MaterialLimitTimeStep::threadJoin(const UserObject & y)
{
  const MaterialLimitTimeStep & mts = static_cast<const MaterialLimitTimeStep &>(y);
  _max_inc = std::max(_max_inc, mts._max_inc);
}
