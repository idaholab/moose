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
#include "ForcingFn.h"

template <>
InputParameters
validParams<ForcingFn>()
{
  return validParams<Kernel>();
}

ForcingFn::ForcingFn(const InputParameters & parameters) : Kernel(parameters) {}

Real
ForcingFn::funcValue()
{
  //  Point pt = _qrule->get_points()[_qp];
  Point pt = _q_point[_qp];

  //  return (pt(0)*pt(0) + pt(1)*pt(1));
  if (_var.number() == 0)
    return (pt(0) * pt(0) + pt(1) * pt(1));
  else
    return -4;
}

Real
ForcingFn::computeQpResidual()
{
  return -funcValue() * _test[_i][_qp];
}
