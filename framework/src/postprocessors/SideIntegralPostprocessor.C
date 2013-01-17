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

#include "SideIntegralPostprocessor.h"

template<>
InputParameters validParams<SideIntegralPostprocessor>()
{
  InputParameters params = validParams<SidePostprocessor>();
  return params;
}

SideIntegralPostprocessor::SideIntegralPostprocessor(const std::string & name, InputParameters parameters) :
    SidePostprocessor(name, parameters),
    _qp(0),
    _integral_value(0)
{}

void
SideIntegralPostprocessor::initialize()
{
  _integral_value = 0;
}

void
SideIntegralPostprocessor::execute()
{
  _integral_value += computeIntegral();
}

Real
SideIntegralPostprocessor::getValue()
{
  gatherSum(_integral_value);
  return _integral_value;
}

void
SideIntegralPostprocessor::threadJoin(const UserObject & y)
{
  const SideIntegralPostprocessor & pps = dynamic_cast<const SideIntegralPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
SideIntegralPostprocessor::computeIntegral()
{
  Real sum = 0;
  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    sum += _JxW[_qp]*_coord[_qp]*computeQpIntegral();
  return sum;
}
