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

#include "AxisymmetricCenterlineAverageValue.h"
// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<AxisymmetricCenterlineAverageValue>()
{
  InputParameters params = validParams<SideAverageValue>();
  return params;
}

AxisymmetricCenterlineAverageValue::AxisymmetricCenterlineAverageValue(
    const InputParameters & parameters)
  : SideAverageValue(parameters), _volume(0)
{
}

Real
AxisymmetricCenterlineAverageValue::volume()
{
  return _current_side_elem->volume();
}

Real
AxisymmetricCenterlineAverageValue::computeIntegral()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * computeQpIntegral();
  return sum;
}
