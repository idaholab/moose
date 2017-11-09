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

#include "StickyBC.h"
#include "MooseVariable.h"

template <>
InputParameters
validParams<StickyBC>()
{
  InputParameters p = validParams<NodalBC>();
  p.addParam<Real>(
      "min_value",
      std::numeric_limits<Real>::lowest(),
      "If the old variable value <= min_value, the variable is fixed at its old value");
  p.addParam<Real>(
      "max_value",
      std::numeric_limits<Real>::max(),
      "If the old variable value >= max_value, the variable is fixed at its old value");
  p.addClassDescription(
      "Imposes the boundary condition $u = u_{old}$ if $u_{old}$ exceeds the bounds provided");
  return p;
}

StickyBC::StickyBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _u_old(_var.nodalSlnOld()),
    _min_value(getParam<Real>("min_value")),
    _max_value(getParam<Real>("max_value"))
{
  if (_min_value > _max_value)
    mooseError("StickyBC: min_value must not be greater than max_value");
}

bool
StickyBC::shouldApply()
{
  const unsigned qp = 0; // this is a NodalBC: all qp = 0
  return (_u_old[qp] <= _min_value || _u_old[qp] >= _max_value);
}

Real
StickyBC::computeQpResidual()
{
  return _u[_qp] - _u_old[_qp];
}
